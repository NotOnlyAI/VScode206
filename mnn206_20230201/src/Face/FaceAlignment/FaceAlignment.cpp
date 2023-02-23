//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#define clip(x, y) (x < 0 ? 0 : (x > y ? y : x))

#include "FaceAlignment.hpp"
#include "M2utils/image_utils_new.h"
#include <opencv2/opencv.hpp>
using namespace std;
using namespace M2;





FaceAlignment::FaceAlignment() {
}


int FaceAlignment::init(int deviceTpye,int print_config,int modelType,float ratio_eye,float ratio_mouth){

    m_print=print_config;
    m_modelType=modelType;
    string mnnPath;

    m_ratio_eye=ratio_eye;
    m_ratio_mouth=ratio_mouth;

    if (modelType==0){ 
        mnnPath="./models206/face_landmark.mnn";;
        dimType = MNN::Tensor::CAFFE;
        in_h=192;
        in_w=192;
        input_blob_names={"input_1"};
        output_blob_names={"conv2d_20","conv2d_30"};
        float mean[3]     = {0.0f, 0.0f, 0.0f};
        float normals[3] = {0.0039215686274509803921568627451f, 0.0039215686274509803921568627451f, 0.0039215686274509803921568627451f};
        ::memcpy(imconfig.mean, mean, sizeof(mean));
        ::memcpy(imconfig.normal, normals, sizeof(normals));
        imconfig.sourceFormat = MNN::CV::BGR;
        imconfig.destFormat = MNN::CV::RGB;
        imconfig.filterType = MNN::CV::NEAREST;
        pretreat=std::shared_ptr<MNN::CV::ImageProcess>(MNN::CV::ImageProcess::create(imconfig));
    }
    else if (modelType==1)
    {
        mnnPath="./models206/FaceAlignment_pfld.mnn";;
        dimType = MNN::Tensor::CAFFE;
        in_h=96;
        in_w=96;
        input_blob_names={"data"};
        output_blob_names={"conv5_fwd"};
        float mean[3]     = {123.0f,   123.0f,   123.0f};
        float normals[3] = {0.01724f, 0.01724f, 0.01724f};
        ::memcpy(imconfig.mean, mean, sizeof(mean));
        ::memcpy(imconfig.normal, normals, sizeof(normals));
        imconfig.sourceFormat = MNN::CV::BGR;
        imconfig.destFormat = MNN::CV::RGB;
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


int FaceAlignment::ForwardBGR(const cv::Mat &image,const M2::Object &face,M2::LandmarkInfo &landmarkinfo) {


    auto start = chrono::steady_clock::now();


    double newx1=max(face.rect.x-0.125*face.rect.width,1.0);
    double newy1=max(face.rect.y-0.125*face.rect.height,1.0);
    double newx2 = min(face.rect.x + 1.125 * face.rect.width, image.cols - 1.0);
    double newy2 = min(face.rect.y + 1.125 * face.rect.height, image.rows - 1.0);
    double dw=min(newx2-(face.rect.x+face.rect.width),face.rect.x-newx1);
    double dh=min(newy2-(face.rect.y+face.rect.height),face.rect.y-newy1);
    double dd=min(dw,dh);


    cv::Rect rect_new;
    rect_new.x=int(face.rect.x-dd);
    rect_new.y=int(face.rect.y-dd);
    rect_new.width=int(face.rect.width+2*dd);
    rect_new.height=int(face.rect.height+2*dd);

    // cv::Rect rect_new;
    // rect_new.x=int(face.rect.x);
    // rect_new.y=int(face.rect.y);
    // rect_new.width=int(face.rect.width);
    // rect_new.height=int(face.rect.height);

    image_h = rect_new.height;
    image_w = rect_new.width;



    cv::Mat crop_image=image(rect_new);
    // cv::imwrite("crop1.jpg",crop_image);
    // exit(0);
    // cv::waitKey(0);

    cv::Mat resize_img;
    cv::resize(crop_image,resize_img,cv::Size(in_w,in_h));
    // cv::imshow("resize",resize_img);
    // cv::waitKey(0);


    pretreat->convert((uint8_t *)resize_img.data, in_w,in_h,0,inputTensors_host[0]);
    inputTensors[0]->copyFromHostTensor(inputTensors_host[0]);
    net->runSession(session);
    for (int i = 0; i < output_blob_names.size(); i++) {
		outputTensors[i]->copyToHostTensor(outputTensors_host[i]);
	}
    // if(m_print>=1){
    //     chrono::duration<double> elapsed2 = chrono::steady_clock::now() - start;
    //     cout << "net time:" << elapsed2.count() << " s" << endl;
    //     outputTensors_host[0]->printShape();
    //     for (int i = 0; i < 20; ++i) {
    //         MNN_PRINT("copy %f\n", outputTensors_host[0]->host<float>()[i]);
    //     }
    // }


    decode(outputTensors_host);


    landmarkinfo.numPoints= m_landmarkInfo.numPoints;
    for (int i = 0; i < landmarkinfo.numPoints;i++) {
        landmarkinfo.landmark[i].x= m_landmarkInfo.landmark[i].x+rect_new.x;
        landmarkinfo.landmark[i].y= m_landmarkInfo.landmark[i].y+rect_new.y;
        // MNN_PRINT("i=%d  %f, %f\n", i,landmarkinfo.landmark[i].x, landmarkinfo.landmark[i].y);
    }
    if(m_modelType==0)
    {
        m_left_eye.x=( landmarkinfo.landmark[33].x+landmarkinfo.landmark[133].x)/2.0;
        m_left_eye.y=(landmarkinfo.landmark[33].y+landmarkinfo.landmark[133].y)/2.0;
        m_right_eye.x=(landmarkinfo.landmark[362].x+landmarkinfo.landmark[263].x)/2.0;
        m_right_eye.y=(landmarkinfo.landmark[362].y+landmarkinfo.landmark[263].y)/2.0;

        float dx1=landmarkinfo.landmark[33].x-landmarkinfo.landmark[133].x;
        float dy1=landmarkinfo.landmark[33].y-landmarkinfo.landmark[133].y;
        double leye_left=sqrt(dx1*dx1+dy1*dy1); 
        float dx3=landmarkinfo.landmark[362].x-landmarkinfo.landmark[263].x;
        float dy3=landmarkinfo.landmark[362].y-landmarkinfo.landmark[263].y;
        double leye_right=sqrt(dx3*dx3+dy3*dy3);

        double newx1=max(m_left_eye.x-leye_left,1.0);
        double newx2 = min(m_left_eye.x +leye_left, image.cols- 1.0);
        if (newx1<=1.0) newx2=newx1+64;
        if (newx2>=image.cols - 1.0) newx1=newx2-64;

        double newy1=max(m_left_eye.y-leye_left,1.0);
        double newy2 = min(m_left_eye.y + leye_left, image.rows - 1.0);
        if (newy1<=1.0) newy2=newy1+2*leye_left;
        if (newy2>=image.rows - 1.0) newy1=newy2-2*leye_left;


        double rightx1=max(m_right_eye.x-leye_right,1.0);
        double rightx2 = min(m_right_eye.x +leye_right, image.cols- 1.0);
        if (rightx1<=1.0) rightx2=rightx1+2*leye_right;
        if (rightx2>=image.cols - 1.0) rightx1=rightx2-2*leye_right;

        double righty1=max(m_right_eye.y-leye_right,1.0);
        double righty2 = min(m_right_eye.y + leye_right, image.rows - 1.0);
        if (righty1<=1.0) righty2=righty1+2*leye_right;
        if (righty2>=image.rows - 1.0) righty1=righty2-2*leye_right;

        m_left_rect.x=int(newx1);
        m_left_rect.y=int(newy1);
        m_left_rect.width=2*leye_left;
        m_left_rect.height=2*leye_left;

        m_right_rect.x=int(rightx1);
        m_right_rect.y=int(righty1);
        m_right_rect.width=2*leye_right;
        m_right_rect.height=2*leye_right;



    }
    else if(m_modelType==1)
    {
        m_left_eye.x=( landmarkinfo.landmark[60].x+landmarkinfo.landmark[64].x)/2.0;
        m_left_eye.y=(landmarkinfo.landmark[60].y+landmarkinfo.landmark[64].y)/2.0;
        m_right_eye.x=( landmarkinfo.landmark[68].x+landmarkinfo.landmark[72].x)/2.0;
        m_right_eye.y=(landmarkinfo.landmark[68].y+landmarkinfo.landmark[72].y)/2.0;


        float dx1=landmarkinfo.landmark[60].x-landmarkinfo.landmark[64].x;
        float dy1=landmarkinfo.landmark[60].y-landmarkinfo.landmark[64].y;
        double leye_left=sqrt(dx1*dx1+dy1*dy1); 
        float dx3=landmarkinfo.landmark[68].x-landmarkinfo.landmark[72].x;
        float dy3=landmarkinfo.landmark[68].y-landmarkinfo.landmark[72].y;
        double leye_right=sqrt(dx3*dx3+dy3*dy3);

        double newx1=max(m_left_eye.x-0.8*leye_left,1.0);
        double newx2 = min(m_left_eye.x +0.8*leye_left, image.cols- 1.0);
        if (newx1<=1.0) newx2=newx1+1.6*leye_left;
        if (newx2>=image.cols - 1.0) newx1=newx2-1.6*leye_left;

        double newy1=max(m_left_eye.y-0.8*leye_left,1.0);
        double newy2 = min(m_left_eye.y + 0.8*leye_left, image.rows - 1.0);
        if (newy1<=1.0) newy2=newy1+1.6*leye_left;
        if (newy2>=image.rows - 1.0) newy1=newy2-1.6*leye_left;


        double rightx1=max(m_right_eye.x-0.8*leye_right,1.0);
        double rightx2 = min(m_right_eye.x +0.8*leye_right, image.cols- 1.0);
        if (rightx1<=1.0) rightx2=rightx1+1.6*leye_right;
        if (rightx2>=image.cols - 1.0) rightx1=rightx2-1.6*leye_right;

        double righty1=max(m_right_eye.y-0.8*leye_right,1.0);
        double righty2 = min(m_right_eye.y + 0.8*leye_right, image.rows - 1.0);
        if (righty1<=1.0) righty2=righty1+1.6*leye_right;
        if (righty2>=image.rows - 1.0) righty1=righty2-1.6*leye_right;

        m_left_rect.x=int(newx1);
        m_left_rect.y=int(newy1);
        m_left_rect.width=1.6*leye_left;
        m_left_rect.height=1.6*leye_left;

        m_right_rect.x=int(rightx1);
        m_right_rect.y=int(righty1);
        m_right_rect.width=1.6*leye_right;
        m_right_rect.height=1.6*leye_right;


    }
    




    if(m_print>=1){
        auto end = chrono::steady_clock::now();
        chrono::duration<double> elapsed = end - start;
        cout << "FaceAlignment inference time:" << elapsed.count() << " s" << endl;
    }
    return 0;
}


int FaceAlignment::DMSJudge(const M2::LandmarkInfo &landmarkinfo,int &DMSTpye)
{
    if(m_modelType==0)
    {
        
        DMSTpye=0;
        m_DMSTpye=0;
        float dx=landmarkinfo.landmark[78].x-landmarkinfo.landmark[308].x;
        float dy=landmarkinfo.landmark[78].y-landmarkinfo.landmark[308].y;
        float lmouth=sqrt(dx*dx+dy*dy);


        float dxx=landmarkinfo.landmark[13].x-landmarkinfo.landmark[14].x;
        float dyy=landmarkinfo.landmark[13].y-landmarkinfo.landmark[14].y;
        float wmouth=sqrt(dxx*dxx+dyy*dyy);

        float ratio_mouth=wmouth/lmouth;

        if(ratio_mouth>m_ratio_mouth)
        {
            if(m_print>=2)
            {
                cout<<ratio_mouth<<";"<<m_ratio_mouth<<endl;
            }

            DMSTpye=1;
            m_DMSTpye=1;
        }
        return 0;
    }

    



    if(m_modelType==1)
    {
        

        DMSTpye=0;
        m_DMSTpye=0;
        float dx=landmarkinfo.landmark[76].x-landmarkinfo.landmark[82].x;
        float dy=landmarkinfo.landmark[76].y-landmarkinfo.landmark[82].y;
        float lmouth=sqrt(dx*dx+dy*dy);


        float dxx=landmarkinfo.landmark[78].x-landmarkinfo.landmark[85].x;
        float dyy=landmarkinfo.landmark[78].y-landmarkinfo.landmark[85].y;
        float wmouth=sqrt(dxx*dxx+dyy*dyy);

        float ratio_mouth=wmouth/lmouth;

        if(ratio_mouth>m_ratio_mouth)
        {
            if(m_print>=2)
            {
                cout<<ratio_mouth<<";"<<m_ratio_mouth<<endl;
            }

            DMSTpye=1;
            m_DMSTpye=1;
        }
        return 0;

    }


}

int FaceAlignment::decode(std::vector< MNN::Tensor*> &outputTensors_host)
{
    // for (int i = 0; i < 10; ++i) {
    //     MNN_PRINT("func %f, %f\n", outputTensors_host[0]->host<float>()[2*i+0], outputTensors_host[0]->host<float>()[2*i+0]);
    // }


    if(m_modelType==0)
    {

        for (int i = 0; i < 468; ++i) {
            m_landmarkInfo.landmark[i].x= outputTensors_host[0]->host<float>()[3*i+0]/float(in_w)*float(image_w);
            m_landmarkInfo.landmark[i].y= outputTensors_host[0]->host<float>()[3*i+1]/float(in_h)*float(image_h);
            // MNN_PRINT("func %f, %f\n", m_landmarkInfo.landmark[i].x, m_landmarkInfo.landmark[i].y);
        }
        m_landmarkInfo.numPoints=468;


        
        return 0;
    }

    // if(m_modelType==1)
    // {
    //     for (int i = 0; i < 106; ++i) {
    //         m_landmarkInfo.landmark[i].x= (outputTensors_host[0]->host<float>()[2*i+0]+1)*0.5*image_w;
    //         m_landmarkInfo.landmark[i].y= (outputTensors_host[0]->host<float>()[2*i+1]+1)*0.5*image_h;
    //         MNN_PRINT("func %f, %f\n", m_landmarkInfo.landmark[i].x, m_landmarkInfo.landmark[i].y);
    //     }
    //     m_landmarkInfo.numPoints=106;


    //     // left_eye_points.clear();
    //     // right_eye_points.clear();

    //     // for (int i = 0; i < 468; ++i) {
    //     //     if(i==33||i==159||i==133||i==145)
    //     //     {
    //     //         M2::Point2f p;
    //     //         p.x=outputTensors_host[0]->host<float>()[3*i];
    //     //         p.y=outputTensors_host[0]->host<float>()[3*i+1];
    //     //         p.z=outputTensors_host[0]->host<float>()[3*i+2];
    //     //         left_eye_points.push_back(p);
    //     //     }
    //     //     if(i==362||i==386||i==263||i=374)
    //     //     {
    //     //         M2::Point2f p;
    //     //         p.x=outputTensors_host[0]->host<float>()[3*i];
    //     //         p.y=outputTensors_host[0]->host<float>()[3*i+1];
    //     //         p.z=outputTensors_host[0]->host<float>()[3*i+2];
    //     //         right_eye_points.push_back(p);
    //     //     }
    //     // }
    //     return 0;
    // }



    // if (m_modelType==1)
    // {
    //     for (int i = 0; i < 68; ++i) {
    //         m_landmarkInfo.landmark[i].x= (outputTensors_host[0]->host<float>()[2*i+0]+1)*0.5*image_w;
    //         m_landmarkInfo.landmark[i].y= (outputTensors_host[0]->host<float>()[2*i+1]+1)*0.5*image_h;
    //         //     MNN_PRINT("func %f, %f\n", outputTensors_host[0]->host<float>()[2*i+0], outputTensors_host[0]->host<float>()[2*i+0]);
    //     }
    //     m_landmarkInfo.numPoints=68;
    //     return 0;
    // }

    if (m_modelType==1)
    {
        float scale_x = static_cast<float>(image_w) / in_w;
        float scale_y = static_cast<float>(image_h) / in_h;
        for (int i = 0; i < 98; ++i) {
            m_landmarkInfo.landmark[i].x= (outputTensors_host[0]->host<float>()[2*i+0])*scale_x;
            m_landmarkInfo.landmark[i].y= (outputTensors_host[0]->host<float>()[2*i+1])*scale_y;
                // MNN_PRINT("func %f, %f\n", outputTensors_host[0]->host<float>()[2*i+0], outputTensors_host[0]->host<float>()[2*i+0]);
        }
        m_landmarkInfo.numPoints=98;
    }



    return 0;

}



FaceAlignment::~FaceAlignment() {
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

