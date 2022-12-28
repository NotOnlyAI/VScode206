//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#define clip(x, y) (x < 0 ? 0 : (x > y ? y : x))

#include "LaneDetect.hpp"
#include "M2utils/nms.h"
#include <opencv2/opencv.hpp>
using namespace std;
using namespace M2;


LaneDetect::LaneDetect() {
}

int LaneDetect::init(int deviceTpye,int print_config){

    m_print=print_config;
    string mnn_path="./models206/Vega_new_extract.mnn";
    net = std::shared_ptr<MNN::Interpreter>(MNN::Interpreter::createFromFile(mnn_path.c_str()));
    dimType = MNN::Tensor::CAFFE;
    MNN_PRINT("LaneDetect Interpreter build, model_path: %s, dimType:%d\n",mnn_path.c_str(),dimType);

    in_h=288;
    in_w=512;


    MNN::ScheduleConfig config;
    config.type=(MNNForwardType)(deviceTpye);
    config.mode = MNN_GPU_TUNING_NORMAL | MNN_GPU_MEMORY_IMAGE;
    MNN::BackendConfig backendConfig;
    backendConfig.precision = MNN::BackendConfig::Precision_Normal;
    config.backendConfig = &backendConfig;
    session = net->createSession(config);
    MNN_PRINT("ScheduleConfig build, config.type: %d \n",config.type);


    float mean[3]     = {123.675f, 116.28f, 103.53f};
    float normals[3] = {0.017125f, 0.01751f, 0.01743f};
    ::memcpy(imconfig.mean, mean, sizeof(mean));
    ::memcpy(imconfig.normal, normals, sizeof(normals));
    imconfig.sourceFormat = MNN::CV::BGR;
    imconfig.destFormat = MNN::CV::BGR;
    imconfig.filterType = MNN::CV::NEAREST;
    pretreat=std::shared_ptr<MNN::CV::ImageProcess>(MNN::CV::ImageProcess::create(imconfig));
    MNN_PRINT("ImageProcess build, sourceFormat: %d, destFormat: %d \n",imconfig.sourceFormat,imconfig.destFormat);



    input_blob_names={ "input"};
    inputTensors.resize(input_blob_names.size());
    inputTensors_host.resize(input_blob_names.size());
    for (int i = 0; i < input_blob_names.size(); i++) {
		inputTensors[i] = net->getSessionInput(session,input_blob_names[0].c_str());
	}

    for (int i = 0; i < input_blob_names.size(); i++) {
		inputTensors_host[i] = new MNN::Tensor(inputTensors[i], dimType);
	}

    output_blob_names={ "287","288"};
    outputTensors.resize(output_blob_names.size());
    outputTensors_host.resize(output_blob_names.size());

    for (int i = 0; i < output_blob_names.size(); i++) {
		outputTensors[i] = net->getSessionOutput(session, output_blob_names[i].c_str());
	}

    for (int i = 0; i < output_blob_names.size(); i++) {
		outputTensors_host[i] = new MNN::Tensor(outputTensors[i], dimType);
	}


    float memoryUsage = 0.0f;
    net->getSessionInfo(session, MNN::Interpreter::MEMORY, &memoryUsage);
    float flops = 0.0f;
    net->getSessionInfo(session, MNN::Interpreter::FLOPS, &flops);
    int backendType[2];
    net->getSessionInfo(session, MNN::Interpreter::BACKENDS, backendType);
    MNN_PRINT("Session Info: memory use %f MB, flops is %f M, backendType is %d, batch size = %d\n", memoryUsage, flops, backendType[0], 1);


    for (int i = 0; i < input_blob_names.size(); i++) {
        // MNN_PRINT("%s\n",input_blob_names[i]);
		inputTensors_host[i]->printShape();
	}

    for (int i = 0; i < output_blob_names.size(); i++) {
        // MNN_PRINT("%s\n",output_blob_names[i]);
		outputTensors_host[i]->printShape();
	}



}

