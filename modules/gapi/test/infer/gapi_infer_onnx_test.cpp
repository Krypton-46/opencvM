// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
//
// Copyright (C) 2020 Intel Corporation

#include "../test_precomp.hpp"

#ifdef HAVE_ONNX

#include <stdexcept>
#include <onnxruntime_cxx_api.h>
#include <ade/util/iota_range.hpp>
#include <codecvt> // wstring_convert

#include <opencv2/gapi/own/convert.hpp>
#include <opencv2/gapi/infer/onnx.hpp>

namespace {
class TestMediaBGR final: public cv::MediaFrame::IAdapter {
    cv::Mat m_mat;
    using Cb = cv::MediaFrame::View::Callback;
    Cb m_cb;

public:
    explicit TestMediaBGR(cv::Mat m, Cb cb = [](){})
        : m_mat(m), m_cb(cb) {
    }
    cv::GFrameDesc meta() const override {
        return cv::GFrameDesc{cv::MediaFormat::BGR, cv::Size(m_mat.cols, m_mat.rows)};
    }
    cv::MediaFrame::View access(cv::MediaFrame::Access) override {
        cv::MediaFrame::View::Ptrs pp = { m_mat.ptr(), nullptr, nullptr, nullptr };
        cv::MediaFrame::View::Strides ss = { m_mat.step, 0u, 0u, 0u };
        return cv::MediaFrame::View(std::move(pp), std::move(ss), Cb{m_cb});
    }
};

class TestMediaNV12 final: public cv::MediaFrame::IAdapter {
    cv::Mat m_y;
    cv::Mat m_uv;
public:
    TestMediaNV12(cv::Mat y, cv::Mat uv) : m_y(y), m_uv(uv) {
    }
    cv::GFrameDesc meta() const override {
        return cv::GFrameDesc{cv::MediaFormat::NV12, cv::Size(m_y.cols, m_y.rows)};
    }
    cv::MediaFrame::View access(cv::MediaFrame::Access) override {
        cv::MediaFrame::View::Ptrs pp = {
            m_y.ptr(), m_uv.ptr(), nullptr, nullptr
        };
        cv::MediaFrame::View::Strides ss = {
            m_y.step, m_uv.step, 0u, 0u
        };
        return cv::MediaFrame::View(std::move(pp), std::move(ss));
    }
};
struct ONNXInitPath {
    ONNXInitPath() {
        const char* env_path = getenv("OPENCV_GAPI_ONNX_MODEL_PATH");
        if (env_path) {
            cvtest::addDataSearchPath(env_path);
        }
    }
};
static ONNXInitPath g_init_path;

cv::Mat initMatrixRandU(const int type, const cv::Size& sz_in) {
    const cv::Mat in_mat1 = cv::Mat(sz_in, type);

    if (CV_MAT_DEPTH(type) < CV_32F) {
        cv::randu(in_mat1, cv::Scalar::all(0), cv::Scalar::all(255));
    } else {
        const int fscale = 256;  // avoid bits near ULP, generate stable test input
        cv::Mat in_mat32s(in_mat1.size(), CV_MAKE_TYPE(CV_32S, CV_MAT_CN(type)));
        cv::randu(in_mat32s, cv::Scalar::all(0), cv::Scalar::all(255 * fscale));
        in_mat32s.convertTo(in_mat1, type, 1.0f / fscale, 0);
    }
    return in_mat1;
}
} // anonymous namespace
namespace opencv_test
{
namespace {
void initTestDataPath()
{
#ifndef WINRT
    static bool initialized = false;
    if (!initialized)
    {
        // Since G-API has no own test data (yet), it is taken from the common space
        const char* testDataPath = getenv("OPENCV_TEST_DATA_PATH");
        if (testDataPath) {
            cvtest::addDataSearchPath(testDataPath);
        }
        initialized = true;
    }
#endif // WINRT
}

// FIXME: taken from the DNN module
void normAssert(cv::InputArray& ref, cv::InputArray& test,
                const char *comment /*= ""*/,
                const double l1 = 0.00001, const double lInf = 0.0001) {
    const double normL1 = cvtest::norm(ref, test, cv::NORM_L1) / ref.getMat().total();
    EXPECT_LE(normL1, l1) << comment;

    const double normInf = cvtest::norm(ref, test, cv::NORM_INF);
    EXPECT_LE(normInf, lInf) << comment;
}

inline std::string findModel(const std::string &model_name) {
    return findDataFile("vision/" + model_name + ".onnx", false);
}

inline void toCHW(const cv::Mat& src, cv::Mat& dst) {
    dst.create(cv::Size(src.cols, src.rows * src.channels()), CV_32F);
    std::vector<cv::Mat> planes;
    for (int i = 0; i < src.channels(); ++i) {
        planes.push_back(dst.rowRange(i * src.rows, (i + 1) * src.rows));
    }
    cv::split(src, planes);
}

inline int toCV(const ONNXTensorElementDataType prec) {
    switch (prec) {
    case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8: return CV_8U;
    case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT: return CV_32F;
    case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32: return CV_32S;
    case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64: return -1;
    default: GAPI_Assert(false && "Unsupported data type");
    }
    return -1;
}

inline std::vector<int64_t> toORT(const cv::MatSize &sz) {
    return cv::to_own<int64_t>(sz);
}

inline std::vector<const char*> getCharNames(const std::vector<std::string>& names) {
    std::vector<const char*> out_vec;
    for (const auto& el : names) {
        out_vec.push_back(el.data());
    }
    return out_vec;
}

template<typename T>
void copyToOut(const cv::Mat& in, cv::Mat& out) {
    const size_t size = std::min(out.total(), in.total());
    std::copy(in.begin<T>(), in.begin<T>() + size, out.begin<T>());
    if (size < out.total()) {
        T* const optr = out.ptr<T>();
        optr[size] = static_cast<T>(-1); // end data mark
    }
}

void remapYolo(const std::unordered_map<std::string, cv::Mat> &onnx,
                     std::unordered_map<std::string, cv::Mat> &gapi) {
    GAPI_Assert(onnx.size() == 1u);
    GAPI_Assert(gapi.size() == 1u);
    // Result from Run method
    const cv::Mat& in = onnx.begin()->second;
    GAPI_Assert(in.depth() == CV_32F);
    // Configured output
    cv::Mat& out = gapi.begin()->second;
    // Simple copy
    copyToOut<float>(in, out);
}

void remapYoloV3(const std::unordered_map<std::string, cv::Mat> &onnx,
                       std::unordered_map<std::string, cv::Mat> &gapi) {
    // Simple copy for outputs
    const cv::Mat& in_boxes = onnx.at("yolonms_layer_1/ExpandDims_1:0");
    const cv::Mat& in_scores = onnx.at("yolonms_layer_1/ExpandDims_3:0");
    const cv::Mat& in_indices = onnx.at("yolonms_layer_1/concat_2:0");
    GAPI_Assert(in_boxes.depth() == CV_32F);
    GAPI_Assert(in_scores.depth() == CV_32F);
    GAPI_Assert(in_indices.depth() == CV_32S);

    cv::Mat& out_boxes = gapi.at("out1");
    cv::Mat& out_scores = gapi.at("out2");
    cv::Mat& out_indices = gapi.at("out3");

    copyToOut<float>(in_boxes, out_boxes);
    copyToOut<float>(in_scores, out_scores);
    copyToOut<int32_t>(in_indices, out_indices);
}

void remapToIESSDOut(const std::vector<cv::Mat> &detections,
                           cv::Mat &ssd_output) {
    for (const auto &det_el : detections) {
        GAPI_Assert(det_el.depth() == CV_32F);
        GAPI_Assert(!det_el.empty());
    }

    // SSD-MobilenetV1 structure check
    ASSERT_EQ(detections[0].total(), 1u);
    ASSERT_EQ(detections[2].total(), detections[0].total() * 100);
    ASSERT_EQ(detections[2].total(), detections[3].total());
    ASSERT_EQ((detections[2].total() * 4), detections[1].total());

    const int num_objects = static_cast<int>(detections[0].ptr<float>()[0]);
    GAPI_Assert(num_objects <= (ssd_output.size[2] - 1));
    const float *in_boxes = detections[1].ptr<float>();
    const float *in_scores = detections[2].ptr<float>();
    const float *in_classes = detections[3].ptr<float>();
    float *ptr = ssd_output.ptr<float>();

    for (int i = 0; i < num_objects; i++) {
        ptr[0] = 0.f;                 // "image_id"
        ptr[1] = in_classes[i];       // "label"
        ptr[2] = in_scores[i];        // "confidence"
        ptr[3] = in_boxes[4 * i + 1]; // left
        ptr[4] = in_boxes[4 * i + 0]; // top
        ptr[5] = in_boxes[4 * i + 3]; // right
        ptr[6] = in_boxes[4 * i + 2]; // bottom

        ptr      += 7;
        in_boxes += 4;
    }

    if (num_objects < ssd_output.size[2] - 1) {
        // put a -1 mark at the end of output blob if there is space left
        ptr[0] = -1.f;
    }
}

void remapSSDPorts(const std::unordered_map<std::string, cv::Mat> &onnx,
                         std::unordered_map<std::string, cv::Mat> &gapi) {
    // Assemble ONNX-processed outputs back to a single 1x1x200x7 blob
    // to preserve compatibility with OpenVINO-based SSD pipeline
    const cv::Mat &num_detections = onnx.at("num_detections:0");
    const cv::Mat &detection_boxes = onnx.at("detection_boxes:0");
    const cv::Mat &detection_scores = onnx.at("detection_scores:0");
    const cv::Mat &detection_classes = onnx.at("detection_classes:0");
    cv::Mat &ssd_output = gapi.at("detection_output");
    remapToIESSDOut({num_detections, detection_boxes, detection_scores, detection_classes}, ssd_output);
}

void remapRCNNPorts(const std::unordered_map<std::string, cv::Mat> &onnx,
                          std::unordered_map<std::string, cv::Mat> &gapi) {
    // Simple copy for outputs
    const cv::Mat& in_boxes = onnx.at("6379");
    const cv::Mat& in_labels = onnx.at("6381");
    const cv::Mat& in_scores = onnx.at("6383");

    GAPI_Assert(in_boxes.depth() == CV_32F);
    GAPI_Assert(in_labels.depth() == CV_32S);
    GAPI_Assert(in_scores.depth() == CV_32F);

    cv::Mat& out_boxes = gapi.at("out1");
    cv::Mat& out_labels = gapi.at("out2");
    cv::Mat& out_scores = gapi.at("out3");

    copyToOut<float>(in_boxes, out_boxes);
    copyToOut<int>(in_labels, out_labels);
    copyToOut<float>(in_scores, out_scores);
}

class ONNXtest : public ::testing::Test {
public:
    std::string model_path;
    size_t num_in, num_out;
    std::vector<cv::Mat> out_gapi;
    std::vector<cv::Mat> out_onnx;
    cv::Mat in_mat1;

