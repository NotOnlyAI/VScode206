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



void LaneDetect_visImg(const cv::Mat &ori_image, std::vector<M2::lane_DECODE> final_lane) 
{
   
    cv::Mat vis_img=ori_image.clone();


    float sx = float(vis_img.cols) / float(512.0);
    float sy = float(vis_img.rows) / float(288.0);
//    cv::Mat qqq = cv::Mat::zeros(720, 1280, CV_8UC3);
    // std::cout<<"final_lane.size():"<<final_lane.size()<<std::endl;
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


int Check_LaneDetet()
{
    cv::Mat image=cv::imread("l1.jpg");
    std::vector<M2::lane_DECODE> final_lane;
    M2_LaneDetect_ForwardBGR(image,final_lane);
    LaneDetect_visImg(image,final_lane);
    return 0;
}


int Check_LaneDetect_video()
{
    cv::VideoCapture cap;
    cap.open("video_0.mp4");
    if (!cap.isOpened())
    {
        cout<<"video failed!"<<endl;
        return -1;
    }
    int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);             //帧宽度
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);           //帧高度
    int totalFrames = cap.get(cv::CAP_PROP_FRAME_COUNT);       //总帧数
    int frameRate = cap.get(cv::CAP_PROP_FPS);                 //帧率 x frames/s
    int ex = static_cast<int>(cap.get(cv::CAP_PROP_FOURCC));
    char EXT[] = { (char)(ex & 0XFF),(char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24),0 };
    cv::VideoWriter wri;
    wri.open("video_0_result.mp4", ex, frameRate, cv::Size(width, height));


    cv::Mat frame;

    double inferenceTime = 0.0;
    int n_frame=0;
    while (1) {
        cap >> frame;
        if (frame.empty())
            break;
        // if (n_frame>10)
        //     break;
        double t1 = static_cast<double>(cv::getTickCount());

        std::vector<M2::lane_DECODE> final_lane;
        M2_LaneDetect_ForwardBGR(frame,final_lane);

        double t2 = static_cast<double>(cv::getTickCount());
        if (inferenceTime == 0){
            inferenceTime = (t2 - t1) / cv::getTickFrequency() * 1000;
        }
        else 
        {
            inferenceTime = inferenceTime * 0.95 + 0.05 * (t2 - t1) / cv::getTickFrequency() * 1000;
        }

        std::stringstream fpsSs;
        fpsSs << "FPS: " << int(1000.0f / inferenceTime * 100) / 100.0f;
        cv::putText(frame, fpsSs.str(), cv::Point(16, 32),cv::FONT_HERSHEY_COMPLEX, 0.8, cv::Scalar(0, 0, 255));




        float sx = float(frame.cols) / float(512.0);
        float sy = float(frame.rows) / float(288.0);

        for (int i = 0; i < final_lane.size(); i++)
        {
            for (int j = 0; j < final_lane[i].Lane.size() - 1; j++) {
                float px = final_lane[i].Lane[j].x;
                float py = final_lane[i].Lane[j].y;
                cv::Point P;
                P.x = int(px * sx);
                P.y = int(py * sy);
                if (P.x > 0 && P.x < 1280) {
                    cv::circle(frame, P, 5, cv::Scalar(0, 0, 255), -1);
                }
            }
        }




        // LaneDetect_visImg(frame,final_lane);
        wri << frame;
        // cv::imshow("11",frame);
        // cv::waitKey(0);
        n_frame++;
    } 

    // cv::Mat image=cv::imread("l1.jpg");
   
    return 0;
}


