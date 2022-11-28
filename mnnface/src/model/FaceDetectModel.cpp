
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <iostream>

#include "FaceDetectModel.h"

#include "common_defs.h"
#include "sdc_fd_utils.h"
#include "sdc_fd_algorithm.h"
#include "hi_model_init.h"
#include "sdc_opencv_api.h"
#include "hi_model_forward.h"
/*----------------------------------------------*
 * �ⲿ����                                *
 *----------------------------------------------*/
extern int fd_algorithm;
extern int fd_utils;
extern int fd_video;


extern int g_running;

SAMPLE_SVP_NNIE_MODEL_S s_stModel;    
SAMPLE_SVP_NNIE_PARAM_S s_stNnieParam = {0};
SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S s_stSsdSoftwareParam = {0};
SAMPLE_SVP_NNIE_CFG_S   stNnieCfg_SDC = {0};

sdc_yuv_frame_s sdcRgb;
VW_YUV_FRAME_S rgbimg;

// 全局变量初始化
static void ResetGlobalVars(void)
{
    memset_s(&s_stModel, sizeof(s_stModel), 0, sizeof(s_stModel));
    memset_s(&s_stNnieParam, sizeof(s_stNnieParam), 0, sizeof(s_stNnieParam));
    memset_s(&s_stSsdSoftwareParam, sizeof(s_stSsdSoftwareParam), 0, sizeof(s_stSsdSoftwareParam));
    memset_s(&stNnieCfg_SDC, sizeof(stNnieCfg_SDC), 0, sizeof(stNnieCfg_SDC));
    memset_s(&sdcRgb, sizeof(sdcRgb), 0, sizeof(sdcRgb));
    memset_s(&rgbimg, sizeof(rgbimg), 0, sizeof(rgbimg));
}   


int32_t FaceDetetectModel::init(std::string &modelName,int model_type)
{   

    ResetGlobalVars();
    std::cout<<"fd_algorithm="<<fd_algorithm<<std::endl;
    int nret=LoadModel(modelName);

    stNnieCfg_SDC.u32MaxInputNum = 1; //max input image num in each batch
    stNnieCfg_SDC.u32MaxRoiNum = 0;
    stNnieCfg_SDC.aenNnieCoreId[0] = SVP_NNIE_ID_0;//set NNIE core
    s_stNnieParam.pstModel = &(s_stModel.stModel);
    
    m_modelType=model_type;
    m_ConfThresh=0.3;
    m_maxNumOfFace=500;
    s_stSsdSoftwareParam.u32ModelType=m_modelType;
    s_stSsdSoftwareParam.f32ConfThresh=m_ConfThresh;
    s_stSsdSoftwareParam.u32maxNumOfFace=m_maxNumOfFace;

    HI_S32 s32Ret;
    s32Ret=HardWare_ParamInit(&stNnieCfg_SDC,&s_stNnieParam);
    s32Ret=SoftWare_ParamInit(&s_stNnieParam,&s_stSsdSoftwareParam);

    return s32Ret;

}


int32_t FaceDetetectModel::Update(float scoreThresh,int maxNumOfFace)
{   
    m_ConfThresh=scoreThresh;
    m_maxNumOfFace=maxNumOfFace;
    s_stSsdSoftwareParam.f32ConfThresh=scoreThresh;
    s_stSsdSoftwareParam.u32maxNumOfFace=m_maxNumOfFace;
    return 0;
}





int32_t FaceDetetectModel::LoadModel(std::string &modelName)
{
    int nret = SDC_LoadModel(fd_algorithm,fd_utils,1,(char *)modelName.c_str(), &s_stModel.stModel,&s_stModel);
    if (nret != OK) {
        LOG_ERROR("Load the NNIE WK file failed, file path: %s", modelName.c_str());
        return ERR;
    } else {
        LOG_DEBUG("Load the NNIE WK file successfully, file path: %s", modelName.c_str());
        // ����ģ���ļ���
        m_modelName = modelName;
        return OK;
    }
}

int32_t FaceDetetectModel::HardWare_ParamInit(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam)
{   
    
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SDC_NNIE_ParamInit(pstNnieCfg,pstNnieParam);
    if (s32Ret != HI_SUCCESS) {
        LOG_ERROR("FaceDetetectModel HardWare_ParamInit failed");
        return ERR;
    } else {
        LOG_DEBUG("FaceDetetectModel HardWare_ParamInit successfully ");
        return OK;
    }
}


int32_t FaceDetetectModel::SoftWare_ParamInit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftWareParam)
{   
    
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SAMPLE_SVP_NNIE_SoftwareInit(pstNnieParam,pstSoftWareParam);
    if (s32Ret != HI_SUCCESS) {
        LOG_ERROR("FaceDetetectModel SoftWare_ParamInit failed");
        return ERR;
    } else {
        LOG_DEBUG("FaceDetetectModel SoftWare_ParamInit successfully ");
        return OK;
    }
}


int32_t FaceDetetectModel::ForwardYUV(char *pYuvForward,SDC_SSD_RESULT_S &stResult,SDC_SSD_OBJECT_INFO_S &stObject)
{   
    int nret;

    // cv::Mat image(cv::Size(112, 112), CV_8UC3);
	// image.data =(HI_U8*)pYuvForward;
    // cv::imwrite("yuv2BGRDetect.jpg",image);


    if (SDC_SVP_ForwardYUV(pYuvForward,&stNnieCfg_SDC,&s_stNnieParam,&s_stSsdSoftwareParam, &stResult,&stObject) != OK) {
        LOG_ERROR("FaceDetetectModel ForwardYUV failed");
        return ERR;
    } else {
        LOG_DEBUG("FaceDetetectModel ForwardYUV successfully ");
        

        return OK;
    }
}


// int32_t FaceDetetectModel::ForwardBGR(sdc_yuv_data_s &yuv_data,SDC_SSD_RESULT_S &stResult)
// {   
//     int nret;
//     nret = SDC_TransYUV2RGB(fd_algorithm, &(yuv_data.frame), &sdcRgb);
//     SDC_Struct2RGB(&sdcRgb, &rgbimg);

//     if (SDC_SVP_ForwardBGR((HI_CHAR *)(uintptr_t)rgbimg.pYuvImgAddr, &stResult) != OK) {
//         LOG_ERROR("FaceDetetectModel ForwardBGR failed");
//         return ERR;
//     } else {
//         LOG_DEBUG("FaceDetetectModel ForwardBGR successfully ");
//         return OK;
//     }
// }


void FaceDetetectModel::Destroy(void)
{
    SDC_UnLoadModel(fd_algorithm,&s_stModel.stModel);
}



FaceDetetectModel::~FaceDetetectModel(void)
{
    LOG_DEBUG("FaceDetetect destructor");
}