    ONNXtest() {
        initTestDataPath();
        env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "test");
        memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        out_gapi.resize(1);
        out_onnx.resize(1);
        // FIXME: It should be an image from own (gapi) directory in opencv extra
        in_mat1 = cv::imread(findDataFile("cv/dpm/cat.png"));
    }

    template<typename T>
    void infer(const std::vector<cv::Mat>& ins, std::vector<cv::Mat>& outs) {
        // Prepare session
#ifndef _WIN32
        session = Ort::Session(env, model_path.data(), session_options);
#else
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        std::wstring w_model_path = converter.from_bytes(model_path.data());
        session = Ort::Session(env, w_model_path.data(), session_options);
#endif
        num_in = session.GetInputCount();
        num_out = session.GetOutputCount();
        GAPI_Assert(num_in == ins.size());
        in_node_names.clear();
        out_node_names.clear();
        // Inputs Run params
        std::vector<Ort::Value> in_tensors;
        for(size_t i = 0; i < num_in; ++i) {
            char* in_node_name_p = session.GetInputName(i, allocator);
            in_node_names.push_back(std::string(in_node_name_p));
            allocator.Free(in_node_name_p);
            in_node_dims = toORT(ins[i].size);
            in_tensors.emplace_back(Ort::Value::CreateTensor<T>(memory_info,
                                                                const_cast<T*>(ins[i].ptr<T>()),
                                                                ins[i].total(),
                                                                in_node_dims.data(),
                                                                in_node_dims.size()));
        }
        // Outputs Run params
        for(size_t i = 0; i < num_out; ++i) {
            char* out_node_name_p = session.GetOutputName(i, allocator);
            out_node_names.push_back(std::string(out_node_name_p));
            allocator.Free(out_node_name_p);
        }
        // Input/output order by names
        const auto in_run_names  = getCharNames(in_node_names);
        const auto out_run_names = getCharNames(out_node_names);
        // Run
        auto result = session.Run(Ort::RunOptions{nullptr},
                                  in_run_names.data(),
                                  &in_tensors.front(),
                                  num_in,
                                  out_run_names.data(),
                                  num_out);
        // Copy outputs
        GAPI_Assert(result.size() == num_out);
        outs.resize(num_out);
        for (size_t i = 0; i < num_out; ++i) {
            const auto info = result[i].GetTensorTypeAndShapeInfo();
            const auto shape = info.GetShape();
            const auto type = toCV(info.GetElementType());
            const std::vector<int> dims(shape.begin(), shape.end());
            if (type != -1){
                cv::Mat(dims, type,
                        reinterpret_cast<void*>(result[i].GetTensorMutableData<uint8_t*>()))
                .copyTo(outs[i]);
            } else {
                std::vector<int> out_vec;
                const size_t total = std::accumulate(dims.begin(), dims.end(), 0);
                const int64_t* ptr = result[i].GetTensorMutableData<int64_t>();
                for (size_t l = 0; l < total; ++l) {
                    out_vec.push_back(static_cast<int>(ptr[l]));
                }
                cv::Mat (dims, CV_32S, out_vec.data()).copyTo(outs[i]);
            }
        }
    }
    // One input/output overload
    template<typename T>
    void infer(const cv::Mat& in, cv::Mat& out) {
        std::vector<cv::Mat> result;
        infer<T>(std::vector<cv::Mat>{in}, result);
        GAPI_Assert(result.size() == 1u);
        out = result.front();
    }
    // One input overload
    template<typename T>
    void infer(const cv::Mat& in, std::vector<cv::Mat>& outs) {
        infer<T>(std::vector<cv::Mat>{in}, outs);
    }

