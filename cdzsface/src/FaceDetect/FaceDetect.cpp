//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#define clip(x, y) (x < 0 ? 0 : (x > y ? y : x))

#include "FaceDetect.hpp"
#include <opencv2/opencv.hpp>
using namespace std;



FaceDetect::FaceDetect() {

    string mnn_path="./models206/FaceDetect.mnn";
    FaceDetect_interpreter = std::shared_ptr<MNN::Interpreter>(MNN::Interpreter::createFromFile(mnn_path.c_str()));
   
    MNN::BackendConfig backendConfig;
    backendConfig.precision = (MNN::BackendConfig::PrecisionMode) 2;
    MNN::ScheduleConfig config;
    config.numThread = 4;
    config.backendConfig = &backendConfig;
    config.type=MNN_FORWARD_OPENCL;

    FaceDetect_session = FaceDetect_interpreter->createSession(config);
    input_tensor = FaceDetect_interpreter->getSessionInput(FaceDetect_session, nullptr);

}


int FaceDetect::Forward(cv::Mat &raw_image) {
    if (raw_image.empty()) {
        std::cout << "image is empty ,please check!" << std::endl;
        return -1;
    }

    image_h = raw_image.rows;
    image_w = raw_image.cols;



    cv::Mat image;
    cv::resize(raw_image, image, cv::Size(in_w, in_h));

    FaceDetect_interpreter->resizeTensor(input_tensor, {1, 3, in_h, in_w});
    FaceDetect_interpreter->resizeSession(FaceDetect_session);
    std::shared_ptr<MNN::CV::ImageProcess> pretreat(MNN::CV::ImageProcess::create(MNN::CV::BGR, MNN::CV::RGB, mean_vals, 3,norm_vals, 3));
    pretreat->convert(image.data, in_w, in_h, image.step[0], input_tensor);

    

    auto start = chrono::steady_clock::now();


    // run network
    FaceDetect_interpreter->runSession(FaceDetect_session);

    // get output data

    string conf0 = "conf0";
    string conf1 = "conf1";
    string conf2 = "conf2";

    string loc0 = "loc0";
    string loc1 = "loc1";
    string loc2 = "loc2";

   
    MNN::Tensor *tensor_conf0 = FaceDetect_interpreter->getSessionOutput(FaceDetect_session, conf0.c_str());
    MNN::Tensor *tensor_conf1 = FaceDetect_interpreter->getSessionOutput(FaceDetect_session, conf1.c_str());
    MNN::Tensor *tensor_conf2 = FaceDetect_interpreter->getSessionOutput(FaceDetect_session, conf2.c_str());

    MNN::Tensor *tensor_loc0 = FaceDetect_interpreter->getSessionOutput(FaceDetect_session, loc0.c_str());
    MNN::Tensor *tensor_loc1 = FaceDetect_interpreter->getSessionOutput(FaceDetect_session, loc1.c_str());
    MNN::Tensor *tensor_loc2 = FaceDetect_interpreter->getSessionOutput(FaceDetect_session, loc2.c_str());

    // MNN::Tensor tensor_conf0_host(tensor_conf0, tensor_conf0->getDimensionType());
    // MNN::Tensor tensor_conf1_host(tensor_conf1, tensor_conf1->getDimensionType());
    // MNN::Tensor tensor_conf2_host(tensor_conf2, tensor_conf2->getDimensionType());

    // MNN::Tensor tensor_loc0_host(tensor_loc0, tensor_loc0->getDimensionType());
    // MNN::Tensor tensor_loc1_host(tensor_loc1, tensor_loc1->getDimensionType());
    // MNN::Tensor tensor_loc2_host(tensor_loc2, tensor_loc2->getDimensionType());


    // tensor_conf0->copyToHostTensor(&tensor_conf0_host);
    // tensor_conf1->copyToHostTensor(&tensor_conf1_host);
    // tensor_conf2->copyToHostTensor(&tensor_conf2_host);
    // tensor_loc0->copyToHostTensor(&tensor_loc0_host);
    // tensor_loc1->copyToHostTensor(&tensor_loc1_host);
    // tensor_loc2->copyToHostTensor(&tensor_loc2_host);


    for (int i = 0; i < 10; i++) {
        std::cout<<i<<"  :"<<tensor_conf0->host<float>()[i * 2 + 0]<<" ,"<<tensor_conf0->host<float>()[i * 2 + 1]<<std::endl;
        std::cout<<i<<"  :"<<tensor_loc0->host<float>()[i * 4 + 0]<<" ,"<<tensor_loc0->host<float>()[i * 4 + 1]<<"  :"<<tensor_loc0->host<float>()[i * 4 + 2]<<" ,"<<tensor_loc0->host<float>()[i * 4 + 3]<<std::endl;
        // if (scores->host<float>()[i * 2 + 1] > score_threshold) {
           
        //     float x_center = boxes->host<float>()[i * 4] * center_variance * priors[i][2] + priors[i][0];
        //     float y_center = boxes->host<float>()[i * 4 + 1] * center_variance * priors[i][3] + priors[i][1];
        //     float w = exp(boxes->host<float>()[i * 4 + 2] * size_variance) * priors[i][2];
        //     float h = exp(boxes->host<float>()[i * 4 + 3] * size_variance) * priors[i][3];

        //     rects.x1 = clip(x_center - w / 2.0, 1) * image_w;
        //     rects.y1 = clip(y_center - h / 2.0, 1) * image_h;
        //     rects.x2 = clip(x_center + w / 2.0, 1) * image_w;
        //     rects.y2 = clip(y_center + h / 2.0, 1) * image_h;
        //     rects.score = clip(scores->host<float>()[i * 2 + 1], 1);
        //     bbox_collection.push_back(rects);
        // }
    }




    // std::vector<FaceInfo> bbox_collection;


    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "inference time:" << elapsed.count() << " s" << endl;

    // generateBBox(bbox_collection, tensor_scores, tensor_boxes);
    // nms(bbox_collection, face_list);
    return 0;
}



FaceDetect::~FaceDetect() {
    FaceDetect_interpreter->releaseModel();
    FaceDetect_interpreter->releaseSession(FaceDetect_session);
}

