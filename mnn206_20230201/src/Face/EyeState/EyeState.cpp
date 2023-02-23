//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#define clip(x, y) (x < 0 ? 0 : (x > y ? y : x))

#include "EyeState.h"
#include "M2utils/image_utils_new.h"
#include <opencv2/opencv.hpp>
using namespace std;
using namespace M2;





EyeState::EyeState() {
}


int EyeState::Init(int deviceTpye,int print_config,int modelType){

    m_print=print_config;
    m_modelType=modelType;


    string mnnPath;

    if (modelType==0){ 
        mnnPath="./models206/eyestate.mnn";;
        dimType = MNN::Tensor::CAFFE;
        in_h=64;
        in_w=64;
        input_blob_names={"input0"};
        output_blob_names={"softmax"};
        float mean[3]     = {0.0f, 0.0f, 0.0f};
        float normals[3] = {0.0039215686274509803921568627451f, 0.0039215686274509803921568627451f, 0.0039215686274509803921568627451f};
        ::memcpy(imconfig.mean, mean, sizeof(mean));
        ::memcpy(imconfig.normal, normals, sizeof(normals));
        imconfig.sourceFormat = MNN::CV::BGR;
        imconfig.destFormat = MNN::CV::BGR;
        imconfig.filterType = MNN::CV::NEAREST;
        pretreat=std::shared_ptr<MNN::CV::ImageProcess>(MNN::CV::ImageProcess::create(imconfig));
    }
    else{
        MNN_PRINT(" wrong modelType %d,should be 0 or 1  \n",modelType);
        MNN_PRINT("\n");
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

    auto shape = inputTensors[0]->shape();
    shape[0]=1;
    net->resizeTensor(inputTensors[0] , shape);
    net->resizeSession(session);

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
        MNN_PRINT("%s\n",input_blob_names[i].c_str());
		inputTensors_host[i]->printShape();
	}


    for (int i = 0; i < output_blob_names.size(); i++) {
        MNN_PRINT("%s\n",output_blob_names[i].c_str());
		outputTensors_host[i]->printShape();
	} 

    return 0;


}


int EyeState::ForwardBGR(const cv::Mat &image,const cv::Rect &left_rect,const cv::Rect &right_rect,int &eyestate) {


    auto start = chrono::steady_clock::now();
    image_h = image.rows;
    image_w = image.cols;



    cv::Mat crop_left_image=image(left_rect);
    cv::Mat resize_left_image;
    cv::resize(crop_left_image,resize_left_image,cv::Size(64,64));
    // cv::Mat gray_left_image;
    // cv::cvtColor(resize_left_image,gray_left_image,cv::COLOR_BGR2GRAY);
    // cv::imshow("left",gray_left_image);
    // cv::waitKey(0);

    cv::Mat crop_right_image=image(right_rect);
    cv::Mat resize_right_image;
    cv::resize(crop_right_image,resize_right_image,cv::Size(64,64));
    // cv::Mat gray_right_image;
    
    // cv::imshow("right",gray_right_image);
    // cv::waitKey(0);
    // cv::imwrite("crop1.jpg",crop_image);
    // // exit(0);
    // cv::waitKey(0);

    // cv::Mat resize_img;
    // cv::resize(crop_image,resize_img,cv::Size(in_w,in_h));
    // cv::imshow("resize",resize_img);
    // cv::waitKey(0);


    pretreat->convert((uint8_t *)resize_left_image.data, in_w,in_h,0,inputTensors_host[0]);
    inputTensors[0]->copyFromHostTensor(inputTensors_host[0]);
    net->runSession(session);
    for (int i = 0; i < output_blob_names.size(); i++) {
		outputTensors[i]->copyToHostTensor(outputTensors_host[i]);
	}
    float eye_prob1=outputTensors_host[0]->host<float>()[0];

    pretreat->convert((uint8_t *)resize_right_image.data, in_w,in_h,0,inputTensors_host[0]);
    inputTensors[0]->copyFromHostTensor(inputTensors_host[0]);
    net->runSession(session);
    for (int i = 0; i < output_blob_names.size(); i++) {
		outputTensors[i]->copyToHostTensor(outputTensors_host[i]);
	}
    float eye_prob2=outputTensors_host[0]->host<float>()[0];

    

    eyestate=0;
    if(eye_prob1>0.5 && eye_prob2>0.5) eyestate=1;

    if(m_print>=1){
        chrono::duration<double> elapsed2 = chrono::steady_clock::now() - start;
        cout << "net time:" << elapsed2.count() << " s" << endl;
        // outputTensors_host[0]->printShape();
        cout<<"eye_prob1:"<<eye_prob1<<"  eye_prob2:"<<eye_prob2<<"   eyestate:"<<eyestate<<endl;
    }


    // decode(outputTensors_host);


    // landmarkinfo.numPoints= m_landmarkInfo.numPoints;
    // cout<<m_landmarkInfo.numPoints<<endl;
    // cout<<landmarkinfo.numPoints<<endl;
    // for (int i = 0; i < landmarkinfo.numPoints;i++) {
    //     landmarkinfo.landmark[i].x= m_landmarkInfo.landmark[i].x+rect_new.x;
    //     landmarkinfo.landmark[i].y= m_landmarkInfo.landmark[i].y+rect_new.y;
    //     // MNN_PRINT("i=%d  %f, %f\n", i,landmarkinfo.landmark[i].x, landmarkinfo.landmark[i].y);
    // }


    if(m_print>=1){
        auto end = chrono::steady_clock::now();
        chrono::duration<double> elapsed = end - start;
        cout << "EyeState inference time:" << elapsed.count() << " s" << endl;
    }
    return 0;
}




int EyeState::decode(std::vector< MNN::Tensor*> &outputTensors_host)
{

    for (int i = 0; i < 5; ++i) {
        m_landmarkInfo.landmark[i].x= outputTensors_host[1]->host<float>()[3*i+0];
        m_landmarkInfo.landmark[i].y= outputTensors_host[1]->host<float>()[3*i+1];
        MNN_PRINT("func %f, %f\n", m_landmarkInfo.landmark[i].x, m_landmarkInfo.landmark[i].y);
    }
    m_landmarkInfo.numPoints=5;

    return 0;

}



EyeState::~EyeState() {
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

    }
}

