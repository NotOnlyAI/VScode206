#ifndef __EVENT_PUBLISHER_H__
#define __EVENT_PUBLISHER_H__

#include <stdint.h>
#include <stdlib.h>
#include "sdc.h"
#include "common_defs.h"

#define ITIT_META_DATA      "itgt.saas.sdc"

struct sdc_shm_cache
{
    void* addr_virt;
    unsigned long addr_phy;
    unsigned int size;
    unsigned int cookie;
};

struct paas_event
{
        char publisher[16]; 
        char name[16]; 
        uint64_t src_timestamp;
        uint64_t tran_timestamp;
        uint32_t id;
        uint32_t length;
        char data[0];
};


// 元数据事件发布者模拟程序
class EventPublisher
{
public:
    EventPublisher(void);
    virtual ~EventPublisher(void);
    int32_t Init(void);
    void    Close(void);
    int32_t SendEvent(const uint8_t data[], int length);

private:
    int32_t shm_copy(const uint8_t data[], size_t len, sdc_shm_cache *shm);
//     int32_t shm_alloc(struct sdc_shm_cache *shm);
//     void    shm_free(struct sdc_shm_cache *shm);
    int32_t m_fdEvent;
    int32_t m_fdCache;
    int32_t m_eventID;  // 事件ID用户自定义修改
};

#endif /* __EVENT_PUBLISHER_H__ */