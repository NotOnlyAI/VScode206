
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "EventSubscriber.h"
#include "sdc.h"
#include "sdc_service.h"


#define TLV_NEXT(tlv_ptr) ((struct tlv *)((tlv_ptr)->val + (tlv_ptr)->len))

#define tlv_for_each(tlv_ptr, item, end) \
        for ((item) = (typeof(item))(tlv_ptr); (struct tlv *)(item) < (struct tlv *)(end) && TLV_NEXT(item) <= (struct tlv *)(end); (item) = TLV_NEXT(item))

#define TLV_CAR_NUMBER 0x0A000008


extern int fd_cache;
extern int fd_event;

EventSubscriber::EventSubscriber(void) : m_fdEvent(-1), m_fdCache(-1)
{
    LOG_DEBUG("Create the meta data subscriber instance");
}

EventSubscriber::~EventSubscriber(void)
{
    LOG_DEBUG("Destroy the meta data subscriber instance");
    // Close();
}

bool EventSubscriber::Init(void)
{
    
    // m_fdEvent = open("/mnt/srvfs/event.paas.sdc", O_RDWR);
    m_fdEvent=fd_event;
    if (m_fdEvent < 0) {
        LOG_ERROR("Open the /mnt/srvfs/event.paas.sdc failed, errno: %d, errmsg: %s", errno, strerror(errno));
        return false;
    }
    LOG_DEBUG("Open the /mnt/srvfs/event.paas.sdc successfully, event_fd: %d", m_fdEvent);

    // m_fdCache = open("/dev/cache", O_RDWR | O_CLOEXEC);
    m_fdCache=fd_cache;
    if (m_fdCache < 0) {
        LOG_ERROR("Open the /dev/cache failed, errno: %d, errmsg: %s", errno, strerror(errno));
        return false;
    }
    LOG_DEBUG("Open the /dev/cache successfully, cache_fd: %d", m_fdCache);

    return true;
}

void EventSubscriber::Close(void)
{
    if (m_fdEvent != -1) {
        close(m_fdEvent);
        m_fdEvent = -1;
    }

    if (m_fdCache != -1) {
        close(m_fdCache);
        m_fdCache = -1;
    }
}

bool EventSubscriber::Subscribe(const string &filter) const
{
    // SUBSCRIBER_NAME，filter由厂商自定义，满足SDC_APP开发要求即可
    paas_event_filter_s filters;
    memset(&filters, 0, sizeof(filters));
    memcpy(filters.subscriber, SUBSCRIBER_NAME, sizeof(filters.subscriber));
    memcpy(filters.name, ITIT_META_DATA, sizeof(filters.name));
    memcpy(filters.filter, filter.c_str(), sizeof(filters.filter));

    sdc_common_head_s head = {0};
    memset(&head, 0, sizeof(head));
    head.version = SDC_VERSION;
    head.url = SDC_URL_PAAS_EVENTD_EVENT;
    head.method = SDC_METHOD_GET;
    head.head_length = sizeof(head);
    head.content_length = sizeof(filters);

    struct iovec iov[] = {
        {.iov_base = &head, .iov_len = sizeof(head)},
        {.iov_base = (void *)&filters, .iov_len = sizeof(filters)},  
    };

    ssize_t retVal = writev(m_fdEvent, iov, 2);
    if (retVal < 0) {
        LOG_ERROR("Subscribe the meta failed, meta_fd: %d, errno: %d, errmsg: %s", m_fdEvent, errno, strerror(errno));
        return false;
    }

    LOG_DEBUG("Subscribe the meta successfully, meta_fd: %d", m_fdEvent);
    return true;
}

// bool EventSubscriber::ReadEvent(void)
// {
//     uint8_t buf[4096] = {0};
//     int32_t recvBytes = read(m_fdEvent, buf, sizeof(buf));
//     if (recvBytes < 0) {
//         LOG_ERROR("Read the message from config.paas.sdc failed, errno: %d, errmsg: %s", errno, strerror(errno));
//         return false;
//     }