    void validate() {
        GAPI_Assert(!out_gapi.empty() && !out_onnx.empty());
        ASSERT_EQ(out_gapi.size(), out_onnx.size());
        const auto size = out_gapi.size();
        for (size_t i = 0; i < size; ++i) {
            normAssert(out_onnx[i], out_gapi[i], "Test outputs");
        }
    }

    void useModel(const std::string& model_name) {
        model_path = findModel(model_name);
    }
private:
    Ort::Env env{nullptr};
    Ort::MemoryInfo memory_info{nullptr};
    Ort::AllocatorWithDefaultOptions allocator;
    Ort::SessionOptions session_options;
    Ort::Session session{nullptr};

    std::vector<int64_t> in_node_dims;
    std::vector<std::string> in_node_names;
    std::vector<std::string> out_node_names;
};

class ONNXClassificationTest : public ONNXtest {
public:
    const cv::Scalar mean = { 0.485, 0.456, 0.406 };
    const cv::Scalar std  = { 0.229, 0.224, 0.225 };

    // Rois for InferList, InferList2
    const std::vector<cv::Rect> rois = {
        cv::Rect(cv::Point{ 0,   0}, cv::Size{80, 120}),
        cv::Rect(cv::Point{50, 100}, cv::Size{250, 360}),
    };

