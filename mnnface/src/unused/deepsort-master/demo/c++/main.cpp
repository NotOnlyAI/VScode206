#include <iostream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <chrono>

#include "Detector.h"
#include "DeepSORT.h"
#include "util.h"

using namespace std;

int main(int argc, const char *argv[]) {

    if (argc < 2 || argc > 3) {
        throw runtime_error("usage: processing <input path> [<scale factor>]");
    }
    auto input_path = string(argv[1]);
    auto scale_factor = argc == 3 ? stoi(argv[2]) : 1;
    cv::VideoCapture cap(input_path);

    if (!cap.isOpened()) {
        throw runtime_error("Cannot open cv::VideoCapture");
    }

    array<int64_t, 2> orig_dim{int64_t(cap.get(cv::CAP_PROP_FRAME_HEIGHT)), int64_t(cap.get(cv::CAP_PROP_FRAME_WIDTH))};
    array<int64_t, 2> inp_dim;
    for (size_t i = 0; i < 2; ++i) {
        auto factor = 1 << 5;
        inp_dim[i] = (orig_dim[i] / 1 / factor + 1) * factor;
    }

    // edit yolo weight's path
    Detector detector(
            inp_dim,
            "weight/yolov3.cfg",
            "weight/yolov3.weights");
    DeepSORT tracker(orig_dim, "weight/ckpt.bin");

    auto image = cv::Mat();
    cv::namedWindow("Output", cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
    while (cap.read(image)) {
        auto frame_processed = static_cast<uint32_t>(cap.get(cv::CAP_PROP_POS_FRAMES)) - 1;

        auto start = chrono::steady_clock::now();

        auto dets = detector.detect(image);
        auto trks = tracker.update(dets, image);

        stringstream str;
        str << "Frame: " << frame_processed << "/" << cap.get(cv::CAP_PROP_FRAME_COUNT) << ", "
            << "FPS: " << fixed << setprecision(2)
            << 1000.0 / chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start).count();
        draw_text(image, str.str(), {0, 0, 0}, {image.cols, 0}, true);

        for (auto &t:trks) {
            cv::Point2f p1(t[1], t[2]);
            cv::Point2f p2(t[3], t[4]);
            draw_bbox(image, p1, p2, to_string(t[0]), color_map(t[0]));
        }

        cv::imshow("Output", image);

        switch (cv::waitKey(1) & 0xFF) {
            case 'q':
                return 0;
            case ' ':
                cv::imwrite(to_string(frame_processed) + ".jpg", image);
                break;
            default:
                break;
        }
    }
}