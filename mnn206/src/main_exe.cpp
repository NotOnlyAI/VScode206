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

int FaceDetect_visImg(const cv::Mat &ori_image,const M2::ObjectInfo &objectinfo)
{
    
    cv::Mat image=ori_image.clone();
    

    for(int i=0;i<objectinfo.ObjectNum;i++)
    {
        std::string text =std::to_string(objectinfo.objects[i].prob);
        int font_face = cv::FONT_HERSHEY_COMPLEX;
        double font_scale = 1;
        int thickness = 1;
    //    int baseline;
    //    cv::Size text_size = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);
        // 将文本框居中绘制
        cv::Point origin;
        origin.x = objectinfo.objects[i].rect.x+20;
        origin.y = objectinfo.objects[i].rect.y+20;
        cv::putText(image, text, origin, font_face, font_scale, cv::Scalar(0, 255, 255), thickness, 8, 0);
        // cv::Rect r = cv::Rect(objectinfo.objects[i].rect.x, objectinfo.objects[i].rect.y, objectinfo.objects[i].rect.width, rectinfo.boxes[i].height);
        cv::rectangle(image, objectinfo.objects[i].rect, cv::Scalar(255, 0, 0), 1, 8, 0);
    }
    cv::imshow("out",image);
    cv::waitKey(0);
    return 0;
}



// void LaneDetect_visImg(const M2::ImgData_T &imagedata, std::vector<M2::lane_DECODE> final_lane) 
// {
//     cv::Mat ori_image(cv::Size(imagedata.width, imagedata.height), CV_8UC3);
// 	ori_image.data =imagedata.data;
//     cv::Mat vis_img=ori_image.clone();


//     float sx = float(imagedata.width) / float(512.0);
//     float sy = float(imagedata.height) / float(288.0);
// //    cv::Mat qqq = cv::Mat::zeros(720, 1280, CV_8UC3);
//     std::cout<<"final_lane.size():"<<final_lane.size()<<std::endl;
//     for (int i = 0; i < final_lane.size(); i++)
//     {
//         for (int j = 0; j < final_lane[i].Lane.size() - 1; j++) {
//             float px = final_lane[i].Lane[j].x;
//             float py = final_lane[i].Lane[j].y;
//             cv::Point P;
//             P.x = int(px * sx);
//             P.y = int(py * sy);
//             if (P.x > 0 && P.x < 1280) {
//                 cv::circle(vis_img, P, 5, cv::Scalar(0, 0, 255), -1);
//             }
//         }
//     }
//     cv::imshow("11",vis_img);
//     cv::waitKey(0);
// }


int FR_Show_PTS(const cv::Mat &ori_image,const M2::LandmarkInfo &landmarks)
{

    cv::Mat image=ori_image.clone();
    for(int i=0;i<landmarks.numPoints;i++)
    {
        cv::Point p1(landmarks.landmark[i].x ,landmarks.landmark[i].y);
        std::cout << i <<"  x: "<< landmarks.landmark[i].x << "   y: "<< landmarks.landmark[i].y<<std::endl;
        cv::circle(image, p1, 1, cv::Scalar(0, 255, 0), -1); 
    }
    cv::imshow("11",image);
    cv::waitKey(0);
    return 1;
}



int main(int argc, char *argv[])
{
    
    auto handle = dlopen("libMNN_CL.so", RTLD_NOW);
    cv::Mat image=cv::imread("2.jpg");
    M2::LandmarkInfo landmarks;
    M2_FaceAlignment_ForwardBGR_MaxFace(image,landmarks);
    FR_Show_PTS(image,landmarks);

    // cv::Mat image;
    // cv::resize(raw_image, image, cv::Size(240, 320));

    // M2::DetectResult result;
    // M2_ObjectDetect_ForwardBGR(image,result);




    // std::vector<M2::lane_DECODE> final_lane;
    // M2_LaneDetect_ForwardBGR(image,final_lane);

    // while(1)
    // {
    //     M2::DetectResult result;
    //     M2_ObjectDetect_ForwardBGR(image,result);
    //     // std::vector<M2::lane_DECODE> final_lane;
    //     // M2_LaneDetect_ForwardBGR(image,final_lane);
    //     sleep(1);
    // }


    // for(int i=0;i<10;i++)
    // {
    //     M2::ObjectInfo objectinfo;
    //     M2_FaceDetect_ForwardBGR(image,objectinfo,0);
    //     FaceDetect_visImg(image,objectinfo);
    // }



    // for(int i=0;i<objectinfo.ObjectNum;i++)
    // {
    //     
    //     M2_FaceAlignment_ForwardBGR(image,objectinfo.objects[i],landmarks);
    //     
    // }

   

    return 0;
}