    void preprocess(const cv::Mat& src, cv::Mat& dst) {
        const int new_h = 224;
        const int new_w = 224;
        cv::Mat tmp, cvt, rsz;
        cv::resize(src, rsz, cv::Size(new_w, new_h));
        rsz.convertTo(cvt, CV_32F, 1.f / 255);
        tmp = (cvt - mean) / std;
        toCHW(tmp, dst);
        dst = dst.reshape(1, {1, 3, new_h, new_w});
    }
};

class ONNXMediaFrameTest : public ONNXClassificationTest {
public:
    const std::vector<cv::Rect> rois = {
        cv::Rect(cv::Point{ 0,   0}, cv::Size{80, 120}),
        cv::Rect(cv::Point{50, 100}, cv::Size{250, 360}),
        cv::Rect(cv::Point{70, 10}, cv::Size{20, 260}),
        cv::Rect(cv::Point{5, 15}, cv::Size{200, 160}),
    };
    cv::Mat m_in_y;
    cv::Mat m_in_uv;
    virtual void SetUp() {
        cv::Size sz{640, 480};
        m_in_y = initMatrixRandU(CV_8UC1, sz);
        m_in_uv = initMatrixRandU(CV_8UC2, sz / 2);
    }
};

class ONNXGRayScaleTest : public ONNXtest {
public:
    void preprocess(const cv::Mat& src, cv::Mat& dst) {
        const int new_h = 64;
        const int new_w = 64;
        cv::Mat cvc, rsz, cvt;
        cv::cvtColor(src, cvc, cv::COLOR_BGR2GRAY);
        cv::resize(cvc, rsz, cv::Size(new_w, new_h));
        rsz.convertTo(cvt, CV_32F);
        toCHW(cvt, dst);
        dst = dst.reshape(1, {1, 1, new_h, new_w});
    }
};

class ONNXWithRemap : public ONNXtest {
public:
    // You can specify any size of the outputs, since we don't know infer result
    // Tests validate a range with results and don't compare empty space
    void validate() {
        GAPI_Assert(!out_gapi.empty() && !out_onnx.empty());
        ASSERT_EQ(out_gapi.size(), out_onnx.size());
        const auto size = out_onnx.size();
        for (size_t i = 0; i < size; ++i) {
            float* op = out_onnx.at(i).ptr<float>();
            float* gp = out_gapi.at(i).ptr<float>();
            const auto out_size = std::min(out_onnx.at(i).total(), out_gapi.at(i).total());
            GAPI_Assert(out_size != 0u);
            for (size_t d_idx = 0; d_idx < out_size; ++d_idx) {
                if (gp[d_idx] == -1) {
                    break; // end of detections
                }
                ASSERT_EQ(op[d_idx], gp[d_idx]);
            }
        }
    }
};

class ONNXRCNN : public ONNXWithRemap {
private:
    const cv::Scalar rcnn_mean = { 102.9801, 115.9465, 122.7717 };
    float range_max = 1333;
    float range_min = 800;
public:
    void preprocess(const cv::Mat& src, cv::Mat& dst) {
        cv::Mat rsz, cvt, chw, mn;
        const auto get_ratio = [&](const int dim) -> float {
                                   return ((dim > range_max) || (dim < range_min))
                                              ? dim > range_max
                                                  ? range_max / dim
                                                  : range_min / dim
                                              : 1.f;
                               };
        const auto ratio_h = get_ratio(src.rows);
        const auto ratio_w = get_ratio(src.cols);
        const auto new_h = static_cast<int>(ratio_h * src.rows);
        const auto new_w = static_cast<int>(ratio_w * src.cols);
        cv::resize(src, rsz, cv::Size(new_w, new_h));
        rsz.convertTo(cvt, CV_32F, 1.f);
        toCHW(cvt, chw);
        mn = chw - rcnn_mean;
        int padded_h = std::ceil(new_h / 32.f) * 32;
        int padded_w = std::ceil(new_w / 32.f) * 32;
        cv::Mat pad_im(cv::Size(padded_w, 3 * padded_h), CV_32F, 0.f);
        pad_im(cv::Rect(0, 0, mn.cols, mn.rows)) += mn;
        dst = pad_im.reshape(1, {3, padded_h, padded_w});
    }
};

class ONNXYoloV3MultiInput : public ONNXWithRemap {
public:
    std::vector<cv::Mat> ins;

private:
    virtual void SetUp() {
        const int yolo_in_h = 416;
        const int yolo_in_w = 416;
        cv::Mat yolov3_input, shape, prep_mat;
        cv::resize(in_mat1, yolov3_input, cv::Size(yolo_in_w, yolo_in_h));
        shape.create(cv::Size(2, 1), CV_32F);
        float* ptr = shape.ptr<float>();
        ptr[0] = in_mat1.cols;
        ptr[1] = in_mat1.rows;
        preprocess(yolov3_input, prep_mat);
        ins = {prep_mat, shape};
    }

    void preprocess(const cv::Mat& src, cv::Mat& dst) {
        cv::Mat cvt;
        src.convertTo(cvt, CV_32F, 1.f / 255.f);
        toCHW(cvt, dst);
        dst = dst.reshape(1, {1, 3, 416, 416});
    }
};
} // anonymous namespace

