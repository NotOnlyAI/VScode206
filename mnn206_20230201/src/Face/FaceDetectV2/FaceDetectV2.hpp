//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#ifndef FaceDetectV2_LA_hpp
#define FaceDetectV2_LA_hpp

#pragma once

#include "MNN/Interpreter.hpp"
#include "MNN/MNNDefine.h"
#include "MNN/Tensor.hpp"
#include "MNN/ImageProcess.hpp"

#include "FaceDetect/FaceDetect.hpp"
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>


// #define MAXFACECOUNT	200
// typedef struct RectDetect
// {
// 	int x;
// 	int y;
// 	int width;
// 	int height;
// }RectDetect;


// typedef struct LabelDetect
// {
// 	int label;
// 	float score;
// }LabelDetect;

// typedef struct RectDetectInfo
// {
// 	int nFaceNum;
// 	RectDetect rects[MAXFACECOUNT];
// 	LabelDetect labels[MAXFACECOUNT];
// }RectDetectInfo;



class FaceDetectV2 {
public:
    FaceDetectV2();

    ~FaceDetectV2();

    int Forward(cv::Mat &raw_image);

    RectDetectInfo  rectinfo;

private:

    int decode(std::vector< MNN::Tensor*> &outputTensors_host);

    std::shared_ptr<MNN::Interpreter> net;
    MNN::Session *session = nullptr;
    MNN::Tensor::DimensionType dimType = MNN::Tensor::CAFFE;



    std::vector< MNN::Tensor*> outputTensors;
    std::vector< MNN::Tensor*> outputTensors_host;
    std::vector<std::string> output_blob_names;


    std::vector< MNN::Tensor*> inputTensors;
    std::vector< MNN::Tensor*> inputTensors_host;
    std::vector<std::string> input_blob_names;



    int in_w;
    int in_h;
    int image_h;
    int image_w;
    

};

#endif /* FaceDetectV2_hpp */
