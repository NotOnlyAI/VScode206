#ifndef __Hi_Result_Show_H__
#define __Hi_Result_Show_H__


#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "sdc.h"
#include "sample_comm_nnie.h"

#ifdef __cplusplus
extern "C"{
#endif

extern HI_S32 SAMPLE_SVP_NNIE_ShowResult(cv::Mat image,SDC_SSD_RESULT_S stResult);


#ifdef __cplusplus
}
#endif

#endif /* __Hi_model_Show_H__ */