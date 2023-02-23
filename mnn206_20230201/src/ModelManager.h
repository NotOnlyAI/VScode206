
#ifndef __ModelManager_APP_H__
#define __ModelManager_APP_H__

#include <string>
#include <memory>
#include <ctime>
#include "MNN/MNNDefine.h"


#include "FaceAlignment/FaceAlignment.hpp"
#include "FaceDetect/FaceDetect.hpp"
#include "LaneDetect/LaneDetect.hpp"
#include "DataSrc/DataSrc.h"



class ModelManager
{
    public:
    int32_t init();
    int32_t CreateThreads();
    int32_t RunTurn();


    private:
    std::shared_ptr<DataSrc> m_dataSrc; 



    static void*FaceDetectThread(void *arg);
    static void*FaceAlignmentThread(void *arg);
    static void*LaneDetectThread(void *arg);


    pthread_t m_tidpFaceDetect;
    pthread_t m_tidpFaceAlignment;
    pthread_t m_tidpLaneDetect;


    std::shared_ptr<FaceDetect> FaceDetectModel;
    std::shared_ptr<FaceAlignment> FaceAlignmentModel;
    std::shared_ptr<LaneDetect> LaneDetectModel;


    cv::Mat img1;
    cv::Mat img2;
    cv::Mat img3;
       
};
 





#endif /* __ModelManager_H__ */