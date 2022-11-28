#include "Alarm.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <securec.h>
#include <utility>
#include "sdc.h"
#include "common_defs.h"


Alarm::Alarm(const string &filePath) : m_alarmFd(-1), m_filePath(filePath), m_ptrParam(nullptr), m_size(0)
{
    LOG_DEBUG("Create the Alarm instance");
}

Alarm::~Alarm(void)
{
    if (m_ptrParam != nullptr) {
        free(m_ptrParam);
        m_ptrParam = nullptr;
    }

    if (m_alarmFd != -1) {
        close(m_alarmFd);
    }
	
	LOG_DEBUG("Destroy the Alarm instance");
}

// 注册App
int32_t Alarm::RegisterAlarm(void)
{
    // 在8.20之前的版本, alarmlist.ini的路径长度不能超过32位，在8.20之后的版本，alarmlist.ini的路径长度不能超过64位
    if (m_filePath.length() > 32 || m_filePath.length() == 0) {
        LOG_ERROR("The length of the file path is more than 32");
        return ERR;
    }

    // 判断配置文件是否存在
    if (access(m_filePath.c_str(), F_OK) != 0) {
        LOG_ERROR("The config file does not exist, errno: %d, errmsg: %s", errno, strerror(errno));
        return ERR;
    }

    m_alarmFd = open("/mnt/srvfs/alarm.paas.sdc", O_RDWR);
	if(m_alarmFd < 0) {
        LOG_ERROR("Open the alarm.paas.sdc failed, errno: %d, errmsg: %s", errno, strerror(errno));
        return ERR;
    }

    sdc_common_head_s head;
    memset(&head, 0, sizeof(head));
    head.version = SDC_VERSION;
    head.url = ALARM_URI_INPUT_REGISTER;
    head.method = HBTP_METHOD_CREATE;
    head.content_length = m_filePath.length();
    head.head_length = sizeof(head);
    
    struct iovec iov[2] = {
        {.iov_base = &head, .iov_len = sizeof(head)},
        {.iov_base = (void *) m_filePath.c_str(), .iov_len = m_filePath.length()}
    };

    int32_t retVal = writev(m_alarmFd, iov, 2);
    if(retVal < 0) {
        LOG_ERROR("Write the alarm config data failed, errno: %d, errmsg: %s", errno, strerror(errno));
        return ERR;
    }

    retVal = read(m_alarmFd, &head, sizeof(head));
    if(retVal < 0 || head.code != SDC_CODE_200) {
        LOG_ERROR("Read the sdc_common_head data failed, retVal: %d, head.code: %d", retVal, head.code);
        return ERR;
	}
    LOG_DEBUG("Create the alarm Source successfully, code:%d filepath:%s", head.code,m_filePath);

    return 0;
}

int32_t Alarm::PublishAlarmEvent(const uint8_t *data, size_t length)
{
    LOG_DEBUG("Left Value assignemt: PublishAlarmEvent");
    // 构造发送数据
    if (ConstructAlarmParam(data, length) < 0) {
        return ERR;
    }
    return AlarmEventHandle(HBTP_METHOD_CREATE);
}

int32_t Alarm::PublishAlarmEvent(uint8_t* &&data, size_t length)
{
    LOG_DEBUG("Right Value assignemt: PublishAlarmEvent");
    // 构造发送数据
    if (ConstructAlarmParam(std::forward<uint8_t*>(data), length) < 0) {
        return ERR;
    }
    return AlarmEventHandle(HBTP_METHOD_CREATE);
}

int32_t Alarm::DeleteAlarmEvent(void)
{
    return AlarmEventHandle(HBTP_METHOD_DELETE);
}

// 告警事件发布
int Alarm::AlarmEventHandle(HBTP_METHOD method)
{
    sdc_common_head_s head;
    memset(&head, 0, sizeof(head));
    head.version = SDC_VERSION;
    head.url = ALARM_URI_EVENT_PUBLISH;
    head.method = method;
    head.content_length = sizeof(ALARM_REPORT_PARAM) + m_size;  // 添加数据部分长度
    head.head_length = sizeof(head);
    struct iovec iov[2] = {
        {.iov_base = &head, .iov_len = sizeof(head)},
        {.iov_base = m_ptrParam, .iov_len = sizeof(ALARM_REPORT_PARAM) + m_size}
    };
    
    int32_t retVal = writev(m_alarmFd, iov, 2);
    if(retVal < 0) {
        LOG_ERROR("Write the alarm data failed, errno: %d, errmsg: %s", errno, strerror(errno));
        return ERR;
    }

    retVal = read(m_alarmFd, &head, sizeof(head));
    if(retVal < 0 || head.code != SDC_CODE_200) {
        LOG_ERROR("Read the sdc_common_head data failed, errno: %d, errmsg: %s", errno, strerror(errno));
        return ERR;
    }
    LOG_DEBUG("Handle the alarm Event successfully, code:%d", head.code);

    return 0;
}

int32_t Alarm::ConstructAlarmParam(const uint8_t *data, size_t length)
{
    if (m_ptrParam != nullptr) {
        free(m_ptrParam);
        m_ptrParam = nullptr;
    }

    m_ptrParam = (ALARM_REPORT_PARAM *) malloc(sizeof(ALARM_REPORT_PARAM) + length);
    if (m_ptrParam == nullptr) {
        LOG_ERROR("Allocate the memory failed, errno: %d, errmsg: %s", errno, strerror(errno));
        return ERR;
    }
    
    (void) memset_s(m_ptrParam, sizeof(ALARM_REPORT_PARAM) + length, 0, m_size);
    // 针对alarmName, alarmSource取值，用户根据实际情况进行修改
    (void) memcpy_s(m_ptrParam->alarmName, sizeof(m_ptrParam->alarmName) - 1, "face", strlen("ptz_action")); // 反斜杠
    (void) memcpy_s(m_ptrParam->alarmSource, sizeof(m_ptrParam->alarmSource) - 1, "test", strlen("ptz")); // 反斜杠
    (void) memcpy_s(m_ptrParam->metaData, length, data, length);

    m_size = length;
    return OK;
}

// 如果消息内容不会变化，则走该函数，避免内存无限制释放
int32_t Alarm::ConstructAlarmParam(uint8_t* &&data, size_t length)
{
    if (m_ptrParam != nullptr) {
        return OK;
    }

    m_ptrParam = (ALARM_REPORT_PARAM *) malloc(sizeof(ALARM_REPORT_PARAM) + length);
    if (m_ptrParam == nullptr) {
        LOG_ERROR("Allocate the memory failed, errno: %d, errmsg: %s", errno, strerror(errno));
        return ERR;
    }

    (void) memset_s(m_ptrParam, sizeof(ALARM_REPORT_PARAM) + length, 0, m_size);
    // 针对alarmName, alarmSource取值，用户根据实际情况进行修改
    (void) memcpy_s(m_ptrParam->alarmName, sizeof(m_ptrParam->alarmName) - 1, "ptz_action", strlen("ptz_action")); // 反斜杠
    (void) memcpy_s(m_ptrParam->alarmSource, sizeof(m_ptrParam->alarmSource) - 1, "ptz", strlen("ptz")); // 反斜杠
    (void) memcpy_s(m_ptrParam->metaData, length, data, length);

    m_size = length;
    return OK;
}
