/******************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   : label_event.h
  版 本 号   : 初稿
  作    者   : athina
  生成日期   : 2020年7月4日
  最近修改   :
  功能描述   : TLV消息格式定义
  函数列表   :
  修改历史   :
  1.日    期   : 2020年7月4日
    作    者   : athina
    修改内容   : 创建文件
******************************************************************************/
#include "label_event.h"

#include <sys/uio.h> 
#include <inttypes.h>
#include <unistd.h>

#include "sdc.h"
#include "sample_comm_nnie.h"
#include "sdc_service.h"

#define LABEL_EVENT_DATA    ("tag.saas.sdc")



extern int fd_cache;



int SDC_LabelEventDel(int fd_event, unsigned int baseid, unsigned int id, char *cAppName)
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
    //printf("ioctl fail\n");
	
    nResult = ioctl(fd_cache, SDC_CACHE_ALLOC,&shm_cache);
    if(nResult != 0)
    {
        printf("ioctl fail\n");
        goto EVENT_FAIL;
    }
    pevent = (LABEL_EVENT_DATA_S *)shm_cache.addr_virt;
	
    pevent->data = (char *)pevent +sizeof(LABEL_EVENT_DATA_S);
    pevent->base.id = baseid;

    (void)sprintf(pevent->base.name, "%s", LABEL_EVENT_DATA);

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
	nResult = writev(fd_event, iov, 3);
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

int SDC_LabelEventPublish(int fd_event, unsigned int baseid, int iDataLen, char *cEventMsg, uint64_t pts)
{
	int nResult;
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
    //nDataLen= sizeof(label) + uiPolygonNum * sizeof(polygon) + iPointNum * sizeof(point) + (uiTagNum - '0')*sizeof(tag) + iStrNum;
    shm_cache.size  = sizeof(LABEL_EVENT_DATA_S) + iDataLen;
    nResult = ioctl(fd_cache, SDC_CACHE_ALLOC,&shm_cache);
    if(nResult != 0)
    {
        printf("ioctl fail\n");
        goto EVENT_FAIL;
    }
    pevent = (LABEL_EVENT_DATA_S *)shm_cache.addr_virt;
	pevent->data = (char *)pevent + sizeof(LABEL_EVENT_DATA_S);
    pevent->base.id = baseid;
	pevent->base.src_timestamp = pts;
	pevent->base.tran_timestamp = pts + 100;/*Ä¬ÈÏÌîÐ´*/
    (void)sprintf(pevent->base.name, "%s", LABEL_EVENT_DATA);
	(void)sprintf(pevent->base.publisher, "%s", "test");

	pevent->base.length = iDataLen;
	memcpy_s(pevent->data, iDataLen, cEventMsg, iDataLen);
	
    printf("length= %d\n",iDataLen);

	shm_event.addr_phy = shm_cache.addr_phy;	
	shm_event.size = shm_cache.size;	
	shm_event.cookie = shm_cache.cookie;	
	nResult = writev(fd_event, iov, 3);
	if(nResult == -1)
    {
        printf("writev fail\n");
        goto EVENT_FAIL;
    }   
	munmap(pevent, sizeof(LABEL_EVENT_DATA_S) + iDataLen);

    return 0;
EVENT_FAIL:
    if(pevent)
    {
        munmap(pevent, sizeof(LABEL_EVENT_DATA_S) + iDataLen);
    }
    return -1;
}