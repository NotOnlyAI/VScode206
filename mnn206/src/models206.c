#include <iostream>



#include "models206.h"


#include "M2utils/config.h"
#include "M2utils/minimal_logging.h"
#include "FaceDetect/FaceDetect.hpp"
#include "LaneDetect/LaneDetect.hpp"



using namespace std;


bool logfile_ok;


FaceDetect myFaceDetect;
LaneDetect myLaneDetect;


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
        int deviceTpye = config.ReadInt("FaceDetect", "deviceTpye", 0);
        
        myFaceDetect.init(deviceTpye,print_config);
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



int M2_FaceDetect(const M2::ImgData_T &imgdata,M2::DetectResult &rectinfo)
{
    
    
    int print_config=M2_FaceDetect_Init(myFaceDetect);
    if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO,"M2_FaceDetect Start! ");}
    
    myFaceDetect.Forward(imgdata,rectinfo);

    #ifdef CHECKSHOW
    myFaceDetect.visImg(imgdata,rectinfo);
    #endif

    if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO, "M2_FaceDetect End! ");}


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
        int deviceTpye = config.ReadInt("LaneDetect", "deviceTpye", 0);
        
        myLaneDetect.init(deviceTpye,print_config);
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


int M2_LaneDetect(const M2::ImgData_T &imgdata,std::vector<M2::lane_DECODE> &final_lane)
{
    int print_config=M2_LaneDetect_Init(myLaneDetect);
    if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO,"M2_LaneDetect Start! ");}

    myLaneDetect.Forward(imgdata,final_lane);

    #ifdef CHECKSHOW
    myLaneDetect.visImg(imgdata,rectinfo);
    #endif


    if(print_config>=0){TFLITE_LOG(tflite::TFLITE_LOG_INFO, "M2_LaneDetect End! ");}


}