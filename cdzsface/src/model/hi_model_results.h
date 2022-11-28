#ifndef __Hi_model_Result_H__
#define __Hi_model_Result_H__


#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#ifdef __cplusplus
extern "C"{
#endif

#include "sdc.h"
#include "sample_comm_nnie.h"






extern HI_S32 SAMPLE_SVP_NNIE_GetResult(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,
    SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam,SDC_SSD_RESULT_S *pstResult,SDC_SSD_OBJECT_INFO_S *pstObject);

extern HI_S32 SAMPLE_FEATURE_NNIE_GetResult(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,
SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam,SDC_FEATURE_INFO_S *pstFeatureInfo);

extern HI_S32 SlimFace_GetResult(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,
    SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam,SDC_SSD_RESULT_S *pstResult);

extern HI_S32 RetinaFace_GetResult(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,
    SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam,SDC_SSD_RESULT_S *pstResult);

extern HI_S32 SSDFace_GetResult(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,
    SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam,SDC_SSD_RESULT_S *pstResult);

extern HI_S32 FCOSFace_GetResult(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,
    SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam,SDC_SSD_RESULT_S *pstResult);



#ifdef __cplusplus
}
#endif

#endif /* __Hi_model_H__ */