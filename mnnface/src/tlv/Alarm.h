#ifndef __MULTI_OSD_H__
#define __MULTI_OSD_H__

#include <string>
#include "sdc.h"

typedef enum {
    ALARM_URI_CONFIG_PARAM      = 0,
    ALARM_URI_INPUT_REGISTER    = 1,
    ALARM_URI_OUTPUT_REGISTER   = 2,
    ALARM_URI_EVENT_PUBLISH     = 3,
    ALARM_URI_SOURCE_PARAM      = 4,
    ALARM_URI_ACTION_PARAM      = 5,
    ALARM_URI_APP_PARAM         = 6,
    ALARM_URI_APPLANG_PARAM     = 7,
    ALARM_URI_ACTIONLANG_PARAM  = 8,
    ALARM_URI_RESTORE_DEFAULT_DATA = 9,
} ALARM_URI_RES;

typedef enum {
    HBTP_METHOD_UNDEF   = 0,
    HBTP_METHOD_CREATE  = 1,
    HBTP_METHOD_GET     = 2,
    HBTP_METHOD_UPDATE  = 3,
    HBTP_METHOD_DELETE  = 4,
    HBTP_METHOD_BUTT,
} HBTP_METHOD;

typedef struct {
    char alarmName[64];     // 告警名称
    char alarmSource[32];   // app名称
    char metaData[0];       // 告警数据
} ALARM_REPORT_PARAM;


using std::string;
class Alarm
{
public:
    // filePath 为alarmlist.ini文件的绝对路径
    Alarm(const string &filePath);
    virtual ~Alarm(void);
    virtual int32_t RegisterAlarm(void);
    // 如果消息内容频繁的变化，则调用此函数
    int32_t PublishAlarmEvent(const uint8_t *data, size_t length);
    // 如果消息内容不会变化，则调用此函数，避免内存重复的申请和释放
    int32_t PublishAlarmEvent(uint8_t* &&data, size_t length);
    int32_t DeleteAlarmEvent(void);
   
private:
    int32_t AlarmEventHandle(HBTP_METHOD method);
    int32_t ConstructAlarmParam(const uint8_t *data, size_t length);
    int32_t ConstructAlarmParam(uint8_t* &&data, size_t length);
    int32_t m_alarmFd;      // 告警事件FD
    string  m_filePath;     // 告警配置文件名
    ALARM_REPORT_PARAM *m_ptrParam; // 告警参数
    size_t  m_size;         // 告警数据长度
};

#endif