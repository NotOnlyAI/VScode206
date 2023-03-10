#include <iostream>



#include "models206.h"


#include "M2utils/config.h"
#include "M2utils/minimal_logging.h"


#include "Face/FaceDetect/FaceDetect.hpp"
#include "Face/FaceAlignment/FaceAlignment.hpp"
#include "Face/EyeLandmarks/EyeLandmarks.h"
#include "Face/EyeState/EyeState.h"


#include "LaneDetect/LaneDetect.hpp"
#include "LaneDetect/LDW.h"
#include "ObjectDetect/ObjectDetect.hpp"


using namespace std;


bool logfile_ok;


FaceDetect myFaceDetect;
LaneDetect myLaneDetect;
FaceAlignment myFaceAlignment;
EyeLandmarks myEyeLandmarks;

EyeState myEyeState;

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
        int printConfig = config.ReadInt("FaceDetect", "print", -1);
        int modelType=config.ReadInt("FaceDetect", "modelType", 0);
        float scoreThresh = config.ReadFloat("FaceDetect", "scoreThresh ", 0.95);
        
        myFaceDetect.Init(printConfig,modelType,scoreThresh);
        myFaceDetect.model_is_ok=true;
        TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myFaceDetect Init ok! ");
        return printConfig;

    }else{
        fd::RrConfig config;
	    config.ReadConfig("./models206/config.ini");
        int printConfig = config.ReadInt("FaceDetect", "print", -1);
        TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myFaceDetect is Already Init");
        return printConfig;
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

       myLaneDetect.Init(deviceTpye,print_config,modelType);
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
       float ratio_eye=config.ReadFloat("FaceAlignment", "ratio_eye", 0.2);
       float ratio_mouth=config.ReadFloat("FaceAlignment", "ratio_mouth", 0.6);

       myFaceAlignment.init(deviceTpye,print_config,modelType,ratio_eye,ratio_mouth);
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


int M2_ObjectDetect_Init(ObjectDetect &myObjectDetect,int batch=1)
{
   if(!myObjectDetect.model_is_ok)
   {
       // if(!logfile_ok) M2_Log_Init();
        fd::RrConfig config;
	    config.ReadConfig("./models206/config.ini");
        int printConfig = config.ReadInt("ObjectDetect", "print", -1);
        int modelType=config.ReadInt("ObjectDetect", "modelType", 0);

        myObjectDetect.InitWithBatch(printConfig,modelType,batch);
        myObjectDetect.model_is_ok=true;
        TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myObjectDetect Init ok! ");
        return printConfig;
   }else{
        fd::RrConfig config;
	    config.ReadConfig("./models206/config.ini");
        int printConfig = config.ReadInt("ObjectDetect", "print", -1);
        TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myObjectDetect is Already Init");
        return printConfig;
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

       myLaneDetect.Init(deviceTpye,print_config,modelType);
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







int M2_EyeLandmarks_Init(EyeLandmarks &myEyeLandmarks)
{
   if(!myEyeLandmarks.model_is_ok)
   {
       // if(!logfile_ok) M2_Log_Init();
       fd::RrConfig config;
	    config.ReadConfig("./models206/config.ini");
       int print_config = config.ReadInt("EyeLandmarks", "print", -1);
       int deviceTpye = config.ReadInt("EyeLandmarks", "deviceType", 0);
       int modelType=config.ReadInt("EyeLandmarks", "modelType", 0);
       float ratio_eye=config.ReadFloat("EyeLandmarks", "ratio_eye", 0.2);
       float ratio_mouth=config.ReadFloat("EyeLandmarks", "ratio_mouth", 0.6);

       myEyeLandmarks.init(deviceTpye,print_config,modelType,ratio_eye,ratio_mouth);
       myEyeLandmarks.model_is_ok=true;
       TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myEyeLandmarks Init ok! ");
       return print_config;

   }else{
       fd::RrConfig config;
	    config.ReadConfig("./models206/config.ini");
       int print_config = config.ReadInt("EyeLandmarks", "print", -1);
       TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myEyeLandmarks is Already Init");
       return print_config;
   }
}


int M2_EyeLandmarks_ForwardBGR(const cv::Mat &image,const M2::Point2f &left_eye,const M2::Point2f &right_eye,M2::LandmarkInfo &landmarkinfo)
{

   int print_config=M2_EyeLandmarks_Init(myEyeLandmarks);
   if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO,"M2_EyeLandmarks Start! ");}

   myEyeLandmarks.ForwardBGR(image,left_eye,right_eye,landmarkinfo);

   if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO, "M2_EyeLandmarks End!\n");}

}

















int M2_EyeState_Init(EyeState &myEyeState)
{
   if(!myEyeState.model_is_ok)
   {
       // if(!logfile_ok) M2_Log_Init();
        fd::RrConfig config;
	    config.ReadConfig("./models206/config.ini");
        int printConfig = config.ReadInt("EyeState", "print", -1);
        int deviceTpye = config.ReadInt("EyeState", "deviceType", 0);
        int modelType=config.ReadInt("EyeState", "modelType", 0);

        myEyeState.Init(deviceTpye,printConfig,modelType);
        myEyeState.model_is_ok=true;
        TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myEyeState Init ok! ");
        return printConfig;
   }else{
        fd::RrConfig config;
	    config.ReadConfig("./models206/config.ini");
        int printConfig = config.ReadInt("EyeState", "print", -1);
        TFLITE_LOG_ONCE(tflite::TFLITE_LOG_INFO,"myEyeState is Already Init");
        return printConfig;
   }
}


