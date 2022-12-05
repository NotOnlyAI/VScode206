#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <math.h>
#include <sys/time.h>

#include "ModelManager.h"
#include <opencv2/opencv.hpp>


const int DEF_THREAD_STACK_SIZE = 262144;   // 256 * 1024 

int32_t ModelManager::init()
{




    img1=cv::imread("1.jpg");   
    img2=cv::imread("1.jpg");  
    img3=cv::imread("l1.jpg");   

    FaceDetectModel=std::make_shared<FaceDetect>();
    FaceAlignmentModel=std::make_shared<FaceAlignment>();
    LaneDetectModel=std::make_shared<LaneDetect>();


    return 0;
}



int32_t ModelManager::CreateThreads()
{
    pthread_attr_t attr;
    (void)pthread_attr_init(&attr);
    (void)pthread_attr_setstacksize(&attr, DEF_THREAD_STACK_SIZE);  // ???????????256KB
    (void)pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // if ((pthread_create(&m_tidpFaceDetect, &attr, ModelManager::FaceDetectThread, (void *)this)) == -1) {
    //     pthread_attr_destroy(&attr);
    //     MNN_PRINT("Create the thread:FaceDetect() failed\n");
    //     return -1;
    // }
    // // pthread_attr_destroy(&attr);
    // MNN_PRINT("Create the thread:FaceDetect() successfully\n");


    // if ((pthread_create(&m_tidpFaceAlignment, &attr, ModelManager::FaceAlignmentThread, (void *)this)) == -1) {
    //     pthread_attr_destroy(&attr);
    //     MNN_PRINT("Create the thread:FaceAlignment() failed\n");
    //     return -1;
    // }
    // // pthread_attr_destroy(&attr);
    // MNN_PRINT("Create the thread:FaceAlignment() successfully\n");


    if ((pthread_create(&m_tidpLaneDetect, &attr, ModelManager::LaneDetectThread, (void *)this)) == -1) {
        pthread_attr_destroy(&attr);
        MNN_PRINT("Create the thread:LaneDetect() failed\n");
        return -1;
    }
    pthread_attr_destroy(&attr);
    MNN_PRINT("Create the thread:LaneDetect() successfully\n");

    return 0;
}


void* ModelManager::FaceDetectThread(void *arg)
{
    ModelManager *thiz = static_cast<ModelManager *>(arg);
    if (thiz == nullptr) {
        return nullptr;
    }
    while (1)
    {
        // MNN_PRINT("FaceDetectThread\n");
        thiz->FaceDetectModel->Forward(thiz->img1);
        // sleep(10);
    }

    
    return nullptr;
}

void* ModelManager::FaceAlignmentThread(void *arg)
{
    ModelManager *thiz = static_cast<ModelManager *>(arg);
    if (thiz == nullptr) {
        return nullptr;
    }
    while (1)
    {
        // MNN_PRINT("FaceAlignmentThread\n");
        thiz->FaceAlignmentModel->Forward(thiz->img2);
        // sleep(5);
    }

    
    return nullptr;
}


void* ModelManager::LaneDetectThread(void *arg)
{
    ModelManager *thiz = static_cast<ModelManager *>(arg);
    if (thiz == nullptr) {
        return nullptr;
    }
    while (1)
    {
        // MNN_PRINT("LaneDetectThread\n");
        thiz->LaneDetectModel->Forward(thiz->img3);
        // sleep(5);
    }

    
    return nullptr;
}

int32_t ModelManager::RunTurn()
{
    while (1)
    {
        FaceDetectModel->Forward(img1);
        FaceAlignmentModel->Forward(img2);
        LaneDetectModel->Forward(img3);
    }
    return 0;

}
