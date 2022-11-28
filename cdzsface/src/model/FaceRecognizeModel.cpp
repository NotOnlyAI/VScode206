/******************************************************************************

                  ��Ȩ���� (C), 2019-2029, SDC OS ��Դ����С������

 ******************************************************************************
  �� �� ��   : Model.cpp
  �� �� ��   : ����
  ��    ��   : athina
  ��������   : 2020��7��4��
  ����޸�   :
  ��������   :Model ҵ������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��7��4��
    ��    ��   : athina
    �޸�����   : �����ļ�

******************************************************************************/

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <vector>

#include "FaceRecognizeModel.h"
#include "common_defs.h"

#include "sdc_fd_utils.h"
#include "sdc_fd_algorithm.h"
#include "hi_model_init.h"
#include "sdc_opencv_api.h"
#include "hi_model_forward.h"
/*----------------------------------------------*
 * �ⲿ����                                *
 *----------------------------------------------*/
extern int fd_algorithm2;
extern int fd_utils;
extern int fd_video;


extern int g_running;

SAMPLE_SVP_NNIE_MODEL_S faceRecognizeModel;    
SAMPLE_SVP_NNIE_PARAM_S faceRecognizeNnieParam = {0};
SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S faceRecognizeSoftwareParam = {0};
SAMPLE_SVP_NNIE_CFG_S   faceRecognizeNnieCfg_SDC = {0};


sdc_yuv_frame_s faceRecognizesdcRgb;
VW_YUV_FRAME_S faceRecognizergbimg;


// 全局变量初始化
static void ResetGlobalVars(void)
{
    memset_s(&faceRecognizeModel, sizeof(faceRecognizeModel), 0, sizeof(faceRecognizeModel));
    memset_s(&faceRecognizeNnieParam, sizeof(faceRecognizeNnieParam), 0, sizeof(faceRecognizeNnieParam));
    memset_s(&faceRecognizeSoftwareParam, sizeof(faceRecognizeSoftwareParam), 0, sizeof(faceRecognizeSoftwareParam));
    memset_s(&faceRecognizeNnieCfg_SDC, sizeof(faceRecognizeNnieCfg_SDC), 0, sizeof(faceRecognizeNnieCfg_SDC));
    memset_s(&faceRecognizesdcRgb, sizeof(faceRecognizesdcRgb), 0, sizeof(faceRecognizesdcRgb));
    memset_s(&faceRecognizergbimg, sizeof(faceRecognizergbimg), 0, sizeof(faceRecognizergbimg));
}   


int32_t FaceRecognizeModel::init(std::string &modelName,int model_type)
{   


    std::cout<<"fd_algorithm2="<<fd_algorithm2<<std::endl;
    ResetGlobalVars();

    int nret=LoadModel(modelName);

    faceRecognizeNnieCfg_SDC.u32MaxInputNum = 1; //max input image num in each batch
    faceRecognizeNnieCfg_SDC.u32MaxRoiNum = 0;
    faceRecognizeNnieCfg_SDC.aenNnieCoreId[0] = SVP_NNIE_ID_0;//set NNIE core
    faceRecognizeNnieParam.pstModel = &(faceRecognizeModel.stModel);
    
    m_modelType=model_type;
    m_ConfThresh=0.6;

    faceRecognizeSoftwareParam.u32ModelType=m_modelType;
    // std::cout<<"u32ModelType="<<faceRecognizeSoftwareParam.u32ModelType<<std::endl;
    faceRecognizeSoftwareParam.f32ConfThresh=0.6;

    HI_S32 s32Ret;
    s32Ret=HardWare_ParamInit(&faceRecognizeNnieCfg_SDC,&faceRecognizeNnieParam);
    s32Ret=SoftWare_ParamInit(&faceRecognizeNnieParam,&faceRecognizeSoftwareParam);

    return s32Ret;

}



int32_t FaceRecognizeModel::LoadModel(std::string &modelName)
{

    int nret = SDC_LoadModel(fd_algorithm2,fd_utils,1,(char *)modelName.c_str(), &faceRecognizeModel.stModel,&faceRecognizeModel);
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

int32_t FaceRecognizeModel::HardWare_ParamInit(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam)
{   
    
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SDC_NNIE_ParamInit(pstNnieCfg,pstNnieParam);
    if (s32Ret != HI_SUCCESS) {
        LOG_ERROR("FaceRecognizeModel HardWare_ParamInit failed");
        return ERR;
    } else {
        LOG_DEBUG("FaceRecognizeModel HardWare_ParamInit successfully ");
        return OK;
    }
}


int32_t FaceRecognizeModel::SoftWare_ParamInit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftWareParam)
{   
    
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SAMPLE_SVP_NNIE_SoftwareInit(pstNnieParam,pstSoftWareParam);
    if (s32Ret != HI_SUCCESS) {
        LOG_ERROR("FaceRecognizeModel SoftWare_ParamInit failed");
        return ERR;
    } else {
        LOG_DEBUG("FaceRecognizeModel SoftWare_ParamInit successfully ");
        return OK;
    }
}


int32_t FaceRecognizeModel::ForwardYUV(char *pYuvForward,SDC_FEATURE_INFO_S &stFeature)
{   
    int nret;



    if (SDC_FEATURE_ForwardYUV(pYuvForward,&faceRecognizeNnieCfg_SDC,&faceRecognizeNnieParam,&faceRecognizeSoftwareParam, &stFeature) != OK) {
        LOG_ERROR("FaceRecognizeModel ForwardYUV failed");
        return ERR;
    } else {
        LOG_DEBUG("FaceRecognizeModel ForwardYUV successfully ");
        

        return OK;
    }
}


// int32_t FaceRecognizeModel::ForwardBGR(sdc_yuv_data_s &yuv_data,SDC_SSD_RESULT_S &stResult)
// {   
//     int nret;
//     nret = SDC_TransYUV2RGB(fd_algorithm2, &(yuv_data.frame), &faceRecognizesdcRgb);
//     SDC_Struct2RGB(&faceRecognizesdcRgb, &faceRecognizergbimg);

//     if (SDC_SVP_ForwardBGR((HI_CHAR *)(uintptr_t)faceRecognizergbimg.pYuvImgAddr, &stResult) != OK) {
//         LOG_ERROR("FaceRecognizeModel ForwardBGR failed");
//         return ERR;
//     } else {
//         LOG_DEBUG("FaceRecognizeModel ForwardBGR successfully ");
//         return OK;
//     }
// }


void FaceRecognizeModel::Destroy(void)
{
    SDC_UnLoadModel(fd_algorithm2,&faceRecognizeModel.stModel);
}


FaceRecognizeModel::~FaceRecognizeModel(void)
{
    LOG_DEBUG("FaceDetetect destructor");
}




