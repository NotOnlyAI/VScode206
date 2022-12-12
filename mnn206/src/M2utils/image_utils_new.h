#ifndef _FACEIMAGE_UTILS_NEW2_H__
#define _FACEIMAGE_UTILS_NEW2_H__

#include "models206_typedef.h"
#include <iostream>

using namespace M2;


#ifdef FACE_CVSHOW
#include <opencv2/opencv.hpp>
#endif



#ifdef FACE_CVSHOW
cv::Mat StructImage_to_CvImage(const STRU_ImgData_T &imagedata);
#endif
// void Image_crop_v2( const STRU_ImgData_T &imgdata,STRU_ImgData_T &result, const STRU_Rect_T &rect );

#endif

