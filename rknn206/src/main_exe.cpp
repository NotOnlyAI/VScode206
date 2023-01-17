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

#include "models206.h"

using namespace std;



int ObjectD_demo()
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
        if (n_frame>100)
            break;
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


int Lane_demo()
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
    wri.open("video_0_lane.mp4", ex, frameRate, cv::Size(width, height));


    cv::Mat frame;

    double inferenceTime = 0.0;
    int n_frame=0;
    while (1) {
        cap >> frame;
        if (frame.empty())
            break;
        if (n_frame>200)
            break;
        cout<<"n_frame:"<<n_frame<<endl;
        double t1 = static_cast<double>(cv::getTickCount());

        int DepartureType;
        std::vector<M2::lane_DECODE> final_lane;
        M2_Lane(frame,final_lane,DepartureType);

        n_frame++;
        if (n_frame<10) continue;

        double t2 = static_cast<double>(cv::getTickCount());
        if (inferenceTime == 0){
            inferenceTime = (t2 - t1) / cv::getTickFrequency() * 1000;
        }
        else 
        {
            inferenceTime = inferenceTime * 0.95 + 0.05 * (t2 - t1) / cv::getTickFrequency() * 1000;
        }

        std::stringstream fpsSs;
        fpsSs << "FPS: " << int(1000.0f / inferenceTime * 100) / 100.0f<<"     Warning: "<< DepartureType;
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

        wri << frame;;
        
    } 

}


int DMS_demo()
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
    wri.open("1_DMS.avi", ex, frameRate, cv::Size(width, height));
    if(!wri.isOpened())
    {
        cout<<"video writer failed!"<<endl;
        return -1;
    }



    cv::Mat frame;
    // cv::Mat frame=cv::imread("dog.jpg");
    double inferenceTime = 0.0;
    int n_frame=0;

    while (1) {
        cap >> frame;
        if (frame.empty())
            break;
        if (n_frame>300)
            break;
        double t1 = static_cast<double>(cv::getTickCount());

        int DMSType;
        M2::LandmarkInfo landmarkinfo;
        M2::ObjectInfo objectinfo;
        M2_DMS(frame,objectinfo,landmarkinfo,DMSType);
        

        n_frame++;
        cout<<"n_frame:"<<n_frame<<endl;
        if(n_frame<10) continue;

        double t2 = static_cast<double>(cv::getTickCount());
        if (inferenceTime == 0){
            inferenceTime = (t2 - t1) / cv::getTickFrequency() * 1000;
        }
        else 
        {
            inferenceTime = inferenceTime * 0.95 + 0.05 * (t2 - t1) / cv::getTickFrequency() * 1000;
        }

        std::stringstream fpsSs;
        fpsSs << "FPS: " << int(1000.0f / inferenceTime * 100) / 100.0f<<"   DMSType:"<<DMSType;
        cv::putText(frame, fpsSs.str(), cv::Point(16, 32),cv::FONT_HERSHEY_COMPLEX, 0.8, cv::Scalar(0, 0, 255));

    
        if (objectinfo.ObjectNum>0)
        {
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

            for(int i=0;i<landmarkinfo.numPoints;i++)
            {
                // cout<<i<<": "<<landmarkinfo.landmark[i].x<<landmarkinfo.landmark[i].y<<endl;
                cv::Point p1(landmarkinfo.landmark[i].x ,landmarkinfo.landmark[i].y);
                cv::circle(frame, p1, 1, cv::Scalar(0, 255, 0), -1); 
            }
        }


        wri << frame;
        cout<<"DMSType:"<<DMSType<<endl;
    } 

    return 0;
}




int main(int argc, char *argv[])
{
    
    Lane_demo();
    // DMS_demo();

    return 0;
}
