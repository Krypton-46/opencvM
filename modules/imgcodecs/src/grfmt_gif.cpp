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
    m_type = CV_8UC3;
    bgColor = -1;
    m_buf_supported = true;
    globalColorTableSize = 0;
    localColorTableSize = 0;
    lzwMinCodeSize = 0;
    hasRead = false;
    hasTransparentColor = false;
    transparentColor = 0;
    opMode = GRFMT_GIF_Nothing;
    top = 0, left = 0, width = 0, height = 0;
    depth = 8;
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

    try {
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
    } catch (...) {
        throw;
    }

    hasRead = false;
    return true;
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

    currentImageCodeStream.allocate(width * height);
    Mat img_;

    switch (opMode) {
        case GifOpMode::GRFMT_GIF_PreviousImage:
            if (lastImage.empty()){
                img_ = Mat::zeros(m_height, m_width, CV_8UC3);
            } else {
                img_ = lastImage;
            }
            break;
        case GifOpMode::GRFMT_GIF_Background:
            // background color is valid iff global color table exists
            CV_Assert(globalColorTableSize > 0);
            img_ = Mat(m_height, m_width, CV_8UC3,
                       Scalar(globalColorTable[bgColor * 3 + 2],
                              globalColorTable[bgColor * 3 + 1],
                              globalColorTable[bgColor * 3]));
            break;
        case GifOpMode::GRFMT_GIF_Nothing:
        case GifOpMode::GRFMT_GIF_Cover:
            // default value
            img_ = Mat::zeros(m_height, m_width, CV_8UC3);
            break;
        default:
            CV_Assert(false);
    }
    lastImage.release();

    auto flags = (uchar)m_strm.getByte();
    localColorTableSize = 0;
    if (flags & 0x80) {
        // local color table
        localColorTableSize = 1 << ((flags & 0x07) + 1);
        localColorTable.allocate(3 * localColorTableSize);
        for (int i = 0; i < 3 * localColorTableSize; i++) {
            localColorTable[i] = (uchar)m_strm.getByte();
        }
    }

    // the case that neither global nor local color table exists is not defined in the GIF standard (but allowed)
    if (!(globalColorTableSize || localColorTableSize)) {
        // go through the length of unused data.
        m_strm.skip(1);
        int len = m_strm.getByte();
        while (len) {
            m_strm.skip(len);
            len = m_strm.getByte();
        }

        lastImage = img_;
        if (!img.empty())
            img_.copyTo(img);

        // release the memory
        img_.release();
        return true;
    }

    // lzw decompression to get the code stream
    hasRead = lzwDecode();

    // convert code stream into pixels on the image
    if (hasRead) {
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
    if (!img.empty())
        img_.copyTo(img);

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
        hasTransparentColor = false;
        opMode = GifOpMode::GRFMT_GIF_Nothing;// default value
        if (!(extensionType ^ 0xF9)) {
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
    for (int i = start; i < height; i+=k) {
        for (int j = 0; j < width; j++) {
            int idx = i * width + j;
            int colorIdx = currentImageCodeStream[idx];
            if (hasTransparentColor && colorIdx == transparentColor) {
                continue;
            }
            if (colorIdx < localColorTableSize) {
                img.at<Vec3b>(top + i, left + j)[0] = localColorTable[colorIdx * 3 + 2]; //B
                img.at<Vec3b>(top + i, left + j)[1] = localColorTable[colorIdx * 3 + 1]; //G
                img.at<Vec3b>(top + i, left + j)[2] = localColorTable[colorIdx * 3];     //R
            } else if (colorIdx < globalColorTableSize) {
                img.at<Vec3b>(top + i, left + j)[0] = globalColorTable[colorIdx * 3 + 2]; //B
                img.at<Vec3b>(top + i, left + j)[1] = globalColorTable[colorIdx * 3 + 1]; //G
                img.at<Vec3b>(top + i, left + j)[2] = globalColorTable[colorIdx * 3];     //R
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
    int colorTableSize = localColorTableSize ? localColorTableSize : globalColorTableSize;
    int lzwTableSize = exitCode;

    int idx = 0;
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
                lzwExtraTable.clear();
                return false;
            }

            // output code
            // 1. renew the lzw extra table
            if (code < colorTableSize) {
                lzwExtraTable[lzwTableSize].suffix = (uchar)code;
                lzwTableSize ++;
                lzwExtraTable[lzwTableSize].prefix.push_back((uchar)code);
                lzwExtraTable[lzwTableSize].length = 2;
            } else if (code <= lzwTableSize && !lzwExtraTable[code].prefix.empty()) {
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
                currentImageCodeStream[idx++] = (uchar)code;
            } else {
                for (int i = 0; i < lzwExtraTable[code].length - 1; i++) {
                    currentImageCodeStream[idx++] = lzwExtraTable[code].prefix[i];
                }
                currentImageCodeStream[idx++] = lzwExtraTable[code].suffix;
            }

            // check if the code size is full
            if (lzwTableSize > (1 << 12)) {
                lzwExtraTable.clear();
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
}

#endif
