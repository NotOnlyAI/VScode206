#include <iostream>
#include <sys/unistd.h>             //提供POSIX操作系统API，如Unix的所有官方版本
#include <sys/socket.h>
#include <netinet/in.h>             //主要定义了一些类型
#include <arpa/inet.h>              //主要定义了格式转换函数
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <pthread.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <dlfcn.h>


#include "models206.h"

using namespace std;



int main(int argc, char *argv[])
{
    
    auto handle = dlopen("libMNN_CL.so", RTLD_NOW);
    cv::Mat img=cv::imread("l1.jpg");

    // cv::imshow("11",img);
    // cv::waitKey(0);

    
    // cv::imshow("11",img);
    // cv::waitKey(0);

    // std::shared_ptr<ModelManager> manager=std::make_shared<ModelManager>();
    // manager->init();
    // manager->RunTurn();

    // std::shared_ptr<FaceDetect> FaceDetectModel=std::make_shared<FaceDetect>();
    // FaceDetectModel->Forward(img);
    // FR_Show_bboxes_and_landmarks(img,FaceDetectModel->rectinfo,"result.jpg");
    

    // std::shared_ptr<PicRecognize> PicRecognizeModel=std::make_shared<PicRecognize>();
    // PicRecognizeModel->Forward(img);



    // std::shared_ptr<FaceRecognize> FaceRecognizeModel=std::make_shared<FaceRecognize>();
    // FaceRecognizeModel->Forward(img);

    // std::shared_ptr<LaneDetect> LaneDetectModel=std::make_shared<LaneDetect>();
    // LaneDetectModel->Forward(img);
    // LaneDetectModel->visImg(img,LaneDetectModel->decode_lane);
    

    while(1)
    {
        M2::DetectResult re;
        M2_FaceDetect(img.data,M2::ImageFormat::BGR,re);
        // PicRecognizeModel->Forward(img);
        // FaceDetectModel->Forward(img);
        // FaceRecognizeModel->Forward(img);
        // LaneDetectModel->Forward(img);

        sleep(1);
    }


    return 0;
}
