/******************************************************************************

                  ��Ȩ���� (C), 2019-2029, SDC OS ��Դ���С������

 ******************************************************************************
  �� �� ��   : yolov3_app.h
  �� �� ��   : ����
  ��    ��   : athina
  ��������   : 2020��7��4��
  ����޸�   :
  ��������   : yolov3ҵ������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��7��4��
    ��    ��   : athina
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __YOLOV3_APP_H__
#define __YOLOV3_APP_H__

#include <string>
#include <memory>
#include "yuv_queue.h"
#include "sdc_data_src.h"
#include "sdc_os_api.h"

namespace HWYolov3App
{
class ProtoBuffer;
class IYuvDataSrc;
class Yolov3App
{
public:
    virtual ~Yolov3App(void);
    int32_t Init(std::string &appName);
    int32_t SetYuvChanAttr(sdc_yuv_channel_param_s &yuv_param);
    int32_t LoadModel(unsigned int loadMode, std::string &modelName);
    virtual int32_t NNieParamInit(void);
    int32_t SubscribeYuvDataAndForward(std::shared_ptr<IYuvDataSrc> dataSrc);
    void    Destroy(void);
    uint32_t GetYuvChannelId(void) const;

protected:
    Yolov3App(void);
    virtual void PreProcessYuvData(sdc_yuv_frame_s &frame, uint8_t *&yuv);
    virtual void PretreatObjectData(SDC_SSD_OBJECT_INFO_S &data) = 0;
    bool ConstructRectAreaData(const SDC_SSD_OBJECT_INFO_S &data, ProtoBuffer &buf);
    bool ConstructTargetData(const SDC_SSD_OBJECT_INFO_S &data, ProtoBuffer &buf);
    
    // ��uclass��Ӧ�����֣�ӳ��Ϊ����Ŀ�꣬Ŀ����Ϣ���ܳ���20�ַ�
    virtual void ClassifyTargets(uint32_t uclass, uint8_t target[20]) = 0;
    // ��ӡĿ������Ϣ
    virtual void PrintObjectInfo(const SDC_SSD_OBJECT_INFO_S &info) const;
    
    SDC_SSD_INPUT_SIZE_S m_videoSize;   // ����ģ�͵Ĵ�С
    SDC_SSD_INPUT_SIZE_S m_inputSize;   // ����ģ�͵Ĵ�С

private:
    Yolov3App(const Yolov3App&);
    Yolov3App& operator=(const Yolov3App&);
    Yolov3App(Yolov3App&&);
    Yolov3App& operator=(Yolov3App&&);

    // ����Ԫ����
    bool ConstructMetaData(const SDC_SSD_RESULT_S &stResult, ProtoBuffer &buf);

    // �����̺߳���
    static void* YuvDataProcThrd(void *arg);    // YUV�����߳�
    static void* VideoSrvReadThrd(void *arg);   // ��Ƶ�����߳�
    static void* WatchdogThread(void *arg);     // ϵͳ��Դ���

    // ж��NNiE WKģ��
    int32_t UnLoadModel(void);

    // չʾ���Ŀ����Ϣ
    void SendObjectsToSims(ProtoBuffer &buf, int32_t pts) const;

    std::string m_appName;
    std::string m_modelName;

    // ReceiveThread & ProcessThread
    pthread_t m_tidpProc;
    pthread_t m_tidpRecv;
    pthread_t m_tidpWatchdog;
    uint32_t  m_yuvChnId;
    YuvQueue m_yuvQue;  // ѭ�����б���Yuv����֡
    std::shared_ptr<IYuvDataSrc> m_dataSrc; // ָ������Դ

    // Yuvѭ�����е�Ĭ�ϴ�СΪ50
    static const int32_t MAX_YUV_BUF_NUM = 50;
    static float m_confidenceThres;     // ���Ŷ���ֵ
};
}
#endif /* __YOLOV3_APP_H__ */