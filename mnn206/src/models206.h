#ifndef _FD_FACEIR_GPU_H_
#define _FD_FACEIR_GPU_H_


#include "models206_typedef.h"

#ifdef _MSC_VER
#define M2_EXPORT __declspec(dllexport)
#else
#define M2_EXPORT __attribute__ ((visibility("default")))
#endif

#define M2_C_EXPORT extern "C" M2_EXPORT




M2_EXPORT int M2_FaceDetect(const M2::ImgData_T &imgdata,M2::DetectResult &result,int max_or_mid);


M2_EXPORT int M2_ObjectDetect_ForwardBGR(const cv::Mat &image,M2::DetectResult &result);


M2_EXPORT int M2_LaneDetect_ForwardBGR(const cv::Mat &image,std::vector<M2::lane_DECODE> &final_lane);


M2_EXPORT int M2_FaceAlignment(const M2::ImgData_T &imgdata,M2::Box cropBox,M2::LandmarkInfo &landmarkinfo);



#endif