int M2_EyeState_ForwardBGR(const cv::Mat &image,const cv::Rect &left_rect,const cv::Rect &right_rect,int &eyestate)
{

   int print_config=M2_EyeState_Init(myEyeState);
   if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO,"M2_EyeState Start! ");}

   myEyeState.ForwardBGR(image,left_rect,right_rect,eyestate);

   if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO, "M2_EyeState End!\n");}

}





int M2_Lane(const cv::Mat &image,std::vector<M2::lane_DECODE> &final_lane,int &LaneType)
{
    cv::Mat bgr_image;
    if (image.channels()==1)
    {
        cv::cvtColor(image,bgr_image,cv::COLOR_GRAY2BGR);
    }
    else{
        bgr_image=image;
    }

    int print_config=M2_Lane_Init(myLaneDetect);
    if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO,"M2_LaneDetect Start! ");}

    myLaneDetect.ForwardBGR(bgr_image,final_lane);
    LaneType = myLDW.Run(final_lane, bgr_image);

    if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO, "M2_LaneDetect End!\n");}

    return 0;
}


int M2_DMS(const cv::Mat &image,M2::ObjectInfo &objectinfo,M2::LandmarkInfo &landmarkinfo,M2::DMSState &dms)
{
    cv::Mat bgr_image;
    if (image.channels()==1)
    {
        cv::cvtColor(image,bgr_image,cv::COLOR_GRAY2BGR);
    }
    else{
        bgr_image=image;
    }

    M2_FaceDetect_ForwardBGR(bgr_image,objectinfo,0);
    int eye_state=0;
    int mouth_state=0;
    int pose_state=0;
    if(objectinfo.ObjectNum>=1)
    {   
        M2_FaceAlignment_ForwardBGR(bgr_image,objectinfo.objects[0],landmarkinfo);
        myFaceAlignment.mouthJudge(landmarkinfo,mouth_state);
        myFaceAlignment.poseJudge(pose_state);
        dms.mouth_state=mouth_state;
        dms.face_state=pose_state;
        M2_EyeState_ForwardBGR(bgr_image,myFaceAlignment.m_left_rect,myFaceAlignment.m_right_rect,eye_state);
        dms.eye_state=eye_state;
    }else
    {
        dms.face_state=1;
    }
    
    return 0;
}



int M2_ObjectDetect_ForwardBGR_BATCH(const std::vector<cv::Mat> &batch_images,std::vector<M2::ObjectInfo> &batch_objectinfos)
{
    int batch=batch_images.size();
    if(batch<0)
    {
        std::cout<<"wrong batch <0"<<std::endl;
    }
    int printConfig=M2_ObjectDetect_Init(myObjectDetect,batch);
    if(printConfig>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO,"M2_ObjectDetect Start! ");}

    myObjectDetect.ForwardBatchBGR(batch_images,batch_objectinfos);

    if(printConfig>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO, "M2_ObjectDetect End!\n");}

}




int M2_ObjectDetect_ForwardBGR_BIGIMAGE(const cv::Mat &image,M2::ObjectInfo &objectinfo,int nrows, int ncols)
{

    int col = image.cols;
    int row = image.rows;
    int pixrows = row /  nrows;
    int pixcols = col /  ncols;

    std::cout<<pixrows<<""<<pixcols<<std::endl;

    std::vector<cv::Mat> batch_images;
    batch_images.resize(nrows*ncols);
    for (int i = 0; i <  nrows; i++)
	{
        for(int j=0;j<ncols;j++)
        {
            cv::Rect rect(j*pixcols,i*pixrows, pixcols, pixrows);
            batch_images.at(i*ncols+j)=image(rect);
        }
	}

    // for (int i = 0; i < batch_images.size(); i++)
	// {
    //    cv::imshow("11",batch_images.at(i));
    //    cv::waitKey(0);
	// }

    std::vector<M2::ObjectInfo> batch_objectinfos;
    M2_ObjectDetect_ForwardBGR_BATCH(batch_images,batch_objectinfos);

    int n=0;
    int rw,rh;
    
    for (int i = 0; i < batch_images.size(); i++)
	{
        if(i<3){
            rh=0;
            rw=i*pixcols;
        }
        else{
            rh=pixrows;
            rw=(i-3)*pixcols;
        }
        // cout<<rh<<","<<rw<<endl;


        for (int j = 0; j < batch_objectinfos.at(i).ObjectNum; j++)
        {
            cv::Rect rect2;
            rect2.x=batch_objectinfos.at(i).objects[j].rect.x+rw;
            rect2.y=batch_objectinfos.at(i).objects[j].rect.y+rh;
            rect2.width=batch_objectinfos.at(i).objects[j].rect.width;
            rect2.height=batch_objectinfos.at(i).objects[j].rect.height;

            cout<<i<<":"<<j<<":"<<rect2.x<<","<<rect2.y<<","<<rect2.width<<","<<rect2.height<<","<<endl;

            objectinfo.objects[n].rect=rect2;
            objectinfo.objects[n].prob=batch_objectinfos.at(i).objects[j].prob;
            objectinfo.objects[n].label=batch_objectinfos.at(i).objects[j].label;
            n++;
        }
        
	}
    objectinfo.ObjectNum=n;
    return 0;


}