//     struct sdc_common_head *head = (struct sdc_common_head *)buf;
//     if (head->url != SDC_URL_PAAS_EVENTD_EVENT) {
//         return true;
//     }
    
//     struct paas_shm_cached_event *shm_event = (paas_shm_cached_event *)&buf[head->head_length];


//     sdc_shm_cache shm_cache;
//     memset(&shm_cache, 0, sizeof(shm_cache));
//     shm_cache.addr_virt=nullptr;
//     shm_cache.addr_phy= (unsigned long) shm_event->addr_phy;
//     shm_cache.size=shm_event->size;
//     shm_cache.cookie=shm_event->cookie;

//     // struct sdc_shm_cache shm_cache = {
//     //     .addr_virt = nullptr,
//     //     .addr_phy = (unsigned long) shm_event->addr_phy,
//     //     .size = shm_event->size,
//     //     .cookie = shm_event->cookie,
//     // };

//     // 映射shm内存
//     if (!shm_mmap(&shm_cache)) {
//         LOG_ERROR("Attach the shared memory failed");
//         return false;
//     }

//     struct paas_event *event = (struct paas_event *)shm_cache.addr_virt;
//     // 解析TLV数据
//     (void) ParseTLVData(event);
//     // 释放shm内存
//     shm_free(&shm_cache);

//     return true;
// }

// bool EventSubscriber::ParseTLVData(struct paas_event *event) const
// {
//     /**
//      * 元数据定义参考：《华为SDC 8.0.2全网智能接口TLV数据详解》
//      * 解析元数据三层TLV里面车牌信息参考
//     struct tlv* tlv_1 = (struct tlv*)paas_event->data;
//     struct tlv* tlv_1_end = (struct tlv*)&paas_event->data[paas_event->length];
//     struct tlv* tlv_2;
//     struct tlv* tlv_3;
//     int has_car = 0;

//     tlv_for_each (tlv_1->val, tlv_2, tlv_1_end) {
//         tlv_for_each (tlv_2->val, tlv_3, TLV_NEXT(tlv_2)) {
//             LOGD("tlv3_type: %X, len: %d", tlv_3->type, tlv_3->len);
//             if (tlv_3->type == TLV_CAR_NUMBER) {
//                 has_car = 1;
//                 ++module->all_count;
//                 snprintf(module->last_car, sizeof(module->last_car), "%.*s", tlv_3->len, tlv_3->val);
//                 LOGI("car nubmer: %s, last: %s", module->car_number, module->last_car);
//                 if (strcmp(module->last_car, module->car_number) == 0) {
//                     ++module->count;
//                     snprintf(module->str_count, sizeof(module->str_count), "%u", module->count);
//                     osd_update(OSD_KEY_COUNTER, module->str_count);
//                 }
//             }
//         }
//     }
//     *
//     * 
//     */

//     printf("name = %s, id = %d, length=%d, data=[", event->name, event->id, event->length);
//     for (uint32_t i = 0; i < event->length; ++i) {
//         printf("%d,", event->data[i]);
//     }
//     printf("]\n");
//     return true;
// }

// bool EventSubscriber::shm_mmap(struct sdc_shm_cache *shm)
// {
//     int nret = ioctl(m_fdCache, SDC_CACHE_MMAP, shm);
//     if (nret < 0) {
//         LOG_ERROR("Execute the ioctl() failed, errno: %d, errmsg: %s", errno, strerror(errno));
//         return false;
//     }

//     return shm->addr_virt != nullptr;
// }

// void EventSubscriber::shm_free(struct sdc_shm_cache* shm)
// {
//     if (shm->addr_virt != nullptr) {
//         munmap(shm->addr_virt, shm->size);
//         shm->addr_virt = nullptr;
//     }
//     shm->size = 0;
// }
