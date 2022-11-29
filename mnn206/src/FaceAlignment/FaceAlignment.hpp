//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#ifndef FaceAlignment_LA1_hpp
#define FaceAlignment_LA1_hpp

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



// flip_parts = ([1, 17], [2, 16], [3, 15], [4, 14], [5, 13], [6, 12], [7, 11], [8, 10],
//     [18, 27], [19, 26], [20, 25], [21, 24], [22, 23],
//     [32, 36], [33, 35],
//     [37, 46], [38, 45], [39, 44], [40, 43], [41, 48], [42, 47],
//     [49, 55], [50, 54], [51, 53], [62, 64], [61, 65], [68, 66], [59, 57], [60, 56])



typedef struct PointDetect
{
	float x;
	float y;
}PointDetect;



typedef struct PointDetectInfo
{
	PointDetect points[68];
}PointDetectInfo;




class FaceAlignment {
public:
    FaceAlignment();

    ~FaceAlignment();

    int Forward(cv::Mat &raw_image);

    PointDetectInfo  landmark68;

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

#endif /* FaceAlignment_hpp */
