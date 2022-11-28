

#include <inttypes.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <math.h>



#include "sdc_fd_utils.h"


/*****************************************************************************
 函 数 名  : SDC_GetHardWareId
 功能描述  : 获取硬件ID
 输入参数  : sdc_hardware_id_s *pstHardWareParas
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年6月8日
    作    者   : jelly
    修改内容   : 新生成函数

*****************************************************************************/


unsigned int trans_id = 0;



int SDC_GetHardWareId(int fd_utils,sdc_hardware_id_s *pstHardWareParas)
{
	sdc_common_head_s head;	
	struct iovec iov[2] = 
    {
		[0] = {.iov_base = &head, .iov_len = sizeof(head) },
		[1] = {.iov_base = pstHardWareParas->id, .iov_len = sizeof(sdc_hardware_id_s) },
	};
	ssize_t retn;

	memset(&head,0,sizeof(head));
	head.version = SDC_VERSION;
	head.url = SDC_URL_HARDWARE_ID;
	head.method = SDC_METHOD_GET;
	head.trans_id = ++trans_id;
	head.head_length = sizeof(head);

	retn = write(fd_utils, &head, sizeof(head));
	if(retn != sizeof(head)) 
    {
		fprintf(stderr, "write to config.iaas.sdc fail: %m\n");
		return ERR;
	}

	retn = readv(fd_utils,iov,2);
	if(retn == ERR) 
    {
		fprintf(stderr, "read from config.iaas.sdc fail!\n");
		return ERR;
	}

	if(head.code != SDC_CODE_200) 
    {
		fprintf(stderr, "read from config.iaas.sdc fail: code = %u\n",head.code);
		return ERR;
	}

	if(head.trans_id != trans_id) 
    {
		fprintf(stderr, "read from config.iaas.sdc fail: wrong trans-id= %u, expected = %u\n",head.trans_id,trans_id);
	}
	
    return OK;
}



/*****************************************************************************
 函 数 名  : SDC_MemAlloc
 功能描述  : 申请内存函数
 输入参数  : int fd                
                           unsigned int size         
                           sdc_mmz_alloc_s* mmz  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年6月8日
    作    者   : jelly
    修改内容   : 新生成函数

*****************************************************************************/
int SDC_MemAlloc(int fd_utils, unsigned int size, int uiCacheFlag, sdc_mmz_alloc_s* pstMemParas)
{
    // fprintf(stdout,"enter into mmz_alloc_cached:size:%d\n",size);
	sdc_common_head_s head;
	memset(&head, 0, sizeof(head));
	head.version = SDC_VERSION;
    head.url = SDC_URL_MMZ;
    head.method = SDC_METHOD_CREATE;
    head.head_length = sizeof(head);
    head.content_length = sizeof(size);

    
	// sdc_common_head_s head = {
	// 	.version = SDC_VERSION,
	// 	.url = SDC_URL_MMZ,
	// 	.method = SDC_METHOD_CREATE,
	// 	.head_length = sizeof(head),// + sizeof(cached_head),
	// 	.content_length = sizeof(size),
	// };

	struct iovec iov[] = {
		{ (void*)&head, sizeof(head) },
		//{ (void*)&cached_head, sizeof(cached_head) },
		{ &size, sizeof(size) }
	};

    
	int nret = writev(fd_utils, iov, 2);
	if(nret < 0) return errno;
	//fprintf(stdout,"mmz_alloc_cached:1\n");

	iov[1].iov_base = pstMemParas;
	iov[1].iov_len = sizeof(*pstMemParas);

	nret = readv(fd_utils, iov,2);
	if(nret < 0) 
    {
        fprintf(stdout,"mmz_alloc_cached:1\n");
        return errno;
    }
	//fprintf(stdout,"mmz_alloc_cached:2\n");

	if(head.code != SDC_CODE_200 || head.head_length != sizeof(head) || head.content_length != sizeof(*pstMemParas)) 
    {
        fprintf(stdout,"mmz_alloc_cached:2,size:%d\n", pstMemParas->size);
        return EIO;       
    }
    // fprintf(stdout,"mmz succeed mmz_alloc_cached:size:%d\n",size);
	return size;
}




/*
*Fulsh cached
*/
HI_S32 SDC_FlushCache(int fd_utils, HI_U64 u64PhyAddr, HI_VOID *pvVirAddr, HI_U32 u32Size)
{
	HI_S32 s32Ret = HI_SUCCESS;
    sdc_mem_s sdc_mem_addr;
    sdc_mem_addr.addr_phy = (void *)u64PhyAddr;
    sdc_mem_addr.addr_virt = pvVirAddr;
    sdc_mem_addr.size = u32Size;

    s32Ret = ioctl(fd_utils, SRVFS_PHYMEM_CACHEFLUSH,&sdc_mem_addr);

	return s32Ret;
}