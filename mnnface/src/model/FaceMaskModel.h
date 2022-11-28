/******************************************************************************

                  ��Ȩ���� (C), 2019-2029, SDC OS ��Դ����С������

 ******************************************************************************
  �� �� ��   : CamareManager.h
  �� �� ��   : ����
  ��    ��   : athina
  ��������   : 2020��7��4��
  ����޸�   :
  ��������   : CamareManagerҵ������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��7��4��
    ��    ��   : athina
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __FaceMaskModel_APP_H__
#define __FaceMaskModel_APP_H__

#include <string>
#include <memory>


#include "sdc_data_src.h"
#include "sample_comm_nnie.h"


class FaceMaskModel
{
    public:
    int32_t init(std::string &modelName,float scoreThresh,int model_type);


    int32_t LoadModel(std::string &modelName);
    int32_t HardWare_ParamInit(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam);
    int32_t SoftWare_ParamInit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftWareParam);

    // int32_t ForwardBGR(sdc_yuv_data_s &yuv_data,SDC_SSD_RESULT_S &stResult,SDC_SSD_OBJECT_INFO_S &stObject);
    int32_t ForwardYUV(char *pYuvForward,SDC_SSD_RESULT_S &stResult,SDC_SSD_OBJECT_INFO_S &stObject);
    void Destroy();
    // int32_t SubscribeYuvData(std::shared_ptr<IYuvDataSrc> dataSrc);

    SDC_SSD_INPUT_SIZE_S m_inputSize;   // ����ģ�͵Ĵ�С
    HI_FLOAT  m_ConfThresh;
    int m_maxNumOfFace;
    virtual ~FaceMaskModel();

    
    private:
    std::string m_modelName;
    int m_modelType;


    // // ReceiveThread & ProcessThread
    // pthread_t m_tidpProc;
    // pthread_t m_tidpRecv;
    // pthread_t m_tidpWatchdog;

    
    // YuvQueue m_yuvQue;  // ѭ�����б���Yuv����֡
    // std::shared_ptr<IYuvDataSrc> m_dataSrc; // ָ������Դ
};






#endif /* __Model_APP_H__ */