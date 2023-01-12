#include <iostream>



#include "models206.h"


#include "M2utils/config.h"
#include "M2utils/minimal_logging.h"
#include "FaceDetect/FaceDetect.hpp"
#include "LaneDetect/LaneDetect.hpp"
#include "LaneDetect/LDW.h"
#include "FaceAlignment/FaceAlignment.hpp"
#include "ObjectDetect/ObjectDetect.hpp"

using namespace std;


bool logfile_ok;


FaceDetect myFaceDetect;
LaneDetect myLaneDetect;
FaceAlignment myFaceAlignment;
ObjectDetect myObjectDetect;
LDW myLDW;

// int M2_Log_Init(){
//     tflite::initLogger();
//     logfile_ok=true;
//     return 0;
// }



int M2_FaceDetect_Init(FaceDetect &myFaceDetect)
{
    if(!myFaceDetect.model_is_ok)
    {
        // if(!logfile_ok) M2_Log_Init();
        fd::RrConfig config;
	    config.ReadConfig("./models206/config.ini");
        int print_config = config.ReadInt("FaceDetect", "print", -1);
        int deviceTpye = config.ReadInt("FaceDetect", "deviceType", 0);
        int modelType=config.ReadInt("FaceDetect", "modelType", 0);
        
        myFaceDetect.init(deviceTpye,print_config,modelType);
        myFaceDetect.model_is_ok=true;
        TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myFaceDetect Init ok! ");
        return print_config;

    }else{
        fd::RrConfig config;
	    config.ReadConfig("./models206/config.ini");
        int print_config = config.ReadInt("FaceDetect", "print", -1);
        TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myFaceDetect is Already Init");
        return print_config;
    }
}



int M2_FaceDetect_ForwardBGR(const cv::Mat &image,M2::ObjectInfo &objectinfo,int max_or_mid=0)
{
 
    int print_config=M2_FaceDetect_Init(myFaceDetect);
    if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO,"M2_FaceDetect Start! ");}
    
    myFaceDetect.ForwardBGR(image,objectinfo,max_or_mid);


    if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO, "M2_FaceDetect End!\n");}
    return 0;
}

int M2_LaneDetect_Init(LaneDetect &myLaneDetect)
{
   if(!myLaneDetect.model_is_ok)
   {
       // if(!logfile_ok) M2_Log_Init();
       fd::RrConfig config;
	   config.ReadConfig("./models206/config.ini");
       int print_config = config.ReadInt("LaneDetect", "print", -1);
       int deviceTpye = config.ReadInt("LaneDetect", "deviceType", 0);
       int modelType=config.ReadInt("LaneDetect", "modelType", 0);

       myLaneDetect.init(deviceTpye,print_config,modelType);
       myLaneDetect.model_is_ok=true;
       TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myLaneDetect Init ok! ");
       return print_config;

   }else{
       fd::RrConfig config;
	    config.ReadConfig("./models206/config.ini");
       int print_config = config.ReadInt("LaneDetect", "print", -1);
       TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myLaneDetect is Already Init");
       return print_config;
   }
}


int M2_LaneDetect_ForwardBGR(const cv::Mat &image,std::vector<M2::lane_DECODE> &final_lane)
{
   int print_config=M2_LaneDetect_Init(myLaneDetect);
   if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO,"M2_LaneDetect Start! ");}

   myLaneDetect.ForwardBGR(image,final_lane);

   #ifdef CHECKSHOW
   myLaneDetect.visImg(imgdata,final_lane);
   #endif


   if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO, "M2_LaneDetect End!\n");}


}


int M2_FaceAlignment_Init(FaceAlignment &myFaceAlignment)
{
   if(!myFaceAlignment.model_is_ok)
   {
       // if(!logfile_ok) M2_Log_Init();
       fd::RrConfig config;
	    config.ReadConfig("./models206/config.ini");
       int print_config = config.ReadInt("FaceAlignment", "print", -1);
       int deviceTpye = config.ReadInt("FaceAlignment", "deviceType", 0);
       int modelType=config.ReadInt("FaceAlignment", "modelType", 0);

       myFaceAlignment.init(deviceTpye,print_config,modelType);
       myFaceAlignment.model_is_ok=true;
       TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myFaceAlignment Init ok! ");
       return print_config;

   }else{
       fd::RrConfig config;
	    config.ReadConfig("./models206/config.ini");
       int print_config = config.ReadInt("FaceAlignment", "print", -1);
       TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myFaceAlignment is Already Init");
       return print_config;
   }
}


int M2_FaceAlignment_ForwardBGR(const cv::Mat &image,const M2::Object &face,M2::LandmarkInfo &landmarkinfo)
{

   int print_config=M2_FaceAlignment_Init(myFaceAlignment);
   if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO,"M2_FaceAlignment Start! ");}

   myFaceAlignment.ForwardBGR(image,face,landmarkinfo);

   if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO, "M2_FaceAlignment End!\n");}

}


