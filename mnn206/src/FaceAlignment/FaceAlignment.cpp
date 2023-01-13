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
    m_ratio_eye=ratio_eye;
    m_ratio_mouth=ratio_mouth;

    string mnnPath;
    mnnPath="./models206/FaceAlignment.mnn" ;
    dimType = MNN::Tensor::TENSORFLOW;
    in_h=256;
    in_w=256;
    input_blob_names={"input_1"};
    output_blob_names={"Identity"};
    float mean[3]     = {127.5f, 127.5f, 127.5f};
    float normals[3] = {0.007843f, 0.007843f, 0.007843f};
    ::memcpy(imconfig.mean, mean, sizeof(mean));
    ::memcpy(imconfig.normal, normals, sizeof(normals));
    imconfig.sourceFormat = MNN::CV::BGR;
    imconfig.destFormat = MNN::CV::RGB;
    imconfig.filterType = MNN::CV::NEAREST;
    pretreat=std::shared_ptr<MNN::CV::ImageProcess>(MNN::CV::ImageProcess::create(imconfig));

    if (modelType==0){ 
        mnnPath="./models206/FaceAlignment.mnn";;
        dimType = MNN::Tensor::TENSORFLOW;
        in_h=256;
        in_w=256;
        input_blob_names={"input_1"};
        output_blob_names={"Identity"};
        float mean[3]     = {127.5f, 127.5f, 127.5f};
        float normals[3] = {0.007843f, 0.007843f, 0.007843f};
        ::memcpy(imconfig.mean, mean, sizeof(mean));
        ::memcpy(imconfig.normal, normals, sizeof(normals));
        imconfig.sourceFormat = MNN::CV::BGR;
        imconfig.destFormat = MNN::CV::RGB;
        imconfig.filterType = MNN::CV::NEAREST;
        pretreat=std::shared_ptr<MNN::CV::ImageProcess>(MNN::CV::ImageProcess::create(imconfig));
    }

    if (modelType==1){ 
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

    image_h = rect_new.height;
    image_w = rect_new.width;


    cv::Mat crop_image=image(rect_new);
    // cv::imshow("crop",crop_image);
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


    landmarkinfo.numPoints= landmark68.numPoints;
    for (int i = 0; i < landmarkinfo.numPoints; ++i) {
        landmarkinfo.landmark[i].x= landmark68.landmark[i].x+rect_new.x;
        landmarkinfo.landmark[i].y= landmark68.landmark[i].y+rect_new.y;
        // MNN_PRINT("i=%d  %f, %f\n", i,landmarkinfo.landmark[i].x, landmarkinfo.landmark[i].y);
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
    if(landmarkinfo.numPoints==98)
    {
        float dx1=landmarkinfo.landmark[60].x-landmarkinfo.landmark[64].x;
        float dy1=landmarkinfo.landmark[60].y-landmarkinfo.landmark[64].y;
        float leye_left=sqrt(dx1*dx1+dy1*dy1);


        float dx2=landmarkinfo.landmark[62].x-landmarkinfo.landmark[66].x;
        float dy2=landmarkinfo.landmark[62].y-landmarkinfo.landmark[66].y;
        float weye_left=sqrt(dx2*dx2+dy2*dy2);

        float dx3=landmarkinfo.landmark[68].x-landmarkinfo.landmark[72].x;
        float dy3=landmarkinfo.landmark[68].y-landmarkinfo.landmark[72].y;
        float leye_right=sqrt(dx3*dx3+dy3*dy3);


        float dx4=landmarkinfo.landmark[70].x-landmarkinfo.landmark[74].x;
        float dy4=landmarkinfo.landmark[70].y-landmarkinfo.landmark[74].y;
        float weye_right=sqrt(dx4*dx4+dy4*dy4);

        float r_left=weye_left/leye_left;
        float r_right=weye_right/leye_right;

        float ratio_eye=0.5*(r_left+r_right);


        float dx=landmarkinfo.landmark[76].x-landmarkinfo.landmark[82].x;
        float dy=landmarkinfo.landmark[76].y-landmarkinfo.landmark[82].y;
        float lmouth=sqrt(dx1*dx1+dy1*dy1);


        float dxx=landmarkinfo.landmark[78].x-landmarkinfo.landmark[85].x;
        float dyy=landmarkinfo.landmark[78].y-landmarkinfo.landmark[85].y;
        float wmouth=sqrt(dxx*dxx+dyy*dyy);

        float ratio_mouth=wmouth/lmouth;

        if(ratio_eye<m_ratio_eye||ratio_mouth>m_ratio_mouth)
        {
            cout<<ratio_eye<<";"<<ratio_mouth<<endl;
            cout<<m_ratio_eye<<";"<<m_ratio_mouth<<endl;
            DMSTpye=2;
            m_DMSTpye=2;
        }
        else
        {
            DMSTpye=1;
            m_DMSTpye=1;
        }
        return 0;

    }

    if(landmarkinfo.numPoints==68)
    {
        float dx1=landmarkinfo.landmark[37].x-landmarkinfo.landmark[40].x;
        float dy1=landmarkinfo.landmark[37].y-landmarkinfo.landmark[40].y;
        float leye_left=sqrt(dx1*dx1+dy1*dy1);


        float dx2=landmarkinfo.landmark[38].x-landmarkinfo.landmark[42].x;
        float dy2=landmarkinfo.landmark[38].y-landmarkinfo.landmark[42].y;
        float weye_left=sqrt(dx2*dx2+dy2*dy2);

        float dx3=landmarkinfo.landmark[43].x-landmarkinfo.landmark[46].x;
        float dy3=landmarkinfo.landmark[43].y-landmarkinfo.landmark[46].y;
        float leye_right=sqrt(dx3*dx3+dy3*dy3);


        float dx4=landmarkinfo.landmark[44].x-landmarkinfo.landmark[48].x;
        float dy4=landmarkinfo.landmark[44].y-landmarkinfo.landmark[48].y;
        float weye_right=sqrt(dx4*dx4+dy4*dy4);

        float r_left=weye_left/leye_left;
        float r_right=weye_right/leye_right;

        float ratio_eye=0.5*(r_left+r_right);


        float dx=landmarkinfo.landmark[49].x-landmarkinfo.landmark[55].x;
        float dy=landmarkinfo.landmark[49].y-landmarkinfo.landmark[55].y;
        float lmouth=sqrt(dx1*dx1+dy1*dy1);


        float dxx=landmarkinfo.landmark[52].x-landmarkinfo.landmark[58].x;
        float dyy=landmarkinfo.landmark[52].y-landmarkinfo.landmark[58].y;
        float wmouth=sqrt(dxx*dxx+dyy*dyy);

        float ratio_mouth=wmouth/lmouth;

        if(ratio_eye<m_ratio_eye||ratio_mouth>m_ratio_mouth)
        {
            DMSTpye=2;
            m_DMSTpye=2;
        }
        else
        {
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
    if (m_modelType==0)
    {
        for (int i = 0; i < 68; ++i) {
            landmark68.landmark[i].x= (outputTensors_host[0]->host<float>()[2*i+0]+1)*0.5*image_w;
            landmark68.landmark[i].y= (outputTensors_host[0]->host<float>()[2*i+1]+1)*0.5*image_h;
            //     MNN_PRINT("func %f, %f\n", outputTensors_host[0]->host<float>()[2*i+0], outputTensors_host[0]->host<float>()[2*i+0]);
        }
        landmark68.numPoints=68;
    }

    if (m_modelType==1)
    {
        float scale_x = static_cast<float>(image_w) / in_w;
        float scale_y = static_cast<float>(image_h) / in_h;
        for (int i = 0; i < 98; ++i) {
            landmark68.landmark[i].x= (outputTensors_host[0]->host<float>()[2*i+0])*scale_x;
            landmark68.landmark[i].y= (outputTensors_host[0]->host<float>()[2*i+1])*scale_y;
                // MNN_PRINT("func %f, %f\n", outputTensors_host[0]->host<float>()[2*i+0], outputTensors_host[0]->host<float>()[2*i+0]);
        }
        landmark68.numPoints=98;
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

