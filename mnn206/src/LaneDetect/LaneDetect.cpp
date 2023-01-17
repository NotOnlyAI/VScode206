//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#define clip(x, y) (x < 0 ? 0 : (x > y ? y : x))

#include "LaneDetect.hpp"
#include <opencv2/opencv.hpp>
using namespace std;
using namespace M2;


static float s_af32ExpCoef[10][16] = {
    {1.0f, 1.00024f, 1.00049f, 1.00073f, 1.00098f, 1.00122f, 1.00147f, 1.00171f, 1.00196f, 1.0022f, 1.00244f, 1.00269f, 1.00293f, 1.00318f, 1.00342f, 1.00367f},
    {1.0f, 1.00391f, 1.00784f, 1.01179f, 1.01575f, 1.01972f, 1.02371f, 1.02772f, 1.03174f, 1.03578f, 1.03984f, 1.04391f, 1.04799f, 1.05209f, 1.05621f, 1.06034f},
    {1.0f, 1.06449f, 1.13315f, 1.20623f, 1.28403f, 1.36684f, 1.45499f, 1.54883f, 1.64872f, 1.75505f, 1.86825f, 1.98874f, 2.117f, 2.25353f, 2.39888f, 2.55359f},
    {1.0f, 2.71828f, 7.38906f, 20.0855f, 54.5981f, 148.413f, 403.429f, 1096.63f, 2980.96f, 8103.08f, 22026.5f, 59874.1f, 162755.0f, 442413.0f, 1.2026e+006f, 3.26902e+006f},
    {1.0f, 8.88611e+006f, 7.8963e+013f, 7.01674e+020f, 6.23515e+027f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f},
    {1.0f, 0.999756f, 0.999512f, 0.999268f, 0.999024f, 0.99878f, 0.998536f, 0.998292f, 0.998049f, 0.997805f, 0.997562f, 0.997318f, 0.997075f, 0.996831f, 0.996588f, 0.996345f},
    {1.0f, 0.996101f, 0.992218f, 0.98835f, 0.984496f, 0.980658f, 0.976835f, 0.973027f, 0.969233f, 0.965455f, 0.961691f, 0.957941f, 0.954207f, 0.950487f, 0.946781f, 0.94309f},
    {1.0f, 0.939413f, 0.882497f, 0.829029f, 0.778801f, 0.731616f, 0.687289f, 0.645649f, 0.606531f, 0.569783f, 0.535261f, 0.502832f, 0.472367f, 0.443747f, 0.416862f, 0.391606f},
    {1.0f, 0.367879f, 0.135335f, 0.0497871f, 0.0183156f, 0.00673795f, 0.00247875f, 0.000911882f, 0.000335463f, 0.00012341f, 4.53999e-005f, 1.67017e-005f, 6.14421e-006f, 2.26033e-006f, 8.31529e-007f, 3.05902e-007f},
    {1.0f, 1.12535e-007f, 1.26642e-014f, 1.42516e-021f, 1.60381e-028f, 1.80485e-035f, 2.03048e-042f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
};

static float SVP_NNIE_QuickExp( int s32Value )
{
    if( s32Value & 0x80000000 )
    {
        s32Value = ~s32Value + 0x00000001;
        return s_af32ExpCoef[5][s32Value & 0x0000000F] * s_af32ExpCoef[6][(s32Value>>4) & 0x0000000F] * s_af32ExpCoef[7][(s32Value>>8) & 0x0000000F] * s_af32ExpCoef[8][(s32Value>>12) & 0x0000000F] * s_af32ExpCoef[9][(s32Value>>16) & 0x0000000F ];
    }
    else
    {
        return s_af32ExpCoef[0][s32Value & 0x0000000F] * s_af32ExpCoef[1][(s32Value>>4) & 0x0000000F] * s_af32ExpCoef[2][(s32Value>>8) & 0x0000000F] * s_af32ExpCoef[3][(s32Value>>12) & 0x0000000F] * s_af32ExpCoef[4][(s32Value>>16) & 0x0000000F ];
    }
}



static int SVP_NNIE_SoftMax( float* pf32Src, uint32_t u32Num)
{
    float f32Max = 0;
    float f32Sum = 0;
    uint32_t i = 0;

    for(i = 0; i < u32Num; ++i)
    {
        if(f32Max < pf32Src[i])
        {
            f32Max = pf32Src[i];
        }
    }

    for(i = 0; i < u32Num; ++i)
    {
        pf32Src[i] = (float)SVP_NNIE_QuickExp((int)((pf32Src[i] - f32Max))*4096);
        f32Sum += pf32Src[i];
    }

    for(i = 0; i < u32Num; ++i)
    {
        pf32Src[i] /= f32Sum;
    }
    return 0;
}


static int SVP_NNIE_SoftMaxV2( float &pf32Src1,float &pf32Src2)
{
    float f32Max = 0;
    float f32Sum = 0;
    uint32_t i = 0;

    if(f32Max <  pf32Src1) f32Max =  pf32Src1;
    if(f32Max <  pf32Src2) f32Max =  pf32Src2;
    pf32Src1 = (float)SVP_NNIE_QuickExp((int)((pf32Src1 - f32Max))*4096);
    pf32Src2 = (float)SVP_NNIE_QuickExp((int)((pf32Src2 - f32Max))*4096);
    f32Sum =pf32Src1+pf32Src2;
    pf32Src1 /= f32Sum;
    pf32Src2 /= f32Sum;
    return 0;
}


LaneDetect::LaneDetect() {
}

int LaneDetect::Init(int deviceTpye,int print_config,int modelType){

    m_print=print_config;
    m_modelType=modelType;

    in_h=288;
    in_w=512;
    dimType = MNN::Tensor::CAFFE;
    float mean[3]     = {123.675f, 116.28f, 103.53f};
    float normals[3] = {0.017125f, 0.01751f, 0.01743f};
    ::memcpy(imconfig.mean, mean, sizeof(mean));
    ::memcpy(imconfig.normal, normals, sizeof(normals));
    imconfig.sourceFormat = MNN::CV::BGR;
    imconfig.destFormat = MNN::CV::BGR;
    imconfig.filterType = MNN::CV::NEAREST;
    pretreat=std::shared_ptr<MNN::CV::ImageProcess>(MNN::CV::ImageProcess::create(imconfig));
    MNN_PRINT("ImageProcess build, sourceFormat: %d, destFormat: %d \n",imconfig.sourceFormat,imconfig.destFormat);

    string mnnPath;
    if(modelType==0){
        mnnPath="./models206/3.mnn";
        input_blob_names={ "input"};
        output_blob_names={ "output_loc","output_cls"};
    }
    else{
        mnnPath="./models206/Vega_96.mnn";
        input_blob_names={ "input"};
        output_blob_names={ "output_cls","output_loc"};
    }

    net = std::shared_ptr<MNN::Interpreter>(MNN::Interpreter::createFromFile(mnnPath.c_str()));
    MNN::ScheduleConfig config;
    config.type=(MNNForwardType)(deviceTpye);
    config.mode = MNN_GPU_TUNING_NORMAL | MNN_GPU_MEMORY_IMAGE;
    MNN::BackendConfig backendConfig;
    backendConfig.precision = MNN::BackendConfig::Precision_Normal;
    backendConfig.memory    = MNN::BackendConfig::Memory_Normal;
    backendConfig.power     = MNN::BackendConfig::Power_Normal;
    config.backendConfig = &backendConfig;
    session = net->createSession(config);
    MNN_PRINT("Interpreter build, model_path: %s, dimType:%d\n",mnnPath.c_str(),dimType);




   
    inputTensors.resize(input_blob_names.size());
    inputTensors_host.resize(input_blob_names.size());
    for (int i = 0; i < input_blob_names.size(); i++) {
		inputTensors[i] = net->getSessionInput(session,input_blob_names[0].c_str());
	}

    for (int i = 0; i < input_blob_names.size(); i++) {
		inputTensors_host[i] = new MNN::Tensor(inputTensors[i], dimType);
	}

    
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

    // if(m_print>=1){
    //     chrono::duration<double> elapsed2 = chrono::steady_clock::now() - start;
    //     cout << "LaneDetect time:" << elapsed2.count() << " s" << endl;
    //     outputTensors_host[0]->printShape();
    //     // for (int i = 0; i < 20; ++i) {
    //     //     MNN_PRINT("copy %f\n",  outputTensors_host[0]->host<float>()[i]);
    //     // }
    // }
    

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
    for (int i = 0; i < 20; ++i) {
        MNN_PRINT("func  %d %f, %f\n", i, outputTensors_host[0]->host<float>()[2*i+0], outputTensors_host[0]->host<float>()[2*i+1]);
    }
    

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
        if(m_modelType==0)
        {
            SVP_NNIE_SoftMaxV2(score0,score1);
        }
        
        if(score1>0.6)
        {
            memcpy(down_anchor_lane, &(outputTensors_host[1]->host<float>()[145*n+0]), points_per_line  * sizeof(float));
            memcpy(up_anchor_lane, &(outputTensors_host[1]->host<float>()[145*n+72]), (points_per_line +1) * sizeof(float));
            for (int k = 0;k < 72; ++k) {
                MNN_PRINT("func2  %d %d %f, %f ,%f\n",n, k,score1, down_anchor_lane[k], up_anchor_lane[k]);
            }
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

