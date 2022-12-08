//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#define clip(x, y) (x < 0 ? 0 : (x > y ? y : x))

#include "FaceAlignment.hpp"
#include "M2utils/image_utils_new.h"
#include <opencv2/opencv.hpp>
using namespace std;




unsigned char M2_crop_databuff[2048*2048*3];



FaceAlignment::FaceAlignment() {


}


int FaceAlignment::init(int deviceTpye,int print_config){

    string mnn_path="./models206/FaceAlignment.mnn";
    net = std::shared_ptr<MNN::Interpreter>(MNN::Interpreter::createFromFile(mnn_path.c_str()));
    dimType = MNN::Tensor::TENSORFLOW;
    in_h=256;
    in_w=256;
    MNN_PRINT("Interpreter build, model_path: %s, dimType:%d\n",mnn_path.c_str(),dimType);

    MNN::ScheduleConfig config;
    config.type  = (MNNForwardType)(deviceTpye);
    // config.type=MNN_FORWARD_OPENCL;
    // config.type=MNN_FORWARD_CPU;
    // BackendConfig bnconfig;
    // bnconfig.precision = BackendConfig::Precision_Low;
    // config.backendConfig = &bnconfig; 
    session = net->createSession(config);
    MNN_PRINT("ScheduleConfig build, config.type: %d \n",config.type);


    
    float mean[3]     = {127.5f, 127.5f, 127.5f};
    float normals[3] = {0.007843f, 0.007843f, 0.007843f};
    ::memcpy(imconfig.mean, mean, sizeof(mean));
    ::memcpy(imconfig.normal, normals, sizeof(normals));
    imconfig.sourceFormat = MNN::CV::BGR;
    imconfig.destFormat = MNN::CV::RGB;
    imconfig.filterType = MNN::CV::NEAREST;
    pretreat=std::shared_ptr<MNN::CV::ImageProcess>(MNN::CV::ImageProcess::create(imconfig));
    MNN_PRINT("ImageProcess build, sourceFormat: %d, destFormat: %d \n",imconfig.sourceFormat,imconfig.destFormat);



    input_blob_names={ "input_1"};
    inputTensors.resize(input_blob_names.size());
    inputTensors_host.resize(input_blob_names.size());
    for (int i = 0; i < input_blob_names.size(); i++) {
		inputTensors[i] = net->getSessionInput(session,input_blob_names[0].c_str());
	}

    for (int i = 0; i < input_blob_names.size(); i++) {
		inputTensors_host[i] = new MNN::Tensor(inputTensors[i], dimType);
	}

    output_blob_names={ "Identity",};
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


int FaceAlignment::Forward(const M2::ImgData_T &imgdata,M2::Box cropBox,M2::LandmarkInfo &landmarkinfo) {


    auto start = chrono::steady_clock::now();


    // cv::Mat image(cv::Size(imgdata.width, imgdata.height), CV_8UC3);
	// image.data =imgdata.data;
	// cv::imshow("in",image);
	// cv::waitKey(0);

    
        
    if (!imgdata.data) {
        std::cout << "image is empty ,please check!" << std::endl;
        return -1;
    }




    M2::ImgData_T crop_imagedata;
    double newx1=max(cropBox.xmin-0.125*cropBox.width,1.0);
    double newy1=max(cropBox.ymin-0.125*cropBox.height,1.0);
//    double neww=min(1.25*rect.width,imagedata.width-newx1-1.0);
//    double newh=min(1.25*rect.height,imagedata.height-newy1-1.0);
    double newx2 = min(cropBox.xmin + 1.125 * cropBox.width, imgdata.width - 1.0);
    double newy2 = min(cropBox.ymin + 1.125 * cropBox.height, imgdata.height - 1.0);


    double dw=min(newx2-(cropBox.xmin+cropBox.width),cropBox.xmin-newx1);
    double dh=min(newy2-(cropBox.ymin+cropBox.height),cropBox.ymin-newy1);
    double dd=min(dw,dh);
    M2::Box rect_new;
    rect_new.xmin=int(cropBox.xmin-dd);
    rect_new.ymin=int(cropBox.ymin-dd);
    rect_new.width=int(cropBox.width+2*dd);
    rect_new.height=int(cropBox.height+2*dd);

    cout<<rect_new.xmin<<","<<rect_new.ymin<<","<<rect_new.width<<","<<rect_new.height<<endl;

    crop_imagedata.data=(unsigned char*)M2_crop_databuff;
    Image_crop_v2(imgdata,crop_imagedata,rect_new);


    // cv::Mat crop_image(cv::Size(crop_imagedata.width, crop_imagedata.height), CV_8UC3);
	// crop_image.data =crop_imagedata.data;
	// cv::imshow("crop",crop_image);
	// cv::waitKey(0);

    image_h = crop_imagedata.height;
    image_w = crop_imagedata.width;

    MNN::CV::ImageFormat sourceFormat=(MNN::CV::ImageFormat)imgdata.dataFormat;
    if(imconfig.sourceFormat!=sourceFormat)
    {
        imconfig.sourceFormat = sourceFormat;
        imconfig.destFormat = MNN::CV::RGB;
        pretreat=std::shared_ptr<MNN::CV::ImageProcess>(MNN::CV::ImageProcess::create(imconfig));
    }



    MNN::CV::Matrix trans;
    trans.setScale((float)(crop_imagedata.width) / (float)(in_w), (float)(crop_imagedata.height) / (float)(in_h));
    pretreat->setMatrix(trans);
    pretreat->convert((uint8_t *)crop_imagedata.data, crop_imagedata.width,crop_imagedata.height,0,inputTensors_host[0]);


    // MNN::CV::Matrix trans;
    // trans.setScale((float)(imgdata.width) / (float)(in_w), (float)(imgdata.height) / (float)(in_h));
    // pretreat->setMatrix(trans);
    // pretreat->convert((uint8_t *)imgdata.data, imgdata.width,imgdata.height,0,inputTensors_host[0]);


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

    for (int i = 0; i < 68; ++i) {
        landmarkinfo.landmark[i].x= landmark68.landmark[i].x+rect_new.xmin;
        landmarkinfo.landmark[i].y= landmark68.landmark[i].y+rect_new.ymin;
        MNN_PRINT("landmarkinfo %f, %f\n", landmarkinfo.landmark[i].x, landmarkinfo.landmark[i].y);
    }



    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "FaceAlignment inference time:" << elapsed.count() << " s" << endl;

    return 0;
}




int FaceAlignment::decode(std::vector< MNN::Tensor*> &outputTensors_host)
{
    // for (int i = 0; i < 10; ++i) {
    //     MNN_PRINT("func %f, %f\n", outputTensors_host[0]->host<float>()[2*i+0], outputTensors_host[0]->host<float>()[2*i+0]);
    // }

    for (int i = 0; i < 68; ++i) {
        landmark68.landmark[i].x= (outputTensors_host[0]->host<float>()[2*i+0]+1)*0.5*image_w;
        landmark68.landmark[i].y= (outputTensors_host[0]->host<float>()[2*i+1]+1)*0.5*image_h;
    //     MNN_PRINT("func %f, %f\n", outputTensors_host[0]->host<float>()[2*i+0], outputTensors_host[0]->host<float>()[2*i+0]);
    }
    return 1;

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