int M2_ObjectDetect_Init(ObjectDetect &myObjectDetect)
{
   if(!myObjectDetect.model_is_ok)
   {
       // if(!logfile_ok) M2_Log_Init();
        fd::RrConfig config;
	    config.ReadConfig("./models206/config.ini");
        int print_config = config.ReadInt("ObjectDetect", "print", -1);
        int deviceTpye = config.ReadInt("ObjectDetect", "deviceType", 0);
        int modelType=config.ReadInt("ObjectDetect", "modelType", 0);

        myObjectDetect.init(deviceTpye,print_config,modelType);
        myObjectDetect.model_is_ok=true;
        TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myObjectDetect Init ok! ");
        return print_config;

   }else{
        fd::RrConfig config;
	    config.ReadConfig("./models206/config.ini");
        int print_config = config.ReadInt("ObjectDetect", "print", -1);
        TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myObjectDetect is Already Init");
        return print_config;
   }
}


int M2_ObjectDetect_ForwardBGR(const cv::Mat &image,M2::ObjectInfo &objectinfo)
{

   int print_config=M2_ObjectDetect_Init(myObjectDetect);
   if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO,"M2_ObjectDetect Start! ");}

   myObjectDetect.ForwardBGR(image,objectinfo);

   if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO, "M2_ObjectDetect End!\n");}

}


int M2_FaceAlignment_ForwardBGR_MaxFace(const cv::Mat &image,M2::ObjectInfo &objectinfo,M2::LandmarkInfo &landmarkinfo)
{
    M2_FaceDetect_ForwardBGR(image,objectinfo,0);
    if(objectinfo.ObjectNum>=1)
    {
        M2_FaceAlignment_ForwardBGR(image,objectinfo.objects[0],landmarkinfo);
    }

    return 0;
    // FR_Show_PTS(image,landmarks);

}



int M2_Lane_Init(LaneDetect &myLaneDetect)
{
   if(!myLaneDetect.model_is_ok)
   {
       // if(!logfile_ok) M2_Log_Init();
       fd::RrConfig config;
	   config.ReadConfig("./models206/config.ini");
       int print_config = config.ReadInt("LaneDetect", "print", -1);
       int deviceTpye = config.ReadInt("LaneDetect", "deviceType", 0);
       int modelType=config.ReadInt("LaneDetect", "modelType", 0);

       myLaneDetect.init(deviceTpye,print_config,modelType);
       myLaneDetect.model_is_ok=true;
       TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myLaneDetect Init ok! ");

       myLDW.Init(20,720+200);
       TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myLDW Init ok! ");

       return print_config;

   }else{
       fd::RrConfig config;
	    config.ReadConfig("./models206/config.ini");
       int print_config = config.ReadInt("LaneDetect", "print", -1);
       TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myLaneDetect is Already Init");
       return print_config;
   }
}



int M2_Lane(const cv::Mat &image,int &DepartureType)
{
    int print_config=M2_Lane_Init(myLaneDetect);
    if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO,"M2_LaneDetect Start! ");}
    std::vector<M2::lane_DECODE> final_lane;
    myLaneDetect.ForwardBGR(image,final_lane);


    DepartureType = myLDW.Run(final_lane, image);
    cout<<"DepartureType: "<<DepartureType<<endl;

    if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO, "M2_LaneDetect End!\n");}

    return 0;
}


int M2_DMS(const cv::Mat &image,int &DMSTpye)
{
    M2::ObjectInfo objectinfo;
    M2_FaceDetect_ForwardBGR(image,objectinfo,0);
    float r=1;

    if(objectinfo.ObjectNum>=1)
    {
        M2::LandmarkInfo landmarks;

        M2_FaceAlignment_ForwardBGR(image,objectinfo.objects[0],landmarks);
        float dx1=landmarks.landmark[60].x-landmarks.landmark[64].x;
        float dy1=landmarks.landmark[60].y-landmarks.landmark[64].y;
        float leye_left=sqrt(dx1*dx1+dy1*dy1);


        float dx2=landmarks.landmark[62].x-landmarks.landmark[66].x;
        float dy2=landmarks.landmark[62].y-landmarks.landmark[66].y;
        float weye_left=sqrt(dx2*dx2+dy2*dy2);

        float dx3=landmarks.landmark[68].x-landmarks.landmark[72].x;
        float dy3=landmarks.landmark[68].y-landmarks.landmark[72].y;
        float leye_right=sqrt(dx3*dx3+dy3*dy3);


        float dx4=landmarks.landmark[70].x-landmarks.landmark[74].x;
        float dy4=landmarks.landmark[70].y-landmarks.landmark[74].y;
        float weye_right=sqrt(dx4*dx4+dy4*dy4);

        float r_left=weye_left/leye_left;
        float r_right=weye_right/leye_right;

        r=0.5*(r_left+r_right);

    }
    

    if(r<0.1)
    {
        DMSTpye=1;
    }
    else
    {
        DMSTpye=0;
    }


    return 0;
    // FR_Show_PTS(image,landmarks);

}