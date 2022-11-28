/******************************************************************************

                  ��Ȩ���� (C), 2019-2029, SDC OS ��Դ���С������

 ******************************************************************************
  �� �� ��   : yolov3_app.cpp
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

#define MODEL_INPUT_SIZE_WIDTH               416  // ģ�������С��
#define MODEL_INPUT_SIZE_HEIGHT              416  // ģ�������С��
namespace HWYolov3App
{
float Yolov3App::m_confidenceThres = 0.5;
// ��󻺴�YUV֡����Ϊ10(SDC_APP����ָ��Ĭ����10)
const int MAX_CACHED_COUNT = 10;
// Ĭ���̶߳�ջ��С
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
    // 1. ���ļ���������
	if (SDC_ServiceCreate() != OK) {
        LOG_ERROR("Create the SDC_OS service failed");
        return ERR;
    }
    LOG_ERROR("Create the SDC_OS service successfully");

    // 2. ��ȡӲ��ID
    sdc_hardware_id_s stHardWareParas;
    if (SDC_GetHardWareId(&stHardWareParas) != OK) {
        LOG_ERROR("Get the HardWare Id failed, channel_id: %s", stHardWareParas.id);
    }
    LOG_DEBUG("Get the HardWare Id successfully, channel_id: %s", stHardWareParas.id);

    // 2. ����YuV����֡����
    if (m_yuvQue.CreateQueue(Yolov3App::MAX_YUV_BUF_NUM) == ERR) {
        LOG_ERROR("Create the Yuv ring queue failed, buffer size: %d", Yolov3App::MAX_YUV_BUF_NUM);
        return ERR;
    }
    LOG_DEBUG("Create the Yuv ring queue successfully, buffer size: %d", Yolov3App::MAX_YUV_BUF_NUM);
    
    m_inputSize.ImageWidth  = MODEL_INPUT_SIZE_WIDTH;
    m_inputSize.ImageHeight = MODEL_INPUT_SIZE_HEIGHT;

    m_appName = appName;

    // ����ϵͳ��Դ�����߳�
    pthread_attr_t attr;
    (void)pthread_attr_init(&attr);
    (void)pthread_attr_setstacksize(&attr, DEF_THREAD_STACK_SIZE);  // �߳�ջĬ������256KB
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
        // ����ģ���ļ���
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
    // 1. ��̬��ȡһ�����õ�ͨ������APPִ��ʱ��Ҫ����ռ�õ�ͨ��
    if (SDC_YuvChnAttrGetIdleYuvChn(fd_video, &m_yuvChnId) != OK) {
        LOG_DEBUG("Get idle Yuv Channel failed, channel_id: %u", m_yuvChnId);
        return ERR;
    }
    LOG_DEBUG("Get idle Yuv Channel successfully, channel_id: %u", m_yuvChnId);

    // 2. ����Yuvͨ������
    if (SDC_yuvchn_set_ext(fd_video, m_yuvChnId, &yuv_param) != OK) {
        LOG_ERROR("Set the attribute of Yuv Channel failed, channel_id: %u", m_yuvChnId);
        return ERR;
    }
    LOG_DEBUG("Set the attribute of Yuv Channel successfully, channel_id: %u", m_yuvChnId);

    // 3:��ȡ��Ƶ�������С
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
    // ж��WKģ���ļ�
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

    m_dataSrc = dataSrc;    // �����ⲿ����Դ
    // �����߳�ջ��С
    pthread_attr_t attr;
    (void)pthread_attr_init(&attr);
    (void)pthread_attr_setstacksize(&attr, DEF_THREAD_STACK_SIZE);  // �߳�ջĬ������256KB

    // �����̴߳���YUV����
    if ((pthread_create(&m_tidpProc, &attr, Yolov3App::YuvDataProcThrd, (void *)this)) == ERR) {
        pthread_attr_destroy(&attr);
        LOG_ERROR("Create the thread:YuvDataProcThrd() failed");
        return ERR;
    }
    LOG_DEBUG("Create the thread:YuvDataProcThrd() successfully");

    // �����̶߳���Ƶ�����߳�
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

    // ���ü��Ŀ�����
    SDC_SSD_RESULT_S stResult;
    stResult.numOfObject = 10;                  // ��������Ŀ�������Ŀ
    stResult.thresh = thiz->m_confidenceThres;  // ���ÿ��Ŷ���ֵ
    SDC_SSD_OBJECT_INFO_S objectArr[10];        // ��������������
    stResult.pObjInfo = objectArr;

    sdc_yuv_data_s stSdcYuvData;                // ����YuvData����ȡYuv����
    ProtoBuffer msgBuf;                         // ������Ϣ����
    while (g_running == 1) {
        // �����Ϣ����
        msgBuf.Clear();

        // �Ӷ�����ȡ��һ֡���ݽ��д���
        int32_t iQueueState = thiz->m_yuvQue.PopQueue(&stSdcYuvData);
        if (iQueueState != QUEUE_STATE_OK) {
            LOG_ASSERT("QUE_PopQueue State:%d", iQueueState);
            continue;
        }

        // ��yuv����֡����Ԥ����
        thiz->PreProcessYuvData(stSdcYuvData.frame, pYuvForward);
        int32_t pts = stSdcYuvData.pts;
        
        clock_gettime(CLOCK_BOOTTIME, &time1);
        // YuvFrame֡���ݽ�������
        if (SDC_SVP_ForwardYUV((char *)pYuvForward, &stResult) != OK) {
            LOG_ERROR("Call SDC_SVP_ForwardBGR() failed");
        }
        clock_gettime(CLOCK_BOOTTIME, &time2);
        
        // �ͷ�Yuv����֡
        thiz->m_dataSrc->FreeYuvData(stSdcYuvData);
        // ������Ŀ����Ϣ����
        thiz->ConstructMetaData(stResult, msgBuf);
        // չʾ������
        thiz->SendObjectsToSims(msgBuf, pts);
        // ��������ʱ��
        LOG_DEBUG("Forward_time: %03lu ms", (time2.tv_sec - time1.tv_sec) *1000 + (time2.tv_nsec - time1.tv_nsec)/1000000);   
    }
    free(pYuvForward);
    return NULL;
}

bool Yolov3App::ConstructMetaData(const SDC_SSD_RESULT_S &stResult, ProtoBuffer &buf)
{
    std::vector<SDC_SSD_OBJECT_INFO_S> objectVect;  // ��ЧĿ������
    for (size_t i = 0; i < stResult.numOfObject; i++) {
        if (stResult.pObjInfo[i].confidence > m_confidenceThres) {
            // ��ӡʶ�����Ϣ
            this->PrintObjectInfo(stResult.pObjInfo[i]);
            // Ԥ����ʶ������
            this->PretreatObjectData(stResult.pObjInfo[i]);
            // ����Ѿ�ʶ���Ŀ��
            objectVect.push_back(stResult.pObjInfo[i]);
        }
    }

    // �޼��Ŀ�꣬����ǰ�˳�
    if (objectVect.empty()) {
        LOG_ASSERT("Cannot foind the effective object");
        return false;
    }

    // ����SIMS��Ϣ���ݣ���Ϣ��ʽ�ο���Label_event.h��struct Label�Ķ���
    uint32_t flag = 1;                                  // չʾ�¼�, 1����ʾADD��0����ʾDelete
    char szAppName[32] = {0};                           // APP����
    strncpy_s(szAppName, sizeof(szAppName), m_appName.c_str(), sizeof(szAppName) - 1);
    uint64_t channel_id = 0;                            // ͨ��ID
    uint16_t objectNum = (uint16_t)objectVect.size();   // Ŀ������
    LOG_DEBUG("Effective objectNum = [%u], Total number Of Objects = [%d]", objectNum, stResult.numOfObject);

    // ����APP��Ϣ
    buf << flag;
    buf.WriteBytes((void *)szAppName, sizeof(szAppName));
    buf << channel_id;
    buf << objectNum;
    
    // ������Ŀ���������
    for (uint16_t i = 0; i < objectNum; i++) {
        this->ConstructRectAreaData(objectVect[i], buf);
    }
    
    // ���죨2 * objectNum�����ַ�����һ�����Ŀ�����2����Ϣ��Ŀ�꣬���Ŷ�
    buf << (uint8_t) (objectNum * 2);                   // �ַ����ĸ���С��256

    // ������Ŀ��ֵ�����Ŷ���Ϣ
    for (uint16_t i = 0; i < objectNum; i++) {
        this->ConstructTargetData(objectVect[i], buf);
    }

    // ������Ϣ��ʱʱ��
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

    // �߳�pthread��ʼ����
    LOG_DEBUG("VideoSrvReadThrd pthread start");
    sdc_yuv_data_s stSdcYuvData;
    while (g_running == 1) {
        int32_t iQueueStoreNum = thiz->m_yuvQue.GetUsedSize();
        if (iQueueStoreNum > 1) {
            LOG_ASSERT("Receive yuv data iQueueStoreNum: %d", iQueueStoreNum);
            continue;
        }

        // ��VideoStream/ImageFile�У���ȡYuv����֡
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

    // ��ȡ�ڴ�����
    MemoryInfo meminfo;
    MemoryData mem;
    uint32_t usedMem = 0;
	uint32_t cpuUsage = 0;

    // ��ȡCPU����
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
    // �߿���ɫ��ÿͨ��һ���ֽڣ���14��Ԫ�أ����߿�14��Ԫ��
    std::array<uint32_t, 14> rectData = {
        0xFF0000,               // ��ɫ
        5,                      // �߿���
        0,                      // �Ƿ�����ɫ��0������䣬1�����
        0xFF0000,               // ��ɫ��ɫ
        0,                      // ��ɫ͸����;
        4,                      // ��������
        (uint32_t)data.x_left,          // x1
        (uint32_t)data.y_top,           // y1
        (uint32_t)data.x_left,          // x2
        (uint32_t)data.y_top + data.h,  // y2
        (uint32_t)data.x_left + data.w, // x3
        (uint32_t)data.y_top + data.h,  // y3
        (uint32_t)data.x_left + data.w, // x4
        (uint32_t)data.y_top,           // y4
    };


    // ��ProtoBuffer�����������
    for (auto &ele : rectData) {
        buf << ele;
    }

    LOG_DEBUG("Construct the Rect area successfully");
    return true;
}

bool Yolov3App::ConstructTargetData(const SDC_SSD_OBJECT_INFO_S &data, ProtoBuffer &buf)
{
    uint32_t color = 0xFF0000;          // ��ɫ
    char font[32] = {0};                // ��������
    strncpy_s(font, sizeof(font), "����", sizeof(font) - 1);
    uint32_t fontSize = 32;             // �����С
    uint32_t x = data.x_left + 25;      // ���Ŀ������x
    uint32_t y = data.y_top + 25;       // ���Ŀ������y

    // ��ProtoBuffer�����������
    buf << color;
    buf.WriteBytes(font, sizeof(font));
    buf << fontSize << x << y;

    uint8_t uclass[20] = {0};           // ���Ŀ������
    // ����Ŀ��ʶ����Ϣ, ��c_class��Ӧ�����֣�ӳ��Ϊ����Ŀ��
    ClassifyTargets(data.c_class, uclass);
    buf << (uint32_t)sizeof(uclass);
    buf.WriteBytes(uclass, sizeof(uclass));

    x = data.x_left + 25;               // ���Ŷ�����x
    y = data.y_top +  600;              // ���Ŷ�����y

    buf << color;
    buf.WriteBytes(font, sizeof(font));
    buf << fontSize << x << y;

    char confidence[32] = {0};
    // ���֡����Ŷȡ���Ӧ��GB2312����
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

    // �������Ԫ����
    (void)SDC_LabelEventDel(fd_event, 0, 0, (char *)m_appName.c_str());
    (void)SDC_LabelEventPublish(fd_event, 0, buf.GetUsed(), (char *)buf.GetData(), pts);
}
} // namespace HWYolov3App
