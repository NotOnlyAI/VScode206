//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#define clip(x, y) (x < 0 ? 0 : (x > y ? y : x))

#include "FaceRecognize.hpp"
#include <opencv2/opencv.hpp>

using namespace std;



FaceRecognize::FaceRecognize() {

    string mnn_path="./models206/FaceRecognize.mnn";
    net = std::shared_ptr<MNN::Interpreter>(MNN::Interpreter::createFromFile(mnn_path.c_str()));
    dimType = MNN::Tensor::CAFFE;
    in_w=112;
    in_h=112;
    


    MNN::ScheduleConfig config;
    // config.type  = MNN_FORWARD_CPU;
    config.type=MNN_FORWARD_OPENCL;
    // config.type=MNN_FORWARD_CPU;
    // BackendConfig bnconfig;
    // bnconfig.precision = BackendConfig::Precision_Low;
    // config.backendConfig = &bnconfig; 
    session = net->createSession(config);


    input_blob_names={ "Data"};
    inputTensors.resize(input_blob_names.size());
    inputTensors_host.resize(input_blob_names.size());
    for (int i = 0; i < input_blob_names.size(); i++) {
		inputTensors[i] = net->getSessionInput(session,input_blob_names[0].c_str());
	}

    for (int i = 0; i < input_blob_names.size(); i++) {
		inputTensors_host[i] = new MNN::Tensor(inputTensors[i], dimType);
	}

    output_blob_names={ "bottleneck",};
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


int FaceRecognize::Forward(cv::Mat &raw_image) {


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
    float mean[3]     = {0.0f, 0.0f, 0.0f};
    float normals[3] = {0.0391f, 0.0391f, 0.0391f};
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


    
    outputTensors_host[0]->printShape();
    for (int i = 0; i < 20; ++i) {
        MNN_PRINT("copy %f, %f\n", outputTensors[0]->host<float>()[i], outputTensors_host[0]->host<float>()[i]);
    }
    
    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "inference time:" << elapsed.count() << " s" << endl;

    return 0;
}





FaceRecognize::~FaceRecognize() {
    net->releaseModel();
    net->releaseSession(session);
    for (int i = 0; i < input_blob_names.size(); i++) {
		delete inputTensors_host[i];
	}
    for (int i = 0; i < output_blob_names.size(); i++) {
		delete outputTensors_host[i];
	}
}