int LaneDetect::ForwardBGR(const cv::Mat &image,std::vector<M2::lane_DECODE> &final_lane) {




    

    auto start = chrono::steady_clock::now();
    image_h = image.rows;
    image_w = image.cols;
    cv::Mat resize_img;
    cv::resize(image,resize_img,cv::Size(in_w,in_h));
    pretreat->convert((uint8_t *)resize_img.data, in_w,in_h,0,inputTensors_host[0]);

    inputTensors[0]->copyFromHostTensor(inputTensors_host[0]);


    net->runSession(session);


    for (int i = 0; i < output_blob_names.size(); i++) {
		outputTensors[i]->copyToHostTensor(outputTensors_host[i]);
	}

    if(m_print>=1){
        chrono::duration<double> elapsed2 = chrono::steady_clock::now() - start;
        cout << "LaneDetect time:" << elapsed2.count() << " s" << endl;
        outputTensors_host[0]->printShape();
        for (int i = 0; i < 20; ++i) {
            MNN_PRINT("copy %f\n",  outputTensors_host[0]->host<float>()[i]);
        }
    }
    

    decode(outputTensors_host);
    selected_lane(m_decode_lane,200);
    LeftRightGet(m_select_lane);

    for (int i = 0; i < m_final_lane_with_type.size(); i++){
        final_lane.push_back(m_final_lane_with_type[i]);
    }

    m_decode_lane.clear();
    m_select_lane.clear();
    m_final_lane_with_type.clear();

    if(m_print>=1){
        chrono::duration<double> elapsed3 = chrono::steady_clock::now() - start;
        cout << "LaneDetect Decode time:" << elapsed3.count() << " s" << endl;
    }

    return 0;
}

bool LessSort(lane_DECODE a, lane_DECODE b)  //����
{
    return (a.prob > b.prob);
}

bool LessSort_point(cv::Point a, cv::Point b) //����
{
    return (a.y > b.y);
}
// �������ߵľ�������
bool moreSort(lane_DECODE a, lane_DECODE b) //����
{
    return (a.dis < b.dis);
}

int compare_greater(cv::Point2f a, cv::Point2f b)  //vector�õ���Զ������ĵ�
{
    return a.x > b.x;
}




int LaneDetect::decode(std::vector< MNN::Tensor*> &outputTensors_host)
{
    // for (int i = 0; i < 20; ++i) {
    //     MNN_PRINT("func  %d %f, %f\n", i, outputTensors_host[0]->host<float>()[2*i+0], outputTensors_host[0]->host<float>()[2*i+1]);
    // }
    

    float score0;
    float score1;

    float down_anchor_lane[72];
    float up_anchor_lane[73];


    int feature_h=18;
    int feature_w=32;
    int points_per_anchor = 4;   // 72/18 = 4
    int points_per_line = 72;   // 72/18 = 4
    float interval = 288.0 / 72.0;

    for (int n=0;n<feature_h*feature_w;n++)
    {
        score0=outputTensors_host[0]->host<float>()[2*n+0];
        score1=outputTensors_host[0]->host<float>()[2*n+1];
        SVP_NNIE_SoftMaxV2(score0,score1);
        if(score1>0.6)
        {
            memcpy(down_anchor_lane, &(outputTensors_host[1]->host<float>()[145*n+0]), points_per_line  * sizeof(float));
            memcpy(up_anchor_lane, &(outputTensors_host[1]->host<float>()[145*n+72]), (points_per_line +1) * sizeof(float));
            // for (int k = 0;k < 72; ++k) {
            //     MNN_PRINT("func2  %d %d %f, %f ,%f\n",n, k,score1, down_anchor_lane[k], up_anchor_lane[k]);
            // }
            float relative_end_pos = up_anchor_lane[0];
            int h=n/feature_w;
            int w=n%feature_w;
            int anchor_y_pos = int((feature_h - 1 - h) * points_per_anchor);
            float anchor_center_x = (1.0 * w + 0.5) * 16;
            float anchor_center_y = (1.0 * h + 0.5) * 16;
            int end_pos = anchor_y_pos;
            int start_pos = anchor_y_pos;
            std::vector<cv::Point> Lane;


            for (int i = 0; i < points_per_line; i++)
            {
                if (i >= relative_end_pos || anchor_y_pos + i >= points_per_line)
                    break;
                int rela_x = up_anchor_lane[1 + i];
                float abs_x = anchor_center_x + rela_x;
                float abs_y = in_h - 1 - (anchor_y_pos + i) * interval;
                cv::Point p;
                p.x = abs_x;
                p.y = abs_y;
                end_pos = anchor_y_pos + i + 1;
                Lane.push_back(p);
            }

            for (int i = 0; i < anchor_y_pos; i++)
            {
                int rela_x = down_anchor_lane[i];
                float abs_x = anchor_center_x + rela_x;
                float abs_y = in_h - 1 - (anchor_y_pos - 1 - i) * interval;
                cv::Point p;
                p.x = abs_x;
                p.y = abs_y;
                start_pos = anchor_y_pos - 1 - i;
                Lane.push_back(p);
            }
            if (Lane.size() >= 2)
            {
                sort(Lane.begin(), Lane.end(), LessSort_point);
                lane_DECODE a;
                a.prob = score1;
                a.start_pos = start_pos;
                a.end_pos = end_pos;
                a.Lane = Lane;
                a.ax = anchor_center_x;
                a.ax = anchor_center_y;
                m_decode_lane.push_back(a);
            }
            //printf("index is %d,prob is %f,  start_pos is %d,  end_pos is  %d, anchor_center_x is %f, anchor_center_y is %f \n", index, score1, start_pos, end_pos, anchor_center_x, anchor_center_y);
        }
    }
   

    return 1;

}



