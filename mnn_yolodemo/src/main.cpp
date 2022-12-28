

#include <iostream>
#include <opencv2/opencv.hpp>
#include <dlfcn.h>
#include <unistd.h>

#include "ObjectDetect/ObjectDetect.hpp"
#include "models206_typedef.h"

using namespace std;

int VisImg(const cv::Mat &ori_image,const M2::ObjectInfo &objectinfo)
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


int main(int argc, char *argv[])
{
    
    auto handle = dlopen("libMNN_CL.so", RTLD_NOW);
    cv::Mat image=cv::imread("dog.jpg");


    
    ObjectDetect myObjectDetect;
    myObjectDetect.init(3,1,1);


    for(int i=0;i<10;i++)
    {
        M2::ObjectInfo objectinfo;
        myObjectDetect.ForwardBGR(image,objectinfo);
        VisImg(image,objectinfo);
        sleep(1);
    }


    return 0;
}
