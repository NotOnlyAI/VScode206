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

int FaceDetect_visImg(const M2::ImgData_T &imagedata,const M2::DetectResult &rectinfo)
{
    
    cv::Mat ori_image(cv::Size(imagedata.width, imagedata.height), CV_8UC3);
	ori_image.data =imagedata.data;

    cv::Mat image=ori_image.clone();
    

    for(int i=0;i<rectinfo.nNum;i++)
    {
        std::string text =std::to_string(rectinfo.labels[i].score);
        int font_face = cv::FONT_HERSHEY_COMPLEX;
        double font_scale = 1;
        int thickness = 1;
    //    int baseline;
    //    cv::Size text_size = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);
        // 将文本框居中绘制
        cv::Point origin;
        origin.x = rectinfo.boxes[i].xmin+20;
        origin.y = rectinfo.boxes[i].ymin+20;
        cv::putText(image, text, origin, font_face, font_scale, cv::Scalar(0, 255, 255), thickness, 8, 0);

        cv::Rect r = cv::Rect(rectinfo.boxes[i].xmin, rectinfo.boxes[i].ymin, rectinfo.boxes[i].width, rectinfo.boxes[i].height);
        cv::rectangle(image, r, cv::Scalar(255, 0, 0), 1, 8, 0);
    }
    cv::imshow("out",image);
    cv::waitKey(0);
    return 0;
}



void LaneDetect_visImg(const M2::ImgData_T &imagedata, std::vector<M2::lane_DECODE> final_lane) 
{
    cv::Mat ori_image(cv::Size(imagedata.width, imagedata.height), CV_8UC3);
	ori_image.data =imagedata.data;
    cv::Mat vis_img=ori_image.clone();


    float sx = float(imagedata.width) / float(512.0);
    float sy = float(imagedata.height) / float(288.0);
//    cv::Mat qqq = cv::Mat::zeros(720, 1280, CV_8UC3);
    std::cout<<"final_lane.size():"<<final_lane.size()<<std::endl;
    for (int i = 0; i < final_lane.size(); i++)
    {
        for (int j = 0; j < final_lane[i].Lane.size() - 1; j++) {
            float px = final_lane[i].Lane[j].x;
            float py = final_lane[i].Lane[j].y;
            cv::Point P;
            P.x = int(px * sx);
            P.y = int(py * sy);
            if (P.x > 0 && P.x < 1280) {
                cv::circle(vis_img, P, 5, cv::Scalar(0, 0, 255), -1);
            }
        }
    }
    cv::imshow("11",vis_img);
    cv::waitKey(0);
}


int FR_Show_PTS(const M2::ImgData_T &imagedata,const M2::LandmarkInfo &landmarks)
{
    cv::Mat ori_image(cv::Size(imagedata.width, imagedata.height), CV_8UC3);
	ori_image.data =imagedata.data;

    cv::Mat image=ori_image.clone();
    for(int i=0;i<68;i++)
    {
        cv::Point p1(landmarks.landmark[i].x ,landmarks.landmark[i].y);
        std::cout << i <<"  x: "<< landmarks.landmark[i].x << "   y: "<< landmarks.landmark[i].y<<std::endl;
        cv::circle(image, p1, 1, cv::Scalar(0, 255, 0), -1); // ç”»åŠå¾„ä¸º1çš„åœ†(ç”»ç‚¹ï¿?
    }
    cv::imshow("11",image);
    cv::waitKey(0);
    return 1;
}



int main(int argc, char *argv[])
{
    
    auto handle = dlopen("libMNN_CL.so", RTLD_NOW);
    cv::Mat image=cv::imread("l1.jpg");
    // cv::Mat image;
    // cv::resize(raw_image, image, cv::Size(240, 320));


    // M2::ImgData_T imagedata;
    // imagedata.data=image.data;
    // imagedata.width=image.cols;
    // imagedata.height=image.rows;
    // imagedata.channel=image.channels();
    // imagedata.stride=0;
    // imagedata.depth=0;
    // imagedata.dataFormat=M2::ImageFormat::BGR;

    // M2::DetectResult result;
    // M2_ObjectDetect_ForwardBGR(image,result);

    std::vector<M2::lane_DECODE> final_lane;
    M2_LaneDetect_ForwardBGR(image,final_lane);

    while(1)
    {
        // M2::DetectResult result;
        // M2_ObjectDetect_ForwardBGR(image,result);
        std::vector<M2::lane_DECODE> final_lane;
        M2_LaneDetect_ForwardBGR(image,final_lane);
        sleep(1);
    }



    //     M2::DetectResult rectinfo;
    // M2_FaceDetect(imagedata,rectinfo,2);


    // for(int i=0;i<rectinfo.nNum;i++)
    // {
    //     M2::LandmarkInfo landmark68;
    //     M2_FaceAlignment(imagedata,rectinfo.boxes[i],landmark68);
    //     for(int j=0;j<68;j++)
    //     {
    //         cout<<landmark68.landmark[j].x<<" ,"<<landmark68.landmark[j].y<<endl;
    //     }
    //     FR_Show_PTS(imagedata,landmark68);
    // }

    // cout<<"here"<<endl;
    
    // FaceDetect_visImg(imagedata,rectinfo);



    // while(1)
    // {
        // std::vector<M2::lane_DECODE> final_lane;
        // M2_LaneDetect(imagedata,final_lane);
    //     std::cout<<"final_lane.size():"<<final_lane.size()<<std::endl;
    //     LaneDetect_visImg(imagedata,final_lane);
    //     sleep(1);
    // }

    return 0;
}
