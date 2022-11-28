//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#define clip(x, y) (x < 0 ? 0 : (x > y ? y : x))

#include "PicRecognize.hpp"
#include <opencv2/opencv.hpp>
using namespace std;



PicRecognize::PicRecognize() {

    string mnn_path="./models206/PicRecognize.mnn";
    net = std::shared_ptr<MNN::Interpreter>(MNN::Interpreter::createFromFile(mnn_path.c_str()));

    MNN::ScheduleConfig config;
    config.type  = MNN_FORWARD_CPU;
    // config.type=MNN_FORWARD_OPENCL;
    // config.type=MNN_FORWARD_CPU;
    // BackendConfig bnconfig;
    // bnconfig.precision = BackendConfig::Precision_Low;
    // config.backendConfig = &bnconfig;  
    session = net->createSession(config);
    input = net->getSessionInput(session, NULL);
    

}


int PicRecognize::Forward(cv::Mat &raw_image) {
    if (raw_image.empty()) {
        std::cout << "image is empty ,please check!" << std::endl;
        return -1;
    }

    image_h = raw_image.rows;
    image_w = raw_image.cols;



    cv::Mat image;
    cv::resize(raw_image, image, cv::Size(in_w, in_h));
    
    auto shape = input->shape();
    std::cout<<"shape:"<<input->shape()[0]<<","<<input->shape()[1]<<","<<input->shape()[2]<<","<<input->shape()[3]<<","<<std::endl;
    net->resizeTensor(input, shape);
    net->resizeSession(session);float memoryUsage = 0.0f;

    net->getSessionInfo(session, MNN::Interpreter::MEMORY, &memoryUsage);
    float flops = 0.0f;
    net->getSessionInfo(session, MNN::Interpreter::FLOPS, &flops);
    int backendType[2];
    net->getSessionInfo(session, MNN::Interpreter::BACKENDS, backendType);
    MNN_PRINT("Session Info: memory use %f MB, flops is %f M, backendType is %d, batch size = %d\n", memoryUsage, flops, backendType[0], 1);


    auto output = net->getSessionOutput(session, NULL);


    std::shared_ptr<MNN::Tensor> inputUser(new MNN::Tensor(input, MNN::Tensor::TENSORFLOW));
    auto bpp          = inputUser->channel();
    auto size_h       = inputUser->height();
    auto size_w       = inputUser->width();
    MNN_PRINT("input: w:%d , h:%d, bpp: %d\n", size_w, size_h, bpp);

    MNN::CV::ImageProcess::Config config;
    float mean[3]     = {103.94f, 116.78f, 123.68f};
    float normals[3] = {0.017f, 0.017f, 0.017f};
    ::memcpy(config.mean, mean, sizeof(mean));
    ::memcpy(config.normal, normals, sizeof(normals));

    std::shared_ptr<MNN::CV::ImageProcess> pretreat(MNN::CV::ImageProcess::create(config));
    pretreat->convert(image.data, in_w, in_h,0, inputUser->host<uint8_t>(),size_w, size_h, bpp, 0, inputUser->getType());
    input->copyFromHostTensor(inputUser.get());



    net->runSession(session);
    auto nhwcTensor = new MNN::Tensor(output, MNN::Tensor::TENSORFLOW);
    output->copyToHostTensor(nhwcTensor);
    for (int i = 0; i < 100; ++i) {
        MNN_PRINT("%f, %f\n", output->host<float>()[i], nhwcTensor->host<float>()[i]);
    }
    delete nhwcTensor;
    // auto dimType = output->getDimensionType();
    // MNN_PRINT("output shape: %d, %d, %d, %d\n", output->shape()[0], output->shape()[1],output->shape()[2],output->shape()[3]);
    // std::shared_ptr<MNN::Tensor> outputUser(new MNN::Tensor(output, dimType));
    // output->copyToHostTensor(outputUser.get());
    // auto type1 = outputUser->getType();
    
    

    
    // std::vector<int> shape_output= outputUser->shape();
    // MNN_PRINT("outputUser shape: %d, %d, %d, %d\n", shape_output[0], shape_output[1],shape_output[2],shape_output[3]);

    // auto value1 = output->host<float>();
    // auto value2 = outputUser->host<float>();
    // std::vector<std::pair<float, float>> tempValues(10);
    // for (int i = 0; i < 10; ++i) {
    //     tempValues[i] = std::make_pair(value1[i], value2[i]);
    //     MNN_PRINT("%f, %f\n", tempValues[i].first, tempValues[i].second);
    // }
    
	// 	std::vector<int> shape_loc=tensor_location->shape();
	// 	for (int j = 0; j < 100; j++) {
	// 		score.push_back(tensor_score->host<float>()[j]);
	// 	}






    




    // PicRecognize_interpreter->resizeTensor(input_tensor, {1, 3, in_h, in_w});
    // PicRecognize_interpreter->resizeSession(PicRecognize_session);
    // std::shared_ptr<MNN::CV::ImageProcess> pretreat(MNN::CV::ImageProcess::create(MNN::CV::BGR, MNN::CV::RGB, mean_vals, 3,norm_vals, 3));
    // pretreat->convert(image.data, in_w, in_h, image.step[0], input_tensor);

    

    auto start = chrono::steady_clock::now();


    // // run network
    // PicRecognize_interpreter->runSession(PicRecognize_session);

    // get output data

    // string conf0 = "conf0";
    // string conf1 = "conf1";
    // string conf2 = "conf2";

    // string loc0 = "loc0";
    // string loc1 = "loc1";
    // string loc2 = "loc2";

   
    // MNN::Tensor *tensor_conf0 = PicRecognize_interpreter->getSessionOutput(PicRecognize_session, conf0.c_str());
    // MNN::Tensor *tensor_conf1 = PicRecognize_interpreter->getSessionOutput(PicRecognize_session, conf1.c_str());
    // MNN::Tensor *tensor_conf2 = PicRecognize_interpreter->getSessionOutput(PicRecognize_session, conf2.c_str());

    // MNN::Tensor *tensor_loc0 = PicRecognize_interpreter->getSessionOutput(PicRecognize_session, loc0.c_str());
    // MNN::Tensor *tensor_loc1 = PicRecognize_interpreter->getSessionOutput(PicRecognize_session, loc1.c_str());
    // MNN::Tensor *tensor_loc2 = PicRecognize_interpreter->getSessionOutput(PicRecognize_session, loc2.c_str());

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


    // for (int i = 0; i < 10; i++) {
    //     std::cout<<i<<"  :"<<tensor_conf0->host<float>()[i * 2 + 0]<<" ,"<<tensor_conf0->host<float>()[i * 2 + 1]<<std::endl;
    //     std::cout<<i<<"  :"<<tensor_loc0->host<float>()[i * 4 + 0]<<" ,"<<tensor_loc0->host<float>()[i * 4 + 1]<<"  :"<<tensor_loc0->host<float>()[i * 4 + 2]<<" ,"<<tensor_loc0->host<float>()[i * 4 + 3]<<std::endl;
    //     // if (scores->host<float>()[i * 2 + 1] > score_threshold) {
           
    //     //     float x_center = boxes->host<float>()[i * 4] * center_variance * priors[i][2] + priors[i][0];
    //     //     float y_center = boxes->host<float>()[i * 4 + 1] * center_variance * priors[i][3] + priors[i][1];
    //     //     float w = exp(boxes->host<float>()[i * 4 + 2] * size_variance) * priors[i][2];
    //     //     float h = exp(boxes->host<float>()[i * 4 + 3] * size_variance) * priors[i][3];

    //     //     rects.x1 = clip(x_center - w / 2.0, 1) * image_w;
    //     //     rects.y1 = clip(y_center - h / 2.0, 1) * image_h;
    //     //     rects.x2 = clip(x_center + w / 2.0, 1) * image_w;
    //     //     rects.y2 = clip(y_center + h / 2.0, 1) * image_h;
    //     //     rects.score = clip(scores->host<float>()[i * 2 + 1], 1);
    //     //     bbox_collection.push_back(rects);
    //     // }
    // }




    // std::vector<FaceInfo> bbox_collection;


    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "inference time:" << elapsed.count() << " s" << endl;

    // generateBBox(bbox_collection, tensor_scores, tensor_boxes);
    // nms(bbox_collection, face_list);
    return 0;
}



PicRecognize::~PicRecognize() {
    net->releaseModel();
    net->releaseSession(session);
}

