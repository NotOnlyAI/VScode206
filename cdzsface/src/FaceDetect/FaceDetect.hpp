//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#ifndef FaceDetect_hpp
#define FaceDetect_hpp

#pragma once

#include "MNN/Interpreter.hpp"
#include "MNN/MNNDefine.h"
#include "MNN/Tensor.hpp"
#include "MNN/ImageProcess.hpp"
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>

#define num_featuremap 4
#define hard_nms 1
#define blending_nms 2 /* mix nms was been proposaled in paper blaze face, aims to minimize the temporal jitter*/
typedef struct FaceInfo {
    float x1;
    float y1;
    float x2;
    float y2;
    float score;

} FaceInfo;

class FaceDetect {
public:
    FaceDetect();

    ~FaceDetect();

    int Forward(cv::Mat &raw_image);

private:

    std::shared_ptr<MNN::Interpreter> FaceDetect_interpreter;
    MNN::Session *FaceDetect_session = nullptr;
    MNN::Tensor *input_tensor = nullptr;

    int in_w=256;
    int in_h=256;
    int image_w;
    int image_h;
    const float mean_vals[3] = {127, 127, 127};
    const float norm_vals[3] = {1.0 / 128, 1.0 / 128, 1.0 / 128};

   
};

#endif /* FaceDetect_hpp */
