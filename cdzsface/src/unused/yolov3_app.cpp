/******************************************************************************

                  版权所有 (C), 2019-2029, SDC OS 开源软件小组所有

 ******************************************************************************
  文 件 名   : yolov3_app.cpp
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
#include "yolov3_app.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <vector>

#include "sdc.h"
#include "sdc_def_ext.h"
#include "sdc_os_api.h"
#include "sdc_api_ext.h"
#include "label_event.h"
#include "proto_buffer.h"
#include "opencv_api.h"
#include "sdc_sys_res.h"

#define MODEL_INPUT_SIZE_WIDTH               416  // 模型输入大小宽
#define MODEL_INPUT_SIZE_HEIGHT              416  // 模型输入大小高
namespace HWYolov3App
{
float Yolov3App::m_confidenceThres = 0.5;
// 最大缓存YUV帧数量为10(SDC_APP开发指南默认填10)
const int MAX_CACHED_COUNT = 10;
// 默认线程堆栈大小
const int DEF_THREAD_STACK_SIZE = 262144;   // 256 * 1024 

Yolov3App::Yolov3App(void) :
    m_appName(""),
    m_modelName(""),
    m_tidpProc(0),
    m_tidpRecv(0),
    m_tidpWatchdog(0),
    m_yuvChnId(0),
    m_dataSrc(nullptr)
{
    LOG_DEBUG("Yolov3App constructor");
    memset_s(&m_inputSize, sizeof(m_inputSize), 0, sizeof(m_inputSize));
}

Yolov3App::~Yolov3App(void)
{
    LOG_DEBUG("Yolov3App destructor");
    Destroy();
}

int32_t Yolov3App::Init(std::string &appName)
{
    // 1. 打开文件服务连接
	if (SDC_ServiceCreate() != OK) {
        LOG_ERROR("Create the SDC_OS service failed");
        return ERR;
    }
    LOG_ERROR("Create the SDC_OS service successfully");

    // 2. 获取硬件ID
    sdc_hardware_id_s stHardWareParas;
    if (SDC_GetHardWareId(&stHardWareParas) != OK) {
        LOG_ERROR("Get the HardWare Id failed, channel_id: %s", stHardWareParas.id);
    }
    LOG_DEBUG("Get the HardWare Id successfully, channel_id: %s", stHardWareParas.id);

    // 2. 创建YuV数据帧队列
    if (m_yuvQue.CreateQueue(Yolov3App::MAX_YUV_BUF_NUM) == ERR) {
        LOG_ERROR("Create the Yuv ring queue failed, buffer size: %d", Yolov3App::MAX_YUV_BUF_NUM);
        return ERR;
    }
    LOG_DEBUG("Create the Yuv ring queue successfully, buffer size: %d", Yolov3App::MAX_YUV_BUF_NUM);
    
    m_inputSize.ImageWidth  = MODEL_INPUT_SIZE_WIDTH;
    m_inputSize.ImageHeight = MODEL_INPUT_SIZE_HEIGHT;

    m_appName = appName;

    // 创建系统资源看管线程
    pthread_attr_t attr;
    (void)pthread_attr_init(&attr);
    (void)pthread_attr_setstacksize(&attr, DEF_THREAD_STACK_SIZE);  // 线程栈默认设置256KB
    (void)pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    // if ((pthread_create(&m_tidpProc, &attr, Yolov3App::WatchdogThread, (void *)this)) == ERR) {
    //     pthread_attr_destroy(&attr);
    //     LOG_ERROR("Create the thread:WatchdogThread() failed");
    //     return ERR;
    // }
    // pthread_attr_destroy(&attr);
    // LOG_DEBUG("Create the thread:WatchdogThread() successfully");

    return OK;
}

int32_t Yolov3App::LoadModel(unsigned int loadMode, std::string &modelName)
{
    int nret = SDC_LoadModel(loadMode, (char *)modelName.c_str(), &s_stYoloModel.stModel);
    if (nret != OK) {
        LOG_ERROR("Load the NNIE WK file failed, file path: %s", modelName.c_str());
        return ERR;
    } else {
        LOG_DEBUG("Load the NNIE WK file successfully, file path: %s", modelName.c_str());
        // 保存模型文件名
        m_modelName = modelName;
        return OK;
    }
}

int32_t Yolov3App::UnLoadModel(void)
{
    return SDC_UnLoadModel(&s_stYoloModel.stModel);
}

int32_t Yolov3App::NNieParamInit(void)
{
    stNnieCfg_SDC.u32MaxInputNum = 1;               // max input image num in each batch
    stNnieCfg_SDC.u32MaxRoiNum = 0;
    stNnieCfg_SDC.aenNnieCoreId[0] = SVP_NNIE_ID_0; // set NNIE core
    s_stYoloNnieParam.pstModel = &s_stYoloModel.stModel;
    if (SAMPLE_SVP_NNIE_Yolov3_ParamInit(fd_utils, &stNnieCfg_SDC, &s_stYoloNnieParam, &s_stYolov3SoftwareParam) != OK) {
        LOG_ERROR("Initalize the NNie parameter failed");
        return ERR;  
    } else {
        LOG_DEBUG("Initalize the NNie parameter successfully");
        return OK;
    }
}

int32_t Yolov3App::SetYuvChanAttr(sdc_yuv_channel_param_s &yuv_param)
{
    // 1. 动态获取一个可用的通道，多APP执行时需要跳过占用的通道
    if (SDC_YuvChnAttrGetIdleYuvChn(fd_video, &m_yuvChnId) != OK) {
        LOG_DEBUG("Get idle Yuv Channel failed, channel_id: %u", m_yuvChnId);
        return ERR;
    }
    LOG_DEBUG("Get idle Yuv Channel successfully, channel_id: %u", m_yuvChnId);

    // 2. 设置Yuv通道属性
    if (SDC_yuvchn_set_ext(fd_video, m_yuvChnId, &yuv_param) != OK) {
        LOG_ERROR("Set the attribute of Yuv Channel failed, channel_id: %u", m_yuvChnId);
        return ERR;
    }
    LOG_DEBUG("Set the attribute of Yuv Channel successfully, channel_id: %u", m_yuvChnId);

    // 3:获取视频的输入大小
    m_videoSize.ImageWidth = yuv_param.width;
    m_videoSize.ImageHeight = yuv_param.height;
    return OK;
}

void Yolov3App::Destroy(void)
{
    if (m_tidpProc != 0) {
        (void)pthread_join(m_tidpProc, NULL);
        m_tidpProc = 0;
    }

    if (m_tidpRecv != 0) {
        (void)pthread_join(m_tidpRecv, NULL);
        m_tidpRecv = 0;
    }
    // 卸载WK模型文件
    UnLoadModel();
}

uint32_t Yolov3App::GetYuvChannelId(void) const
{
    return m_yuvChnId;
}

int32_t Yolov3App::SubscribeYuvDataAndForward(std::shared_ptr<IYuvDataSrc> dataSrc)
{
    if (dataSrc == nullptr) {
        LOG_ERROR("The data source is not given");
        return ERR;
    }

    m_dataSrc = dataSrc;    // 保存外部数据源
    // 设置线程栈大小
    pthread_attr_t attr;
    (void)pthread_attr_init(&attr);
    (void)pthread_attr_setstacksize(&attr, DEF_THREAD_STACK_SIZE);  // 线程栈默认设置256KB

    // 创建线程处理YUV数据
    if ((pthread_create(&m_tidpProc, &attr, Yolov3App::YuvDataProcThrd, (void *)this)) == ERR) {
        pthread_attr_destroy(&attr);
        LOG_ERROR("Create the thread:YuvDataProcThrd() failed");
        return ERR;
    }
    LOG_DEBUG("Create the thread:YuvDataProcThrd() successfully");

    // 创建线程读视频服务线程
    if ((pthread_create(&m_tidpRecv, &attr, Yolov3App::VideoSrvReadThrd, (void *)this)) == ERR) {
        pthread_attr_destroy(&attr);
        LOG_ERROR("Create the thread:VideoSrvReadThrd() failed");
        return ERR;
    }
    pthread_attr_destroy(&attr);
    LOG_ERROR("Create the thread:VideoSrvReadThrd() successfully");
    return OK;
}

void* Yolov3App::YuvDataProcThrd(void *arg)
{
    Yolov3App *thiz = static_cast<Yolov3App *>(arg);
    if (thiz == nullptr) {
        return nullptr;
    }

    if (thiz->m_dataSrc == nullptr) {
        LOG_ERROR("The data source is not given");
        return nullptr;
    }

    uint8_t* pYuvForward = NULL;
    int wh = thiz->m_inputSize.ImageWidth * thiz->m_inputSize.ImageHeight;
    pYuvForward = (uint8_t *)malloc(sizeof(uint8_t) * wh * 3 / 2);
    if (pYuvForward == nullptr) {
        LOG_ERROR("YuvDataProcThrd malloc failed");
        return nullptr;
    }

    struct timespec time1 = {0, 0};
    struct timespec time2 = {0, 0};
    LOG_DEBUG("SDC_YuvDataProc pthread start");

    // 设置检测目标参数
    SDC_SSD_RESULT_S stResult;
    stResult.numOfObject = 10;                  // 设置推理目标最大数目
    stResult.thresh = thiz->m_confidenceThres;  // 设置可信度阈值
    SDC_SSD_OBJECT_INFO_S objectArr[10];        // 推理结果保存数组
    stResult.pObjInfo = objectArr;

    sdc_yuv_data_s stSdcYuvData;                // 定义YuvData用于取Yuv数据
    ProtoBuffer msgBuf;                         // 定义消息数组
    while (g_running == 1) {
        // 清空消息数组
        msgBuf.Clear();

        // 从队列里取出一帧数据进行处理
        int32_t iQueueState = thiz->m_yuvQue.PopQueue(&stSdcYuvData);
        if (iQueueState != QUEUE_STATE_OK) {
            LOG_ASSERT("QUE_PopQueue State:%d", iQueueState);
            continue;
        }

        // 对yuv数据帧进行预处理
        thiz->PreProcessYuvData(stSdcYuvData.frame, pYuvForward);
        int32_t pts = stSdcYuvData.pts;
        
        clock_gettime(CLOCK_BOOTTIME, &time1);
        // YuvFrame帧数据进行推理
        if (SDC_SVP_ForwardYUV((char *)pYuvForward, &stResult) != OK) {
            LOG_ERROR("Call SDC_SVP_ForwardBGR() failed");
        }
        clock_gettime(CLOCK_BOOTTIME, &time2);
        
        // 释放Yuv数据帧
        thiz->m_dataSrc->FreeYuvData(stSdcYuvData);
        // 构造检测目标消息数据
        thiz->ConstructMetaData(stResult, msgBuf);
        // 展示推理结果
        thiz->SendObjectsToSims(msgBuf, pts);
        // 计算推理时常
        LOG_DEBUG("Forward_time: %03lu ms", (time2.tv_sec - time1.tv_sec) *1000 + (time2.tv_nsec - time1.tv_nsec)/1000000);   
    }
    free(pYuvForward);
    return NULL;
}

bool Yolov3App::ConstructMetaData(const SDC_SSD_RESULT_S &stResult, ProtoBuffer &buf)
{
    std::vector<SDC_SSD_OBJECT_INFO_S> objectVect;  // 有效目标数组
    for (size_t i = 0; i < stResult.numOfObject; i++) {
        if (stResult.pObjInfo[i].confidence > m_confidenceThres) {
            // 打印识别度信息
            this->PrintObjectInfo(stResult.pObjInfo[i]);
            // 预处理识别数据
            this->PretreatObjectData(stResult.pObjInfo[i]);
            // 添加已经识别的目标
            objectVect.push_back(stResult.pObjInfo[i]);
        }
    }

    // 无检测目标，则提前退出
    if (objectVect.empty()) {
        LOG_ASSERT("Cannot foind the effective object");
        return false;
    }

    // 构造SIMS消息内容，消息格式参考：Label_event.h中struct Label的定义
    uint32_t flag = 1;                                  // 展示事件, 1：表示ADD，0：表示Delete
    char szAppName[32] = {0};                           // APP名字
    strncpy_s(szAppName, sizeof(szAppName), m_appName.c_str(), sizeof(szAppName) - 1);
    uint64_t channel_id = 0;                            // 通道ID
    uint16_t objectNum = (uint16_t)objectVect.size();   // 目标数量
    LOG_DEBUG("Effective objectNum = [%u], Total number Of Objects = [%d]", objectNum, stResult.numOfObject);

    // 构造APP信息
    buf << flag;
    buf.WriteBytes((void *)szAppName, sizeof(szAppName));
    buf << channel_id;
    buf << objectNum;
    
    // 构造检测目标矩形区域
    for (uint16_t i = 0; i < objectNum; i++) {
        this->ConstructRectAreaData(objectVect[i], buf);
    }
    
    // 构造（2 * objectNum）个字符串，一个检测目标包含2个信息：目标，置信度
    buf << (uint8_t) (objectNum * 2);                   // 字符串的个数小于256

    // 构造检测目标值和置信度信息
    for (uint16_t i = 0; i < objectNum; i++) {
        this->ConstructTargetData(objectVect[i], buf);
    }

    // 设置消息超时时间
    buf << (uint8_t)1;
    return true;
}

void *Yolov3App::VideoSrvReadThrd(void *arg)
{
    Yolov3App *thiz = static_cast<Yolov3App *>(arg);
    if (thiz == nullptr) {
        return nullptr;
    }

    if (thiz->m_dataSrc == nullptr) {
        LOG_ERROR("The data source is not given");
        return nullptr;
    }

    // 线程pthread开始运行
    LOG_DEBUG("VideoSrvReadThrd pthread start");
    sdc_yuv_data_s stSdcYuvData;
    while (g_running == 1) {
        int32_t iQueueStoreNum = thiz->m_yuvQue.GetUsedSize();
        if (iQueueStoreNum > 1) {
            LOG_ASSERT("Receive yuv data iQueueStoreNum: %d", iQueueStoreNum);
            continue;
        }

        // 从VideoStream/ImageFile中，获取Yuv数据帧
        if (thiz->m_dataSrc->GetYuvData(stSdcYuvData)) {
            int32_t iQueueState = thiz->m_yuvQue.PushQueue(&stSdcYuvData);
            if (iQueueState != QUEUE_STATE_OK) {
                LOG_ERROR("QUE_PushQueue State:%d!\n", iQueueState);
                thiz->m_dataSrc->FreeYuvData(stSdcYuvData);
                usleep(5000);
            }
        }
    }
    return nullptr;
}

void* Yolov3App::WatchdogThread(void *arg)
{
    Yolov3App *thiz = static_cast<Yolov3App *>(arg);
    if (thiz == nullptr) {
        return nullptr;
    }

    // 获取内存数据
    MemoryInfo meminfo;
    MemoryData mem;
    uint32_t usedMem = 0;
	uint32_t cpuUsage = 0;

    // 获取CPU数据
    CpuInfo cpuInfo;
    while (true) {
		meminfo.GetMemory(mem);
        meminfo.GetProcMem(usedMem);
		cpuInfo.CalcUsage(cpuUsage);
        LOG_DEBUG("total_memory:%uMB, sys_used_mem:%uMB, sys_mem_usage:%u%%, cur_used_mem:%uMB, cur_mem_usage:%u%%, cpu_usage:%u%%\n", \
            mem.uiMemTotal/1024, mem.uiUsedMem/1024, mem.uiUsedMem * 100 / mem.uiMemTotal, usedMem/1024, usedMem *100 /mem.uiMemTotal, cpuUsage);
        sleep(20);
    }
    return nullptr;
}

void Yolov3App::PreProcessYuvData(sdc_yuv_frame_s &frame, uint8_t *&yuv)
{
}

void Yolov3App::PrintObjectInfo(const SDC_SSD_OBJECT_INFO_S &info) const
{
}

bool Yolov3App::ConstructRectAreaData(const SDC_SSD_OBJECT_INFO_S &data, ProtoBuffer &buf)
{
    // 边框颜色，每通道一个字节，共14个元素，画线框共14个元素
    std::array<uint32_t, 14> rectData = {
        0xFF0000,               // 红色
        5,                      // 边框宽度
        0,                      // 是否填充底色，0：不填充，1：填充
        0xFF0000,               // 底色颜色
        0,                      // 底色透明度;
        4,                      // 坐标点个数
        (uint32_t)data.x_left,          // x1
        (uint32_t)data.y_top,           // y1
        (uint32_t)data.x_left,          // x2
        (uint32_t)data.y_top + data.h,  // y2
        (uint32_t)data.x_left + data.w, // x3
        (uint32_t)data.y_top + data.h,  // y3
        (uint32_t)data.x_left + data.w, // x4
        (uint32_t)data.y_top,           // y4
    };


    // 向ProtoBuffer数组添加数据
    for (auto &ele : rectData) {
        buf << ele;
    }

    LOG_DEBUG("Construct the Rect area successfully");
    return true;
}

bool Yolov3App::ConstructTargetData(const SDC_SSD_OBJECT_INFO_S &data, ProtoBuffer &buf)
{
    uint32_t color = 0xFF0000;          // 颜色
    char font[32] = {0};                // 字体类型
    strncpy_s(font, sizeof(font), "宋体", sizeof(font) - 1);
    uint32_t fontSize = 32;             // 字体大小
    uint32_t x = data.x_left + 25;      // 检测目标坐标x
    uint32_t y = data.y_top + 25;       // 检测目标坐标y

    // 向ProtoBuffer数组添加数据
    buf << color;
    buf.WriteBytes(font, sizeof(font));
    buf << fontSize << x << y;

    uint8_t uclass[20] = {0};           // 检测目标名称
    // 构造目标识别信息, 将c_class对应的数字，映射为具体目标
    ClassifyTargets(data.c_class, uclass);
    buf << (uint32_t)sizeof(uclass);
    buf.WriteBytes(uclass, sizeof(uclass));

    x = data.x_left + 25;               // 置信度坐标x
    y = data.y_top +  600;              // 置信度坐标y

    buf << color;
    buf.WriteBytes(font, sizeof(font));
    buf << fontSize << x << y;

    char confidence[32] = {0};
    // 汉字“置信度”对应的GB2312编码
    uint8_t tips[] = {0xD6, 0xC3, 0xD0, 0xC5, 0xB6, 0xC8, 0x00};
    sprintf_s(confidence, sizeof(confidence), "%s %2.2f%%", tips, (float)data.confidence * 100);

    buf << (uint32_t)sizeof(confidence);
    buf.WriteBytes(confidence, sizeof(confidence));
  
    LOG_DEBUG("Construct the target data successfully");
    return true;
}

void Yolov3App::SendObjectsToSims(ProtoBuffer &buf, int32_t pts) const
{
    if (buf.Empty()) {
        LOG_ASSERT("The message buff is empty");
        return;
    }

    // 清除所有元数据
    (void)SDC_LabelEventDel(fd_event, 0, 0, (char *)m_appName.c_str());
    (void)SDC_LabelEventPublish(fd_event, 0, buf.GetUsed(), (char *)buf.GetData(), pts);
}
} // namespace HWYolov3App
