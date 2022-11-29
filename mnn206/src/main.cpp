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

#include "MNN/Interpreter.hpp"
#include "MNN/MNNDefine.h"
#include "MNN/Tensor.hpp"
#include "MNN/ImageProcess.hpp"
#include "FaceDetect/FaceDetect.hpp"
#include "FaceDetectV2/FaceDetectV2.hpp"
#include "PicRecognize/PicRecognize.hpp"
#include "FaceRecognize/FaceRecognize.hpp"


using namespace std;



int FR_Show_bboxes_and_landmarks(cv::Mat image,RectDetectInfo rectinfo,std::string save_path)
{
    cv::Mat image_bbox=image;
    for(int i=0;i<rectinfo.nFaceNum;i++)
    {
        std::string text =std::to_string(rectinfo.labels[i].score);
        int font_face = cv::FONT_HERSHEY_COMPLEX;
        double font_scale = 1;
        int thickness = 1;
//        int baseline;
//        cv::Size text_size = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);
        //将文本框居中绘制
        cv::Point origin;
        origin.x = rectinfo.rects[i].x+20;
        origin.y = rectinfo.rects[i].y+20;
        cv::putText(image, text, origin, font_face, font_scale, cv::Scalar(0, 255, 255), thickness, 8, 0);

        cv::Rect r = cv::Rect(rectinfo.rects[i].x, rectinfo.rects[i].y, rectinfo.rects[i].width, rectinfo.rects[i].height);
        cv::rectangle(image_bbox, r, cv::Scalar(255, 0, 0), 1, 8, 0);
    }
    cv::imwrite(save_path,image_bbox);

    return 1;
}



int main(int argc, char *argv[])
{
    
    auto handle = dlopen("libMNN_CL.so", RTLD_NOW);
    cv::Mat img=cv::imread("4.jpg");
    // cv::imshow("11",img);
    // cv::waitKey(0);

    // std::shared_ptr<FaceDetect> FaceDetectModel=std::make_shared<FaceDetect>();
    // FaceDetectModel->Forward(img);
    // FR_Show_bboxes_and_landmarks(img,FaceDetectModel->rectinfo,"result.jpg");
    

    // std::shared_ptr<PicRecognize> PicRecognizeModel=std::make_shared<PicRecognize>();
    // PicRecognizeModel->Forward(img);



    // std::shared_ptr<FaceRecognize> FaceRecognizeModel=std::make_shared<FaceRecognize>();
    // FaceRecognizeModel->Forward(img);

    std::shared_ptr<FaceDetectV2> FaceDetectV2Model=std::make_shared<FaceDetectV2>();
    FaceDetectV2Model->Forward(img);
    FR_Show_bboxes_and_landmarks(img,FaceDetectV2Model->rectinfo,"result.jpg");

    while(1)
    {
        std::cout<<"hello world!6"<<std::endl;
        // PicRecognizeModel->Forward(img);
        // FaceDetectModel->Forward(img);
        // FaceRecognizeModel->Forward(img);
        FaceDetectV2Model->Forward(img);

        sleep(1);
    }


    return 0;
}
