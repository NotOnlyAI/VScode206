//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#ifndef FaceAlignment_LA1_hpp1
#define FaceAlignment_LA1_hpp1

#pragma once

#include "MNN/Interpreter.hpp"
#include "MNN/MNNDefine.h"
#include "MNN/Tensor.hpp"
#include "MNN/ImageProcess.hpp"

#include "models206_typedef.h"
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



using namespace M2;



class FaceAlignment {
public:
    FaceAlignment();

    ~FaceAlignment();

    int ForwardBGR(const cv::Mat &image,const M2::Object &face,M2::LandmarkInfo &landmarkinfo);

    int DMSJudge(const M2::LandmarkInfo &landmarkinfo,int &DMSTpye);

    int init(int deviceTpye,int print_config,int modelType,float ratio_eye,float ratio_mouth);
    
    LandmarkInfo  m_landmarkInfo;

    int m_DMSTpye=0;

    M2::Point2f m_left_eye;
    M2::Point2f m_right_eye;


    bool model_is_ok=false;


private:

    

    int decode(std::vector< MNN::Tensor*> &outputTensors_host);

    std::shared_ptr<MNN::Interpreter> net;
    MNN::Session *session = nullptr;
    MNN::CV::ImageProcess::Config imconfig;
    std::shared_ptr<MNN::CV::ImageProcess> pretreat;
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
    int m_print;
    int m_modelType;
    float m_ratio_eye;
    float m_ratio_mouth;

    std::vector< M2::Point2f> left_eye_points;
    std::vector< M2::Point2f> right_eye_points;

};

#endif /* FaceAlignment_hpp */
