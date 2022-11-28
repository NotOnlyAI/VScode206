#ifndef __UTILS_EVENT_H__
#define __UTILS_EVENT_H__

#include <stdint.h>
#include <sdc.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */


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

struct sdc_shm_cache
{
    void* addr_virt;
    unsigned long addr_phy;
    unsigned int size;
    unsigned int cookie;
};

struct sample_event
{
	struct paas_event base;
	unsigned char data[2000000];
};



// 发送元数据
extern int SDC_UtilsEventDel(int fp, unsigned int baseid, unsigned int id, char *cAppName);

extern int SDC_UtilsEventPublish(int fp, unsigned int baseid, UINT64 iDataLen, UCHAR *cEventMsg, uint64_t pts);


#ifdef __cplusplus
#if __cplusplus
}
#endif 
#endif/* End of #ifdef __cplusplus */

#endif  /* __LABEL_EVENT_H__ */