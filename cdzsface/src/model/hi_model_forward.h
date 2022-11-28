#ifndef __Hi_data_init_H__
#define __Hi_data_init_H__


#ifdef __cplusplus
extern "C"{
#endif

#include "sdc.h"
#include "sample_comm_nnie.h"
#include "hi_comm_video.h"


#define YUV_CHANNEL_LEN (3)
typedef struct YUV_FRAME
{
    UINT32 uWidth;
    UINT32 uHeight;

    //VW_PIXEL_FORMAT_E enPixelFormat;
    int enPixelFormat;

    UINT64 ulPhyAddr[YUV_CHANNEL_LEN];
    UINT64 ulVirAddr[YUV_CHANNEL_LEN];
    UINT32 uStride[YUV_CHANNEL_LEN];

    UINT64 ullpts;

    UINT32 uVbBlk;
    UINT32 uPoolId;
    unsigned char* pYuvImgAddr;
    UINT32 uFrmSize;

} VW_YUV_FRAME_S;


extern void SDC_Struct2RGB(sdc_yuv_frame_s *pstSdcRGBFrame, VW_YUV_FRAME_S *pstRGBFrameData);
extern HI_S32 SAMPLE_SVP_NNIE_FillSrcData(SAMPLE_SVP_NNIE_CFG_S* pstNnieCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S* pstInputDataIdx);
// extern int SDC_SVP_ForwardBGR(HI_CHAR *pcSrcBGR, SDC_SSD_RESULT_S *pstResult);
extern int SDC_SVP_ForwardYUV(HI_CHAR *pcSrcYUV, SAMPLE_SVP_NNIE_CFG_S* pstNnieCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
    SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam,
    SDC_SSD_RESULT_S *pstResult,
    SDC_SSD_OBJECT_INFO_S *pstObject);

extern int SDC_FEATURE_ForwardYUV(HI_CHAR *pcSrcYUV, SAMPLE_SVP_NNIE_CFG_S* pstNnieCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
    SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam,
    SDC_FEATURE_INFO_S *pstFeatureInfo);
// extern int SDC_SVP_ForwardYUV(HI_CHAR *pcSrcYUV, SDC_SSD_RESULT_S *pstResult);
#ifdef __cplusplus
}
#endif

#endif /* __Hi_data_init_H__ */