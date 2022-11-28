#ifndef __META_EVENT_H__
#define __META_EVENT_H__

#include <stdint.h>
#include <string>
#include "sdc.h"
#include "common_defs.h"


#define SUBSCRIBER_NAME     "EventSubscriber"
#define ITIT_META_DATA      "itgt.saas.sdc"




struct tlv {
    uint32_t tag;
    uint32_t len;
    char val[0];
};



using std::string;
class EventSubscriber
{
public:
    EventSubscriber(void);
    virtual ~EventSubscriber(void);
    bool Init(void);
    void Close(void);
    bool Subscribe(const string &filter) const;
    // bool ReadEvent(void);

// protected:
//     // 解析元数据事件，需要改写ParseTLVData()函数
//     virtual bool ParseTLVData(struct paas_event *event) const;
    
private:
//     bool shm_mmap(struct sdc_shm_cache *shm);
//     void shm_free(struct sdc_shm_cache *shm);

    int32_t m_fdEvent;      // 元数据FD
    int32_t m_fdCache;      // Cache
};

#endif /* __META_EVENT_H__ */