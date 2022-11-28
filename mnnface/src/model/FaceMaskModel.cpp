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

#include "FaceMaskModel.h"
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

SAMPLE_SVP_NNIE_MODEL_S faceMaskModel;    
SAMPLE_SVP_NNIE_PARAM_S faceMaskNnieParam = {0};
SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S faceMaskSoftwareParam = {0};
SAMPLE_SVP_NNIE_CFG_S   faceMaskNnieCfg_SDC = {0};


sdc_yuv_frame_s faceMasksdcRgb;
VW_YUV_FRAME_S faceMaskrgbimg;


// 全局变量初始化
static void ResetGlobalVars(void)
{
    memset_s(&faceMaskModel, sizeof(faceMaskModel), 0, sizeof(faceMaskModel));
    memset_s(&faceMaskNnieParam, sizeof(faceMaskNnieParam), 0, sizeof(faceMaskNnieParam));
    memset_s(&faceMaskSoftwareParam, sizeof(faceMaskSoftwareParam), 0, sizeof(faceMaskSoftwareParam));
    memset_s(&faceMaskNnieCfg_SDC, sizeof(faceMaskNnieCfg_SDC), 0, sizeof(faceMaskNnieCfg_SDC));
    memset_s(&faceMasksdcRgb, sizeof(faceMasksdcRgb), 0, sizeof(faceMasksdcRgb));
    memset_s(&faceMaskrgbimg, sizeof(faceMaskrgbimg), 0, sizeof(faceMaskrgbimg));
}   


int32_t FaceMaskModel::init(std::string &modelName,float scoreThresh,int model_type)
{   


    std::cout<<"fd_algorithm2="<<fd_algorithm2<<std::endl;
    ResetGlobalVars();

    int nret=LoadModel(modelName);

    faceMaskNnieCfg_SDC.u32MaxInputNum = 1; //max input image num in each batch
    faceMaskNnieCfg_SDC.u32MaxRoiNum = 0;
    faceMaskNnieCfg_SDC.aenNnieCoreId[0] = SVP_NNIE_ID_0;//set NNIE core
    faceMaskNnieParam.pstModel = &(faceMaskModel.stModel);
    
    m_modelType=model_type;
    m_ConfThresh=scoreThresh;

    faceMaskSoftwareParam.u32ModelType=m_modelType;
    // std::cout<<"u32ModelType="<<faceMaskSoftwareParam.u32ModelType<<std::endl;
    faceMaskSoftwareParam.f32ConfThresh=scoreThresh;

    HI_S32 s32Ret;
    s32Ret=HardWare_ParamInit(&faceMaskNnieCfg_SDC,&faceMaskNnieParam);
    s32Ret=SoftWare_ParamInit(&faceMaskNnieParam,&faceMaskSoftwareParam);

    return s32Ret;

}



int32_t FaceMaskModel::LoadModel(std::string &modelName)
{

    int nret = SDC_LoadModel(fd_algorithm2,fd_utils,1,(char *)modelName.c_str(), &faceMaskModel.stModel,&faceMaskModel);
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

int32_t FaceMaskModel::HardWare_ParamInit(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam)
{   
    
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SDC_NNIE_ParamInit(pstNnieCfg,pstNnieParam);
    if (s32Ret != HI_SUCCESS) {
        LOG_ERROR("FaceMaskModel HardWare_ParamInit failed");
        return ERR;
    } else {
        LOG_DEBUG("FaceMaskModel HardWare_ParamInit successfully ");
        return OK;
    }
}


int32_t FaceMaskModel::SoftWare_ParamInit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftWareParam)
{   
    
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SAMPLE_SVP_NNIE_SoftwareInit(pstNnieParam,pstSoftWareParam);
    if (s32Ret != HI_SUCCESS) {
        LOG_ERROR("FaceMaskModel SoftWare_ParamInit failed");
        return ERR;
    } else {
        LOG_DEBUG("FaceMaskModel SoftWare_ParamInit successfully ");
        return OK;
    }
}


int32_t FaceMaskModel::ForwardYUV(char *pYuvForward,SDC_SSD_RESULT_S &stResult,SDC_SSD_OBJECT_INFO_S &stObject)
{   
    int nret;
    if (SDC_SVP_ForwardYUV(pYuvForward,&faceMaskNnieCfg_SDC,&faceMaskNnieParam,&faceMaskSoftwareParam, &stResult,&stObject) != OK) {
        LOG_ERROR("FaceMaskModel ForwardYUV failed");
        return ERR;
    } else {
        LOG_DEBUG("FaceMaskModel ForwardYUV successfully ");
        

        return OK;
    }
}


// int32_t FaceMaskModel::ForwardBGR(sdc_yuv_data_s &yuv_data,SDC_SSD_RESULT_S &stResult)
// {   
//     int nret;
//     nret = SDC_TransYUV2RGB(fd_algorithm2, &(yuv_data.frame), &faceMasksdcRgb);
//     SDC_Struct2RGB(&faceMasksdcRgb, &faceMaskrgbimg);

//     if (SDC_SVP_ForwardBGR((HI_CHAR *)(uintptr_t)faceMaskrgbimg.pYuvImgAddr, &stResult) != OK) {
//         LOG_ERROR("FaceMaskModel ForwardBGR failed");
//         return ERR;
//     } else {
//         LOG_DEBUG("FaceMaskModel ForwardBGR successfully ");
//         return OK;
//     }
// }


void FaceMaskModel::Destroy(void)
{
    SDC_UnLoadModel(fd_algorithm2,&faceMaskModel.stModel);
}



FaceMaskModel::~FaceMaskModel(void)
{
    LOG_DEBUG("FaceDetetect destructor");
}




