/******************************************************************************

                  ��Ȩ���� (C), 2019-2029, SDC OS ��Դ����С������

 ******************************************************************************
  �� �� ��   : opencv_api.h
  �� �� ��   : ����
  ��      ��  s30001871
  ��������   : 2020��7��4��
  ����޸�   :
  ��������   : opencv_api������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��7��4��
  ��    ��   ��s30001871
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __HW_OPENCV_API_H__
#define __HW_OPENCV_API_H__
#include <stdio.h>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "sdc.h"
#include "sample_comm_nnie.h"

extern int yuvFrame2Mat(sdc_yuv_frame_s &yuvFrame,cv::Mat &dst);
extern void TransFormMat2Yuv(cv::Mat &img, unsigned char *yuv_nv21);
extern int ProcessImgPath(const char *imgPath, int &w, int &h, uint8_t **pyuv);
extern int CropInGivenRect(sdc_yuv_frame_s &yuvFrame, int xmin,int ymin,int width,int height, std::vector<unsigned char>& jpgData);
extern void BGR2yuv_nv21(cv::Mat& img, unsigned char * pyuv);
// bool YUVResize(sdc_yuv_frame_s &yuvFrame, int resize_w, int resize_h, unsigned char * pyuv);

// float GetScaleRatio(int src_w, int src_h,int forwardsize_w, int forwardsize_h);

// void SetSdcYuvFrame(int src_w, int src_h, sdc_yuv_frame_s *pstRGBFrameData);

// int ProcessInputPath(const char* imgPath, int &input_w, int &input_h, uint8_t **pyuv);

// void DrawImage(const char* imgPath, SDC_SSD_RESULT_S* stResult, int src_w, int src_h, int for_w,int for_h);

// int TransYuv2Jpg(unsigned char *yuvAddr, int w, int h, const char *imgPath);

// void readJpgData(const char *imgPath, std::vector<unsigned char>& jpgData);

// int CropInGivenRect(sdc_yuv_frame_s &yuvFrame, const SDC_SSD_RESULT_S &stResult, std::vector<unsigned char>& jpgData);

#endif /* __HW_OPENCV_API_H__ */