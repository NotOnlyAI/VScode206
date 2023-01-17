#ifndef _FD_FACEIR_GPU_H_
#define _FD_FACEIR_GPU_H_


#include "models206_typedef.h"

#ifdef _MSC_VER
#define M2_EXPORT __declspec(dllexport)
#else
#define M2_EXPORT __attribute__ ((visibility("default")))
#endif

#define M2_C_EXPORT extern "C" M2_EXPORT


/**
* @brief Lane Detect And Lane Waring.
* @param image opencv brg format input image
* @param final_lane  detect lane result
* @param LaneType    2 warning; 0 or 1 no warning;
* @return 0.
*/
M2_EXPORT int M2_Lane(const cv::Mat &image,std::vector<M2::lane_DECODE> &final_lane,int &LaneType);




/**
* @brief DMS Detect And DMS Waring.
* @param image opencv brg format input image
* @param objectinfo  detect MaxSizeFace result
* @param landmarkinfo  detect MaxSizeFace Landmark result
* @param DMSTpye    2 warning; 0 or 1 no waring;
* @return 0.
*/
M2_EXPORT int M2_DMS(const cv::Mat &image,M2::ObjectInfo &objectinfo,M2::LandmarkInfo &landmarkinfo,int &DMSTpye);





// not used;
M2_EXPORT int M2_FaceDetect_ForwardBGR(const cv::Mat &image,M2::ObjectInfo &objectinfo,int max_or_mid);
M2_EXPORT int M2_ObjectDetect_ForwardBGR(const cv::Mat &image,M2::ObjectInfo &objectinfo);
M2_EXPORT int M2_LaneDetect_ForwardBGR(const cv::Mat &image,std::vector<M2::lane_DECODE> &final_lane);
M2_EXPORT int M2_FaceAlignment_ForwardBGR(const cv::Mat &image,const M2::Object &face,M2::LandmarkInfo &landmarkinfo);
M2_EXPORT int M2_FaceAlignment_ForwardBGR_MaxFace(const cv::Mat &image,M2::ObjectInfo &objectinfo,M2::LandmarkInfo &landmarkinfo);
//

#endif

