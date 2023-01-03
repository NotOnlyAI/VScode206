//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#ifndef LaneDetect_LA21_hpp
#define LaneDetect_LA21_hpp

#pragma once

#include "MNN/Interpreter.hpp"
#include "MNN/MNNDefine.h"
#include "MNN/Tensor.hpp"
#include "MNN/ImageProcess.hpp"

#include "models206_typedef.h"
#include "M2utils/nms.h"

#include <opencv2/opencv.hpp>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>


using namespace M2;


class LaneDetect {
public:
    LaneDetect();

    ~LaneDetect();

    
    int ForwardBGR(const cv::Mat &image,std::vector<lane_DECODE> &final_lane);
    int init(int deviceTpye,int print_config,int modelType);


    std::vector<lane_DECODE> m_decode_lane;
    std::vector<lane_DECODE> m_select_lane;
    std::vector<lane_DECODE> m_final_lane_with_type;


    
     int model_is_ok;
private:

    
    int decode(std::vector< MNN::Tensor*> &outputTensors_host);
    int selected_lane(std::vector<lane_DECODE> ALL_LANE, int thresh);
    void LeftRightGet(std::vector<lane_DECODE>& final_lane);
    float calc_err_dis_with_pos(lane_DECODE L_1, lane_DECODE L_2);
    

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
    int m_modelType;
   
    

};

#endif /* LaneDetect_hpp */
