/******************************************************************************

                  版权所有 (C), 2019-2029, SDC OS 开源软件小组所有

 ******************************************************************************
  文 件 名   : yolov3_app.h
  版 本 号   : 初稿
  作    者   : athina
  生成日期   : 2020年7月4日
  最近修改   :
  功能描述   : yolov3业务流程
  函数列表   :
  修改历史   :
  1.日    期   : 2020年7月4日
    作    者   : athina
    修改内容   : 创建文件

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
    
    // 将uclass对应的数字，映射为具体目标，目标信息不能超过20字符
    virtual void ClassifyTargets(uint32_t uclass, uint8_t target[20]) = 0;
    // 打印目标检测信息
    virtual void PrintObjectInfo(const SDC_SSD_OBJECT_INFO_S &info) const;
    
    SDC_SSD_INPUT_SIZE_S m_videoSize;   // 输入模型的大小
    SDC_SSD_INPUT_SIZE_S m_inputSize;   // 输入模型的大小

private:
    Yolov3App(const Yolov3App&);
    Yolov3App& operator=(const Yolov3App&);
    Yolov3App(Yolov3App&&);
    Yolov3App& operator=(Yolov3App&&);

    // 构造元数据
    bool ConstructMetaData(const SDC_SSD_RESULT_S &stResult, ProtoBuffer &buf);

    // 处理线程函数
    static void* YuvDataProcThrd(void *arg);    // YUV处理线程
    static void* VideoSrvReadThrd(void *arg);   // 视频服务线程
    static void* WatchdogThread(void *arg);     // 系统资源监控

    // 卸载NNiE WK模型
    int32_t UnLoadModel(void);

    // 展示检测目标信息
    void SendObjectsToSims(ProtoBuffer &buf, int32_t pts) const;

    std::string m_appName;
    std::string m_modelName;

    // ReceiveThread & ProcessThread
    pthread_t m_tidpProc;
    pthread_t m_tidpRecv;
    pthread_t m_tidpWatchdog;
    uint32_t  m_yuvChnId;
    YuvQueue m_yuvQue;  // 循环队列保存Yuv数据帧
    std::shared_ptr<IYuvDataSrc> m_dataSrc; // 指定数据源

    // Yuv循环队列的默认大小为50
    static const int32_t MAX_YUV_BUF_NUM = 50;
    static float m_confidenceThres;     // 置信度阈值
};
}
#endif /* __YOLOV3_APP_H__ */