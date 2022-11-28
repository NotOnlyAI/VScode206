#ifndef __SDC_FD_UTILS_H__
#define __SDC_FD_UTILS_H__


#ifdef __cplusplus
extern "C"{
#endif


#include "sdc.h"
#include "sample_comm_nnie.h"

extern int SDC_GetHardWareId(int fd_utils,sdc_hardware_id_s *pstHardWareParas);
extern int SDC_MemAlloc(int fd_utils, unsigned int size, int uiCacheFlag, sdc_mmz_alloc_s* pstMemParas);
extern HI_S32 SDC_FlushCache(int fd_utils,HI_U64 u64PhyAddr, HI_VOID *pvVirAddr, HI_U32 u32Size);
#ifdef __cplusplus
}
#endif

#endif /* __SDC_FD_UTILS_H__ */