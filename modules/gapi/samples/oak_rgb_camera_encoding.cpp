#include <fstream>

#include <opencv2/gapi.hpp>
#include <opencv2/gapi/core.hpp>
#include <opencv2/gapi/gframe.hpp>

#include <opencv2/gapi/oak/oak.hpp>
#include <opencv2/gapi/streaming/format.hpp> // BGR accessor

#include <opencv2/highgui.hpp> // CommandLineParser

const std::string keys =
    "{ h help  |              | Print this help message }"
    "{ output  | output.h265  | Path to the output .h265 video file }";

int main(int argc, char *argv[]) {
    cv::CommandLineParser cmd(argc, argv, keys);
    if (cmd.has("help")) {
        cmd.printMessage();
        return 0;
    }

    const std::string output = cmd.get<std::string>("output");

    cv::GFrame in;
    cv::GArray<uint8_t> encoded = cv::gapi::oak::encode(in, {});

    auto args = cv::compile_args(cv::gapi::oak::ColorCameraParams{}, cv::gapi::oak::kernels());

    auto pipeline = cv::GComputation(cv::GIn(in), cv::GOut(encoded)).compileStreaming(std::move(args));

    // Graph execution /////////////////////////////////////////////////////////
    pipeline.setSource(cv::gapi::wip::make_src<cv::gapi::oak::ColorCamera>());
    pipeline.start();

    std::vector<uint8_t> out_h265_data;

    std::ofstream out_h265_file;
    out_h265_file.open(output, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);

    // Pull 300 frames from the camera
    uint32_t frames = 30;
    uint32_t pulled = 0;

    while (pulled++ < frames &&
           pipeline.pull(cv::gout(out_h265_data))) {
        std::cout << pulled << std::endl;
        if (out_h265_file.is_open()) {
            out_h265_file.write(reinterpret_cast<const char*>(out_h265_data.data()),
                                                              out_h265_data.size());
        }
    }
}