int Check_ObjectDetect_video()
{
    cv::VideoCapture cap;
    cap.open("video_0.mp4");
    if (!cap.isOpened())
    {
        cout<<"video failed!"<<endl;
        return -1;
    }
    int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);             //帧宽度
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);           //帧高度
    int totalFrames = cap.get(cv::CAP_PROP_FRAME_COUNT);       //总帧数
    int frameRate = cap.get(cv::CAP_PROP_FPS);                 //帧率 x frames/s
    cout<<"frameRate:"<<frameRate<<endl;
    int ex = static_cast<int>(cap.get(cv::CAP_PROP_FOURCC));
    char EXT[] = { (char)(ex & 0XFF),(char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24),0 };
    cv::VideoWriter wri;
    wri.open("video_0_result2.mp4", ex, 10, cv::Size(width, height));


    cv::Mat frame;
    // cv::Mat frame=cv::imread("dog.jpg");
    double inferenceTime = 0.0;
    int n_frame=0;
    cap >> frame;
    M2::ObjectInfo objectinfo;
    M2_ObjectDetect_ForwardBGR(frame,objectinfo);

    while (1) {
        cap >> frame;
        if (frame.empty())
            break;
        // if (n_frame>20)
        //     break;
        double t1 = static_cast<double>(cv::getTickCount());

        M2::ObjectInfo objectinfo;
        M2_ObjectDetect_ForwardBGR(frame,objectinfo);

        double t2 = static_cast<double>(cv::getTickCount());
        if (inferenceTime == 0){
            inferenceTime = (t2 - t1) / cv::getTickFrequency() * 1000;
        }
        else 
        {
            inferenceTime = inferenceTime * 0.95 + 0.05 * (t2 - t1) / cv::getTickFrequency() * 1000;
        }

        std::stringstream fpsSs;
        fpsSs << "FPS: " << int(1000.0f / inferenceTime * 100) / 100.0f;
        cv::putText(frame, fpsSs.str(), cv::Point(16, 32),cv::FONT_HERSHEY_COMPLEX, 0.8, cv::Scalar(0, 0, 255));


        for(int i=0;i<objectinfo.ObjectNum;i++)
        {
            std::string text =std::to_string(int(objectinfo.objects[i].prob*100));
            int font_face = cv::FONT_HERSHEY_COMPLEX;
            double font_scale = 0.8;
            int thickness = 1;
        //    int baseline;
        //    cv::Size text_size = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);
            // 将文本框居中绘制
            cv::Point origin;
            origin.x = objectinfo.objects[i].rect.x+20;
            origin.y = objectinfo.objects[i].rect.y+20;
            cv::putText(frame, text, origin, font_face, font_scale, cv::Scalar(0, 255, 255), thickness, 8, 0);
            // cv::Rect r = cv::Rect(objectinfo.objects[i].rect.x, objectinfo.objects[i].rect.y, objectinfo.objects[i].rect.width, rectinfo.boxes[i].height);
            cv::rectangle(frame, objectinfo.objects[i].rect, cv::Scalar(255, 0, 0), 1, 8, 0);
        }




        // FaceDetect_visImg(frame,objectinfo);
        wri << frame;
        // cv::imshow("11",frame);
        // cv::waitKey(0);
        n_frame++;
    } 

    // cv::Mat image=cv::imread("l1.jpg");
   
    return 0;
}

int Check_FaceDetect_video()
{
    cv::VideoCapture cap;
    cap.open("1.mp4");
    if (!cap.isOpened())
    {
        cout<<"video failed!"<<endl;
        return -1;
    }
    int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);             //帧宽度
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);           //帧高度
    int totalFrames = cap.get(cv::CAP_PROP_FRAME_COUNT);       //总帧数
    int frameRate = cap.get(cv::CAP_PROP_FPS);                 //帧率 x frames/s
    cout<<"frameRate:"<<frameRate<<endl;
    int ex = static_cast<int>(cap.get(cv::CAP_PROP_FOURCC));
    char EXT[] = { (char)(ex & 0XFF),(char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24),0 };
    cv::VideoWriter wri;
    wri.open("1_result.mp4", ex, frameRate, cv::Size(width, height));


    cv::Mat frame;
    // cv::Mat frame=cv::imread("dog.jpg");
    double inferenceTime = 0.0;
    int n_frame=0;
    cap >> frame;
    M2::ObjectInfo objectinfo;
    M2_FaceDetect_ForwardBGR(frame,objectinfo,0);

    while (1) {
        cap >> frame;
        if (frame.empty())
            break;
        // if (n_frame>20)
        //     break;
        double t1 = static_cast<double>(cv::getTickCount());

        M2::ObjectInfo objectinfo;
        M2_FaceDetect_ForwardBGR(frame,objectinfo,0);

        double t2 = static_cast<double>(cv::getTickCount());
        if (inferenceTime == 0){
            inferenceTime = (t2 - t1) / cv::getTickFrequency() * 1000;
        }
        else 
        {
            inferenceTime = inferenceTime * 0.95 + 0.05 * (t2 - t1) / cv::getTickFrequency() * 1000;
        }

        std::stringstream fpsSs;
        fpsSs << "FPS: " << int(1000.0f / inferenceTime * 100) / 100.0f;
        cv::putText(frame, fpsSs.str(), cv::Point(16, 32),cv::FONT_HERSHEY_COMPLEX, 0.8, cv::Scalar(0, 0, 255));


        for(int i=0;i<objectinfo.ObjectNum;i++)
        {
            std::string text =std::to_string(int(objectinfo.objects[i].prob*100));
            int font_face = cv::FONT_HERSHEY_COMPLEX;
            double font_scale = 0.8;
            int thickness = 1;
        //    int baseline;
        //    cv::Size text_size = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);
            // 将文本框居中绘制
            cv::Point origin;
            origin.x = objectinfo.objects[i].rect.x+20;
            origin.y = objectinfo.objects[i].rect.y+20;
            cv::putText(frame, text, origin, font_face, font_scale, cv::Scalar(0, 255, 255), thickness, 8, 0);
            // cv::Rect r = cv::Rect(objectinfo.objects[i].rect.x, objectinfo.objects[i].rect.y, objectinfo.objects[i].rect.width, rectinfo.boxes[i].height);
            cv::rectangle(frame, objectinfo.objects[i].rect, cv::Scalar(255, 0, 0), 1, 8, 0);
        }
  
        


        // FaceDetect_visImg(frame,objectinfo);
        wri << frame;
        // cv::imshow("11",frame);
        // cv::waitKey(0);
        n_frame++;
    } 

    // cv::Mat image=cv::imread("l1.jpg");
   
    return 0;
}


