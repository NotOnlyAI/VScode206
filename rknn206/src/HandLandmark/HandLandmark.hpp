//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#ifndef HandLandmark_LA21_hpp1
#define HandLandmark_LA21_hpp1

#pragma once



#include "models206_typedef.h"
#include "rknn_api.h"
#include "rga.h"
#include "RgaUtils.h"
#include "im2d.h"

#include <opencv2/opencv.hpp>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <sys/time.h>


using namespace M2;


class HandLandmark {
public:
    HandLandmark();

    ~HandLandmark();

    
    int ForwardBGR(const cv::Mat &image,std::vector<lane_DECODE> &final_lane);
    int Init(int deviceTpye,int print_config,int modelType);


    std::vector<lane_DECODE> m_decode_lane;
    std::vector<lane_DECODE> m_select_lane;
    std::vector<lane_DECODE> m_final_lane_with_type;


    
     int model_is_ok;
private:

    
    int decode();
    int selected_lane(std::vector<lane_DECODE> ALL_LANE, int thresh);
    void LeftRightGet(std::vector<lane_DECODE>& final_lane);
    float calc_err_dis_with_pos(lane_DECODE L_1, lane_DECODE L_2);
    



    int channel = 3;
    int width   = 0;
    int height  = 0;
    int img_width;
    int img_height;


    int m_print;
    int m_modelType;

    rknn_context   ctx;
    rknn_input_output_num io_num;
    rknn_tensor_attr input_attrs[1];
    rknn_tensor_attr output_attrs[4];
    rknn_input inputs[1];
    rknn_output outputs[4];

};

#endif /* HandLandmark_hpp */
