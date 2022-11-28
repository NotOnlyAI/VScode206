#include "EventPublisher.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <errno.h>
#include "sdc_service.h"

extern int fd_cache;
extern int fd_event;

EventPublisher::EventPublisher(void) : m_fdEvent(-1), m_fdCache(-1), m_eventID(100)
{
    LOG_DEBUG("Create the EventPublisher instance");
}

EventPublisher::~EventPublisher(void)
{
    LOG_DEBUG("Destroy the EventPublisher instance");
    Close();
}

int32_t EventPublisher::Init(void)
{
    // m_fdEvent = open("/mnt/srvfs/event.paas.sdc", O_RDWR);
    m_fdEvent=fd_event;
    if (m_fdEvent < 0) {
        LOG_ERROR("m_fdEvent < 0,goto fail;\n");
        return ERR;
    }

    // m_fdCache = open("/dev/cache", O_RDWR);
    m_fdCache = fd_cache;
    if (m_fdCache < 0) {
        LOG_ERROR("m_fdCache < 0,goto fail, errno: %d, errmsg: %s;\n", errno, strerror(errno));
        return ERR;
    }
    return OK;
}

void EventPublisher::Close(void)
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

int32_t EventPublisher::shm_copy(const uint8_t data[], size_t len, sdc_shm_cache *shm)
{
	//共享内存的长度包括事件内容和事件头的长度
	memset(shm, 0, sizeof(*shm));
	shm->size = len + sizeof(pass_event);
	int32_t retVal = shm_alloc(shm);
	if(retVal != 0) {
        return ERR;
    }
  
	//拷贝事件到共享内存空间并设置事件头部信息
	struct pass_event *head = (pass_event*)shm->addr_virt;
	strncpy(head->name , ITIT_META_DATA, sizeof(head->name));
	head->id = m_eventID;   // 事件ID，用户自定义修改
	head->length = len;
	
	memcpy(head + 1, data, len);
    return OK;
}

// /** 申请共享内存的地址用于拷贝元数据内容 */
// int32_t EventPublisher::shm_alloc(struct sdc_shm_cache *shm)
// {
// 	int32_t retVal = ioctl(m_fdCache, SDC_CACHE_ALLOC, shm);
//     if(retVal != 0) {
//         LOG_ERROR("Allocate the shared memory failed, cache_fd: %d, errno: %d, errmsg: %s", m_fdCache, errno, strerror(errno));
//         return ERR;
//     }
// 	return OK;
// }

// void EventPublisher::shm_free(struct sdc_shm_cache* shm)
// {
//     if (shm->addr_virt != nullptr) {
//         munmap(shm->addr_virt, shm->size);
//         shm->addr_virt = nullptr;
//     }
//     shm->size = 0;
// }

int32_t EventPublisher::SendEvent(const uint8_t data[], int length)
{

    sdc_extend_head_s shm_head = {
        .type = SDC_HEAD_SHM_CACHED_EVENT,
        .length = 8,
    };

    paas_shm_cached_event_s shm_event;

    sdc_common_head_s head;
    memset(&head, 0, sizeof(head));
    head.version = SDC_VERSION;
    head.url = SDC_URL_PAAS_EVENTD_EVENT;
    head.method = SDC_METHOD_CREATE;
    head.head_length = sizeof(head) + sizeof(shm_head);
    head.content_length = sizeof(shm_event);

    struct iovec iov[3];
    iov[0].iov_base = &head;
    iov[0].iov_len = sizeof(head);
    iov[1].iov_base = &shm_head;
    iov[1].iov_len = sizeof(shm_head);
    iov[2].iov_base = &shm_event;
    iov[2].iov_len = sizeof(shm_event);

    struct sdc_shm_cache shm;
    int32_t nret = shm_copy(data, length, &shm);


    
	

	// const char* name_prefix  = CACHE_META_DATA;
	// int nret, event_len = 0;

	// event_len = sizeof(event) + sizeof(head) + sizeof(shm_head);
    // printf("event_len = %d\n", event_len);



//     if (nret < 0) {
//         LOG_ERROR("Shared Memory copied failed");
//         return ERR;
//     }

//     shm_event.addr_phy = shm.addr_phy;
//     shm_event.size = shm.size;
//     shm_event.cookie = shm.cookie;
//     nret = writev(m_fdEvent, iov, 3);
//     if (nret < 0) {
//         LOG_ERROR("Write the event message failed");
//     }
//     shm_free(&shm);

    return nret;
}
