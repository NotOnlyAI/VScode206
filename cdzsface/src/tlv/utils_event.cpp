/******************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   : label_event.h
  版 本 号   : 初稿
  作    者   :  s30001871
  生成日期   : 2020年7月4日
  最近修改   :
  功能描述   : TLV消息格式定义
  函数列表   :
  修改历史   :
  1.日    期   : 2020年7月4日
    作    者   :  s30001871
    修改内容   : 创建文件
******************************************************************************/
#include "utils_event.h"
#include "label_event.h"

#include <sys/uio.h> 
#include <inttypes.h>
#include <unistd.h>

#include "sdc.h"
#include "sample_comm_nnie.h"
#include "sdc_def_ext.h"
#include "sdc_service.h"

#define LABEL_EVENT_DATA    ("tag.saas.sdc")

#define CACHE_META_DATA    ("itgt.saas.sdc")
// #define CACHE_META_DATA    ("facedetect")

extern int fd_cache;


int SDC_UtilsEventDel(int fp, unsigned int baseid, unsigned int id, char *cAppName)
{
    int nDataLen;
	int nResult;
	char *pcTemp = NULL;
	
    paas_shm_cached_event_s shm_event;
    sdc_extend_head_s shm_head = {
        .type = SDC_HEAD_SHM_CACHED_EVENT,
        .length = 8,
    };

    sdc_common_head_s head;
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

    SDC_SHM_CACHE_S shm_cache;
    memset(&shm_cache, 0, sizeof(shm_cache));

    LABEL_EVENT_DATA_S * pevent = NULL;
    nDataLen = sizeof(label);
    shm_cache.size  = sizeof(LABEL_EVENT_DATA_S) + nDataLen;
	shm_cache.ttl = 0;
	
    nResult = ioctl(fd_cache, SDC_CACHE_ALLOC,&shm_cache);
    if(nResult != 0)
    {
        printf("ioctl fail\n");
        goto EVENT_FAIL;
    }
    pevent = (LABEL_EVENT_DATA_S *)shm_cache.addr_virt;
	
    pevent->data = (char *)pevent +sizeof(LABEL_EVENT_DATA_S);
    pevent->base.id = baseid;

    (void)sprintf(pevent->base.name, "%s", CACHE_META_DATA); // change it 

	pevent->base.length = nDataLen;
	memset(pevent->data, 0, sizeof(nDataLen));
    pcTemp = pevent->data;

	*(uint32_t *)pcTemp = 0;//add
	pcTemp += sizeof(uint32_t);
	
	strcpy_s(pcTemp, 32, cAppName); //app name
	pcTemp += 32;
	
	*(uint64_t *)pcTemp = id;

	shm_event.addr_phy = shm_cache.addr_phy;	
	shm_event.size = shm_cache.size;	
	shm_event.cookie = shm_cache.cookie;
	nResult = writev(fp, iov, 3);
	if(nResult == -1)
    {
        printf("ioctl fail\n");
        goto EVENT_FAIL;
    }   
    munmap(pevent, sizeof(LABEL_EVENT_DATA_S) + nDataLen);

    return 0;
EVENT_FAIL:
    if(pevent)
    {
        munmap(pevent, sizeof(LABEL_EVENT_DATA_S) + nDataLen);
    }
    return -1;
}

int SDC_UtilsEventPublish(int fp, unsigned int baseid, UINT64 iDataLen, UCHAR *cEventMsg, uint64_t pts)
{
    paas_shm_cached_event_s shm_event;
    sdc_extend_head_s shm_head = {
        .type = SDC_HEAD_SHM_CACHED_EVENT,
        .length = 8,
    };

    sdc_common_head_s head;
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

    printf("sizeof(head) = %lu\n", sizeof(head));
    printf("sizeof(shm_head) = %lu\n", sizeof(shm_head));
    printf("sizeof(shm_event) = %lu\n", sizeof(shm_event));


    struct sample_event * event;
	struct sdc_shm_cache shm_cache;
	const char* name_prefix  = CACHE_META_DATA;
	int nret, event_len = 0;

	event_len = sizeof(event) + sizeof(head) + sizeof(shm_head);
    printf("event_len = %d\n", event_len);

    memset(&shm_cache, 0, sizeof(shm_cache));
	shm_cache.size = sizeof(*event);
    printf("shm_cache.size = %d\n", shm_cache.size);
    nret = ioctl(fd_cache, SDC_CACHE_ALLOC, &shm_cache);
    if(nret) {
        printf("cache alloc fail: %m\n");
        goto EVENT_FAIL;
    }
    event = (sample_event *)shm_cache.addr_virt;
    snprintf(event->base.name, sizeof(event->base.name), "%s", name_prefix);
    event->base.id = baseid;
    memcpy_s(event->data, iDataLen, cEventMsg, iDataLen);

    event->base.length = iDataLen;

    shm_event.addr_phy = shm_cache.addr_phy;
    shm_event.size = shm_cache.size;
    shm_event.cookie = shm_cache.cookie;
    nret = writev(fp, iov, 3);
    printf("shm_event.size = %d\n", shm_event.size);

    if(nret == -1) {
        printf("writev fail: %m\n");
        goto EVENT_FAIL;
    }
    printf("publish event: name= %s, id = %d\n", event->base.name,event->base.id);
    munmap(event, sizeof(*event));
    return 0;
EVENT_FAIL:
    if(event)
    {
        munmap(event, sizeof(*event));
    }
    return -1;
}