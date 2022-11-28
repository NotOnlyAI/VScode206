
#ifndef __FaceRecognizeModel_APP_H__
#define __FaceRecognizeModel_APP_H__

#include <string>
#include <memory>


#include "sdc_data_src.h"
#include "sample_comm_nnie.h"


class FaceRecognizeModel
{
    public:
    int32_t init(std::string &modelName,int model_type);


    int32_t LoadModel(std::string &modelName);
    int32_t HardWare_ParamInit(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam);
    int32_t SoftWare_ParamInit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftWareParam);

    // int32_t ForwardBGR(sdc_yuv_data_s &yuv_data,SDC_SSD_RESULT_S &stResult,SDC_SSD_OBJECT_INFO_S &stObject);
    int32_t ForwardYUV(char *pYuvForward,SDC_FEATURE_INFO_S &stFeature);
    void Destroy();
    // int32_t SubscribeYuvData(std::shared_ptr<IYuvDataSrc> dataSrc);

    SDC_SSD_INPUT_SIZE_S m_inputSize;   // ����ģ�͵Ĵ�С
    HI_FLOAT  m_ConfThresh;
    int m_maxNumOfFace;
    virtual ~FaceRecognizeModel();

    
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