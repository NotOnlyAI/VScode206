//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#ifndef FaceDetect_hpp
#define FaceDetect_hpp

#pragma once

#include "MNN/Interpreter.hpp"
#include "MNN/MNNDefine.h"
#include "MNN/Tensor.hpp"
#include "MNN/ImageProcess.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include "models206_typedef.h"
#include <opencv2/opencv.hpp>




class FaceDetect {
public:
    FaceDetect();

    ~FaceDetect();

    int Forward(const M2::ImgData_T &imgdata,M2::DetectResult &re,int max_or_mid);

    int init(int deviceTpye,int print_config);


    int visImg(const M2::ImgData_T &imgdata,const M2::DetectResult &re);


    M2::DetectResult  m_rectinfo;
    bool model_is_ok=false;

private:

    int decode(std::vector< MNN::Tensor*> &outputTensors_host);

    std::shared_ptr<MNN::Interpreter> net;
    std::shared_ptr<MNN::CV::ImageProcess> pretreat;
    MNN::CV::ImageProcess::Config imconfig;

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
    int m_print;

};

#endif /* FaceDetect_hpp */