float LaneDetect::calc_err_dis_with_pos(lane_DECODE L_1, lane_DECODE L_2) // ���������ߵľ���
{
    int max_start_pos = max(L_1.start_pos, L_2.start_pos);
    int min_end_pos = min(L_1.end_pos, L_2.end_pos);
    if (min_end_pos <= max_start_pos || max_start_pos < 0 || min_end_pos < 1)
        return 10e6;
    std::vector<cv::Point> pts1 = L_1.Lane;
    std::vector<cv::Point> pts2 = L_2.Lane;
    float dis = 0.0;
    for (int i = max_start_pos; i < min_end_pos; i++) {
        dis += abs(pts1[i - L_1.start_pos].x - pts2[i - L_2.start_pos].x);
    }
    dis = dis / (min_end_pos - max_start_pos);
    float dis_start = abs(L_1.Lane[max_start_pos - L_1.start_pos].x - L_2.Lane[max_start_pos - L_2.start_pos].x);
    dis = max(dis, dis_start);
    float dis_end = abs(L_1.Lane[min_end_pos - 1 - L_1.start_pos].x - L_2.Lane[min_end_pos - 1 - L_2.start_pos].x);
    dis = max(dis, dis_end);
    dis = max(dis_start, dis_end);
    //cout << "dis is " << dis << "\n";
    return dis;
}

int LaneDetect::selected_lane(std::vector<lane_DECODE> ALL_LANE, int thresh)
{
    // std::vector<lane_DECODE> save_LANE = {};
    sort(ALL_LANE.begin(), ALL_LANE.end(), LessSort);

    int NumLane = ALL_LANE.size();
    if (NumLane == 0)
    {
        return -1;
    }

    bool selected[ALL_LANE.size()] = { false };

    for (int i = 0; i < ALL_LANE.size() - 1; i++)
    {
        if (selected[i])
            continue;
        m_select_lane.push_back(ALL_LANE[i]);
        selected[i] = true;
        for (int j = i + 1; j < ALL_LANE.size() - 1; j++)
        {
            float dis = calc_err_dis_with_pos(ALL_LANE[i], ALL_LANE[j]);

            if (dis <= thresh) {
                selected[j] = true;
            }
        }
    }
    return 0;
}

void LaneDetect::LeftRightGet(std::vector<lane_DECODE>& final_lane)
{
    float sx1 = float(image_w) / float(in_w);
    float sy1 = float(image_h) / float(in_h);
    std::vector<lane_DECODE> right;
    std::vector<lane_DECODE> left;
    for (int i = 0; i < final_lane.size(); i++) {
        for (int j = 0; j < final_lane[i].Lane.size(); j++) {
            float px = final_lane[i].Lane[j].x * sx1;
            float py = final_lane[i].Lane[j].y * sy1;
            if (px > 0 && px < 1280) {
                if (px - 640 < 0) {
                    final_lane[i].dis = abs(px - 640);
                    left.push_back(final_lane[i]);
                    break;
                }
                if (px - 640 >= 0) {
                    final_lane[i].dis = px - 640;
                    right.push_back(final_lane[i]);
                    break;
                }
            }
        }
    }
    if (left.size() > 0)
    {
        sort(left.begin(), left.end(), moreSort);
        for (int i = 0; i < left.size(); i++) {
            if (i == 0) {
                left[i].LeftRightType = -1;
            }
            else
            {
                left[i].LeftRightType = -2;
            }
            m_final_lane_with_type.push_back(left[i]);
        }
    }

    if (right.size() > 0) {
        sort(right.begin(), right.end(), moreSort);
        for (int i = 0; i < right.size(); i++) {
            if (i == 0) {
                right[i].LeftRightType = 1;
            }
            else
            {
                right[i].LeftRightType = 2;
            }
            m_final_lane_with_type.push_back(right[i]);
        }
    }
}

LaneDetect::~LaneDetect() {
    if (net!=nullptr){
        net->releaseModel();
        net->releaseSession(session);
        for (int i = 0; i < input_blob_names.size(); i++) {
            // delete inputTensors[i];
            delete inputTensors_host[i];
        }
        for (int i = 0; i < output_blob_names.size(); i++) {
            // delete outputTensors[i];
            delete outputTensors_host[i];
        }
        inputTensors.clear();
        inputTensors_host.clear();
        outputTensors.clear();
        outputTensors_host.clear();
        input_blob_names.clear();
        output_blob_names.clear();
        m_decode_lane.clear();
        m_select_lane.clear();
        m_final_lane_with_type.clear();
    }
   
}

