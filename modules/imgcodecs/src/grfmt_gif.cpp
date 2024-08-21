// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level
// directory of this distribution and at http://opencv.org/license.html

#include "precomp.hpp"
#include "grfmt_gif.hpp"

#ifdef HAVE_IMGCODEC_GIF
namespace cv
{
//////////////////////////////////////////////////////////////////////
////                        GIF Decoder                           ////
//////////////////////////////////////////////////////////////////////
GifDecoder::GifDecoder() {
    m_signature = R"(GIF)";
    m_type = CV_8UC4;
    bgColor = -1;
    m_buf_supported = true;
    globalColorTableSize = 0;
    localColorTableSize = 0;
    localColorTable.allocate(3 * 256); // maximum size of a color table
    lzwMinCodeSize = 0;
    hasRead = false;
    hasTransparentColor = false;
    transparentColor = 0;
    opMode = GRFMT_GIF_Nothing;
    top = 0, left = 0, width = 0, height = 0;
    depth = 8;
    idx = 0;
}

GifDecoder::~GifDecoder() {
    close();
}

bool GifDecoder::readHeader() {
    if (!m_buf.empty()) {
        if (!m_strm.open(m_buf)) {
            return false;
        }
    } else if (!m_strm.open(m_filename)) {
        return false;
    }

    String signature(6, ' ');
    m_strm.getBytes((uchar*)signature.data(), 6);
    CV_Assert(signature == R"(GIF87a)" || signature == R"(GIF89a)");

    // #1: read logical screen descriptor
    m_width = m_strm.getWord();
    m_height = m_strm.getWord();
    CV_Assert(m_width > 0 && m_height > 0);

    char flags = (char)m_strm.getByte();

    // the background color -> index in the global color table, valid only if the global color table is present
    bgColor = m_strm.getByte();
    m_strm.skip(1); // Skip the aspect ratio

    // #2: read global color table
    depth = ((flags & 0x70) >> 4) + 1;
    if (flags & 0x80) {
        globalColorTableSize = 1 << ((flags & 0x07) + 1);
        globalColorTable.allocate(3 * globalColorTableSize);
        for (int i = 0; i < 3 * globalColorTableSize; i++) {
            globalColorTable[i] = (uchar)m_strm.getByte();
        }
    }

    // get the frame count
    bool success = getFrameCount_();

    hasRead = false;
    return success;
}

bool GifDecoder::readData(Mat &img) {
    if (hasRead) {
        lastImage.copyTo(img);
        return true;
    }

    readExtensions();
    // Image separator
    CV_Assert(!(m_strm.getByte()^0x2C));
    left = m_strm.getWord();
    top = m_strm.getWord();
    width = m_strm.getWord();
    height = m_strm.getWord();
    CV_Assert(width > 0 && height > 0 && left + width <= m_width && top + height <= m_height);

    imgCodeStream.allocate(width * height);
    Mat img_;

    switch (opMode) {
        case GifOpMode::GRFMT_GIF_PreviousImage:
            if (lastImage.empty()){
                img_ = Mat::zeros(m_height, m_width, CV_8UC4);
            } else {
                img_ = lastImage;
            }
            break;
        case GifOpMode::GRFMT_GIF_Background:
            // background color is valid iff global color table exists
            CV_Assert(globalColorTableSize > 0);
            if (hasTransparentColor && transparentColor == bgColor) {
                img_ = Mat(m_height, m_width, CV_8UC4,
                           Scalar(globalColorTable[bgColor * 3 + 2],
                                  globalColorTable[bgColor * 3 + 1],
                                  globalColorTable[bgColor * 3], 0));
            } else {
                img_ = Mat(m_height, m_width, CV_8UC4,
                           Scalar(globalColorTable[bgColor * 3 + 2],
                                  globalColorTable[bgColor * 3 + 1],
                                  globalColorTable[bgColor * 3], 255));
            }
            break;
        case GifOpMode::GRFMT_GIF_Nothing:
        case GifOpMode::GRFMT_GIF_Cover:
            // default value
            img_ = Mat::zeros(m_height, m_width, CV_8UC4);
            break;
        default:
            CV_Assert(false);
    }
    lastImage.release();

    auto flags = (uchar)m_strm.getByte();
    if (flags & 0x80) {
        // local color table
        localColorTableSize = 1 << ((flags & 0x07) + 1);
        for (int i = 0; i < 3 * localColorTableSize; i++) {
            localColorTable[i] = (uchar)m_strm.getByte();
        }
    } else if (globalColorTableSize) {
        /*
         * According to the GIF Specification at https://www.w3.org/Graphics/GIF/spec-gif89a.txt:
         *   "Both types of color tables are optional, making it possible for a Data Stream to contain
         * numerous graphics without a color table at all."
         *   The specification recommended that the decoder save the last Global Color Table used
         * until another Global Color Table is encountered, here we also save the last Local Color Table used
         * in case of there is no such thing as "last Global Color Table used". Thus, we only refresh the
         * Local Color Table when a Global Color Table or last Global Color Table used is present.
         */
        localColorTableSize = 0;
    }

    // lzw decompression to get the code stream
    hasRead = lzwDecode();

    // convert code stream into pixels on the image
    if (hasRead) {
        idx = 0;
        if (!(flags & 0x40)) {
            // no interlace, simply convert the code stream into pixels from top to down
            code2pixel(img_, 0, 1);
        } else {
            // consider the interlace mode, the image will be rendered in four separate passes
            code2pixel(img_, 0, 8);
            code2pixel(img_, 4, 8);
            code2pixel(img_, 2, 4);
            code2pixel(img_, 1, 2);
        }
    }

    lastImage = img_;
    if (!img.empty()) {
        if (img.channels() == 3){
            if (m_use_rgb) {
                cvtColor(img_, img, COLOR_BGRA2RGB);
            } else {
                cvtColor(img_, img, COLOR_BGRA2BGR);
            }
        } else {
            if (m_use_rgb) {
                cvtColor(img_, img, COLOR_BGRA2RGBA);
            } else {
                img_.copyTo(img);
            }
        }
    }

    // release the memory
    img_.release();

    return hasRead;
}

bool GifDecoder::nextPage() {
    if (hasRead) {
        hasRead = false;
        // end of a gif file
        if(!(m_strm.getByte() ^ 0x3B)) return false;
        m_strm.setPos(m_strm.getPos() - 1);
        return true;
    } else {
        bool success;
        try {
            Mat emptyImg;
            success = readData(emptyImg);
            emptyImg.release();
        } catch(...) {
            return false;
        }
        return success;
    }
}

void GifDecoder::readExtensions() {
    uchar len;
    while (!(m_strm.getByte() ^ 0x21)) {
        auto extensionType = (uchar)m_strm.getByte();

        // read graphic control extension
        // the scope of this extension is the next image or plain text extension
        if (!(extensionType ^ 0xF9)) {
            hasTransparentColor = false;
            opMode = GifOpMode::GRFMT_GIF_Nothing;// default value
            len = (uchar)m_strm.getByte();
            CV_Assert(len == 4);
            auto flags = (uchar)m_strm.getByte();
            m_strm.getWord(); // delay time, not used
            opMode = (GifOpMode)((flags & 0x1C) >> 2);
            hasTransparentColor = flags & 0x01;
            transparentColor = (uchar)m_strm.getByte();
        }

        // skip other kinds of extensions
        len = (uchar)m_strm.getByte();
        while (len) {
            m_strm.skip(len);
            len = (uchar)m_strm.getByte();
        }
    }
    // roll back to the block identifier
    m_strm.setPos(m_strm.getPos() - 1);
}

void GifDecoder::code2pixel(Mat& img, int start, int k){
    for (int i = start; i < height; i += k) {
        for (int j = 0; j < width; j++) {
            uchar colorIdx = imgCodeStream[idx++];
            if (hasTransparentColor && colorIdx == transparentColor) {
                if (opMode != GifOpMode::GRFMT_GIF_PreviousImage) {
                    if (colorIdx < localColorTableSize) {
                        img.at<Vec4b>(top + i, left + j) =
                                Vec4b(localColorTable[colorIdx * 3 + 2], // B
                                      localColorTable[colorIdx * 3 + 1], // G
                                      localColorTable[colorIdx * 3],     // R
                                      0);                                // A
                    } else if (colorIdx < globalColorTableSize) {
                        img.at<Vec4b>(top + i, left + j) =
                                Vec4b(globalColorTable[colorIdx * 3 + 2], // B
                                      globalColorTable[colorIdx * 3 + 1], // G
                                      globalColorTable[colorIdx * 3],     // R
                                      0);                                 // A
                    } else {
                        img.at<Vec4b>(top + i, left + j) = Vec4b(0, 0, 0, 0);
                    }
                }
                continue;
            }
            if (colorIdx < localColorTableSize) {
                img.at<Vec4b>(top + i, left + j) =
                        Vec4b(localColorTable[colorIdx * 3 + 2], // B
                              localColorTable[colorIdx * 3 + 1], // G
                              localColorTable[colorIdx * 3],     // R
                              255);                              // A
            } else if (colorIdx < globalColorTableSize) {
                img.at<Vec4b>(top + i, left + j) =
                        Vec4b(globalColorTable[colorIdx * 3 + 2], // B
                              globalColorTable[colorIdx * 3 + 1], // G
                              globalColorTable[colorIdx * 3],     // R
                              255);                               // A
            } else if (!(localColorTableSize || globalColorTableSize)) {
                /*
                 * According to the GIF Specification at https://www.w3.org/Graphics/GIF/spec-gif89a.txt:
                 *   "If no color table is available at all, the decoder is free to use a system color table
                 * or a table of its own. In that case, the decoder may use a color table with as many colors
                 * as its hardware is able to support; it is recommended that such a table have black and
                 * white as its first two entries, so that monochrome images can be rendered adequately."
                 */
                uchar intensity = colorIdx ^ 1 ? colorIdx : 255;
                img.at<Vec4b>(top + i, left + j) =
                        Vec4b(intensity, intensity, intensity, 255);
            } else {
                CV_Assert(false);
            }
        }
    }
}

bool GifDecoder::lzwDecode() {
    // initialization
    lzwMinCodeSize = m_strm.getByte();
    int lzwCodeSize = lzwMinCodeSize + 1;
    int clearCode = 1 << lzwMinCodeSize;
    int exitCode = clearCode + 1;
    CV_Assert(lzwCodeSize > 2 && lzwCodeSize <= 12);
    std::vector<lzwNodeD> lzwExtraTable((1 << 12) + 1);
    int colorTableSize = clearCode;
    int lzwTableSize = exitCode;

    idx = 0;
    int leftBits = 0;
    uint32_t src = 0;
    auto blockLen = (uchar)m_strm.getByte();
    while (blockLen) {
        if (leftBits < lzwCodeSize) {
            src |= m_strm.getByte() << leftBits;
            blockLen --;
            leftBits += 8;
        }

        while (leftBits >= lzwCodeSize) {
            // get the code
            uint16_t code = src & ((1 << lzwCodeSize) - 1);
            src >>= lzwCodeSize;
            leftBits -= lzwCodeSize;

            // clear code
            if (!(code ^ clearCode)) {
                lzwExtraTable.clear();
                // reset the code size, the same as that in the initialization part
                lzwCodeSize  = lzwMinCodeSize + 1;
                lzwTableSize = exitCode;
                continue;
            }
            // end of information
            if (!(code ^ exitCode)) {
                lzwExtraTable.clear();
                lzwCodeSize  = lzwMinCodeSize + 1;
                lzwTableSize = exitCode;
                break;
            }

            // check if the code stream is full
            if (idx == width * height) {
                return false;
            }

            // output code
            // 1. renew the lzw extra table
            if (code < colorTableSize) {
                lzwExtraTable[lzwTableSize].suffix = (uchar)code;
                lzwTableSize ++;
                lzwExtraTable[lzwTableSize].prefix.clear();
                lzwExtraTable[lzwTableSize].prefix.push_back((uchar)code);
                lzwExtraTable[lzwTableSize].length = 2;
            } else if (code <= lzwTableSize) {
                lzwExtraTable[lzwTableSize].suffix = lzwExtraTable[code].prefix[0];
                lzwTableSize ++;
                lzwExtraTable[lzwTableSize].prefix = lzwExtraTable[code].prefix;
                lzwExtraTable[lzwTableSize].prefix.push_back(lzwExtraTable[code].suffix);
                lzwExtraTable[lzwTableSize].length = lzwExtraTable[code].length + 1;
            } else {
                return false;
            }

            // 2. output to the code stream
            if (code < colorTableSize) {
                imgCodeStream[idx++] = (uchar)code;
            } else {
                for (int i = 0; i < lzwExtraTable[code].length - 1; i++) {
                    imgCodeStream[idx++] = lzwExtraTable[code].prefix[i];
                }
                imgCodeStream[idx++] = lzwExtraTable[code].suffix;
            }

            // check if the code size is full
            if (lzwTableSize > (1 << 12)) {
                return false;
            }

            // check if the bit length is full
            if (lzwTableSize == (1 << lzwCodeSize)) {
                lzwCodeSize < 12 ? lzwCodeSize++ : lzwCodeSize;
            }
        }

        // go to the next block if this block has been read out
        if (!blockLen) {
            blockLen = (uchar)m_strm.getByte();
        }
    }

    return idx == width * height;
}

ImageDecoder GifDecoder::newDecoder() const {
    return makePtr<GifDecoder>();
}

void GifDecoder::close() {
    while (!lastImage.empty()) lastImage.release();
    m_strm.close();
}

bool GifDecoder::getFrameCount_() {
    m_frame_count = 0;
    auto type = (uchar)m_strm.getByte();
    while (type != 0x3B) {
        if (!(type ^ 0x21)) {
            // skip all kinds of the extensions
            m_strm.skip(1);
            int len = m_strm.getByte();
            while (len) {
                m_strm.skip(len);
                len = m_strm.getByte();
            }
        } else if (!(type ^ 0x2C)) {
            // skip image data
            m_frame_count ++;
            // skip left, top, width, height
            m_strm.skip(8);
            int flags = m_strm.getByte();
            // skip local color table
            if (flags & 0x80) {
                m_strm.skip(3 * (1 << ((flags & 0x07) + 1)));
            }
            // skip lzw min code size
            m_strm.skip(1);
            int len = m_strm.getByte();
            while (len) {
                m_strm.skip(len);
                len = m_strm.getByte();
            }
        } else {
            CV_Assert(false);
        }
        type = (uchar)m_strm.getByte();
    }
    // roll back to the block identifier
    m_strm.setPos(0);
    return skipHeader();
}

bool GifDecoder::skipHeader() {
    String signature(6, ' ');
    m_strm.getBytes((uchar *) signature.data(), 6);
    // skip height and width
    m_strm.skip(4);
    char flags = (char) m_strm.getByte();
    // skip the background color and the aspect ratio
    m_strm.skip(2);
    // skip the global color table
    if (flags & 0x80) {
        m_strm.skip(3 * (1 << ((flags & 0x07) + 1)));
    }
    return signature == R"(GIF87a)" || signature == R"(GIF89a)";
}

} // namespace cv