TEST_F(ONNXClassificationTest, Infer)
{
    useModel("classification/squeezenet/model/squeezenet1.0-9");
    // ONNX_API code
    cv::Mat processed_mat;
    preprocess(in_mat1, processed_mat);
    infer<float>(processed_mat, out_onnx.front());
    // G_API code
    G_API_NET(SqueezNet, <cv::GMat(cv::GMat)>, "squeeznet");
    cv::GMat in;
    cv::GMat out = cv::gapi::infer<SqueezNet>(in);
    cv::GComputation comp(cv::GIn(in), cv::GOut(out));
    // NOTE: We have to normalize U8 tensor
    // so cfgMeanStd() is here
    auto net = cv::gapi::onnx::Params<SqueezNet> { model_path }.cfgMeanStd({ mean }, { std });
    comp.apply(cv::gin(in_mat1),
               cv::gout(out_gapi.front()),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}

TEST_F(ONNXClassificationTest, InferTensor)
{
    useModel("classification/squeezenet/model/squeezenet1.0-9");
    // Create tensor
    cv::Mat tensor;
    preprocess(in_mat1, tensor);
    // ONNX_API code
    infer<float>(tensor, out_onnx.front());
    // G_API code
    G_API_NET(SqueezNet, <cv::GMat(cv::GMat)>, "squeeznet");
    cv::GMat in;
    cv::GMat out = cv::gapi::infer<SqueezNet>(in);
    cv::GComputation comp(cv::GIn(in), cv::GOut(out));
    auto net = cv::gapi::onnx::Params<SqueezNet> { model_path };
    comp.apply(cv::gin(tensor),
               cv::gout(out_gapi.front()),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}

TEST_F(ONNXClassificationTest, InferROI)
{
    useModel("classification/squeezenet/model/squeezenet1.0-9");
    const auto ROI = rois.at(1);
    // ONNX_API code
    cv::Mat roi_mat;
    preprocess(in_mat1(ROI), roi_mat);
    infer<float>(roi_mat, out_onnx.front());
    // G_API code
    G_API_NET(SqueezNet, <cv::GMat(cv::GMat)>, "squeeznet");
    cv::GMat in;
    cv::GOpaque<cv::Rect> rect;
    cv::GMat out = cv::gapi::infer<SqueezNet>(rect, in);
    cv::GComputation comp(cv::GIn(in, rect), cv::GOut(out));
    // NOTE: We have to normalize U8 tensor
    // so cfgMeanStd() is here
    auto net = cv::gapi::onnx::Params<SqueezNet> { model_path }.cfgMeanStd({ mean }, { std });
    comp.apply(cv::gin(in_mat1, ROI),
               cv::gout(out_gapi.front()),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}

TEST_F(ONNXClassificationTest, InferROIList)
{
    useModel("classification/squeezenet/model/squeezenet1.0-9");
    // ONNX_API code
    out_onnx.resize(rois.size());
    for (size_t i = 0; i < rois.size(); ++i) {
        cv::Mat roi_mat;
        preprocess(in_mat1(rois[i]), roi_mat);
        infer<float>(roi_mat, out_onnx[i]);
    }
    // G_API code
    G_API_NET(SqueezNet, <cv::GMat(cv::GMat)>, "squeeznet");
    cv::GMat in;
    cv::GArray<cv::Rect> rr;
    cv::GArray<cv::GMat> out = cv::gapi::infer<SqueezNet>(rr, in);
    cv::GComputation comp(cv::GIn(in, rr), cv::GOut(out));
    // NOTE: We have to normalize U8 tensor
    // so cfgMeanStd() is here
    auto net = cv::gapi::onnx::Params<SqueezNet> { model_path }.cfgMeanStd({ mean }, { std });
    comp.apply(cv::gin(in_mat1, rois),
               cv::gout(out_gapi),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}

TEST_F(ONNXClassificationTest, Infer2ROIList)
{
    useModel("classification/squeezenet/model/squeezenet1.0-9");
    // ONNX_API code
    out_onnx.resize(rois.size());
    for (size_t i = 0; i < rois.size(); ++i) {
        cv::Mat roi_mat;
        preprocess(in_mat1(rois[i]), roi_mat);
        infer<float>(roi_mat, out_onnx[i]);
    }
    // G_API code
    G_API_NET(SqueezNet, <cv::GMat(cv::GMat)>, "squeeznet");
    cv::GMat in;
    cv::GArray<cv::Rect> rr;
    cv::GArray<cv::GMat> out = cv::gapi::infer2<SqueezNet>(in, rr);
    cv::GComputation comp(cv::GIn(in, rr), cv::GOut(out));
    // NOTE: We have to normalize U8 tensor
    // so cfgMeanStd() is here
    auto net = cv::gapi::onnx::Params<SqueezNet> { model_path }.cfgMeanStd({ mean }, { std });
    comp.apply(cv::gin(in_mat1, rois),
               cv::gout(out_gapi),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}

TEST_F(ONNXWithRemap, InferDynamicInputTensor)
{
    useModel("object_detection_segmentation/tiny-yolov2/model/tinyyolov2-8");
    // Create tensor
    cv::Mat cvt, rsz, tensor;
    cv::resize(in_mat1, rsz, cv::Size{416, 416});
    rsz.convertTo(cvt, CV_32F, 1.f / 255.f);
    toCHW(cvt, tensor);
    tensor = tensor.reshape(1, {1, 3, 416, 416});
    // ONNX_API code
    infer<float>(tensor, out_onnx.front());
    // G_API code
    G_API_NET(YoloNet, <cv::GMat(cv::GMat)>, "YoloNet");
    cv::GMat in;
    cv::GMat out = cv::gapi::infer<YoloNet>(in);
    cv::GComputation comp(cv::GIn(in), cv::GOut(out));
    auto net = cv::gapi::onnx::Params<YoloNet>{ model_path }
        .cfgPostProc({cv::GMatDesc{CV_32F, {1, 125, 13, 13}}}, remapYolo)
        .cfgOutputLayers({"out"});
    comp.apply(cv::gin(tensor),
               cv::gout(out_gapi.front()),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}

TEST_F(ONNXGRayScaleTest, InferImage)
{
    useModel("body_analysis/emotion_ferplus/model/emotion-ferplus-8");
    // ONNX_API code
    cv::Mat prep_mat;
    preprocess(in_mat1, prep_mat);
    infer<float>(prep_mat, out_onnx.front());
    // G_API code
    G_API_NET(EmotionNet, <cv::GMat(cv::GMat)>, "emotion-ferplus");
    cv::GMat in;
    cv::GMat out = cv::gapi::infer<EmotionNet>(in);
    cv::GComputation comp(cv::GIn(in), cv::GOut(out));
    auto net = cv::gapi::onnx::Params<EmotionNet> { model_path }
        .cfgNormalize({ false }); // model accepts 0..255 range in FP32;
    comp.apply(cv::gin(in_mat1),
               cv::gout(out_gapi.front()),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}

TEST_F(ONNXWithRemap, InferMultiOutput)
{
    useModel("object_detection_segmentation/ssd-mobilenetv1/model/ssd_mobilenet_v1_10");
    // ONNX_API code
    const auto prep_mat = in_mat1.reshape(1, {1, in_mat1.rows, in_mat1.cols, in_mat1.channels()});
    infer<uint8_t>(prep_mat, out_onnx);
    cv::Mat onnx_conv_out({1, 1, 200, 7}, CV_32F);
    remapToIESSDOut({out_onnx[3], out_onnx[0], out_onnx[2], out_onnx[1]}, onnx_conv_out);
    out_onnx.clear();
    out_onnx.push_back(onnx_conv_out);
    // G_API code
    G_API_NET(MobileNet, <cv::GMat(cv::GMat)>, "ssd_mobilenet");
    cv::GMat in;
    cv::GMat out = cv::gapi::infer<MobileNet>(in);
    cv::GComputation comp(cv::GIn(in), cv::GOut(out));
    auto net = cv::gapi::onnx::Params<MobileNet>{ model_path }
        .cfgOutputLayers({"detection_output"})
        .cfgPostProc({cv::GMatDesc{CV_32F, {1, 1, 200, 7}}}, remapSSDPorts);
    comp.apply(cv::gin(in_mat1),
               cv::gout(out_gapi.front()),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}

TEST_F(ONNXMediaFrameTest, InferBGR)
{
    useModel("classification/squeezenet/model/squeezenet1.0-9");
    // ONNX_API code
    cv::Mat processed_mat;
    preprocess(in_mat1, processed_mat);
    infer<float>(processed_mat, out_onnx.front());
    // G_API code
    auto frame = MediaFrame::Create<TestMediaBGR>(in_mat1);
    G_API_NET(SqueezNet, <cv::GMat(cv::GMat)>, "squeeznet");
    cv::GFrame in;
    cv::GMat out = cv::gapi::infer<SqueezNet>(in);
    cv::GComputation comp(cv::GIn(in), cv::GOut(out));
    // NOTE: We have to normalize U8 tensor
    // so cfgMeanStd() is here
    auto net = cv::gapi::onnx::Params<SqueezNet> { model_path }.cfgMeanStd({ mean }, { std });
    comp.apply(cv::gin(frame),
               cv::gout(out_gapi.front()),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}

TEST_F(ONNXMediaFrameTest, InferYUV)
{
    useModel("classification/squeezenet/model/squeezenet1.0-9");
    const auto frame = MediaFrame::Create<TestMediaNV12>(m_in_y, m_in_uv);
    // ONNX_API code
    cv::Mat pp;
    cvtColorTwoPlane(m_in_y, m_in_uv, pp, cv::COLOR_YUV2BGR_NV12);
    cv::Mat processed_mat;
    preprocess(pp, processed_mat);
    infer<float>(processed_mat, out_onnx.front());
    // G_API code
    G_API_NET(SqueezNet, <cv::GMat(cv::GMat)>, "squeeznet");
    cv::GFrame in;
    cv::GMat out = cv::gapi::infer<SqueezNet>(in);
    cv::GComputation comp(cv::GIn(in), cv::GOut(out));
    // NOTE: We have to normalize U8 tensor
    // so cfgMeanStd() is here
    auto net = cv::gapi::onnx::Params<SqueezNet> { model_path }.cfgMeanStd({ mean }, { std });
    comp.apply(cv::gin(frame),
               cv::gout(out_gapi.front()),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}

TEST_F(ONNXMediaFrameTest, InferROIBGR)
{
    useModel("classification/squeezenet/model/squeezenet1.0-9");
    auto frame = MediaFrame::Create<TestMediaBGR>(in_mat1);
    // ONNX_API code
    cv::Mat roi_mat;
    preprocess(in_mat1(rois.front()), roi_mat);
    infer<float>(roi_mat, out_onnx.front());
    // G_API code
    G_API_NET(SqueezNet, <cv::GMat(cv::GMat)>, "squeeznet");
    cv::GFrame in;
    cv::GOpaque<cv::Rect> rect;
    cv::GMat out = cv::gapi::infer<SqueezNet>(rect, in);
    cv::GComputation comp(cv::GIn(in, rect), cv::GOut(out));
    // NOTE: We have to normalize U8 tensor
    // so cfgMeanStd() is here
    auto net = cv::gapi::onnx::Params<SqueezNet> { model_path }.cfgMeanStd({ mean }, { std });
    comp.apply(cv::gin(frame, rois.front()),
               cv::gout(out_gapi.front()),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}

TEST_F(ONNXMediaFrameTest, InferROIYUV)
{
    useModel("classification/squeezenet/model/squeezenet1.0-9");
    const auto frame = MediaFrame::Create<TestMediaNV12>(m_in_y, m_in_uv);
    // ONNX_API code
    cv::Mat pp;
    cvtColorTwoPlane(m_in_y, m_in_uv, pp, cv::COLOR_YUV2BGR_NV12);
    cv::Mat roi_mat;
    preprocess(pp(rois.front()), roi_mat);
    infer<float>(roi_mat, out_onnx.front());
    // G_API code
    G_API_NET(SqueezNet, <cv::GMat(cv::GMat)>, "squeeznet");
    cv::GFrame in;
    cv::GOpaque<cv::Rect> rect;
    cv::GMat out = cv::gapi::infer<SqueezNet>(rect, in);
    cv::GComputation comp(cv::GIn(in, rect), cv::GOut(out));
    // NOTE: We have to normalize U8 tensor
    // so cfgMeanStd() is here
    auto net = cv::gapi::onnx::Params<SqueezNet> { model_path }.cfgMeanStd({ mean }, { std });
    comp.apply(cv::gin(frame, rois.front()),
               cv::gout(out_gapi.front()),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}

TEST_F(ONNXMediaFrameTest, InferListBGR)
{
    useModel("classification/squeezenet/model/squeezenet1.0-9");
    const auto frame = MediaFrame::Create<TestMediaBGR>(in_mat1);
    // ONNX_API code
    out_onnx.resize(rois.size());
    for (size_t i = 0; i < rois.size(); ++i) {
        cv::Mat roi_mat;
        preprocess(in_mat1(rois[i]), roi_mat);
        infer<float>(roi_mat, out_onnx[i]);
    }
    // G_API code
    G_API_NET(SqueezNet, <cv::GMat(cv::GMat)>, "squeeznet");
    cv::GFrame in;
    cv::GArray<cv::Rect> rr;
    cv::GArray<cv::GMat> out = cv::gapi::infer<SqueezNet>(rr, in);
    cv::GComputation comp(cv::GIn(in, rr), cv::GOut(out));
    // NOTE: We have to normalize U8 tensor
    // so cfgMeanStd() is here
    auto net = cv::gapi::onnx::Params<SqueezNet> { model_path }.cfgMeanStd({ mean }, { std });
    comp.apply(cv::gin(frame, rois),
               cv::gout(out_gapi),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}

TEST_F(ONNXMediaFrameTest, InferListYUV)
{
    useModel("classification/squeezenet/model/squeezenet1.0-9");
    const auto frame = MediaFrame::Create<TestMediaNV12>(m_in_y, m_in_uv);
    // ONNX_API code
    cv::Mat pp;
    cvtColorTwoPlane(m_in_y, m_in_uv, pp, cv::COLOR_YUV2BGR_NV12);
    out_onnx.resize(rois.size());
    for (size_t i = 0; i < rois.size(); ++i) {
        cv::Mat roi_mat;
        preprocess(pp(rois[i]), roi_mat);
        infer<float>(roi_mat, out_onnx[i]);
    }
    // G_API code
    G_API_NET(SqueezNet, <cv::GMat(cv::GMat)>, "squeeznet");
    cv::GFrame in;
    cv::GArray<cv::Rect> rr;
    cv::GArray<cv::GMat> out = cv::gapi::infer<SqueezNet>(rr, in);
    cv::GComputation comp(cv::GIn(in, rr), cv::GOut(out));
    // NOTE: We have to normalize U8 tensor
    // so cfgMeanStd() is here
    auto net = cv::gapi::onnx::Params<SqueezNet> { model_path }.cfgMeanStd({ mean }, { std });
    comp.apply(cv::gin(frame, rois),
               cv::gout(out_gapi),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}

TEST_F(ONNXMediaFrameTest, InferList2BGR)
{
    useModel("classification/squeezenet/model/squeezenet1.0-9");
    const auto frame = MediaFrame::Create<TestMediaBGR>(in_mat1);
    // ONNX_API code
    out_onnx.resize(rois.size());
    for (size_t i = 0; i < rois.size(); ++i) {
        cv::Mat roi_mat;
        preprocess(in_mat1(rois[i]), roi_mat);
        infer<float>(roi_mat, out_onnx[i]);
    }
    // G_API code
    G_API_NET(SqueezNet, <cv::GMat(cv::GMat)>, "squeeznet");
    cv::GFrame in;
    cv::GArray<cv::Rect> rr;
    cv::GArray<cv::GMat> out = cv::gapi::infer2<SqueezNet>(in, rr);
    cv::GComputation comp(cv::GIn(in, rr), cv::GOut(out));
    // NOTE: We have to normalize U8 tensor
    // so cfgMeanStd() is here
    auto net = cv::gapi::onnx::Params<SqueezNet> { model_path }.cfgMeanStd({ mean }, { std });
    comp.apply(cv::gin(frame, rois),
               cv::gout(out_gapi),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}

TEST_F(ONNXMediaFrameTest, InferList2YUV)
{
    useModel("classification/squeezenet/model/squeezenet1.0-9");
    const auto frame = MediaFrame::Create<TestMediaNV12>(m_in_y, m_in_uv);
    // ONNX_API code
    cv::Mat pp;
    cvtColorTwoPlane(m_in_y, m_in_uv, pp, cv::COLOR_YUV2BGR_NV12);
    out_onnx.resize(rois.size());
    for (size_t i = 0; i < rois.size(); ++i) {
        cv::Mat roi_mat;
        preprocess(pp(rois[i]), roi_mat);
        infer<float>(roi_mat, out_onnx[i]);
    }
    // G_API code
    G_API_NET(SqueezNet, <cv::GMat(cv::GMat)>, "squeeznet");
    cv::GFrame in;
    cv::GArray<cv::Rect> rr;
    cv::GArray<cv::GMat> out = cv::gapi::infer2<SqueezNet>(in, rr);
    cv::GComputation comp(cv::GIn(in, rr), cv::GOut(out));
    // NOTE: We have to normalize U8 tensor
    // so cfgMeanStd() is here
    auto net = cv::gapi::onnx::Params<SqueezNet> { model_path }.cfgMeanStd({ mean }, { std });
    comp.apply(cv::gin(frame, rois),
               cv::gout(out_gapi),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}

TEST_F(ONNXYoloV3MultiInput, InferConstInput)
{
    useModel("object_detection_segmentation/yolov3/model/yolov3-10");
    // ONNX_API code
    infer<float>(ins, out_onnx);
    // G_API code
    using OUT = std::tuple<cv::GMat, cv::GMat, cv::GMat>;
    G_API_NET(YoloNet, <OUT(cv::GMat)>, "yolov3");
    auto net = cv::gapi::onnx::Params<YoloNet>{model_path}
        .constInput("image_shape", ins[1])
        .cfgInputLayers({"input_1"})
        .cfgOutputLayers({"out1", "out2", "out3"})
        .cfgPostProc({cv::GMatDesc{CV_32F, {1, 10000, 4}},
                      cv::GMatDesc{CV_32F, {1, 80, 10000}},
                      cv::GMatDesc{CV_32S, {5, 3}}}, remapYoloV3);
    cv::GMat in, out1, out2, out3;
    std::tie(out1, out2, out3) = cv::gapi::infer<YoloNet>(in);
    cv::GComputation comp(cv::GIn(in), cv::GOut(out1, out2, out3));
    out_gapi.resize(num_out);
    comp.apply(cv::gin(ins[0]),
               cv::gout(out_gapi[0], out_gapi[1], out_gapi[2]),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}

TEST_F(ONNXYoloV3MultiInput, InferBSConstInput)
{
    // This test checks the case when a const input is used
    // and all input layer names are specified.
    // Const input has the advantage. It is expected behavior.
    useModel("object_detection_segmentation/yolov3/model/yolov3-10");
    // Tensor with incorrect image size
    // is used for check case when InputLayers and constInput have same names
    cv::Mat bad_shape;
    bad_shape.create(cv::Size(2, 1), CV_32F);
    float* ptr = bad_shape.ptr<float>();
    ptr[0] = 590;
    ptr[1] = 12;
    // ONNX_API code
    infer<float>(ins, out_onnx);
    // G_API code
    using OUT = std::tuple<cv::GMat, cv::GMat, cv::GMat>;
    G_API_NET(YoloNet, <OUT(cv::GMat, cv::GMat)>, "yolov3");
    auto net = cv::gapi::onnx::Params<YoloNet>{model_path}
    // Data from const input will be used to infer
        .constInput("image_shape", ins[1])
    // image_shape - const_input has same name
        .cfgInputLayers({"input_1", "image_shape"})
        .cfgOutputLayers({"out1", "out2", "out3"})
        .cfgPostProc({cv::GMatDesc{CV_32F, {1, 10000, 4}},
                      cv::GMatDesc{CV_32F, {1, 80, 10000}},
                      cv::GMatDesc{CV_32S, {5, 3}}}, remapYoloV3);
    cv::GMat in1, in2, out1, out2, out3;
    std::tie(out1, out2, out3) = cv::gapi::infer<YoloNet>(in1, in2);
    cv::GComputation comp(cv::GIn(in1, in2), cv::GOut(out1, out2, out3));
    out_gapi.resize(num_out);
    comp.apply(cv::gin(ins[0], bad_shape),
               cv::gout(out_gapi[0], out_gapi[1], out_gapi[2]),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}

TEST_F(ONNXRCNN, InferRCNN)
{
    useModel("object_detection_segmentation/faster-rcnn/model/FasterRCNN-10");
    cv::Mat dst;
    preprocess(in_mat1, dst);
    // ONNX_API code
    infer<float>(dst, out_onnx);
    // G_API code
    using FRCNNOUT = std::tuple<cv::GMat,cv::GMat,cv::GMat>;
    G_API_NET(FasterRCNN, <FRCNNOUT(cv::GMat)>, "FasterRCNN");
    auto net = cv::gapi::onnx::Params<FasterRCNN>{model_path}
        .cfgOutputLayers({"out1", "out2", "out3"})
        .cfgPostProc({cv::GMatDesc{CV_32F, {7,4}},
                      cv::GMatDesc{CV_32S, {7}},
                      cv::GMatDesc{CV_32F, {7}}}, remapRCNNPorts);
    cv::GMat in, out1, out2, out3;
    std::tie(out1, out2, out3) = cv::gapi::infer<FasterRCNN>(in);
    cv::GComputation comp(cv::GIn(in), cv::GOut(out1, out2, out3));
    out_gapi.resize(num_out);
    comp.apply(cv::gin(dst),
               cv::gout(out_gapi[0], out_gapi[1], out_gapi[2]),
               cv::compile_args(cv::gapi::networks(net)));
    // Validate
    validate();
}
} // namespace opencv_test

#endif //  HAVE_ONNX
