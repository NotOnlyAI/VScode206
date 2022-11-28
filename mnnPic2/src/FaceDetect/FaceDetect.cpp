//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#define clip(x, y) (x < 0 ? 0 : (x > y ? y : x))

#include "FaceDetect.hpp"
#include <opencv2/opencv.hpp>
#include "nms.h"
using namespace std;


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



FaceDetect::FaceDetect() {

    string mnn_path="./models206/FaceDetect.mnn";
    net = std::shared_ptr<MNN::Interpreter>(MNN::Interpreter::createFromFile(mnn_path.c_str()));
    dimType = MNN::Tensor::TENSORFLOW;
    


    MNN::ScheduleConfig config;
    config.type  = MNN_FORWARD_CPU;
    // config.type=MNN_FORWARD_OPENCL;
    // config.type=MNN_FORWARD_CPU;
    // BackendConfig bnconfig;
    // bnconfig.precision = BackendConfig::Precision_Low;
    // config.backendConfig = &bnconfig; 
    session = net->createSession(config);


    input_blob_names={ "x"};
    inputTensors.resize(input_blob_names.size());
    inputTensors_host.resize(input_blob_names.size());
    for (int i = 0; i < input_blob_names.size(); i++) {
		inputTensors[i] = net->getSessionInput(session,input_blob_names[0].c_str());
	}

    for (int i = 0; i < input_blob_names.size(); i++) {
		inputTensors_host[i] = new MNN::Tensor(inputTensors[i], dimType);
	}

    output_blob_names={ "Identity","Identity_1","Identity_2","Identity_3","Identity_4","Identity_5","Identity_6","Identity_7",};
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


int FaceDetect::Forward(cv::Mat &raw_image) {


    auto start = chrono::steady_clock::now();


    if (raw_image.empty()) {
        std::cout << "image is empty ,please check!" << std::endl;
        return -1;
    }

    image_h = raw_image.rows;
    image_w = raw_image.cols;

    cv::Mat image;
    cv::resize(raw_image, image, cv::Size(in_w, in_h));



    MNN::CV::ImageProcess::Config config;
    float mean[3]     = {104.0f, 117.0f, 123.0f};
    float normals[3] = {1.0f, 1.0f, 1.0f};
    ::memcpy(config.mean, mean, sizeof(mean));
    ::memcpy(config.normal, normals, sizeof(normals));
    config.sourceFormat = MNN::CV::BGR;
    config.destFormat = MNN::CV::BGR;



    std::shared_ptr<MNN::CV::ImageProcess> pretreat(MNN::CV::ImageProcess::create(config));
    pretreat->convert(image.data, in_w, in_h,0,inputTensors_host[0]);
    inputTensors[0]->copyFromHostTensor(inputTensors_host[0]);


    net->runSession(session);

    for (int i = 0; i < output_blob_names.size(); i++) {
		outputTensors[i]->copyToHostTensor(outputTensors_host[i]);
	}


    
    // outputTensors_host[0]->printShape();
    // for (int i = 0; i < 20; ++i) {
    //     MNN_PRINT("copy %f, %f\n", outputTensors[0]->host<float>()[i], outputTensors_host[0]->host<float>()[i]);
    // }
    
    decode(outputTensors_host);


    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "inference time:" << elapsed.count() << " s" << endl;

    return 0;
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


int FaceDetect::decode(std::vector< MNN::Tensor*> &outputTensors_host)
{
    // for (int i = 0; i < 10; ++i) {
    //     MNN_PRINT("func %f, %f\n", outputTensors_host[0]->host<float>()[2*i+0], outputTensors_host[0]->host<float>()[2*i+0]);
    // }


    float box[3000][5];
    float landmark[3000][10];

    std::vector<int> indexes = {};
    std::vector<float*> vec_boxs = {};
    std::vector<float*> vec_landmarks = {};
    std::vector<int> nms_indexes = {};
    std::vector<float*> nms_boxs={};


    for (int i = 0; i < output_blob_names.size(); i++) {
        // MNN_PRINT("%s\n",output_blob_names[i]);
		outputTensors_host[i]->printShape();
	}

    int start=0;
    int end=4;
    int grid_hs[4];
    int grid_ws[4];
    int BboxNumEachGrid[4]={3,2,2,3};
    float BoxSteps[4]={8,16,32,64};
    float BoxMinSizes[4][3] = {{10, 16, 24}, {32, 48,0}, {64, 96,0}, {128, 192, 256}};
    grid_hs[0]=40;grid_hs[1]=20;grid_hs[2]=10;grid_hs[3]=5;
    grid_ws[0]=30;grid_ws[1]=15;grid_ws[2]=8;grid_ws[3]=4;



    float f32Score0;
    float f32Score1;
    float f32Score;
    float f32X;
    float f32Y;
    float f32Xmin;
    float f32Ymin;
    float f32Xmax;
    float f32Ymax;
    float f32Width;
    float f32Height;
    int N=0;
    int BboxNum=0;
    for(int i = start; i < end; i++)
    {
        uint32_t u32Offset=0;
        uint32_t u32OffsetScore=0;


        int  grid_h = grid_hs[i];
        int  grid_w = grid_ws[i];
        int  grid_c_bbox =4;

        for(int j = 0; j < grid_h*grid_w; j++)
        {


            for(int k = 0; k < BboxNumEachGrid[i]; k++)
            {


               
                u32OffsetScore = (j * BboxNumEachGrid[i] + k) * 2;
                SVP_NNIE_SoftMax(&(outputTensors_host[i]->host<float>()[u32OffsetScore]), 2);
                f32Score0=outputTensors_host[i]->host<float>()[u32OffsetScore];
                f32Score1=outputTensors_host[i]->host<float>()[u32OffsetScore+1];

                
                // u32Offset = (j * BboxNumEachGrid[i] + k) * 4;
                // f32X = outputTensors_host[i+4]->host<float>()[u32Offset + 0];
                // f32Y = outputTensors_host[i+4]->host<float>()[u32Offset + 1];
                // f32Width = outputTensors_host[i+4]->host<float>()[u32Offset + 2];
                // f32Height = outputTensors_host[i+4]->host<float>()[u32Offset + 3];

                // // cout<<N<<":"<<f32Score0<<","<<f32Score1<<endl;

                // cout<<N<<":"<<f32X<<","<<f32Y <<","<<f32Width<<","<<f32Height<<","<<f32Score1<<endl;

                if(f32Score1>0.5)
                {
                    uint32_t x = j % grid_w;
                    uint32_t y = j / grid_w;
                    float dense_cx = (x+0.5) * BoxSteps[i] / in_w;
                    float dense_cy = (y+0.5) * BoxSteps[i] / in_h;
                    float s_kx = BoxMinSizes[i][k] / in_w;
                    float s_ky = BoxMinSizes[i][k] / in_h;

                    float max_val=0.999;
                    float min_val=0.001;


                    u32Offset = (j * BboxNumEachGrid[i] + k) * 4;

                    f32X = outputTensors_host[i+4]->host<float>()[u32Offset + 0]*0.1*s_kx+dense_cx;
                    f32Y = outputTensors_host[i+4]->host<float>()[u32Offset + 1]*0.1*s_ky+dense_cy;
                    f32Width = exp(outputTensors_host[i+4]->host<float>()[u32Offset + 2]*0.2)*s_kx;
                    f32Height = exp(outputTensors_host[i+4]->host<float>()[u32Offset + 3]*0.2)*s_ky;

                    // cout<<u32Offset<<" score:"<<f32Score1<<","<<f32X<<","<<f32Y<<","<<f32Width<<","<<f32Height<<","<<endl;

                    f32Xmin = min(float(f32X-0.5*f32Width),max_val);
                    f32Ymin = min(float(f32Y-0.5*f32Height),max_val);
                    f32Xmax  =min(float(f32X+0.5*f32Width),max_val);
                    f32Ymax = min(float(f32Y+0.5*f32Height),max_val);
                    f32Xmin = max(f32Xmin,min_val);
                    f32Ymin = max(f32Ymin,min_val);
                    f32Xmax = max(f32Xmax,min_val);
                    f32Ymax = max(f32Ymax,min_val);
                    box[N][0] = f32Xmin;
                    box[N][1] = f32Ymin;
                    box[N][2] = max(f32Xmax-f32Xmin,min_val);
                    box[N][3] = max(f32Ymax-f32Ymin,min_val);
                    box[N][4] = f32Score1;
                    vec_boxs.push_back(box[N]);

                    // cout<<u32OffsetScore<<" score:"<<f32Score1<<","<<f32Xmin<<","<<f32Ymin<<","<<f32Xmax<<","<<f32Ymax<<","<<endl;
                    BboxNum++;
                }
                N++;
            }
        }
    }
    cout<<"BboxNum:"<<BboxNum<<endl;
    nms_boxs = nms(vec_boxs, 0.5,0.5,nms_indexes);
    cout<<"nms_boxs:"<<nms_boxs.size()<<endl;
    
    rectinfo.nFaceNum=0;
    // landmarkinfo.nFaceNum=0;

    for (int j = 0; j < int(nms_boxs.size()); j++)
    {
        RectDetect rect;
        LabelDetect label_;
        rect.x=nms_boxs.at(j)[0]*image_w;
        rect.y=nms_boxs.at(j)[1]*image_h;
        rect.width=nms_boxs.at(j)[2]*image_w;
        rect.height=nms_boxs.at(j)[3]*image_h;
        label_.label=1;
        label_.score=nms_boxs.at(j)[4];

        rectinfo.rects[j] = rect;
        rectinfo.labels[j] = label_;

        


        // STRU_Landmark_T landmark_t;
        // landmark_t.point[0].x=nms_landmarks.at(j)[0]*float(origin_image_width);
        // landmark_t.point[0].y=nms_landmarks.at(j)[1]*float(origin_image_height);
        // landmark_t.point[1].x=nms_landmarks.at(j)[2]*origin_image_width;
        // landmark_t.point[1].y=nms_landmarks.at(j)[3]*origin_image_height;
        // landmark_t.point[2].x=nms_landmarks.at(j)[4]*origin_image_width;
        // landmark_t.point[2].y=nms_landmarks.at(j)[5]*origin_image_height;
        // landmark_t.point[3].x=nms_landmarks.at(j)[6]*origin_image_width;
        // landmark_t.point[3].y=nms_landmarks.at(j)[7]*origin_image_height;
        // landmark_t.point[4].x=nms_landmarks.at(j)[8]*origin_image_width;
        // landmark_t.point[4].y=nms_landmarks.at(j)[9]*origin_image_height;
        // landmark_t.score=1;
        // landmarkinfo.landmark[j]=landmark_t;
//        cout<<j<<":"<<landmarkinfo.landmark[j].point[0].x<<endl;

    }

    rectinfo.nFaceNum=rectinfo.nFaceNum+int(nms_boxs.size());
    // landmarkinfo.nFaceNum=landmarkinfo.nFaceNum+int(nms_boxs.size());
    cout<<"rectinfo.nFaceNum:"<<rectinfo.nFaceNum<<endl;
    return 1;

}



FaceDetect::~FaceDetect() {
    net->releaseModel();
    net->releaseSession(session);
    for (int i = 0; i < input_blob_names.size(); i++) {
		delete inputTensors_host[i];
	}
    for (int i = 0; i < output_blob_names.size(); i++) {
		delete outputTensors_host[i];
	}
}