namespace cv
{
//////////////////////////////////////////////////////////////////////
////                        GIF Encoder                           ////
//////////////////////////////////////////////////////////////////////
static const char* fmtGifHeader = "GIF89a";
GifEncoder::GifEncoder() {
    m_description = "Graphics Interchange Format 89a(*.gif)";
    m_height = 0, m_width = 0;
    width = 0, height = 0, top = 0, left = 0;
    m_buf_supported = true;
    opMode = GRFMT_GIF_Cover;
    transparentColor = 0; // index of the transparent color, default 0. currently it is a constant number
    lzwMaxCodeSize = 12; // the maximum code size, default 12. currently it is a constant number

    // default value of the params
    loopCount = 0; // infinite loops by default
    criticalTransparency = 1; // critical transparency, default 1, range from 0 to 255, 0 means no transparency
    frameDelay = 5; // 20fps by default, 10ms per unit
    bitDepth = 8; // the number of bits per pixel, default 8, currently it is a constant number
    lzwMinCodeSize = 8; // the minimum code size, default 8, this changes as the color number changes
    colorNum = 256; // the number of colors in the color table, default 256
    dithering = 0; // the level dithering, default 0
    globalColorTableSize = 256, localColorTableSize = 0;
}

GifEncoder::~GifEncoder() {
    close();
}

bool GifEncoder::isFormatSupported(int depth) const {
    return depth == CV_8U;
}

bool GifEncoder::write(const Mat &img, const std::vector<int> &params) {
    std::vector<Mat> img_vec;
    Mat img_;
    img.copyTo(img_);
    img_vec.push_back(img_);
    return writeFrames(img_vec, params);
}

bool GifEncoder::writemulti(const std::vector<Mat> &img_vec, const std::vector<int> &params) {
    std::vector<Mat> img_vec_;
    for (const auto &img : img_vec) {
        if (img.empty()) {
            return false;
        }
        Mat img_;
        img.copyTo(img_);
        img_vec_.push_back(img_);
    }
    return writeFrames(img_vec_, params);
}

bool GifEncoder::writeFrames(std::vector<Mat>& img_vec,
                             const std::vector<int>& params) {
    if (img_vec.empty()) {
        return false;
    }

    if (m_buf) {
        if (!strm.open(*m_buf)) {
            return false;
        }
    } else if (!strm.open(m_filename)) {
        return false;
    }

    // confirm the params
    for (size_t i = 0; i < params.size(); i += 2) {
        switch (params[i]) {
            case IMWRITE_GIF_LOOP:
                loopCount = std::min(std::max(params[i + 1], 0), 65535); // loop count is in 2 bytes
                break;
            case IMWRITE_GIF_SPEED:
                frameDelay = 100 - std::min(std::max(params[i + 1] - 1, 0), 99); // from 10ms to 1000ms
                break;
            case IMWRITE_GIF_QUALITY:
                lzwMinCodeSize = std::min(std::max(params[i + 1], 3), 8);
                colorNum = 1 << lzwMinCodeSize;
                globalColorTableSize = colorNum;
                break;
            case IMWRITE_GIF_DITHER:
                dithering = std::min(std::max(params[i + 1], -1), 3);
                break;
            case IMWRITE_GIF_TRANSPARENCY:
                criticalTransparency = (uchar)std::min(std::max(params[i + 1], 0), 255);
                break;
            case IMWRITE_GIF_COLORTABLE:
                localColorTableSize = std::min(std::max(params[i + 1], 0), 1);
                break;
        }
    }
    if (criticalTransparency) {
        lzwMinCodeSize = std::min(8, lzwMinCodeSize + 1);
        colorNum = 1 << lzwMinCodeSize;
        globalColorTableSize = colorNum;
    }

    if (dithering != 3) {
        for (auto &img : img_vec) {
            int depth = (int)ceil(log2(colorNum) / 3) + dithering;
            ditheringKernel(img, depth, criticalTransparency);
        }
    }
    bool result = writeHeader(img_vec);
    if (!result) {
        strm.close();
        return false;
    }

    for (const auto &img : img_vec) {
        result = writeFrame(img);
        if (!result) {
            strm.close();
            if (!m_buf)
                remove(m_filename.c_str());
            return false;
        }
    }

    strm.putByte(0x3B); // trailer
    strm.close();
    return true;
}

ImageEncoder GifEncoder::newEncoder() const {
    return makePtr<GifEncoder>();
}

bool GifEncoder::writeFrame(const Mat &img) {
    if (img.empty()) {
        return false;
    }
    height = m_height, width = m_width;

    // graphic control extension
    strm.putByte(0x21); // extension introducer
    strm.putByte(0xF9); // graphic control label
    strm.putByte(0x04); // block size, fixed number
    // flag is a packed field, and the first 3 bits are reserved
    uchar flag = opMode << 2;
    if (criticalTransparency)
        flag |= 1;
    strm.putByte(flag);
    strm.putWord(frameDelay);
    strm.putByte(transparentColor);
    strm.putByte(0x00); // end of the extension

    // image descriptor
    strm.putByte(0x2C); // image separator
    strm.putWord(left);
    strm.putWord(top);
    strm.putWord(width);
    strm.putWord(height);
    flag = localColorTableSize > 0 ? 0x80 : 0x00;
    if (localColorTableSize > 0) {
        std::vector<Mat> img_vec;
        img_vec.push_back(img);
        getColorTable(img_vec, false);
    }
    flag |= lzwMinCodeSize - 1;
    strm.putByte(flag);
    if (localColorTableSize > 0) {
        strm.putBytes(localColorTable.data(), localColorTableSize * 3);
    }

    imgCodeStream.allocate(width * height);
    bool result = pixel2code(img);
    if (result) result = lzwEncode();

    return result;
}

bool GifEncoder::lzwEncode() {
    strm.putByte(lzwMinCodeSize);
    int lzwCodeSize = lzwMinCodeSize + 1;
    // add clear code to the head of the output stream
    int bitLeft = lzwCodeSize;
    size_t output = (size_t)1 << lzwMinCodeSize;

    lzwTable.allocate((1 << 12) * 256);
    // clear lzwTable
    memset(lzwTable.data(), 0, (1 << 12) * 256 * sizeof(int));

    // next code
    int idx = (1 << lzwMinCodeSize) + 2;

    int bufferLen = 0;
    uchar buffer[256];

    //initialize
    int32_t prev = imgCodeStream[0];

    for (int64_t i = 1; i < height * width; i++) {
        // add the output code to the output buffer
        while (bitLeft >= 8) {
            buffer[bufferLen++] = (uchar)output;
            output >>= 8;
            bitLeft -= 8;
            if(bufferLen == 255) {
                strm.putByte(255);
                strm.putBytes(buffer, 255);
                bufferLen = 0;
            }
        }

        uchar c = imgCodeStream[i];
        // prev + currentCode(c) is not in the table
        if(lzwTable[prev * 256 + c] == 0){
            output |= (prev << bitLeft);
            bitLeft += lzwCodeSize;
            lzwTable[prev * 256 + c] = idx;
            prev = c;
            // check if the bit length is full
            if(idx == (1 << lzwCodeSize)){
                lzwCodeSize ++;
            }
            idx ++;
            // if the lzwTable is full, add clear code to the output
            if(idx == (1 << lzwMaxCodeSize)){
                output |= ((1 << lzwMinCodeSize) << bitLeft);
                bitLeft += lzwCodeSize;
                memset(lzwTable.data(), 0, (1 << 12) * 256 * sizeof(int));
                // next code
                idx = (1 << lzwMinCodeSize) + 2;
                lzwCodeSize = lzwMinCodeSize + 1;
            }
        } else{
            prev = lzwTable[prev * 256 + c];
        }
    }

    // end of the code
    output |= (prev << bitLeft);
    bitLeft += lzwCodeSize;
    output |= (((1 << lzwMinCodeSize) + 1) << bitLeft);
    bitLeft += lzwCodeSize;
    while (bitLeft >= 8) {
        buffer[bufferLen++] = (uchar)output;
        output >>= 8;
        bitLeft -= 8;
        if(bufferLen == 255) {
            strm.putByte(255);
            strm.putBytes(buffer, 255);
            bufferLen = 0;
        }
    }
    if (bitLeft > 0) {
        buffer[bufferLen++] = (uchar)output;
    }
    if (bufferLen > 0){
        strm.putByte(bufferLen);
        strm.putBytes(buffer, bufferLen);
    }
    // end of the block
    strm.putByte(0);

    return true;
}

bool GifEncoder::writeHeader(const std::vector<Mat>& img_vec) {
    strm.putBytes(fmtGifHeader, (int)strlen(fmtGifHeader));

    if (img_vec[0].empty()) {
        return false;
    }
    m_width = img_vec[0].cols, m_height = img_vec[0].rows;
    if (m_width <= 0 || m_height <= 0 || m_width > 65535 || m_height > 65535) {
        return false;
    }
    strm.putWord(m_width);
    strm.putWord(m_height);

    // by default, set the global color table
    uchar flags = (globalColorTableSize > 0) << 7; // global color table flag
    getColorTable(img_vec, true);
    flags |= (bitDepth - 1) << 4; // bit depth
    flags |= (lzwMinCodeSize - 1); // global color table size
    strm.putByte(flags);
    strm.putByte(0); // background color, default value
    strm.putByte(0); // aspect ratio, default value
    if (globalColorTableSize > 0) {
        strm.putBytes(globalColorTable.data(), globalColorTableSize * 3);
    }


    // add application extension to set the loop count
    strm.putByte(0x21); // GIF extension code
    strm.putByte(0xFF); // application extension table
    strm.putByte(0x0B); // length of application block, in decimal is 11
    strm.putBytes(R"(NETSCAPE2.0)", 11); // application authentication code
    strm.putByte(0x03); // length of application block, in decimal is 3
    strm.putByte(0x01); // identifier
    strm.putWord(loopCount);
    strm.putByte(0x00); // end of the extension

    return true;
}

bool GifEncoder::pixel2code(const Mat &img) {
    if(img.empty()) return false;
    CV_Assert(img.rows == (top + height) && img.cols == (left + width));

    // turn the image into the code stream and set the colorNum
    CV_Assert(colorNum <= 256 && (colorNum <= localColorTableSize || colorNum <= globalColorTableSize));
    OctreeColorQuant quant = localColorTableSize > 0 ? quantL : quantG;

    if (img.type() == CV_8UC3) {
        for(int i = 0; i < height; i++){
            for(int j = 0; j < width; j++){
                // set codeStream
                imgCodeStream[i * width + j] = quant.getLeaf(img.at<Vec3b>(i, j)[2],
                                                             img.at<Vec3b>(i, j)[1],
                                                             img.at<Vec3b>(i, j)[0]);
            }
        }
    } else if (img.type() == CV_8UC4) {
        for(int i = 0; i < height; i++){
            for(int j = 0; j < width; j++){
                if (img.at<Vec4b>(i, j)[3] < criticalTransparency) {
                    imgCodeStream[i * width + j] = transparentColor;
                    continue;
                }
                imgCodeStream[i * width + j] = quant.getLeaf(img.at<Vec4b>(i, j)[2],
                                                             img.at<Vec4b>(i, j)[1],
                                                             img.at<Vec4b>(i, j)[0]);
            }
        }
    } else {
        CV_Assert(false);
    }
    return true;
}

void GifEncoder::getColorTable(const std::vector<Mat> &img_vec, bool isGlobal) {
    // generate the global/local color table (color quantification)
    if (img_vec.empty()) return;
    CV_Assert(isGlobal || img_vec.size() == 1);
    if (isGlobal) {
        quantG = OctreeColorQuant(colorNum, bitDepth, criticalTransparency);
        quantG.addMats(img_vec);
        globalColorTable.allocate(colorNum * 3);
        quantG.getPalette(globalColorTable.data());
    } else {
        quantL = OctreeColorQuant(colorNum, bitDepth, criticalTransparency);
        quantL.addMat(img_vec[0]);
        localColorTable.allocate(colorNum * 3);
        quantL.getPalette(localColorTable.data());
    }
}

void GifEncoder::ditheringKernel(Mat &img, int depth, uchar criticalTransparency) {
    if (img.empty()) {
        return;
    } else if (img.type() == CV_8UC3){
        Mat error = Mat::zeros(img.rows + 2, img.cols + 2, CV_32FC3);
        int constant = (1 << (9 - depth)) - 1;
        Vec3f bias = Vec3f(0.5, 0.5, 0.5);
        for (int i = 0; i < img.rows; i++) {
            for (int j = 0; j < img.cols; j++) {
                Vec3f old_pixel = (Vec3f)img.at<Vec3b>(i, j) + error.at<Vec3f>(i + 1, j + 1);
                Vec3b new_pixel = (Vec3b)(old_pixel / constant + bias) * constant;
                img.at<Vec3b>(i, j) = new_pixel;
                Vec3f diff = old_pixel - (Vec3f)new_pixel;
                error.at<Vec3f>(i + 1, j + 2) += diff * 7 / 16; //     (i, j + 1)
                error.at<Vec3f>(i + 2, j) += diff * 3 / 16;     // (i + 1, j - 1)
                error.at<Vec3f>(i + 2, j + 1) += diff * 5 / 16; // (i + 1, j)
                error.at<Vec3f>(i + 2, j + 2) += diff / 16;     // (i + 1, j + 1)
            }
        }
    } else if (img.type() == CV_8UC4) {
        Mat error = Mat::zeros(img.rows + 2, img.cols + 2, CV_32FC4);
        int constant = (1 << (9 - depth)) - 1;
        Vec4f bias = Vec4f(0.5, 0.5, 0.5, 0.5);
        for (int i = 0; i < img.rows; i++) {
            for (int j = 0; j < img.cols; j++) {
                // transparent color should not be dithered
                if (img.at<Vec4b>(i, j)[3] < criticalTransparency) {
                    continue;
                }
                Vec4f old_pixel = (Vec4f)img.at<Vec4b>(i, j) + error.at<Vec4f>(i + 1, j + 1);
                Vec4b new_pixel = (Vec4b)(old_pixel / constant + bias) * constant;
                new_pixel[3] = img.at<Vec4b>(i, j)[3];
                img.at<Vec4b>(i, j) = new_pixel;
                Vec4f diff = old_pixel - (Vec4f)new_pixel;
                error.at<Vec4f>(i + 1, j + 2) += diff * 7 / 16; //     (i, j + 1)
                error.at<Vec4f>(i + 2, j) += diff * 3 / 16;     // (i + 1, j - 1)
                error.at<Vec4f>(i + 2, j + 1) += diff * 5 / 16; // (i + 1, j)
                error.at<Vec4f>(i + 2, j + 2) += diff / 16;     // (i + 1, j + 1)
            }
        }
    } else {
        CV_Assert(false);
    }
}

void GifEncoder::close() {
    if (strm.isOpened()) {
        strm.close();
    }
}


//////////////////////////////////////////////////////////////////////
////                      Color Quantization                      ////
//////////////////////////////////////////////////////////////////////
GifEncoder::OctreeColorQuant::OctreeNode::OctreeNode() {
    this->isLeaf = false;
    level = 0;
    index = 0;
    for (auto &i: children) {
        i = nullptr;
    }
    leaf = 0, pixelCount = 0;
    redSum = greenSum = blueSum = 0;
}

GifEncoder::OctreeColorQuant::OctreeColorQuant(int maxColors, int bitLength, uchar criticalTransparency) {
    m_maxColors = maxColors;
    m_bitLength = bitLength;
    m_leafCount = criticalTransparency ? 1 : 0;
    m_criticalTransparency = criticalTransparency;
    root = std::make_shared<OctreeNode>();
    r = g = b = 0;
    for (int i = 0; i < bitLength; i++) {
        m_nodeList[i] = std::vector<std::shared_ptr<OctreeNode>>();
    }
}

void GifEncoder::OctreeColorQuant::addMat(const Mat &img) {
    if (img.empty()) {
        return;
    } else if (img.type() == CV_8UC3) {
        for (int i = 0; i < img.rows; i++) {
            for (int j = 0; j < img.cols; j++) {
                addColor(img.at<Vec3b>(i, j)[2],
                         img.at<Vec3b>(i, j)[1],
                         img.at<Vec3b>(i, j)[0]);
            }
        }
    } else if (img.type() == CV_8UC4) {
        for (int i = 0; i < img.rows; i++) {
            for (int j = 0; j < img.cols; j++) {
                if (img.at<Vec4b>(i, j)[3] < m_criticalTransparency) {
                    r = img.at<Vec4b>(i, j)[2];
                    g = img.at<Vec4b>(i, j)[1];
                    b = img.at<Vec4b>(i, j)[0];
                    continue;
                }
                addColor(img.at<Vec4b>(i, j)[2],
                         img.at<Vec4b>(i, j)[1],
                         img.at<Vec4b>(i, j)[0]);
            }
        }
    } else {
        CV_Assert(false);
    }
}

void GifEncoder::OctreeColorQuant::addMats(const std::vector<Mat> &img_vec) {
    for (const auto& img: img_vec) {
        addMat(img);
    }
    if (m_maxColors < m_leafCount) {
        reduceTree();
    }
}

void GifEncoder::OctreeColorQuant::addColor(int red, int green, int blue) {
    std::shared_ptr<OctreeNode> node = root;
    for (int level = 0; level < m_bitLength; level++) {
        node -> pixelCount++;
        node -> redSum += red;
        node -> greenSum += green;
        node -> blueSum += blue;
        if(node -> isLeaf){
            break;
        }
        int shift = m_bitLength - level;
        int index = ((red >> shift) & 1) << 2 | ((green >> shift) & 1) << 1 | ((blue >> shift) & 1);
        if (node->children[index] == nullptr) {
            node->children[index] = std::make_shared<OctreeNode>();
            m_nodeList[level].push_back(node->children[index]);
        }
        node = node->children[index];
        if (level == m_bitLength - 1){
            node -> pixelCount++;
            node -> redSum += red;
            node -> greenSum += green;
            node -> blueSum += blue;
        }
    }
    if (!(node -> isLeaf)) {
        m_leafCount++;
        node -> isLeaf = true;
    }
}

// return the relative index of the leaf node
uchar GifEncoder::OctreeColorQuant::getLeaf(uchar red, uchar green, uchar blue) {
    std::shared_ptr<OctreeNode> node = root;
    for (int level = 0; level <= m_bitLength; level++) {
        if (node->isLeaf) {
            break;
        }
        int shift = m_bitLength - level;
        int index = ((red >> shift) & 1) << 2 | ((green >> shift) & 1) << 1 | ((blue >> shift) & 1);
        if (node->children[index] == nullptr) {
            CV_Assert(false);
        }
        node = node->children[index];
    }
    return node->index;
}

// get the palette
int GifEncoder::OctreeColorQuant::getPalette(uchar* colorTable) {
    CV_Assert(colorTable != nullptr);
    uchar index = 0;
    if (m_criticalTransparency) {
        colorTable[index * 3]     = r;
        colorTable[index * 3 + 1] = g;
        colorTable[index * 3 + 2] = b;
        index++;
    }
    for (int i = 0; i < m_bitLength; i++) {
        for (const auto& node : m_nodeList[i]) {
            if (node -> isLeaf) {
                colorTable[index * 3]     = (uchar)(node -> redSum / node -> pixelCount);
                colorTable[index * 3 + 1] = (uchar)(node -> greenSum / node -> pixelCount);
                colorTable[index * 3 + 2] = (uchar)(node -> blueSum / node -> pixelCount);
                node -> index = index++;
            }
            if (index == m_leafCount) {
                break;
            }
        }
    }
    return m_leafCount;
}

void GifEncoder::OctreeColorQuant::reduceTree() {
    // reduce to max color
    int level = 0;
    for (int i = 0; i < m_bitLength; i++) {
        auto size = (int32_t)m_nodeList[i].size() + 1;
        if (m_maxColors < size) {
            level = i - 1;
            break;
        }
    }
    for (const auto& node : m_nodeList[level + 1]) {
        recurseReduce(node);
    }

    while(m_maxColors < m_leafCount) {
        int minPixelCount = INT_MAX;
        std::shared_ptr<OctreeNode> minNode = nullptr;
        for (const auto& node : m_nodeList[level]) {
            if (node->pixelCount < minPixelCount && !(node->isLeaf)) {
                minPixelCount = node->pixelCount;
                minNode = node;
            }
        }
        CV_Assert(minNode != nullptr);
        recurseReduce(minNode);
    }
}

void GifEncoder::OctreeColorQuant::recurseReduce(const std::shared_ptr<OctreeNode>& node) {
    // reduce all the children of the node
    if (node == nullptr || node->isLeaf) {
        return;
    }
    std::vector<std::shared_ptr<OctreeNode>> stack;
    stack.push_back(node);

    while (!stack.empty()) {
        std::shared_ptr<OctreeNode> child = stack.back();
        stack.pop_back();
        if (child->isLeaf) {
            m_leafCount--;
            child->isLeaf = false;
        } else {
            for (int i = 0; i < m_bitLength; i++) {
                if (child->children[i] != nullptr) {
                    stack.push_back(child->children[i]);
                }
            }
        }
    }
    m_leafCount++;
    node -> isLeaf = true;
}

} // namespace cv2
#endif