int Check_FaceLandmark_video()
{
    cv::VideoCapture cap;
    cap.open("1.mp4");
    if (!cap.isOpened())
    {
        cout<<"video failed!"<<endl;
        return -1;
    }
    int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);             //帧宽度
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);           //帧高度
    int totalFrames = cap.get(cv::CAP_PROP_FRAME_COUNT);       //总帧数
    int frameRate = cap.get(cv::CAP_PROP_FPS);                 //帧率 x frames/s
    cout<<"frameRate:"<<frameRate<<endl;
    int ex = static_cast<int>(cap.get(cv::CAP_PROP_FOURCC));
    char EXT[] = { (char)(ex & 0XFF),(char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24),0 };
    cv::VideoWriter wri;
    wri.open("1_result_2.mp4", ex, frameRate, cv::Size(width, height));


    cv::Mat frame;
    // cv::Mat frame=cv::imread("dog.jpg");
    double inferenceTime = 0.0;
    int n_frame=0;
    cap >> frame;
    M2::LandmarkInfo landmarkinfo;
    M2::ObjectInfo objectinfo;
    M2_FaceAlignment_ForwardBGR_MaxFace(frame,objectinfo,landmarkinfo);

    while (1) {
        cap >> frame;
        if (frame.empty())
            break;
        // if (n_frame>20)
        //     break;
        double t1 = static_cast<double>(cv::getTickCount());

        M2::LandmarkInfo landmarkinfo;
        M2::ObjectInfo objectinfo;
        M2_FaceAlignment_ForwardBGR_MaxFace(frame,objectinfo,landmarkinfo);

        double t2 = static_cast<double>(cv::getTickCount());
        if (inferenceTime == 0){
            inferenceTime = (t2 - t1) / cv::getTickFrequency() * 1000;
        }
        else 
        {
            inferenceTime = inferenceTime * 0.95 + 0.05 * (t2 - t1) / cv::getTickFrequency() * 1000;
        }

        std::stringstream fpsSs;
        fpsSs << "FPS: " << int(1000.0f / inferenceTime * 100) / 100.0f;
        cv::putText(frame, fpsSs.str(), cv::Point(16, 32),cv::FONT_HERSHEY_COMPLEX, 0.8, cv::Scalar(0, 0, 255));

        for(int i=0;i<objectinfo.ObjectNum;i++)
        {
            std::string text =std::to_string(int(objectinfo.objects[i].prob*100));
            int font_face = cv::FONT_HERSHEY_COMPLEX;
            double font_scale = 0.8;
            int thickness = 1;
        //    int baseline;
        //    cv::Size text_size = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);
            // 将文本框居中绘制
            cv::Point origin;
            origin.x = objectinfo.objects[i].rect.x+20;
            origin.y = objectinfo.objects[i].rect.y+20;
            cv::putText(frame, text, origin, font_face, font_scale, cv::Scalar(0, 255, 255), thickness, 8, 0);
            // cv::Rect r = cv::Rect(objectinfo.objects[i].rect.x, objectinfo.objects[i].rect.y, objectinfo.objects[i].rect.width, rectinfo.boxes[i].height);
            cv::rectangle(frame, objectinfo.objects[i].rect, cv::Scalar(255, 0, 0), 1, 8, 0);
        }


        if (objectinfo.ObjectNum>0)
        {
            
            for(int i=0;i<landmarkinfo.numPoints;i++)
            {
                // cout<<i<<": "<<landmarkinfo.landmark[i].x<<landmarkinfo.landmark[i].y<<endl;
                cv::Point p1(landmarkinfo.landmark[i].x ,landmarkinfo.landmark[i].y);
                cv::circle(frame, p1, 1, cv::Scalar(0, 255, 0), -1); 
            }
        }
        


        // FaceDetect_visImg(frame,objectinfo);
        wri << frame;
        // cv::imshow("11",frame);
        // cv::waitKey(0);
        n_frame++;
    } 

    // cv::Mat image=cv::imread("l1.jpg");
   
    return 0;
}





int main(int argc, char *argv[])
{
    
    auto handle = dlopen("libMNN_CL.so", RTLD_NOW);
    Check_LaneDetet_video();
    // Check_ObjectDetect_video();
    // Check_FaceDetect_video();
    // Check_FaceLandmark_video();

    // for(int i=0;i<10;i++)
    // {
    //     Check_LaneDetet();
    // }

    // {
    //         M2::LandmarkInfo landmarks;
    // M2_FaceAlignment_ForwardBGR_MaxFace(image,landmarks);
    // FR_Show_PTS(image,landmarks);
    // }


    // cv::Mat image;
    // cv::resize(raw_image, image, cv::Size(240, 320));

    // M2::DetectResult result;
    // M2_ObjectDetect_ForwardBGR(image,result);




    // std::vector<M2::lane_DECODE> final_lane;
    // M2_LaneDetect_ForwardBGR(image,final_lane);

    // while(1)
    // {
    //     M2::ObjectInfo objectinfo;
    //     M2_ObjectDetect_ForwardBGR(image,objectinfo);

    //     FaceDetect_visImg(image,objectinfo);
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
