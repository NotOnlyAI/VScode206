

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



#include "sdc_os_api.h"
#include "sample_comm_svp.h"
#include "sample_comm_ive.h"
#include "sample_comm_nnie.h"

#include "hi_type.h"

// #define NNIE_MODEL_CONTENT_MMZ 0
// #define NNIE_MODEL_CONTENT_FILE 1

int fd_video = -1;
int fd_codec = -1;
int fd_utils = -1;
int fd_algorithm = -1;
int fd_event = -1;
int fd_cache = -1;
unsigned int trans_id = 0;

/*ssd para*/
extern SAMPLE_SVP_NNIE_MODEL_S s_stSsdModel;
extern SAMPLE_SVP_NNIE_PARAM_S s_stSsdNnieParam;
extern SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S s_stSsdSoftwareParam;
extern SAMPLE_SVP_NNIE_CFG_S   stNnieCfg_SDC;



/*****************************************************************************
 函 数 名  : SDC_ServiceCreate
 功能描述  : 打开服务文件句柄
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年6月8日
    作    者   : jelly
    修改内容   : 新生成函数

*****************************************************************************/
int SDC_ServiceCreate(void)
{
	fd_video = open("/mnt/srvfs/video.iaas.sdc",O_RDWR);
	if(fd_video < 0) goto video_fail;
	fd_codec = open("/mnt/srvfs/codec.iaas.sdc",O_RDWR);
	if(fd_codec < 0) goto codec_fail;
	fd_utils = open("/mnt/srvfs/utils.iaas.sdc",O_RDWR);
	if(fd_utils < 0) goto config_fail;
	fd_algorithm = open("/mnt/srvfs/algorithm.iaas.sdc",O_RDWR);
	if(fd_algorithm < 0) goto algorithm_fail;

	fd_event = open("/mnt/srvfs/event.paas.sdc",O_RDWR);
	if(fd_event < 0) goto event_fail;
	fd_cache = open("/dev/cache",O_RDWR);
	if(fd_cache < 0) goto cache_fail;
	
	return 0;

cache_fail:
	fprintf(stderr, "open /dev/cache fail in SDC_ServiceCreate!\r\n");
	close(fd_event);
	fd_event = -1;
event_fail:
	fprintf(stderr, "open event fail in SDC_ServiceCreate!\r\n");
	close(fd_algorithm);
	fd_algorithm = -1;
algorithm_fail:
	fprintf(stderr, "errno:%d open algorithm.iaas.sdc fail！\r\n", errno);
	close(fd_utils);
	fd_utils = -1;
config_fail:
	close(fd_codec);
	fprintf(stderr, "errno:%d open utils.iaas.sdc fail！\r\n", errno);
	fd_codec = -1;
codec_fail:
	close(fd_video);
	fprintf(stderr, "errno:%d open codec.iaas.sdc fail！\r\n", errno);
	fd_video = -1;
video_fail:
	fprintf(stderr, "errno:%d open video.iaas.sdc fail！\r\n", errno);
	return errno;
}





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
int SDC_GetHardWareId(sdc_hardware_id_s *pstHardWareParas)
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
 函 数 名  : SDC_YuvChnAttrSet
 功能描述  : 设置YUV通道参数
 输入参数  : int fd  
                           int uiYuvChnId
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年6月8日
    作    者   : jelly
    修改内容   : 新生成函数

*****************************************************************************/
int SDC_YuvChnAttrSet(int fd, int uiYuvChnId)
{
	sdc_yuv_channel_param_s param = 
    {
		.channel = uiYuvChnId,
        .width = 300,
        .height = 300,
		.fps = 3,
        .on_off = 1,
		.format = SDC_YVU_420SP,
	};

	sdc_common_head_s head;
	memset(&head, 0, sizeof(head));
	head.version = SDC_VERSION;
    head.url = SDC_URL_YUV_CHANNEL;
    head.method = SDC_METHOD_UPDATE;
    head.head_length = sizeof(head);
    head.content_length = sizeof(param);


	// sdc_common_head_s head = 
    // {
	// 	.version = SDC_VERSION, //0x5331
	// 	.url = SDC_URL_YUV_CHANNEL, //0x00
	// 	.method = SDC_METHOD_UPDATE, //0x02
	// 	.content_length = sizeof(param),
	// 	.head_length = sizeof(head),
	// };
    
	struct iovec iov[2] = { {.iov_base = &head, .iov_len = sizeof(head)},
		{.iov_base = &param, .iov_len = sizeof(param) }};
	int nret;

	nret = writev(fd, iov, 2);
	if(nret < 0)
	{
	   fprintf(stdout,"writev fail yuv_channel_set,response:%d,url:%d,code:%d,method:%d\n",head.response,head.url,head.code, head.method);
	   return errno;
	}

	fprintf(stdout,"yuv_channel_set write succeed \n");

	nret = read(fd,&head, sizeof(head));
	if(nret < 0 || head.code != SDC_CODE_200)
	{
	   fprintf(stdout,"read fail response:%d,url:%d,code:%d, method:%d\n",head.response,head.url,head.code, head.method);
	   return errno;
	}

	fprintf(stdout,"yuv_channel_set read succeed content_length:%d \n", head.content_length);
	
	return OK;
}



/*****************************************************************************
 函 数 名  : SDC_YuvChnAttrGet
 功能描述  : 查询YUV逻辑通道信息
 输入参数  : int fd  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年6月8日
    作    者   : jelly
    修改内容   : 新生成函数

*****************************************************************************/
int SDC_YuvChnAttrGet(int fd)
{
	int nret,i;
	char buf[1024] = { 0 };
	sdc_yuv_channel_info_s* info;
	sdc_common_head_s* head = (sdc_common_head_s*)buf;
	sdc_chan_query_s* param;

	/** query all channels' info */
	head->version = SDC_VERSION; //0x5331
	head->url = SDC_URL_YUV_CHANNEL; //0x00
	head->method = SDC_METHOD_GET; //0x02
	head->head_length = sizeof(*head);

	param = (sdc_chan_query_s*)(buf+head->head_length);
    param->channel = 0;/*通道ID为0时查询的是所有的逻辑通道*/
    head->content_length = sizeof(sdc_chan_query_s);

	head->content_length = 0;
    
	nret = write(fd, head, head->head_length);
	if(nret < 0)
	{
	   fprintf(stdout,"writev fail yuv_channel_get,response:%d,url:%d,code:%d,method:%d\n",head->response,head->url,head->code, head->method);
	   return errno;
	}
	fprintf(stdout,"yuv_channel_get write succeed \n");

	nret = read(fd,buf,sizeof(buf));
	if(nret < 0 || head->code != SDC_CODE_200)
	{
	   fprintf(stdout,"read fail response:%d,url:%d,code:%d, method:%d\n",head->response,head->url,head->code, head->method);
	   return errno;
	}
	
	fprintf(stdout,"yuv_channel_get read succeed content_length:%d \n", head->content_length);


	info = (sdc_yuv_channel_info_s*)&buf[head->head_length];
	for(i = 0; i < head->content_length / sizeof(*info); ++i, ++info) 
	{
		fprintf(stdout,"channel info:is_snap_channel:%d,src_id:%d,subscribe_cnt:%d,width:%d,height:%d,channel:%d,fps:%d,on_off:%d\n",
			info->is_snap_channel, info->src_id, info->subscribe_cnt, info->max_resolution.width, 
			info->max_resolution.height, info->param.channel, info->param.fps, info->param.on_off);
	}
    return 0;
}


/*****************************************************************************
 函 数 名  : SDC_YuvChnAttrGetIdleYuvChn
 功能描述  : 查询YUV逻辑通道信息
 输入参数  : int fd  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年6月8日
    作    者   : jelly
    修改内容   : 新生成函数

*****************************************************************************/
int SDC_YuvChnAttrGetIdleYuvChn(int fd, unsigned int *puiChnId)
{
	int nret,i;
	char buf[4096] = { 0 };
	sdc_yuv_channel_info_s* info;
	sdc_common_head_s* head = (sdc_common_head_s*)buf;
	sdc_chan_query_s* param;

	/** query all channels' info */
	head->version = SDC_VERSION; //0x5331
	head->url = SDC_URL_YUV_CHANNEL; //0x00
	head->method = SDC_METHOD_GET; //0x02
	head->head_length = sizeof(*head);

	param = (sdc_chan_query_s*)(buf+head->head_length);
    param->channel = 0;/*通道ID为0时查询的是所有的逻辑通道*/
    //head->content_length = sizeof(sdc_chan_query_s);

	head->content_length = 0;
    
	nret = write(fd, head, head->head_length);
	if(nret < 0)
	{
	   fprintf(stdout,"writev fail yuv_channel_get,response:%d,url:%d,code:%d,method:%d\n",head->response,head->url,head->code, head->method);
	   return errno;
	}
	//fprintf(stdout,"yuv_channel_get write succeed \n");

	nret = read(fd,buf,sizeof(buf));
	if(nret < 0 || head->code != SDC_CODE_200)
	{
	   fprintf(stdout,"read fail response:%d,url:%d,code:%d, method:%d\n",head->response,head->url,head->code, head->method);
	   return errno;
	}
	
	//fprintf(stdout,"yuv_channel_get read succeed content_length:%d \n", head->content_length);

	info = (sdc_yuv_channel_info_s*)&buf[head->head_length];
	for(i = 0; i < head->content_length / sizeof(*info); i++, info++) 
	{
	    if ((info->is_snap_channel == 0)
			&& (info->subscribe_cnt ==0)
			&& (info->nResolutionModitfy ==1))
    	{
    	    *puiChnId = info->param.channel;
			fprintf(stdout,"Get yuv video chn：%d\n", info->param.channel);
            return 0;
    	}
	}
	
    return ERR;
}

int SDC_YuvDataReq(int fd, int extendheadflag, unsigned int uiChnId, unsigned int uiMaxUsedBufNum)
{
	int nret;
	char buf[1024] = { 0 };
	sdc_common_head_s* head = (sdc_common_head_s*) buf;
	sdc_extend_head_s* extend_head;
	uint32_t* channel;

	head->version = SDC_VERSION;
	head->url = SDC_URL_YUV_DATA;
	head->method = SDC_METHOD_GET;
	head->head_length = sizeof(*head);

	fprintf(stdout,"yuv_frame_get:extendheadflag:%d\n",extendheadflag);

    if (0x01 == (extendheadflag&0x01))
	{
	    extend_head = (sdc_extend_head_s*)&buf[head->head_length];
		extend_head->type = SDC_HEAD_YUV_SYNC;
		extend_head->length = sizeof(*extend_head);
		extend_head->reserve = 0;
		head->head_length += sdc_extend_head_length(extend_head);
	}

	if (0x02 == (extendheadflag&0x02))
	{
		extend_head = (sdc_extend_head_s*)&buf[head->head_length];
		extend_head->type = SDC_HEAD_YUV_CACHED_COUNT_MAX;
		extend_head->length = sizeof(*extend_head);
		extend_head->reserve = uiMaxUsedBufNum;
	    
		head->head_length += sdc_extend_head_length(extend_head);
	}

	if (0x04 == (extendheadflag&0x04))
	{
		extend_head = (sdc_extend_head_s*)&buf[head->head_length];
		extend_head->type = SDC_HEAD_YUV_PARAM_MASK;
		extend_head->length = sizeof(*extend_head);
		extend_head->reserve = 8;

		head->head_length += sdc_extend_head_length(extend_head);
	}

	channel = (uint32_t*)&buf[head->head_length];
	
    channel[0] = uiChnId;          
    head->content_length = 1 * sizeof(uint32_t);
        
	nret = write(fd, head, head->head_length + head->content_length);
	if(nret < 0)
	{
	   fprintf(stdout,"writev fail yuv_frame_get,response:%d,url:%d,code:%d,method:%d\n",
            head->response,head->url,head->code, head->method);
	   return ERR;
	}

	nret = read(fd, buf,sizeof(buf));
	if(nret < 0)
	{
	   fprintf(stdout,"read fail yuv_frame_get,response:%d,url:%d,code:%d,method:%d\n",
            head->response,head->url,head->code, head->method);
	   return ERR;
	}
    
    return OK;    
}




void SDC_YuvDataFree(int fd,  sdc_yuv_data_s *yuv_data)
{
	int nret;
	char buf[1024] = { 0 };
	sdc_common_head_s* head = (sdc_common_head_s*) buf;

    /** free yuv_data */

    head->version = SDC_VERSION;
	head->url = SDC_URL_YUV_DATA;
	head->method = SDC_METHOD_DELETE;
	head->head_length = sizeof(sdc_common_head_s);    
    head->response = head->code = 0;
    head->content_length = sizeof(sdc_yuv_data_s); 
    
    memcpy(&buf[head->head_length],yuv_data,sizeof(sdc_yuv_data_s));
    
    nret = write(fd, buf, head->head_length + sizeof(sdc_yuv_data_s)); 
    if(nret < 0)
    {
        fprintf(stdout,"writev fail yuv_frame_get,response:%d,url:%d,code:%d,method:%d\n",
        head->response,head->url,head->code, head->method);
    }

    return;
}



int32_t SDC_GetYuvData(sdc_yuv_data_s &data)
{
    uint8_t cMsgReadBuf[1024];
    sdc_common_head_s *pstSdcMsgHead = (sdc_common_head_s *)cMsgReadBuf;
	int iReadLen = 0;

	iReadLen = read(fd_video, (void *)cMsgReadBuf, sizeof(cMsgReadBuf));
    if (iReadLen < 0) {
        fprintf(stdout,"Read the Yuv frame fail response:%d,url:%d,code:%d, method:%d\n",
                pstSdcMsgHead->response,pstSdcMsgHead->url,pstSdcMsgHead->code, pstSdcMsgHead->method);
        return ERR;
    }
    // fprintf(stdout,"Read the Yuv frame succeed response:%d,url:%d,code:%d, method:%d, iReadLen:%d\n",
    //         pstSdcMsgHead->response,pstSdcMsgHead->url,pstSdcMsgHead->code, pstSdcMsgHead->method, iReadLen);


    if (pstSdcMsgHead->url == SDC_URL_YUV_DATA) {
        if (pstSdcMsgHead->content_length != 0) {
            (void)memcpy(&data, /*sizeof(data),*/ cMsgReadBuf + pstSdcMsgHead->head_length, sizeof(data));
            return OK;
        }
    }
    return ERR;
}


int SDC_TransYUV2RGB(int fd, sdc_yuv_frame_s *yuv, sdc_yuv_frame_s *rgb)
{
    sdc_common_head_s head;
    int nRet;
    struct iovec iov[2];

        
    // fill head struct 
    memset(&head, 0, sizeof(head));
    head.version = SDC_VERSION;
    head.url = 3;
    head.method = SDC_METHOD_CREATE;
    head.head_length = sizeof(head);
    head.content_length = sizeof(sdc_yuv_frame_s);

    iov[0].iov_base = &head;
    iov[0].iov_len = sizeof(head);
    iov[1].iov_base = yuv;
    iov[1].iov_len = sizeof(sdc_yuv_frame_s);

    // write request
    nRet = writev(fd_algorithm, iov, sizeof(iov)/sizeof(iov[0]));
    if (nRet < 0)
    {
        fprintf(stdout,"Error:failed SDC_TransYUV2RGB,nRet:%d!\n",nRet);
        return ERR;
    }
    // read response
    iov[1].iov_base = rgb;
    iov[1].iov_len = sizeof(sdc_yuv_frame_s);
    nRet = readv(fd_algorithm, iov, 2);
    if (head.code != SDC_CODE_200 || nRet < 0 || head.content_length != sizeof(sdc_yuv_frame_s))
    {
        fprintf(stdout,"Err:SDC_TransYUV2RGB,nRet:%d,rsp_head.code:%d!\n",
            nRet, head.code);
        return ERR;
    } 
    return OK;

}


void SDC_Struct2RGB(sdc_yuv_frame_s *pstSdcRGBFrame, VW_YUV_FRAME_S *pstRGBFrameData)
{
    pstRGBFrameData->enPixelFormat = PIXEL_FORMAT_RGB_888;
    pstRGBFrameData->pYuvImgAddr = (CHAR *)pstSdcRGBFrame->addr_virt;
    pstRGBFrameData->ulPhyAddr[0] = pstSdcRGBFrame->addr_phy;
    pstRGBFrameData->ulPhyAddr[1] = pstSdcRGBFrame->addr_phy + (pstSdcRGBFrame->height * pstSdcRGBFrame->stride);
    pstRGBFrameData->ulPhyAddr[2] = pstSdcRGBFrame->addr_phy + (pstSdcRGBFrame->height * pstSdcRGBFrame->stride)*2;

    pstRGBFrameData->ulVirAddr[0] = pstSdcRGBFrame->addr_virt;
    pstRGBFrameData->ulVirAddr[1] = pstSdcRGBFrame->addr_virt + (pstSdcRGBFrame->height * pstSdcRGBFrame->stride);
    pstRGBFrameData->ulVirAddr[2] = pstSdcRGBFrame->addr_virt + (pstSdcRGBFrame->height * pstSdcRGBFrame->stride)*2;

    pstRGBFrameData->uWidth = pstSdcRGBFrame->width;
    pstRGBFrameData->uHeight = pstSdcRGBFrame->height;
    pstRGBFrameData->uStride[0] = pstSdcRGBFrame->stride;
    pstRGBFrameData->uStride[1] = pstSdcRGBFrame->stride;
    pstRGBFrameData->uStride[2] = pstSdcRGBFrame->stride;
    pstRGBFrameData->uFrmSize = pstSdcRGBFrame->size;
    pstRGBFrameData->uPoolId = pstSdcRGBFrame->cookie[0];
    pstRGBFrameData->uVbBlk = pstSdcRGBFrame->cookie[1];
    return ;    
}
void SDC_DisplayYuvData(sdc_yuv_data_s* yuv_data)
{
    fprintf(stdout,"SDC_DisplayYuvData,channel:%d,reserve:%d,pts:%ld,pts_sys:%ld,addr_phy:0x%lx,addr_virt:0x%lx,size:%d\n",
		yuv_data->channel, yuv_data->reserve,yuv_data->pts,yuv_data->pts_sys,
		yuv_data->frame.addr_phy,yuv_data->frame.addr_virt,yuv_data->frame.size);
    return;
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
int SDC_MemAlloc(int fd, unsigned int size, int uiCacheFlag, sdc_mmz_alloc_s* pstMemParas)
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

    
	int nret = writev(fd, iov, 2);
	if(nret < 0) return errno;
	//fprintf(stdout,"mmz_alloc_cached:1\n");

	iov[1].iov_base = pstMemParas;
	iov[1].iov_len = sizeof(*pstMemParas);

	nret = readv(fd, iov,2);
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

/*****************************************************************************
 函 数 名  : SDC_MemFree
 功能描述  : 释放申请的内存
 输入参数  : int fd                
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
void SDC_MemFree(int fd, sdc_mmz_alloc_s* pstMemParas)
{
    sdc_common_head_s head;
	memset(&head, 0, sizeof(head));
	head.version = SDC_VERSION;
    head.url = SDC_URL_MMZ;
    head.method = SDC_METHOD_DELETE;
    head.head_length = sizeof(head);
    head.content_length = sizeof(*pstMemParas);

	// sdc_common_head_s head = 
    // {
	// 	.version = SDC_VERSION,
	// 	.url = SDC_URL_MMZ,
	// 	.method = SDC_METHOD_DELETE,
	// 	.head_length = sizeof(head),
	// 	.content_length = sizeof(*pstMemParas),
	// };

	struct iovec iov[] = {
		{ (void*)&head, sizeof(head) },
		{ pstMemParas, sizeof(*pstMemParas) }
	};

	(void)writev(fd, iov, sizeof(iov)/sizeof(iov[0]));
    return ;
}
int SDC_ModelDecript(sdc_mmz_alloc_s *pstMmzAddr)
{
    if(pstMmzAddr == NULL)
    {
        printf("Err in SDC_ModelDecript, pstMmzAddr is null\n");
        return ERR;
    }    
    return OK;
}
void SDC_DisplayExtendHead(sdc_extend_head_s* extendhead)
{
	fprintf(stdout, "*******SDC_DisplayExtendHead********:type=%u, length=%u, reserve=%u\n", 
        extendhead->type,extendhead->length,extendhead->reserve);
    return;
}

int SDC_LoadModel(unsigned int uiLoadMode, char *pucModelFileName, SVP_NNIE_MODEL_S *pstModel)
{
    int s32Ret = 0;
    int ret = 0;
    sdc_extend_head_s* extend_head;
    unsigned int uFileSize;
    sdc_mmz_alloc_s stMmzAddr;
    if ((NULL == pstModel) || (NULL == pucModelFileName))
    {
        fprintf(stdout,"Err in SDC_LoadModel, pstModel or pucModelFileName is null\n");
        return -1;
    }
    
    fprintf(stdout,"Strat Load model, pucModelFileName:%s!\n", pucModelFileName);


    


    // sdc_common_head_s phead;
	// memset(&phead, 0, sizeof(phead));
	// phead.version = SDC_VERSION;
    // phead.url = SDC_URL_NNIE_MODEL;
    // phead.method = SDC_METHOD_CREATE;
    // phead.head_length = sizeof(sdc_common_head_s);
    // phead.content_length = MAX_MODULE_PATH;

    char buf[1024] = {0};
    sdc_common_head_s *phead = (sdc_common_head_s *)buf;
    phead->version = SDC_VERSION;
    phead->url = SDC_URL_NNIE_MODEL;
    phead->method = SDC_METHOD_CREATE;
    phead->head_length = sizeof(sdc_common_head_s);
    phead->content_length = MAX_MODULE_PATH;


    /*模式 0，不带扩展头，默认内存方式加载*/
    if (uiLoadMode == 1)/*模式 1，带扩展头，扩展头参数指定为内存方式加载*/
    {
        FILE *fp = fopen(pucModelFileName, "rb");
        if(fp == NULL)
        {
            fprintf(stdout,"modelfile fopen %s fail!\n", pucModelFileName);
            return -1;
        }
        ret = fseek(fp,0L,SEEK_END);
        if(ret != 0)
        {
            fprintf(stdout,"check nnie file SEEK_END, fseek fail.");
            fclose(fp);
            return -1;
        }
        
        uFileSize = ftell(fp);
        ret = fseek(fp,0L,SEEK_SET);
        if(0 != ret)
        {
            fprintf(stdout,"check nnie file SEEK_SET, fseek fail.");
            fclose(fp);
            return -1;
        }

        stMmzAddr.size = uFileSize;
        ret = SDC_MemAlloc(fd_utils, uFileSize, 0, &stMmzAddr); // param 2: 0 no cache, 1 cache
        if(ret != stMmzAddr.size)
        {
            fprintf(stdout,"SDC_MmzAlloc ret %d, readsize %d", ret, stMmzAddr.size);
            return -1;
        }
        ret = fread((HI_VOID*)(uintptr_t)stMmzAddr.addr_virt, 1, stMmzAddr.size, fp);
        if(ret != stMmzAddr.size)
        {
            fprintf(stdout,"filesize %d, readsize %d", ret, stMmzAddr.size);
            return -1;
        }
        /*用户执行调用算法程序对传入文件进行解码*/
        if(SDC_ModelDecript(&stMmzAddr))
        {
            fprintf(stdout,"SDC_ModelDecript Fail!");
            return -1;
        }
        extend_head = (sdc_extend_head_s *)&buf[phead->head_length];
        extend_head->type = 1;//NNIE_NNIE_MODEL_OP
        extend_head->length = sizeof(*extend_head);
        extend_head->reserve = 0;/*0 或者不带是内存方式，1 是文件名方式*/
        phead->head_length += sizeof(sdc_extend_head_s);
        // iov[1].iov_base = &stMmzAddr; 
    }


    struct iovec iov[2] = 
    {
        [0] = { .iov_base = buf, .iov_len = sizeof(sdc_common_head_s)+ sizeof(sdc_extend_head_s)},
        [1] = { .iov_base = &stMmzAddr,.iov_len = MAX_MODULE_PATH}
    };


    s32Ret = writev(fd_algorithm, iov, 2);
    if (s32Ret < 0)
    {
        fprintf(stdout,"creat nnie,write to algorithm.iaas.sdc fail: %m\n");
        return -1;
    }
    
    /*模型加载后立即释放*/
    // if (uiLoadMode < 2)mmz_free(fd_config, &stMmzAddr);
    struct rsp_strcut 
    {
        sdc_common_head_s head;
        SVP_NNIE_MODEL_S model;
    }rsp_strcut_tmp;

    s32Ret = read(fd_algorithm, &rsp_strcut_tmp, sizeof(rsp_strcut_tmp));
    if(s32Ret == -1)
    {
        fprintf(stdout,"get_channel_data fail: %m\n");
        return -1;
    }
    if(s32Ret > sizeof(rsp_strcut_tmp))
    {
        fprintf(stdout,"get_channel_data truncated, data len: %d > %zu\n", s32Ret, sizeof(rsp_strcut_tmp));
        return -1;
    }
    if (s32Ret < 0 || rsp_strcut_tmp.head.code != SDC_CODE_200 || rsp_strcut_tmp.head.content_length <= 0)
    {
        fprintf(stdout,"get nnie create response, read from algorithm.iaas.sdc fail,s32Ret:%d, code=%d,length=%d\n", 
            s32Ret, rsp_strcut_tmp.head.code, rsp_strcut_tmp.head.content_length);
        return -1;
    }
    else 
    {
        s_stSsdModel.stModel = rsp_strcut_tmp.model;
        memcpy(pstModel, &rsp_strcut_tmp.model,sizeof(SVP_NNIE_MODEL_S));
        fprintf(stdout, "Load model Suscess!\n"); 
    }
    
    return OK;
}

int SDC_UnLoadModel(SVP_NNIE_MODEL_S *pstModel)
{
    int nRet = -1;
    if (NULL != pstModel)
    {
        sdc_common_head_s head;
        struct iovec iov[2] = {
        [0] = {.iov_base = &head , .iov_len = sizeof(head)},
        [1] = {.iov_base = pstModel, .iov_len = 
        sizeof(SVP_NNIE_MODEL_S)}};
        // fill head struct 
        memset(&head, 0, sizeof(head));
        head.version = SDC_VERSION;
        head.url = SDC_URL_NNIE_MODEL;
        head.method = SDC_METHOD_DELETE;
        head.head_length = sizeof(head);
        head.content_length = sizeof(SVP_NNIE_MODEL_S);
        nRet = writev(fd_algorithm, iov, sizeof(iov)/sizeof(iov[0]));
        if (nRet < 0)
        {
            fprintf(stdout,"Errin SDC_UnLoadModel:failed to unload nnie module!\n");
        } 
    }
    else
    {
        fprintf(stdout,"Err in SDC_UnLoadModel:module pointer is NULL!\n");
    }
    
    return 0;
}

/*****************************************************************************
*   Prototype    : SDC_NNIE_ParamDeinit
*   Description  : Deinit NNIE parameters
*   Input        : SAMPLE_SVP_NNIE_PARAM_S        *pstNnieParam     NNIE Parameter
*                  SAMPLE_SVP_NNIE_SOFTWARE_MEM_S *pstSoftWareMem   software mem
*
*
*
*
*   Output       :
*   Return Value :  HI_S32,HI_SUCCESS:Success,Other:failure
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-20
*           Author       :
*           Modification : Create
*
*****************************************************************************/
HI_S32 SDC_NNIE_ParamDeinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, int fd)
{
    sdc_mmz_alloc_s stMemParas;
	SAMPLE_SVP_CHECK_EXPR_RET(NULL == pstNnieParam,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error, pstNnieParam can't be NULL!\n");

	#if 0
	if(0!=pstNnieParam->stTaskBuf.u64PhyAddr && 0!=pstNnieParam->stTaskBuf.u64VirAddr)
	{
		SAMPLE_SVP_MMZ_FREE(pstNnieParam->stTaskBuf.u64PhyAddr,pstNnieParam->stTaskBuf.u64VirAddr);
		pstNnieParam->stTaskBuf.u64PhyAddr = 0;
		pstNnieParam->stTaskBuf.u64VirAddr = 0;
	}
	#endif
	
    if(0!=pstNnieParam->stStepBuf.u64PhyAddr && 0!=pstNnieParam->stStepBuf.u64VirAddr)
	{
		stMemParas.addr_phy = pstNnieParam->stStepBuf.u64PhyAddr;
		stMemParas.addr_virt = pstNnieParam->stStepBuf.u64VirAddr;

        SDC_MemFree(fd, &stMemParas);
		
		pstNnieParam->stStepBuf.u64PhyAddr = 0;
		pstNnieParam->stStepBuf.u64VirAddr = 0;
	}
	return HI_SUCCESS;
}


/*****************************************************************************
*   Prototype    : SAMPLE_SVP_NNIE_FillForwardInfo
*   Description  : fill NNIE forward ctrl information
*   Input        : SAMPLE_SVP_NNIE_CFG_S   *pstNnieCfg       NNIE configure info
* 	               SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam     NNIE parameter
*
*
*
*   Output       :
*   Return Value : HI_S32,HI_SUCCESS:Success,Other:failure
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-20
*           Author       :
*           Modification : Create
*
*****************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_FillForwardInfo(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam)
{
	HI_U32 i = 0, j = 0;
	HI_U32 u32Offset = 0;
	HI_U32 u32Num = 0;

	for(i = 0; i < pstNnieParam->pstModel->u32NetSegNum; i++)
	{
        /*fill forwardCtrl info*/
		if(SVP_NNIE_NET_TYPE_ROI == pstNnieParam->pstModel->astSeg[i].enNetType)
		{
			pstNnieParam->astForwardWithBboxCtrl[i].enNnieId = pstNnieCfg->aenNnieCoreId[i];
			pstNnieParam->astForwardWithBboxCtrl[i].u32SrcNum = pstNnieParam->pstModel->astSeg[i].u16SrcNum;
			pstNnieParam->astForwardWithBboxCtrl[i].u32DstNum = pstNnieParam->pstModel->astSeg[i].u16DstNum;
			pstNnieParam->astForwardWithBboxCtrl[i].u32ProposalNum = 1;
			pstNnieParam->astForwardWithBboxCtrl[i].u32NetSegId = i;
			#if 0
			pstNnieParam->astForwardWithBboxCtrl[i].stTmpBuf = pstNnieParam->stTmpBuf;
			pstNnieParam->astForwardWithBboxCtrl[i].stTskBuf.u64PhyAddr= pstNnieParam->stTaskBuf.u64PhyAddr+u32Offset;
			pstNnieParam->astForwardWithBboxCtrl[i].stTskBuf.u64VirAddr= pstNnieParam->stTaskBuf.u64VirAddr+u32Offset;
			pstNnieParam->astForwardWithBboxCtrl[i].stTskBuf.u32Size= pstNnieParam->au32TaskBufSize[i];
			#endif
		}
		else if(SVP_NNIE_NET_TYPE_CNN == pstNnieParam->pstModel->astSeg[i].enNetType ||
            SVP_NNIE_NET_TYPE_RECURRENT== pstNnieParam->pstModel->astSeg[i].enNetType)
		{


			pstNnieParam->astForwardCtrl[i].enNnieId = pstNnieCfg->aenNnieCoreId[i];
			pstNnieParam->astForwardCtrl[i].u32SrcNum = pstNnieParam->pstModel->astSeg[i].u16SrcNum;
			pstNnieParam->astForwardCtrl[i].u32DstNum = pstNnieParam->pstModel->astSeg[i].u16DstNum;
			pstNnieParam->astForwardCtrl[i].u32NetSegId = i;
			#if 0
			pstNnieParam->astForwardCtrl[i].stTmpBuf = pstNnieParam->stTmpBuf;
			pstNnieParam->astForwardCtrl[i].stTskBuf.u64PhyAddr= pstNnieParam->stTaskBuf.u64PhyAddr+u32Offset;
			pstNnieParam->astForwardCtrl[i].stTskBuf.u64VirAddr= pstNnieParam->stTaskBuf.u64VirAddr+u32Offset;
			pstNnieParam->astForwardCtrl[i].stTskBuf.u32Size= pstNnieParam->au32TaskBufSize[i];
			#endif
		}
		u32Offset += pstNnieParam->au32TaskBufSize[i];/*这里赋值要审视下，好像参数没用*/

        /*fill src blob info*/
		for(j = 0; j < pstNnieParam->pstModel->astSeg[i].u16SrcNum; j++)
	    {
            /*Recurrent blob*/
            if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->pstModel->astSeg[i].astSrcNode[j].enType)
            {
                pstNnieParam->astSegData[i].astSrc[j].enType = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].enType;
                pstNnieParam->astSegData[i].astSrc[j].unShape.stSeq.u32Dim = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.u32Dim;
                pstNnieParam->astSegData[i].astSrc[j].u32Num = pstNnieCfg->u32MaxInputNum;
                pstNnieParam->astSegData[i].astSrc[j].unShape.stSeq.u64VirAddrStep = pstNnieCfg->au64StepVirAddr[i*SAMPLE_SVP_NNIE_EACH_SEG_STEP_ADDR_NUM];
            }
            else
            {
    		    pstNnieParam->astSegData[i].astSrc[j].enType = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].enType;
    	        pstNnieParam->astSegData[i].astSrc[j].unShape.stWhc.u32Chn = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.stWhc.u32Chn;
    	        pstNnieParam->astSegData[i].astSrc[j].unShape.stWhc.u32Height = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.stWhc.u32Height;
    	        pstNnieParam->astSegData[i].astSrc[j].unShape.stWhc.u32Width = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.stWhc.u32Width;
    	        pstNnieParam->astSegData[i].astSrc[j].u32Num = pstNnieCfg->u32MaxInputNum;
            }
	    }

        /*fill dst blob info*/
		if(SVP_NNIE_NET_TYPE_ROI == pstNnieParam->pstModel->astSeg[i].enNetType)
		{
			u32Num = pstNnieCfg->u32MaxRoiNum*pstNnieCfg->u32MaxInputNum;
		}
		else
		{
			u32Num = pstNnieCfg->u32MaxInputNum;
		}

		for(j = 0; j < pstNnieParam->pstModel->astSeg[i].u16DstNum; j++)
		{
            if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->pstModel->astSeg[i].astDstNode[j].enType)
            {
    			pstNnieParam->astSegData[i].astDst[j].enType = pstNnieParam->pstModel->astSeg[i].astDstNode[j].enType;
    			pstNnieParam->astSegData[i].astDst[j].unShape.stSeq.u32Dim =
                    pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.u32Dim;
                pstNnieParam->astSegData[i].astDst[j].u32Num = u32Num;
                pstNnieParam->astSegData[i].astDst[j].unShape.stSeq.u64VirAddrStep =
                    pstNnieCfg->au64StepVirAddr[i*SAMPLE_SVP_NNIE_EACH_SEG_STEP_ADDR_NUM+1];
            }
            else
            {
    		    pstNnieParam->astSegData[i].astDst[j].enType = pstNnieParam->pstModel->astSeg[i].astDstNode[j].enType;
    		    pstNnieParam->astSegData[i].astDst[j].unShape.stWhc.u32Chn = pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.stWhc.u32Chn;
    		    pstNnieParam->astSegData[i].astDst[j].unShape.stWhc.u32Height = pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.stWhc.u32Height;
    		    pstNnieParam->astSegData[i].astDst[j].unShape.stWhc.u32Width = pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.stWhc.u32Width;
    		    pstNnieParam->astSegData[i].astDst[j].u32Num = u32Num;
            }
		}
	}
	return HI_SUCCESS;
}

/*****************************************************************************
*   Prototype    : SAMPLE_SVP_NNIE_GetBlobMemSize
*   Description  : Get blob mem size
*   Input        : SVP_NNIE_NODE_S astNnieNode[]   NNIE Node
*                  HI_U32          u32NodeNum      Node num
*                  HI_U32          astBlob[]       blob struct
*                  HI_U32          u32Align        stride align type
*                  HI_U32          *pu32TotalSize  Total size
*                  HI_U32          au32BlobSize[]  blob size
*
*
*
*
*   Output       :
*   Return Value : VOID
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-20
*           Author       :
*           Modification : Create
*
*****************************************************************************/
void SAMPLE_SVP_NNIE_GetBlobMemSize(SVP_NNIE_NODE_S astNnieNode[], HI_U32 u32NodeNum,
	HI_U32 u32TotalStep,SVP_BLOB_S astBlob[], HI_U32 u32Align, HI_U32* pu32TotalSize,HI_U32 au32BlobSize[])
{
	HI_U32 i = 0;
	HI_U32 u32Size = 0;
	HI_U32 u32Stride = 0;

	for(i = 0; i < u32NodeNum; i++)
	{
		if(SVP_BLOB_TYPE_S32== astNnieNode[i].enType||SVP_BLOB_TYPE_VEC_S32== astNnieNode[i].enType||
            SVP_BLOB_TYPE_SEQ_S32== astNnieNode[i].enType)
		{
			u32Size = sizeof(HI_U32);
		}
		else
		{
			u32Size = sizeof(HI_U8);
		}
        if(SVP_BLOB_TYPE_SEQ_S32 == astNnieNode[i].enType)
        {
            if(SAMPLE_SVP_NNIE_ALIGN_16 == u32Align)
    		{
    			u32Stride = SAMPLE_SVP_NNIE_ALIGN16(astNnieNode[i].unShape.u32Dim*u32Size);
    		}
    		else
    		{
    			u32Stride = SAMPLE_SVP_NNIE_ALIGN32(astNnieNode[i].unShape.u32Dim*u32Size);
    		}
            au32BlobSize[i] = u32TotalStep*u32Stride;
        }
        else
        {
            if(SAMPLE_SVP_NNIE_ALIGN_16 == u32Align)
    		{
    			u32Stride = SAMPLE_SVP_NNIE_ALIGN16(astNnieNode[i].unShape.stWhc.u32Width*u32Size);
    		}
    		else
    		{
    			u32Stride = SAMPLE_SVP_NNIE_ALIGN32(astNnieNode[i].unShape.stWhc.u32Width*u32Size);
    		}
    		au32BlobSize[i] = astBlob[i].u32Num*u32Stride*astNnieNode[i].unShape.stWhc.u32Height*
    			astNnieNode[i].unShape.stWhc.u32Chn;
        }
		*pu32TotalSize += au32BlobSize[i];
	    astBlob[i].u32Stride = u32Stride;
	}
}

/*****************************************************************************
*   Prototype    : SAMPLE_SVP_NNIE_GetTaskAndBlobBufSize
*   Description  : Get taskinfo and blob memory size
*   Input        : SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam     NNIE parameter
* 	                HI_U32                  *pu32TaskInfoSize Task info size
*                  HI_U32                  *pu32TmpBufSize    Tmp buffer size
*                  SAMPLE_SVP_NNIE_BLOB_SIZE_S  astBlobSize[] each seg input and output blob mem size
*                  HI_U32                  *pu32TotalSize     Total mem size
*
*
*   Output       :
*   Return Value : HI_S32,HI_SUCCESS:Success,Other:failure
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-20
*           Author       :
*           Modification : Create
*
*****************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_GetTaskAndBlobBufSize(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,HI_U32*pu32TotalTaskBufSize, HI_U32*pu32TmpBufSize,
    SAMPLE_SVP_NNIE_BLOB_SIZE_S astBlobSize[],HI_U32*pu32TotalSize)
{
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 i = 0, j = 0;
    HI_U32 u32TotalStep = 0;

	#if 0
	/*Get each seg's task buf size*/
	s32Ret = HI_MPI_SVP_NNIE_GetTskBufSize(pstNnieCfg->u32MaxInputNum, pstNnieCfg->u32MaxRoiNum,
		pstNnieParam->pstModel, pstNnieParam->au32TaskBufSize,pstNnieParam->pstModel->u32NetSegNum);
	SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error,HI_MPI_SVP_NNIE_GetTaskSize failed!\n");

    /*Get total task buf size*/
	*pu32TotalTaskBufSize = 0;
	for(i = 0; i < pstNnieParam->pstModel->u32NetSegNum; i++)
	{
		*pu32TotalTaskBufSize += pstNnieParam->au32TaskBufSize[i];
	}

	/*Get tmp buf size*/
	*pu32TmpBufSize = pstNnieParam->pstModel->u32TmpBufSize;
	*pu32TotalSize += *pu32TotalTaskBufSize + *pu32TmpBufSize;
	#endif

	/*calculate Blob mem size*/
	for(i = 0; i < pstNnieParam->pstModel->u32NetSegNum; i++)
	{
        if(SVP_NNIE_NET_TYPE_RECURRENT == pstNnieParam->pstModel->astSeg[i].enNetType)
        {
            for(j = 0; j < pstNnieParam->astSegData[i].astSrc[0].u32Num; j++)
            {
                u32TotalStep += *((HI_S32*)pstNnieParam->astSegData[i].astSrc[0].unShape.stSeq.u64VirAddrStep+j);
            }
        }
		/*the first seg's Src Blob mem size, other seg's src blobs from the output blobs of
		those segs before it or from software output results*/
		if(i == 0)
		{
			SAMPLE_SVP_NNIE_GetBlobMemSize(&(pstNnieParam->pstModel->astSeg[i].astSrcNode[0]),
				pstNnieParam->pstModel->astSeg[i].u16SrcNum,u32TotalStep,&(pstNnieParam->astSegData[i].astSrc[0]),
				SAMPLE_SVP_NNIE_ALIGN_16, pu32TotalSize, &(astBlobSize[i].au32SrcSize[0]));
		}

		/*Get each seg's Dst Blob mem size*/
		SAMPLE_SVP_NNIE_GetBlobMemSize(&(pstNnieParam->pstModel->astSeg[i].astDstNode[0]),
			pstNnieParam->pstModel->astSeg[i].u16DstNum,u32TotalStep,&(pstNnieParam->astSegData[i].astDst[0]),
			SAMPLE_SVP_NNIE_ALIGN_16, pu32TotalSize, &(astBlobSize[i].au32DstSize[0]));
	}
	return s32Ret;
}

/*****************************************************************************
*   Prototype    : SAMPLE_SVP_NNIE_ParamInit
*   Description  : Fill info of NNIE Forward parameters
*   Input        : SAMPLE_SVP_NNIE_CFG_S   *pstNnieCfg    NNIE configure parameter
* 		            SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam	 NNIE parameters
*
*
*
*   Output       :
*   Return Value : HI_S32,HI_SUCCESS:Success,Other:failure
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-03-14
*           Author       :
*           Modification : Create
*
*****************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_ParamInit(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam)
{
	HI_U32 i = 0, j = 0;
	HI_U32 u32TotalSize = 0;
	HI_U32 u32TotalTaskBufSize = 0;
	HI_U32 u32TmpBufSize = 0;
	HI_S32 s32Ret = HI_SUCCESS;
	//HI_U32 u32Offset = 0;
	HI_U64 u64PhyAddr = 0;
	HI_U8 *pu8VirAddr = NULL;
	SAMPLE_SVP_NNIE_BLOB_SIZE_S astBlobSize[SVP_NNIE_MAX_NET_SEG_NUM] = {0};
    sdc_mmz_alloc_s stMemParas;

	/*fill forward info*/
	s32Ret = SAMPLE_SVP_NNIE_FillForwardInfo(pstNnieCfg,pstNnieParam);
	SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error,SAMPLE_SVP_NNIE_FillForwardCtrl failed!\n");


	/*Get taskInfo and Blob mem size*/
	s32Ret = SAMPLE_SVP_NNIE_GetTaskAndBlobBufSize(pstNnieCfg,pstNnieParam,&u32TotalTaskBufSize,
		&u32TmpBufSize,astBlobSize,&u32TotalSize);
	SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error,SAMPLE_SVP_NNIE_GetTaskAndBlobBufSize failed!\n");
  

	/*Malloc mem*/

    s32Ret = SDC_MemAlloc(fd_utils,u32TotalSize,1,&stMemParas);
    u64PhyAddr = stMemParas.addr_phy;
    pu8VirAddr = (HI_U8 *)stMemParas.addr_virt;

	fprintf(stdout,"SDC_MemAlloc u32TotalSize %d, size %d\n", u32TotalSize, stMemParas.size);

	
    u32TotalSize = stMemParas.size;


    SAMPLE_SVP_CHECK_EXPR_RET(u32TotalSize != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error,Malloc memory failed,u32TotalSize:%d,s32Ret:%d!\n", u32TotalSize, s32Ret);
	memset(pu8VirAddr, 0, u32TotalSize);
	SDC_FlushCache(u64PhyAddr,(void*)pu8VirAddr,u32TotalSize);


	
	/*fill each blob's mem addr*/
	u64PhyAddr =  u64PhyAddr;//+u32TotalTaskBufSize+u32TmpBufSize;
	pu8VirAddr = pu8VirAddr;//+u32TotalTaskBufSize+u32TmpBufSize;
	for(i = 0; i < pstNnieParam->pstModel->u32NetSegNum; i++)
	{
		/*first seg has src blobs, other seg's src blobs from the output blobs of
		those segs before it or from software output results*/
		if(0 == i)
		{
			for(j = 0; j < pstNnieParam->pstModel->astSeg[i].u16SrcNum; j++)
			{
				if(j!=0)
				{
					u64PhyAddr += astBlobSize[i].au32SrcSize[j-1];
					pu8VirAddr += astBlobSize[i].au32SrcSize[j-1];
				}
				pstNnieParam->astSegData[i].astSrc[j].u64PhyAddr = u64PhyAddr;
				pstNnieParam->astSegData[i].astSrc[j].u64VirAddr = (HI_U64)pu8VirAddr;
			}
			u64PhyAddr += astBlobSize[i].au32SrcSize[j-1];
			pu8VirAddr += astBlobSize[i].au32SrcSize[j-1];
		}

		/*fill the mem addrs of each seg's output blobs*/
		for(j = 0; j < pstNnieParam->pstModel->astSeg[i].u16DstNum; j++)
		{
			if(j!=0)
			{
				u64PhyAddr += astBlobSize[i].au32DstSize[j-1];
				pu8VirAddr += astBlobSize[i].au32DstSize[j-1];
			}
			pstNnieParam->astSegData[i].astDst[j].u64PhyAddr = u64PhyAddr;
			pstNnieParam->astSegData[i].astDst[j].u64VirAddr = (HI_U64)pu8VirAddr;
		}
		u64PhyAddr += astBlobSize[i].au32DstSize[j-1];
		pu8VirAddr += astBlobSize[i].au32DstSize[j-1];
	}
	return HI_SUCCESS;
}
/*****************************************************************************
*   Prototype    : SDC_NNIE_ParamInit
*   Description  : Init NNIE  parameters
*   Input        : SAMPLE_SVP_NNIE_CFG_S   *pstNnieCfg    NNIE configure parameter
*                  SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam    NNIE parameters
*
*
*
*   Output       :
*   Return Value : HI_S32,HI_SUCCESS:Success,Other:failure
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-20
*           Author       :
*           Modification : Create
*
*****************************************************************************/
HI_S32 SDC_NNIE_ParamInit(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, int fd)
{
	HI_S32 s32Ret = HI_SUCCESS;
    fprintf(stdout,"START SDC_NNIE_ParamInit!\n");
    /*check*/
    SAMPLE_SVP_CHECK_EXPR_RET((NULL == pstNnieCfg || NULL == pstNnieParam),HI_ERR_SVP_NNIE_ILLEGAL_PARAM,
        SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,pstNnieCfg and pstNnieParam can't be NULL!\n");
    SAMPLE_SVP_CHECK_EXPR_RET((NULL == pstNnieParam->pstModel),HI_ERR_SVP_NNIE_ILLEGAL_PARAM,
        SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,pstNnieParam->pstModel can't be NULL!\n");

	/*NNIE parameter initialization */
	s32Ret = SAMPLE_SVP_NNIE_ParamInit(pstNnieCfg,pstNnieParam);
	SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error, SAMPLE_SVP_NNIE_ParamInit failed!\n");
    fprintf(stdout,"END SDC_NNIE_ParamInit!\n");
	return s32Ret;
FAIL:
	s32Ret = SDC_NNIE_ParamDeinit(pstNnieParam, fd);
	SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error, SDC_NNIE_ParamDeinit failed!\n");
	return HI_FAILURE;
}


/*
*Fulsh cached
*/
HI_S32 SDC_FlushCache(HI_U64 u64PhyAddr, HI_VOID *pvVirAddr, HI_U32 u32Size)
{
	HI_S32 s32Ret = HI_SUCCESS;
    sdc_mem_s sdc_mem_addr;
    sdc_mem_addr.addr_phy = (void *)u64PhyAddr;
    sdc_mem_addr.addr_virt = pvVirAddr;
    sdc_mem_addr.size = u32Size;

    s32Ret = ioctl(fd_utils, SRVFS_PHYMEM_CACHEFLUSH,&sdc_mem_addr);

	return s32Ret;
}

/*****************************************************************************
* Prototype :   SAMPLE_SVP_NNIE_Ssd_GetResultTmpBuf
* Description : this function is used to Get SSD GetResult tmp buffer size
* Input :     SAMPLE_SVP_NNIE_PARAM_S*               pstNnieParam     [IN]  the pointer to SSD NNIE parameter
*              SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S*   pstSoftwareParam [IN]  the pointer to SSD software parameter
*
*
*
*
* Output :
* Return Value : HI_SUCCESS: Success;Error codes: Failure.
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-11-10
* Author :
* Modification : Create
*
*****************************************************************************/
HI_U32 SAMPLE_SVP_NNIE_Ssd_GetResultTmpBuf(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,
    SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam)
{
    HI_U32 u32PriorBoxSize = 0;
    HI_U32 u32SoftMaxSize = 0;
    HI_U32 u32DetectionSize = 0;
    HI_U32 u32TotalSize = 0;
    HI_U32 u32PriorNum = 0;
    HI_U32 i = 0;

    /*priorbox size*/
    for(i = 0; i < pstNnieParam->pstModel->astSeg[0].u16DstNum/2; i++)
    {
        u32PriorBoxSize += pstSoftwareParam->au32PriorBoxHeight[i]*pstSoftwareParam->au32PriorBoxWidth[i]*
            SAMPLE_SVP_NNIE_COORDI_NUM*2*(pstSoftwareParam->u32MaxSizeNum+pstSoftwareParam->u32MinSizeNum+
            pstSoftwareParam->au32InputAspectRatioNum[i]*2*pstSoftwareParam->u32MinSizeNum)*sizeof(HI_U32);
    }
    pstSoftwareParam->stPriorBoxTmpBuf.u32Size = u32PriorBoxSize;
    u32TotalSize+=u32PriorBoxSize;

    /*softmax size*/
    for(i = 0; i < pstSoftwareParam->u32ConcatNum; i++)
    {
        u32SoftMaxSize += pstSoftwareParam->au32SoftMaxInChn[i]*sizeof(HI_U32);
    }
    pstSoftwareParam->stSoftMaxTmpBuf.u32Size = u32SoftMaxSize;
    u32TotalSize+=u32SoftMaxSize;

    /*detection size*/
    for(i = 0; i < pstSoftwareParam->u32ConcatNum; i++)
    {
        u32PriorNum+=pstSoftwareParam->au32DetectInputChn[i]/SAMPLE_SVP_NNIE_COORDI_NUM;
    }
    u32DetectionSize+=u32PriorNum*SAMPLE_SVP_NNIE_COORDI_NUM*sizeof(HI_U32);
    u32DetectionSize+=u32PriorNum*SAMPLE_SVP_NNIE_PROPOSAL_WIDTH*sizeof(HI_U32)*2;
    u32DetectionSize+=u32PriorNum*2*sizeof(HI_U32);
    pstSoftwareParam->stGetResultTmpBuf.u32Size = u32DetectionSize;
    u32TotalSize+=u32DetectionSize;

    return u32TotalSize;
}

/******************************************************************************
* function : Ssd software para init
******************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_Ssd_SoftwareInit(int fd, SAMPLE_SVP_NNIE_CFG_S* pstCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftWareParam)
{   
    fprintf(stdout,"START SAMPLE_SVP_NNIE_Ssd_SoftwareInit!\n");
    HI_U32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ClassNum = 0;
    HI_U32 u32TotalSize = 0;
    HI_U32 u32DstRoiSize = 0;
    HI_U32 u32DstScoreSize = 0;
    HI_U32 u32ClassRoiNumSize = 0;
    HI_U32 u32TmpBufTotalSize = 0;
    HI_U64 u64PhyAddr = 0;
    HI_U8* pu8VirAddr = NULL;
    sdc_mmz_alloc_s stMemParas;

    /*Set Conv Parameters*/
    /*the SSD sample report resule is after permute operation,
     conv result is (C, H, W), after permute, the report node's
     (C1, H1, W1) is (H, W, C), the stride of report result is aligned according to C dim*/
    for(i = 0; i < 12; i++)
    {
        pstSoftWareParam->au32ConvHeight[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Chn;
        pstSoftWareParam->au32ConvWidth[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Height;
        pstSoftWareParam->au32ConvChannel[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Width;
        if(i%2==1)
        {
            pstSoftWareParam->au32ConvStride[i/2] = SAMPLE_SVP_NNIE_ALIGN16(pstSoftWareParam->au32ConvChannel[i]*sizeof(HI_U32))/sizeof(HI_U32);
        }
    }

    /*Set PriorBox Parameters*/
	pstSoftWareParam->au32PriorBoxWidth[0] = 40;// 38;
	pstSoftWareParam->au32PriorBoxWidth[1] = 20;// 19;
    pstSoftWareParam->au32PriorBoxWidth[2] = 10;
    pstSoftWareParam->au32PriorBoxWidth[3] = 5;
    pstSoftWareParam->au32PriorBoxWidth[4] = 3;
    pstSoftWareParam->au32PriorBoxWidth[5] = 1;

	pstSoftWareParam->au32PriorBoxHeight[0] = 40;// 38;
	pstSoftWareParam->au32PriorBoxHeight[1] = 20;// 19;
    pstSoftWareParam->au32PriorBoxHeight[2] = 10;
    pstSoftWareParam->au32PriorBoxHeight[3] = 5;
    pstSoftWareParam->au32PriorBoxHeight[4] = 3;
    pstSoftWareParam->au32PriorBoxHeight[5] = 1;

    pstSoftWareParam->u32OriImHeight = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Height;
    pstSoftWareParam->u32OriImWidth = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Width;

    pstSoftWareParam->af32PriorBoxMinSize[0][0] = 30.0f;
    pstSoftWareParam->af32PriorBoxMinSize[1][0] = 60.0f;
    pstSoftWareParam->af32PriorBoxMinSize[2][0] = 111.0f;
    pstSoftWareParam->af32PriorBoxMinSize[3][0] = 162.0f;
    pstSoftWareParam->af32PriorBoxMinSize[4][0] = 213.0f;
    pstSoftWareParam->af32PriorBoxMinSize[5][0] = 264.0f;

    pstSoftWareParam->af32PriorBoxMaxSize[0][0] = 60.0f;
    pstSoftWareParam->af32PriorBoxMaxSize[1][0] = 111.0f;
    pstSoftWareParam->af32PriorBoxMaxSize[2][0] = 162.0f;
    pstSoftWareParam->af32PriorBoxMaxSize[3][0] = 213.0f;
    pstSoftWareParam->af32PriorBoxMaxSize[4][0] = 264.0f;
    pstSoftWareParam->af32PriorBoxMaxSize[5][0] = 315.0f;

    pstSoftWareParam->u32MinSizeNum = 1;
    pstSoftWareParam->u32MaxSizeNum = 1;
    pstSoftWareParam->bFlip= HI_TRUE;
    pstSoftWareParam->bClip= HI_FALSE;

    pstSoftWareParam->au32InputAspectRatioNum[0] = 1;
    pstSoftWareParam->au32InputAspectRatioNum[1] = 2;
    pstSoftWareParam->au32InputAspectRatioNum[2] = 2;
    pstSoftWareParam->au32InputAspectRatioNum[3] = 2;
    pstSoftWareParam->au32InputAspectRatioNum[4] = 1;
    pstSoftWareParam->au32InputAspectRatioNum[5] = 1;

    pstSoftWareParam->af32PriorBoxAspectRatio[0][0] = 2;
    pstSoftWareParam->af32PriorBoxAspectRatio[0][1] = 0;
    pstSoftWareParam->af32PriorBoxAspectRatio[1][0] = 2;
	pstSoftWareParam->af32PriorBoxAspectRatio[1][1] = 2;// 3;
    pstSoftWareParam->af32PriorBoxAspectRatio[2][0] = 2;
	pstSoftWareParam->af32PriorBoxAspectRatio[2][1] = 2;// 3;
    pstSoftWareParam->af32PriorBoxAspectRatio[3][0] = 2;
	pstSoftWareParam->af32PriorBoxAspectRatio[3][1] = 2;// 3;
    pstSoftWareParam->af32PriorBoxAspectRatio[4][0] = 2;
    pstSoftWareParam->af32PriorBoxAspectRatio[4][1] = 0;
    pstSoftWareParam->af32PriorBoxAspectRatio[5][0] = 2;
    pstSoftWareParam->af32PriorBoxAspectRatio[5][1] = 0;

    pstSoftWareParam->af32PriorBoxStepWidth[0] = 8;
    pstSoftWareParam->af32PriorBoxStepWidth[1] = 16;
    pstSoftWareParam->af32PriorBoxStepWidth[2] = 32;
    pstSoftWareParam->af32PriorBoxStepWidth[3] = 64;
	pstSoftWareParam->af32PriorBoxStepWidth[4] = 106;// 100;
	pstSoftWareParam->af32PriorBoxStepWidth[5] = 320;// 300;

    pstSoftWareParam->af32PriorBoxStepHeight[0] = 8;
    pstSoftWareParam->af32PriorBoxStepHeight[1] = 16;
    pstSoftWareParam->af32PriorBoxStepHeight[2] = 32;
    pstSoftWareParam->af32PriorBoxStepHeight[3] = 64;
	pstSoftWareParam->af32PriorBoxStepHeight[4] = 106;// 100;
	pstSoftWareParam->af32PriorBoxStepHeight[5] = 320;// 300;

    pstSoftWareParam->f32Offset = 0.5f;

    pstSoftWareParam->as32PriorBoxVar[0] = (HI_S32)(0.1f*SAMPLE_SVP_NNIE_QUANT_BASE);
    pstSoftWareParam->as32PriorBoxVar[1] = (HI_S32)(0.1f*SAMPLE_SVP_NNIE_QUANT_BASE);
    pstSoftWareParam->as32PriorBoxVar[2] = (HI_S32)(0.2f*SAMPLE_SVP_NNIE_QUANT_BASE);
    pstSoftWareParam->as32PriorBoxVar[3] = (HI_S32)(0.2f*SAMPLE_SVP_NNIE_QUANT_BASE);

    /*Set Softmax Parameters*/
	pstSoftWareParam->u32SoftMaxInHeight = 8;// 21;
	pstSoftWareParam->au32SoftMaxInChn[0] = 46208;// 121296;
	pstSoftWareParam->au32SoftMaxInChn[1] = 17328;//45486;
	pstSoftWareParam->au32SoftMaxInChn[2] = 4800;// 12600;
	pstSoftWareParam->au32SoftMaxInChn[3] = 1200;// 3150;
	pstSoftWareParam->au32SoftMaxInChn[4] = 288;// 756;
	pstSoftWareParam->au32SoftMaxInChn[5] = 32;// 84;

	pstSoftWareParam->u32ConcatNum = 5;// 6;
    pstSoftWareParam->u32SoftMaxOutWidth = 1;
	pstSoftWareParam->u32SoftMaxOutHeight = 8;// 21;
    pstSoftWareParam->u32SoftMaxOutChn = 8732;

    /*Set DetectionOut Parameters*/
	pstSoftWareParam->u32ClassNum = 8;// 21;
    pstSoftWareParam->u32TopK = 200;//400;
    pstSoftWareParam->u32KeepTopK = 100;//200;
    pstSoftWareParam->u32NmsThresh = (HI_U16)(0.3f*SAMPLE_SVP_NNIE_QUANT_BASE);
    pstSoftWareParam->u32ConfThresh = 1;
    pstSoftWareParam->au32DetectInputChn[0] = 23104;
    pstSoftWareParam->au32DetectInputChn[1] = 8664;
    pstSoftWareParam->au32DetectInputChn[2] = 2400;
    pstSoftWareParam->au32DetectInputChn[3] = 600;
    pstSoftWareParam->au32DetectInputChn[4] = 144;
    pstSoftWareParam->au32DetectInputChn[5] = 16;

    /*Malloc assist buffer memory*/
    u32ClassNum = pstSoftWareParam->u32ClassNum;
    u32TotalSize = SAMPLE_SVP_NNIE_Ssd_GetResultTmpBuf(pstNnieParam,pstSoftWareParam);
    u32DstRoiSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*pstSoftWareParam->u32TopK*sizeof(HI_U32)*SAMPLE_SVP_NNIE_COORDI_NUM);
    u32DstScoreSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*pstSoftWareParam->u32TopK*sizeof(HI_U32));
    u32ClassRoiNumSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*sizeof(HI_U32));
    u32TotalSize = u32TotalSize+u32DstRoiSize+u32DstScoreSize+u32ClassRoiNumSize;

    s32Ret = SDC_MemAlloc(fd,u32TotalSize,1,&stMemParas);
    u64PhyAddr = stMemParas.addr_phy;
	// pu8VirAddr = (void*)stMemParas.addr_virt;
    pu8VirAddr = (HI_U8*)stMemParas.addr_virt;
    u32TotalSize = stMemParas.size;
    #if 0
    s32Ret = SAMPLE_COMM_SVP_MallocCached("SAMPLE_SSD_INIT",NULL,(HI_U64*)&u64PhyAddr,
        (void**)&pu8VirAddr,u32TotalSize);

    #endif
    
    SAMPLE_SVP_CHECK_EXPR_RET(u32TotalSize != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error,Malloc memory failed!\n");
    memset(pu8VirAddr,0, u32TotalSize);
    SDC_FlushCache(u64PhyAddr,(void*)pu8VirAddr,u32TotalSize);

   /*set each tmp buffer addr*/
    pstSoftWareParam->stPriorBoxTmpBuf.u64PhyAddr = u64PhyAddr;
    pstSoftWareParam->stPriorBoxTmpBuf.u64VirAddr = (HI_U64)(pu8VirAddr);

    pstSoftWareParam->stSoftMaxTmpBuf.u64PhyAddr = u64PhyAddr+
        pstSoftWareParam->stPriorBoxTmpBuf.u32Size;
    pstSoftWareParam->stSoftMaxTmpBuf.u64VirAddr = (HI_U64)(pu8VirAddr+
        pstSoftWareParam->stPriorBoxTmpBuf.u32Size);

    pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr = u64PhyAddr+
        pstSoftWareParam->stPriorBoxTmpBuf.u32Size+pstSoftWareParam->stSoftMaxTmpBuf.u32Size;
    pstSoftWareParam->stGetResultTmpBuf.u64VirAddr = (HI_U64)(pu8VirAddr+
        pstSoftWareParam->stPriorBoxTmpBuf.u32Size+ pstSoftWareParam->stSoftMaxTmpBuf.u32Size);

    u32TmpBufTotalSize = pstSoftWareParam->stPriorBoxTmpBuf.u32Size+
        pstSoftWareParam->stSoftMaxTmpBuf.u32Size + pstSoftWareParam->stGetResultTmpBuf.u32Size;

    /*set result blob*/
    pstSoftWareParam->stDstRoi.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stDstRoi.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize;
    pstSoftWareParam->stDstRoi.u64VirAddr = (HI_U64)(pu8VirAddr+u32TmpBufTotalSize);
    pstSoftWareParam->stDstRoi.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*
        pstSoftWareParam->u32TopK*sizeof(HI_U32)*SAMPLE_SVP_NNIE_COORDI_NUM);
    pstSoftWareParam->stDstRoi.u32Num = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Width = u32ClassNum*
        pstSoftWareParam->u32TopK*SAMPLE_SVP_NNIE_COORDI_NUM;

    pstSoftWareParam->stDstScore.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stDstScore.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize+u32DstRoiSize;
    pstSoftWareParam->stDstScore.u64VirAddr = (HI_U64)(pu8VirAddr+u32TmpBufTotalSize+u32DstRoiSize);
    pstSoftWareParam->stDstScore.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*
        pstSoftWareParam->u32TopK*sizeof(HI_U32));
    pstSoftWareParam->stDstScore.u32Num = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Width = u32ClassNum*
        pstSoftWareParam->u32TopK;

    pstSoftWareParam->stClassRoiNum.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stClassRoiNum.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize+
        u32DstRoiSize+u32DstScoreSize;
    pstSoftWareParam->stClassRoiNum.u64VirAddr = (HI_U64)(pu8VirAddr+u32TmpBufTotalSize+
        u32DstRoiSize+u32DstScoreSize);
    pstSoftWareParam->stClassRoiNum.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*sizeof(HI_U32));
    pstSoftWareParam->stClassRoiNum.u32Num = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Width = u32ClassNum;
    fprintf(stdout,"END SAMPLE_SVP_NNIE_Ssd_SoftwareInit!\n");
    return HI_SUCCESS;
}



HI_S32 SAMPLE_SVP_NNIE_Ssd_ParamInit(int fd, SAMPLE_SVP_NNIE_CFG_S* pstCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SDC_NNIE_ParamInit(pstCfg,pstNnieParam, fd);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),SDC_NNIE_ParamInit failed!\n",s32Ret);

    /*init software para*/
    s32Ret = SAMPLE_SVP_NNIE_Ssd_SoftwareInit(fd, pstCfg,pstNnieParam,pstSoftWareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),SAMPLE_SVP_NNIE_Ssd_SoftwareInit failed!\n",s32Ret);

    return s32Ret;
INIT_FAIL_0:
    // s32Ret = SAMPLE_SVP_NNIE_Ssd_Deinit(pstNnieParam,pstSoftWareParam,NULL, fd);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error(%#x),SAMPLE_SVP_NNIE_Ssd_Deinit failed!\n",s32Ret);
    return HI_FAILURE;
}



/******************************************************************************
* function : Fill Src Data
******************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_FillSrcData(SAMPLE_SVP_NNIE_CFG_S* pstNnieCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S* pstInputDataIdx)
{
    FILE* fp = NULL;
    HI_U32 i =0, j = 0, n = 0;
    HI_U32 u32Height = 0, u32Width = 0, u32Chn = 0, u32Stride = 0, u32Dim = 0;
    HI_U32 u32VarSize = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8*pu8PicAddr = NULL;
    HI_U32*pu32StepAddr = NULL;
    HI_U32 u32SegIdx = pstInputDataIdx->u32SegIdx;
    HI_U32 u32NodeIdx = pstInputDataIdx->u32NodeIdx;
    HI_U32 u32TotalStepNum = 0;
	HI_U8 *pu8BGR = HI_NULL;
	HI_U8 *pu8YUV = HI_NULL;
	HI_BOOL bRBG2BGR = HI_TRUE;
	
	// RBG => BGR
	#define B_BASE_OFFSET (1*u32Stride*u32Height)
	#define G_BASE_OFFSET (2*u32Stride*u32Height)
	#define R_BASE_OFFSET (0*u32Stride*u32Height)
	HI_U8 *p_B_data = HI_NULL; 
	HI_U8 *p_G_data = HI_NULL;
	HI_U8 *p_R_data = HI_NULL;
        printf("%s  %d %d %d %d!\n", __FILE__,__LINE__,B_BASE_OFFSET,u32Stride,u32Height);
								
    /*open file*/
    if (NULL != pstNnieCfg->pszPic)
    {
        printf("%s  %d!\n", __FILE__,__LINE__);

        fp = fopen(pstNnieCfg->pszPic,"rb");
        SAMPLE_SVP_CHECK_EXPR_RET(NULL == fp,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error, open file failed!\n");
    }
	else if(NULL != pstNnieCfg->pszBGR)
    {
	    pu8BGR = (HI_U8*)pstNnieCfg->pszBGR; 
        printf("pu8BGR = %p\n", pu8BGR);
	}
	else if(NULL != pstNnieCfg->pszYUV)
    {
	    pu8YUV = (HI_U8*)pstNnieCfg->pszYUV; 
        printf("pu8YUV = %p\n", pu8YUV);
	}
    printf("%s  %d!\n", __FILE__,__LINE__);
    /*get data size*/
    if(SVP_BLOB_TYPE_U8 <= pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType &&
        SVP_BLOB_TYPE_YVU422SP >= pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType)
    {
        u32VarSize = sizeof(HI_U8);
    }
    else
    {
        u32VarSize = sizeof(HI_U32);
    }
    printf("%s  %d  %d %d  %d!\n", __FILE__,__LINE__,B_BASE_OFFSET,u32Stride,u32Height);
    /*fill src data*/
    if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType)
    {
        printf("%s  %d!\n", __FILE__,__LINE__);
        u32Dim = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stSeq.u32Dim;
        u32Stride = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Stride;
        pu32StepAddr = (HI_U32*)(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stSeq.u64VirAddrStep);
        pu8PicAddr = (HI_U8*)(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr);
        for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++)
        {
            for(i = 0;i < *(pu32StepAddr+n); i++)
            {
                s32Ret = fread(pu8PicAddr,u32Dim*u32VarSize,1,fp);
                SAMPLE_SVP_CHECK_EXPR_GOTO(1 != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,Read image file failed!\n");
                pu8PicAddr += u32Stride;
            }
            u32TotalStepNum += *(pu32StepAddr+n);
        }
        SDC_FlushCache(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64PhyAddr,
            (HI_VOID *) pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr,
            u32TotalStepNum*u32Stride);
    }
    else
    {
        printf("%s  %d  %d  %d  %d!\n", __FILE__,__LINE__,B_BASE_OFFSET,u32Stride,u32Height);
        u32Height = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stWhc.u32Height;
        u32Width = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stWhc.u32Width;
        u32Chn = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stWhc.u32Chn;
        u32Stride = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Stride;
        pu8PicAddr = (HI_U8*)(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr);
       printf("%s  %d %d %d %d %d %d  %d!\n", __FILE__,__LINE__,u32Height,u32Width,u32Chn,u32Stride, B_BASE_OFFSET,pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType);
        if(SVP_BLOB_TYPE_YVU420SP== pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType)
        {
            printf("%s  %d!\n", __FILE__,__LINE__);
            for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++)
            {
                for(i = 0; i < u32Chn*u32Height/2; i++)
                {
					if(fp)
					{
						s32Ret = fread(pu8PicAddr,u32Width*u32VarSize,1,fp);
					}
					else
					{
						//printf("pu8YUV current = %p\n", pu8YUV);
                        memcpy_s(pu8PicAddr, u32Width*u32VarSize, pu8YUV, u32Width*u32VarSize);
						//printf("pu8YUV current2 = %p\n", pu8YUV);
						pu8YUV += 304;
					}
                    //SAMPLE_SVP_CHECK_EXPR_GOTO(1 != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,Read image file failed!\n");
					//printf("u32Stride = %d\n", u32Stride);
                    pu8PicAddr += u32Stride;
                }
            }
        }
        else if(SVP_BLOB_TYPE_YVU422SP== pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType)
        {
            printf("%s  %d!\n", __FILE__,__LINE__);
            for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++)
            {
                for(i = 0; i < u32Height*2; i++)
                {
                    s32Ret = fread(pu8PicAddr,u32Width*u32VarSize,1,fp);
                    SAMPLE_SVP_CHECK_EXPR_GOTO(1 != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,Read image file failed!\n");
                    pu8PicAddr += u32Stride;
                }
            }
        }
        else
        {
             printf("%s  %d %d %d %d %d %d!\n", __FILE__,__LINE__,B_BASE_OFFSET,G_BASE_OFFSET,R_BASE_OFFSET,u32Stride,u32Height);
			if(bRBG2BGR) // RBG => BGR
			{
				p_B_data = pu8BGR + B_BASE_OFFSET;
				p_G_data = pu8BGR + G_BASE_OFFSET;
				p_R_data = pu8BGR + R_BASE_OFFSET;
			}
            for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++)
            {
                for(i = 0;i < u32Chn; i++)
                {
                    for(j = 0; j < u32Height; j++)
                    {
						if(HI_NULL != fp)
						{  
							s32Ret = fread(pu8PicAddr,u32Width*u32VarSize,1,fp);
							SAMPLE_SVP_CHECK_EXPR_GOTO(1 != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,Read image file failed!\n");
						}
						else if(HI_NULL != pstNnieCfg->pszBGR)
						{
                           // printf("u32Width*u32VarSize = %d  %d  %d  %d \n", u32Width*u32VarSize,i,j,u32Stride);
							if(bRBG2BGR) // RBG => BGR
							{
							    if(u32Chn == 0) //copy B
								{
                                    memcpy_s(pu8PicAddr, u32Width*u32VarSize, p_B_data, u32Width*u32VarSize);
									p_B_data += u32Stride;
								}
								else if(u32Chn == 1) //copy G
								{
                                    memcpy_s(pu8PicAddr, u32Width*u32VarSize, p_G_data, u32Width*u32VarSize);
									p_G_data += u32Stride;
								}
								else // copy R
								{
                                    memcpy_s(pu8PicAddr, u32Width*u32VarSize, p_R_data, u32Width*u32VarSize);
									p_R_data += u32Stride;
								}
							}
							else
							{
                                memcpy_s(pu8PicAddr, u32Width*u32VarSize, pu8BGR, u32Width*u32VarSize);
								pu8BGR += u32Stride;
							}
						}
						//pu8BGR += u32Width*u32VarSize;
						//printf("u32Stride = %d\n", u32Stride);
						pu8PicAddr += u32Stride;
                    }
                }
            }
        }
         printf("%s  %d!\n", __FILE__,__LINE__);
        SDC_FlushCache(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64PhyAddr,
            (HI_VOID *) pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr,
            pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num*u32Chn*u32Height*u32Stride);
    }
    if(fp != HI_NULL)
        fclose(fp);
    return HI_SUCCESS;

FAIL:
    if(fp != HI_NULL)
        fclose(fp);
    fclose(fp);
    return HI_FAILURE;
}

int SDC_Nnie_Forward(sdc_nnie_forward_s *p_sdc_nnie_forward)
{
    int nRet;
    sdc_common_head_s rsp_head;
    sdc_common_head_s head;
    struct iovec iov[2] = 
    {
        [0] = {.iov_base = &head, .iov_len = sizeof(head)},
        [1] = {.iov_base = p_sdc_nnie_forward, .iov_len = sizeof(*p_sdc_nnie_forward)}
    };
    // fill head struct 
    memset(&head, 0, sizeof(head));
    head.version = SDC_VERSION;
    head.url = SDC_URL_NNIE_FORWARD;
    head.method = SDC_METHOD_GET;
    head.head_length = sizeof(head);
    head.content_length = sizeof(*p_sdc_nnie_forward);

    // write request
    nRet = writev(fd_algorithm, iov, sizeof(iov)/sizeof(iov[0]));
    if (nRet < 0)
    {
        fprintf(stdout,"Error:failed to write info to NNIE Forward,nRet:%d!\n",nRet);
        return ERR;
    }
    // read response
    iov[0].iov_base = &rsp_head;
    iov[0].iov_len = sizeof(rsp_head);
    nRet = readv(fd_algorithm, iov, 1);
    if (rsp_head.code != SDC_CODE_200 || nRet < 0)
    {
        fprintf(stdout,"Error:failed to read info from NNIE Forward,nRet:%d,rsp_head.code:%d!\n",
            nRet, rsp_head.code);
        return ERR;
    } 
    return OK;
}

/******************************************************************************
* function : NNIE Forward
******************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_Forward(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S* pstInputDataIdx,
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S* pstProcSegIdx,HI_BOOL bInstant)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0, j = 0;
    //HI_BOOL bFinish = HI_FALSE;
    //SVP_NNIE_HANDLE hSvpNnieHandle = 0;
    // HI_U32 u32TotalStepNum = 0;
    sdc_nnie_forward_s sdc_nnie_forward;

	#if 0
    SDC_FlushCache(pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u64PhyAddr,
        (HI_VOID *) pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u64VirAddr,
        pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u32Size);
	#endif

    /*set input blob according to node name*/
    if(pstInputDataIdx->u32SegIdx != pstProcSegIdx->u32SegIdx)
    {
        for(i = 0; i < pstNnieParam->pstModel->astSeg[pstProcSegIdx->u32SegIdx].u16SrcNum; i++)
        {
            for(j = 0; j < pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].u16DstNum; j++)
            {
                if(0 == strncmp(pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].astDstNode[j].szName,
                    pstNnieParam->pstModel->astSeg[pstProcSegIdx->u32SegIdx].astSrcNode[i].szName,
                    SVP_NNIE_NODE_NAME_LEN))
                {
                    pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc[i] =
                        pstNnieParam->astSegData[pstInputDataIdx->u32SegIdx].astDst[j];
                    break;
                }
            }
            SAMPLE_SVP_CHECK_EXPR_RET((j == pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].u16DstNum),
                HI_FAILURE,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,can't find %d-th seg's %d-th src blob!\n",
                pstProcSegIdx->u32SegIdx,i);
        }
    }

    /*NNIE_Forward*/


    
    memcpy(&sdc_nnie_forward.model, pstNnieParam->pstModel,  sizeof(SVP_NNIE_MODEL_S));
    sdc_nnie_forward.forward_ctrl.max_batch_num = 1;
	sdc_nnie_forward.forward_ctrl.max_bbox_num = 0;/*此处需要根据算法模型的ROI个数决定，max_bbox_num = max_roi_num(ROI个数)*/
    sdc_nnie_forward.forward_ctrl.netseg_id = pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].u32NetSegId;
    
    memcpy(sdc_nnie_forward.astSrc, pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc,  16*sizeof(SVP_DST_BLOB_S));
    memcpy(sdc_nnie_forward.astDst, pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst,  16*sizeof(SVP_DST_BLOB_S));



/*目前NNIE的服务化接口有一个BUG，最大max_bbox_num有限制，最新版本会修复使用服务化接口*/
#if 1
    s32Ret = SDC_Nnie_Forward(&sdc_nnie_forward);
    if(s32Ret != OK) 
    {   
        printf("%s  %d!\n", __FILE__,__LINE__);
		fprintf(stderr, "Err in SDC_Nnie_Forward, s32Ret: %d\n", s32Ret);
		return ERR;
	}
    printf("%s  %d!\n", __FILE__,__LINE__);
#else
    
    /*NNIE_Forward*/
    s32Ret = HI_MPI_SVP_NNIE_Forward(&hSvpNnieHandle,
        pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc,
        pstNnieParam->pstModel, pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst,
        &pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx], bInstant);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error,HI_MPI_SVP_NNIE_Forward failed!\n");

    if(bInstant)
    {
        /*Wait NNIE finish*/
        while(HI_ERR_SVP_NNIE_QUERY_TIMEOUT == (s32Ret = HI_MPI_SVP_NNIE_Query(pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].enNnieId,
            hSvpNnieHandle, &bFinish, HI_TRUE)))
        {
            usleep(100);
            SAMPLE_SVP_TRACE(SAMPLE_SVP_ERR_LEVEL_INFO,
                "HI_MPI_SVP_NNIE_Query Query timeout!\n");
        }
    }


    //bFinish = HI_FALSE;
    for(i = 0; i < pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].u32DstNum; i++)
    {
        if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].enType)
        {
            for(j = 0; j < pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num; j++)
            {
                u32TotalStepNum += *((HI_U32*)(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stSeq.u64VirAddrStep)+j);
            }
            SDC_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                (HI_VOID *) pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr,
                u32TotalStepNum*pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);

        }
        else
        {

            SDC_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                (HI_VOID *) pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr,
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Chn*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Height*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);
        }
    }

#endif

    return s32Ret;
}


/*****************************************************************************
* Prototype :   SVP_NNIE_Ssd_PriorBoxForward
* Description : this function is used to get SSD priorbox
* Input :     HI_U32 u32PriorBoxWidth            [IN] prior box width
*             HI_U32 u32PriorBoxHeight           [IN] prior box height
*             HI_U32 u32OriImWidth               [IN] input image width
*             HI_U32 u32OriImHeight              [IN] input image height
*             HI_U32 f32PriorBoxMinSize          [IN] prior box min size
*             HI_U32 u32MinSizeNum               [IN] min size num
*             HI_U32 f32PriorBoxMaxSize          [IN] prior box max size
*             HI_U32 u32MaxSizeNum               [IN] max size num
*             HI_BOOL bFlip                      [IN] whether do Flip
*             HI_BOOL bClip                      [IN] whether do Clip
*             HI_U32  u32InputAspectRatioNum     [IN] aspect ratio num
*             HI_FLOAT af32PriorBoxAspectRatio[] [IN] aspect ratio value
*             HI_FLOAT f32PriorBoxStepWidth      [IN] prior box step width
*             HI_FLOAT f32PriorBoxStepHeight     [IN] prior box step height
*             HI_FLOAT f32Offset                 [IN] offset value
*             HI_S32   as32PriorBoxVar[]         [IN] prior box variance
*             HI_S32*  ps32PriorboxOutputData    [OUT] output reslut
*
* Output :
* Return Value : HI_SUCCESS: Success;Error codes: Failure.
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-11-10
* Author :
* Modification : Create
*
*****************************************************************************/
HI_S32 SVP_NNIE_Ssd_PriorBoxForward(HI_U32 u32PriorBoxWidth,
    HI_U32 u32PriorBoxHeight, HI_U32 u32OriImWidth, HI_U32 u32OriImHeight,
    HI_FLOAT* pf32PriorBoxMinSize, HI_U32 u32MinSizeNum, HI_FLOAT* pf32PriorBoxMaxSize,
    HI_U32 u32MaxSizeNum, HI_BOOL bFlip, HI_BOOL bClip, HI_U32 u32InputAspectRatioNum,
    HI_FLOAT af32PriorBoxAspectRatio[],HI_FLOAT f32PriorBoxStepWidth,
    HI_FLOAT f32PriorBoxStepHeight,HI_FLOAT f32Offset,HI_S32 as32PriorBoxVar[],
    HI_S32* ps32PriorboxOutputData)
{
    HI_U32 u32AspectRatioNum = 0;
    HI_U32 u32Index = 0;
    HI_FLOAT af32AspectRatio[SAMPLE_SVP_NNIE_SSD_ASPECT_RATIO_NUM] = { 0 };
    HI_U32 u32NumPrior = 0;
    HI_FLOAT f32CenterX = 0;
    HI_FLOAT f32CenterY = 0;
    HI_FLOAT f32BoxHeight = 0;
    HI_FLOAT f32BoxWidth = 0;
    HI_FLOAT f32MaxBoxWidth = 0;
    HI_U32 i = 0;
    HI_U32 j = 0;
    HI_U32 n = 0;
    HI_U32 h = 0;
    HI_U32 w = 0;
    SAMPLE_SVP_CHECK_EXPR_RET((HI_TRUE == bFlip && u32InputAspectRatioNum >
        (SAMPLE_SVP_NNIE_SSD_ASPECT_RATIO_NUM-1)/2),HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error,when bFlip is true, u32InputAspectRatioNum(%d) can't be greater than %d!\n",
        u32InputAspectRatioNum, (SAMPLE_SVP_NNIE_SSD_ASPECT_RATIO_NUM-1)/2);
    SAMPLE_SVP_CHECK_EXPR_RET((HI_FALSE == bFlip && u32InputAspectRatioNum >
        (SAMPLE_SVP_NNIE_SSD_ASPECT_RATIO_NUM-1)),HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error,when bFlip is false, u32InputAspectRatioNum(%d) can't be greater than %d!\n",
        u32InputAspectRatioNum, (SAMPLE_SVP_NNIE_SSD_ASPECT_RATIO_NUM-1));

    // generate aspect_ratios
    u32AspectRatioNum = 0;
    af32AspectRatio[0] = 1;
    u32AspectRatioNum++;
    for (i = 0; i < u32InputAspectRatioNum; i++)
    {
        af32AspectRatio[u32AspectRatioNum++] = af32PriorBoxAspectRatio[i];
        if (bFlip)
        {
            af32AspectRatio[u32AspectRatioNum++] = 1.0f / af32PriorBoxAspectRatio[i];
        }
    }
    u32NumPrior = u32MinSizeNum * u32AspectRatioNum + u32MaxSizeNum;

    u32Index = 0;
    for (h = 0; h < u32PriorBoxHeight; h++)
    {
        for (w = 0; w < u32PriorBoxWidth; w++)
        {
            f32CenterX = (w + f32Offset) * f32PriorBoxStepWidth;
            f32CenterY = (h + f32Offset) * f32PriorBoxStepHeight;
            for (n = 0; n < u32MinSizeNum; n++)
            {
                /*** first prior ***/
                f32BoxHeight = pf32PriorBoxMinSize[n];
                f32BoxWidth = pf32PriorBoxMinSize[n];
                ps32PriorboxOutputData[u32Index++] = (HI_S32)(f32CenterX - f32BoxWidth * SAMPLE_SVP_NNIE_HALF);
                ps32PriorboxOutputData[u32Index++] = (HI_S32)(f32CenterY - f32BoxHeight * SAMPLE_SVP_NNIE_HALF);
                ps32PriorboxOutputData[u32Index++] = (HI_S32)(f32CenterX + f32BoxWidth * SAMPLE_SVP_NNIE_HALF);
                ps32PriorboxOutputData[u32Index++] = (HI_S32)(f32CenterY + f32BoxHeight * SAMPLE_SVP_NNIE_HALF);
                /*** second prior ***/
                if(u32MaxSizeNum>0)
                {
                    f32MaxBoxWidth = sqrt(pf32PriorBoxMinSize[n] * pf32PriorBoxMaxSize[n]);
                    f32BoxHeight = f32MaxBoxWidth;
                    f32BoxWidth = f32MaxBoxWidth;
                    ps32PriorboxOutputData[u32Index++] = (HI_S32)(f32CenterX - f32BoxWidth * SAMPLE_SVP_NNIE_HALF);
                    ps32PriorboxOutputData[u32Index++] = (HI_S32)(f32CenterY - f32BoxHeight * SAMPLE_SVP_NNIE_HALF);
                    ps32PriorboxOutputData[u32Index++] = (HI_S32)(f32CenterX + f32BoxWidth * SAMPLE_SVP_NNIE_HALF);
                    ps32PriorboxOutputData[u32Index++] = (HI_S32)(f32CenterY + f32BoxHeight * SAMPLE_SVP_NNIE_HALF);
                }
                /**** rest of priors, skip AspectRatio == 1 ****/
                for (i = 1; i < u32AspectRatioNum; i++)
                {
                    f32BoxWidth = (HI_FLOAT)(pf32PriorBoxMinSize[n] * sqrt( af32AspectRatio[i] ));
                    f32BoxHeight = (HI_FLOAT)(pf32PriorBoxMinSize[n]/sqrt( af32AspectRatio[i] ));
                    ps32PriorboxOutputData[u32Index++] = (HI_S32)(f32CenterX - f32BoxWidth * SAMPLE_SVP_NNIE_HALF);
                    ps32PriorboxOutputData[u32Index++] = (HI_S32)(f32CenterY - f32BoxHeight * SAMPLE_SVP_NNIE_HALF);
                    ps32PriorboxOutputData[u32Index++] = (HI_S32)(f32CenterX + f32BoxWidth * SAMPLE_SVP_NNIE_HALF);
                    ps32PriorboxOutputData[u32Index++] = (HI_S32)(f32CenterY + f32BoxHeight * SAMPLE_SVP_NNIE_HALF);
                }
            }
        }
    }
    /************ clip the priors' coordidates, within [0, u32ImgWidth] & [0, u32ImgHeight] *************/
    if (bClip)
    {
        for (i = 0; i < (HI_U32)(u32PriorBoxWidth * u32PriorBoxHeight * SAMPLE_SVP_NNIE_COORDI_NUM*u32NumPrior / 2); i++)
        {
            ps32PriorboxOutputData[2 * i] = SAMPLE_SVP_NNIE_MIN((HI_U32)SAMPLE_SVP_NNIE_MAX(ps32PriorboxOutputData[2 * i], 0), u32OriImWidth);
            ps32PriorboxOutputData[2 * i + 1] = SAMPLE_SVP_NNIE_MIN((HI_U32)SAMPLE_SVP_NNIE_MAX(ps32PriorboxOutputData[2 * i + 1], 0), u32OriImHeight);
        }
    }
    /*********************** get var **********************/
    for (h = 0; h < u32PriorBoxHeight; h++)
    {
        for (w = 0; w < u32PriorBoxWidth; w++)
        {
            for (i = 0; i < u32NumPrior; i++)
            {
                for (j = 0; j < SAMPLE_SVP_NNIE_COORDI_NUM; j++)
                {
                    ps32PriorboxOutputData[u32Index++] = (HI_S32)as32PriorBoxVar[j];
                }
            }
        }
    }
    return HI_SUCCESS;
}

/*****************************************************************************
* Prototype :   SVP_NNIE_SSD_SoftMax
* Description : this function is used to do softmax for SSD
* Input :       HI_S32*           pf32Src          [IN]   the pointer to input array
*               HI_S32            s32ArraySize     [IN]   the array size
*               HI_S32*           ps32Dst          [OUT]  the pointer to output array
*
*
*
*
* Output :
* Return Value : void
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-03-10
* Author :
* Modification : Create
*
*****************************************************************************/
HI_S32 SVP_NNIE_SSD_SoftMax(HI_S32* ps32Src, HI_S32 s32ArraySize, HI_S32* ps32Dst)
{
    HI_S32 s32Max = 0;
    HI_S32 s32Sum = 0;
    HI_S32 i = 0;
    for (i = 0; i < s32ArraySize; ++i)
    {
        if (s32Max < ps32Src[i])
        {
            s32Max = ps32Src[i];
        }
    }
    for (i = 0; i < s32ArraySize; ++i)
    {
        ps32Dst[i] = (HI_S32)(SAMPLE_SVP_NNIE_QUANT_BASE* exp((HI_FLOAT)(ps32Src[i] - s32Max) / SAMPLE_SVP_NNIE_QUANT_BASE));
        s32Sum += ps32Dst[i];
    }
    for (i = 0; i < s32ArraySize; ++i)
    {
        ps32Dst[i] = (HI_S32)(((HI_FLOAT)ps32Dst[i] / (HI_FLOAT)s32Sum) * SAMPLE_SVP_NNIE_QUANT_BASE);
    }
    return HI_SUCCESS;
}
/*****************************************************************************
* Prototype :   SVP_NNIE_Ssd_SoftmaxForward
* Description : this function is used to do SSD softmax
* Input :     HI_U32 u32SoftMaxInHeight          [IN] softmax input height
*             HI_U32 au32SoftMaxInChn[]          [IN] softmax input channel
*             HI_U32 u32ConcatNum                [IN] concat num
*             HI_U32 au32ConvStride[]            [IN] conv stride
*             HI_U32 u32SoftMaxOutWidth          [IN] softmax output width
*             HI_U32 u32SoftMaxOutHeight         [IN] softmax output height
*             HI_U32 u32SoftMaxOutChn            [IN] softmax output channel
*             HI_S32* aps32SoftMaxInputData[]    [IN] softmax input data
*             HI_S32* ps32SoftMaxOutputData      [OUT]softmax output data
*
*
* Output :
* Return Value : HI_SUCCESS: Success;Error codes: Failure.
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-11-10
* Author :
* Modification : Create
*
*****************************************************************************/
HI_S32 SVP_NNIE_Ssd_SoftmaxForward(HI_U32 u32SoftMaxInHeight,
    HI_U32 au32SoftMaxInChn[], HI_U32 u32ConcatNum, HI_U32 au32ConvStride[],
    HI_U32 au32SoftMaxWidth[],HI_S32* aps32SoftMaxInputData[], HI_S32* ps32SoftMaxOutputData)
{
    HI_S32* ps32InputData = NULL;
    HI_S32* ps32OutputTmp = NULL;
    HI_U32 u32OuterNum = 0;
    HI_U32 u32InnerNum = 0;
    HI_U32 u32InputChannel = 0;
    HI_U32 i = 0;
    HI_U32 u32ConcatCnt = 0;
    HI_S32 s32Ret = 0;
    HI_U32 u32Stride = 0;
    HI_U32 u32Skip = 0;
    HI_U32 u32Left = 0;
    ps32OutputTmp = ps32SoftMaxOutputData;
    for (u32ConcatCnt = 0; u32ConcatCnt < u32ConcatNum; u32ConcatCnt++)
    {
        ps32InputData = aps32SoftMaxInputData[u32ConcatCnt];
        u32Stride = au32ConvStride[u32ConcatCnt];
        u32InputChannel = au32SoftMaxInChn[u32ConcatCnt];
        u32OuterNum = u32InputChannel / u32SoftMaxInHeight;
        u32InnerNum = u32SoftMaxInHeight;
        u32Skip = au32SoftMaxWidth[u32ConcatCnt] / u32InnerNum;
        u32Left = u32Stride - au32SoftMaxWidth[u32ConcatCnt];
        for (i = 0; i < u32OuterNum; i++)
        {
            s32Ret = SVP_NNIE_SSD_SoftMax(ps32InputData, (HI_S32)u32InnerNum,ps32OutputTmp);
            if ((i + 1) % u32Skip == 0)
            {
                ps32InputData += u32Left;
            }
            ps32InputData += u32InnerNum;
            ps32OutputTmp += u32InnerNum;
        }
    }
    return s32Ret;
}



/*****************************************************************************
* Prototype :   SVP_NNIE_Ssd_DetectionOutForward
* Description : this function is used to get detection result of SSD
* Input :     HI_U32 u32ConcatNum            [IN] SSD concat num
*             HI_U32 u32ConfThresh           [IN] confidence thresh
*             HI_U32 u32ClassNum             [IN] class num
*             HI_U32 u32TopK                 [IN] Topk value
*             HI_U32 u32KeepTopK             [IN] KeepTopK value
*             HI_U32 u32NmsThresh            [IN] NMS thresh
*             HI_U32 au32DetectInputChn[]    [IN] detection input channel
*             HI_S32* aps32AllLocPreds[]     [IN] Location prediction
*             HI_S32* aps32AllPriorBoxes[]   [IN] prior box
*             HI_S32* ps32ConfScores         [IN] confidence score
*             HI_S32* ps32AssistMemPool      [IN] assist buffer
*             HI_S32* ps32DstScoreSrc        [OUT] result of score
*             HI_S32* ps32DstBboxSrc         [OUT] result of Bbox
*             HI_S32* ps32RoiOutCntSrc       [OUT] result of the roi num of each class
*
*
* Output :
* Return Value : HI_SUCCESS: Success;Error codes: Failure.
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-11-10
* Author :
* Modification : Create
*
*****************************************************************************/
HI_S32 SVP_NNIE_Ssd_DetectionOutForward(HI_U32 u32ConcatNum,
    HI_U32 u32ConfThresh,HI_U32 u32ClassNum, HI_U32 u32TopK, HI_U32 u32KeepTopK, HI_U32 u32NmsThresh,
    HI_U32 au32DetectInputChn[], HI_S32* aps32AllLocPreds[], HI_S32* aps32AllPriorBoxes[],
    HI_S32* ps32ConfScores, HI_S32* ps32AssistMemPool, HI_S32* ps32DstScoreSrc,
    HI_S32* ps32DstBboxSrc, HI_S32* ps32RoiOutCntSrc)
{
    HI_S32* ps32LocPreds = NULL;
    HI_S32* ps32PriorBoxes = NULL;
    HI_S32* ps32PriorVar = NULL;
    HI_S32* ps32AllDecodeBoxes = NULL;
    HI_S32* ps32DstScore = NULL;
    HI_S32* ps32DstBbox = NULL;
    HI_S32* ps32ClassRoiNum = NULL;
    HI_U32 u32RoiOutCnt = 0;
    HI_S32* ps32SingleProposal = NULL;
    HI_S32* ps32AfterTopK = NULL;
    SAMPLE_SVP_NNIE_STACK_S* pstStack = NULL;
    HI_U32 u32PriorNum = 0;
    HI_U32 u32NumPredsPerClass = 0;
    HI_FLOAT f32PriorWidth = 0;
    HI_FLOAT f32PriorHeight = 0;
    HI_FLOAT f32PriorCenterX = 0;
    HI_FLOAT f32PriorCenterY = 0;
    HI_FLOAT f32DecodeBoxCenterX = 0;
    HI_FLOAT f32DecodeBoxCenterY = 0;
    HI_FLOAT f32DecodeBoxWidth = 0;
    HI_FLOAT f32DecodeBoxHeight = 0;
    HI_U32 u32SrcIdx = 0;
    HI_U32 u32AfterFilter = 0;
    HI_U32 u32AfterTopK = 0;
    HI_U32 u32KeepCnt = 0;
    HI_U32 i = 0;
    HI_U32 j = 0;
    HI_U32 u32Offset = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    u32PriorNum = 0;
    for (i = 0; i < u32ConcatNum; i++)
    {
        u32PriorNum += au32DetectInputChn[i] / SAMPLE_SVP_NNIE_COORDI_NUM;
    }
    //prepare for Assist MemPool
    ps32AllDecodeBoxes = ps32AssistMemPool;
    ps32SingleProposal = ps32AllDecodeBoxes + u32PriorNum * SAMPLE_SVP_NNIE_COORDI_NUM;
    ps32AfterTopK = ps32SingleProposal + SAMPLE_SVP_NNIE_PROPOSAL_WIDTH * u32PriorNum;
    pstStack = (SAMPLE_SVP_NNIE_STACK_S*)(ps32AfterTopK + u32PriorNum * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH);
    u32SrcIdx = 0;
    for (i = 0; i < u32ConcatNum; i++)
    {
        /********** get loc predictions ************/
        ps32LocPreds = aps32AllLocPreds[i];
        u32NumPredsPerClass = au32DetectInputChn[i] / SAMPLE_SVP_NNIE_COORDI_NUM;
        /********** get Prior Bboxes ************/
        ps32PriorBoxes = aps32AllPriorBoxes[i];
        ps32PriorVar = ps32PriorBoxes + u32NumPredsPerClass*SAMPLE_SVP_NNIE_COORDI_NUM;
        for (j = 0; j < u32NumPredsPerClass; j++)
        {
            f32PriorWidth = (HI_FLOAT)(ps32PriorBoxes[j*SAMPLE_SVP_NNIE_COORDI_NUM+2] - ps32PriorBoxes[j*SAMPLE_SVP_NNIE_COORDI_NUM]);
            f32PriorHeight = (HI_FLOAT)(ps32PriorBoxes[j*SAMPLE_SVP_NNIE_COORDI_NUM+3] - ps32PriorBoxes[j*SAMPLE_SVP_NNIE_COORDI_NUM + 1]);
            f32PriorCenterX = (ps32PriorBoxes[j*SAMPLE_SVP_NNIE_COORDI_NUM+2] + ps32PriorBoxes[j*SAMPLE_SVP_NNIE_COORDI_NUM])*SAMPLE_SVP_NNIE_HALF;
            f32PriorCenterY = (ps32PriorBoxes[j*SAMPLE_SVP_NNIE_COORDI_NUM+3] + ps32PriorBoxes[j*SAMPLE_SVP_NNIE_COORDI_NUM+1])*SAMPLE_SVP_NNIE_HALF;

            f32DecodeBoxCenterX = ((HI_FLOAT)ps32PriorVar[j*SAMPLE_SVP_NNIE_COORDI_NUM]/SAMPLE_SVP_NNIE_QUANT_BASE)*
                ((HI_FLOAT)ps32LocPreds[j*SAMPLE_SVP_NNIE_COORDI_NUM]/SAMPLE_SVP_NNIE_QUANT_BASE)*f32PriorWidth+f32PriorCenterX;

            f32DecodeBoxCenterY = ((HI_FLOAT)ps32PriorVar[j*SAMPLE_SVP_NNIE_COORDI_NUM+1]/SAMPLE_SVP_NNIE_QUANT_BASE)*
                ((HI_FLOAT)ps32LocPreds[j*SAMPLE_SVP_NNIE_COORDI_NUM+1]/SAMPLE_SVP_NNIE_QUANT_BASE)*f32PriorHeight+f32PriorCenterY;

            f32DecodeBoxWidth = exp(((HI_FLOAT)ps32PriorVar[j*SAMPLE_SVP_NNIE_COORDI_NUM+2]/SAMPLE_SVP_NNIE_QUANT_BASE)*
                ((HI_FLOAT)ps32LocPreds[j*SAMPLE_SVP_NNIE_COORDI_NUM+2]/SAMPLE_SVP_NNIE_QUANT_BASE))*f32PriorWidth;

            f32DecodeBoxHeight = exp(((HI_FLOAT)ps32PriorVar[j*SAMPLE_SVP_NNIE_COORDI_NUM+3]/SAMPLE_SVP_NNIE_QUANT_BASE)*
                ((HI_FLOAT)ps32LocPreds[j*SAMPLE_SVP_NNIE_COORDI_NUM+3]/SAMPLE_SVP_NNIE_QUANT_BASE))*f32PriorHeight;

            ps32AllDecodeBoxes[u32SrcIdx++] = (HI_S32)(f32DecodeBoxCenterX - f32DecodeBoxWidth * SAMPLE_SVP_NNIE_HALF);
            ps32AllDecodeBoxes[u32SrcIdx++] = (HI_S32)(f32DecodeBoxCenterY - f32DecodeBoxHeight * SAMPLE_SVP_NNIE_HALF);
            ps32AllDecodeBoxes[u32SrcIdx++] = (HI_S32)(f32DecodeBoxCenterX + f32DecodeBoxWidth * SAMPLE_SVP_NNIE_HALF);
            ps32AllDecodeBoxes[u32SrcIdx++] = (HI_S32)(f32DecodeBoxCenterY + f32DecodeBoxHeight * SAMPLE_SVP_NNIE_HALF);
        }
    }
    /********** do NMS for each class *************/
    u32AfterTopK = 0;
    for (i = 0; i < u32ClassNum; i++)
    {
        for (j = 0; j < u32PriorNum; j++)
        {
            ps32SingleProposal[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH] = ps32AllDecodeBoxes[j * SAMPLE_SVP_NNIE_COORDI_NUM];
            ps32SingleProposal[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 1] = ps32AllDecodeBoxes[j * SAMPLE_SVP_NNIE_COORDI_NUM + 1];
            ps32SingleProposal[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 2] = ps32AllDecodeBoxes[j * SAMPLE_SVP_NNIE_COORDI_NUM + 2];
            ps32SingleProposal[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 3] = ps32AllDecodeBoxes[j * SAMPLE_SVP_NNIE_COORDI_NUM + 3];
            ps32SingleProposal[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 4] = ps32ConfScores[j*u32ClassNum + i];
            ps32SingleProposal[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 5] = 0;
        }
        s32Ret = SVP_NNIE_NonRecursiveArgQuickSort(ps32SingleProposal, 0, u32PriorNum - 1, pstStack,u32TopK);
        u32AfterFilter = (u32PriorNum < u32TopK) ? u32PriorNum : u32TopK;
        s32Ret = SVP_NNIE_NonMaxSuppression(ps32SingleProposal, u32AfterFilter, u32NmsThresh, u32AfterFilter);
        u32RoiOutCnt = 0;
        ps32DstScore = (HI_S32*)ps32DstScoreSrc;
        ps32DstBbox = (HI_S32*)ps32DstBboxSrc;
        ps32ClassRoiNum = (HI_S32*)ps32RoiOutCntSrc;
        ps32DstScore += (HI_S32)u32AfterTopK;
        ps32DstBbox += (HI_S32)(u32AfterTopK * SAMPLE_SVP_NNIE_COORDI_NUM);
        for (j = 0; j < u32TopK; j++)
        {
            if (ps32SingleProposal[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 5] == 0 &&
                ps32SingleProposal[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 4] > (HI_S32)u32NmsThresh)
            {
                ps32DstScore[u32RoiOutCnt] = ps32SingleProposal[j * 6 + 4];
                ps32DstBbox[u32RoiOutCnt * SAMPLE_SVP_NNIE_COORDI_NUM] = ps32SingleProposal[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH];
                ps32DstBbox[u32RoiOutCnt * SAMPLE_SVP_NNIE_COORDI_NUM + 1] = ps32SingleProposal[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 1];
                ps32DstBbox[u32RoiOutCnt * SAMPLE_SVP_NNIE_COORDI_NUM + 2] = ps32SingleProposal[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 2];
                ps32DstBbox[u32RoiOutCnt * SAMPLE_SVP_NNIE_COORDI_NUM + 3] = ps32SingleProposal[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 3];
                u32RoiOutCnt++;
            }
        }
        ps32ClassRoiNum[i] = (HI_S32)u32RoiOutCnt;
        u32AfterTopK += u32RoiOutCnt;
    }

    u32KeepCnt = 0;
    u32Offset = 0;
    if (u32AfterTopK > u32KeepTopK)
    {
        u32Offset = ps32ClassRoiNum[0];
        for (i = 1; i < u32ClassNum; i++)
        {
            ps32DstScore = (HI_S32*)ps32DstScoreSrc;
            ps32DstBbox = (HI_S32*)ps32DstBboxSrc;
            ps32ClassRoiNum = (HI_S32*)ps32RoiOutCntSrc;
            ps32DstScore += (HI_S32)(u32Offset);
            ps32DstBbox += (HI_S32)(u32Offset * SAMPLE_SVP_NNIE_COORDI_NUM);
            for (j = 0; j < (HI_U32)ps32ClassRoiNum[i]; j++)
            {
                ps32AfterTopK[u32KeepCnt * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH] = ps32DstBbox[j * SAMPLE_SVP_NNIE_COORDI_NUM];
                ps32AfterTopK[u32KeepCnt * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 1] = ps32DstBbox[j * SAMPLE_SVP_NNIE_COORDI_NUM + 1];
                ps32AfterTopK[u32KeepCnt * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 2] = ps32DstBbox[j * SAMPLE_SVP_NNIE_COORDI_NUM + 2];
                ps32AfterTopK[u32KeepCnt * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 3] = ps32DstBbox[j * SAMPLE_SVP_NNIE_COORDI_NUM + 3];
                ps32AfterTopK[u32KeepCnt * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 4] = ps32DstScore[j];
                ps32AfterTopK[u32KeepCnt * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 5] = i;
                u32KeepCnt++;
            }
            u32Offset = u32Offset + ps32ClassRoiNum[i];
        }
        s32Ret = SVP_NNIE_NonRecursiveArgQuickSort(ps32AfterTopK, 0, u32KeepCnt - 1, pstStack,u32KeepCnt);

        u32Offset = 0;
        u32Offset = ps32ClassRoiNum[0];
        for (i = 1; i < u32ClassNum; i++)
        {
            u32RoiOutCnt = 0;
            ps32DstScore = (HI_S32*)ps32DstScoreSrc;
            ps32DstBbox = (HI_S32*)ps32DstBboxSrc;
            ps32ClassRoiNum = (HI_S32*)ps32RoiOutCntSrc;
            ps32DstScore += (HI_S32)(u32Offset);
            ps32DstBbox += (HI_S32)(u32Offset * SAMPLE_SVP_NNIE_COORDI_NUM);
            for (j = 0; j < u32KeepTopK; j++)
            {
                if (ps32AfterTopK[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 5] == i)
                {
                    ps32DstScore[u32RoiOutCnt] = ps32AfterTopK[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 4];
                    ps32DstBbox[u32RoiOutCnt * SAMPLE_SVP_NNIE_COORDI_NUM] = ps32AfterTopK[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH];
                    ps32DstBbox[u32RoiOutCnt * SAMPLE_SVP_NNIE_COORDI_NUM + 1] = ps32AfterTopK[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 1];
                    ps32DstBbox[u32RoiOutCnt * SAMPLE_SVP_NNIE_COORDI_NUM + 2] = ps32AfterTopK[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 2];
                    ps32DstBbox[u32RoiOutCnt * SAMPLE_SVP_NNIE_COORDI_NUM + 3] = ps32AfterTopK[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 3];
                    u32RoiOutCnt++;
                }
            }
            ps32ClassRoiNum[i] = (HI_S32)u32RoiOutCnt;
            u32Offset += u32RoiOutCnt;
        }
    }
    return s32Ret;
}



/*****************************************************************************
* Prototype :   SVP_NNIE_NonRecursiveArgQuickSort
* Description : this function is used to do quick sort
* Input :       HI_S32*             ps32Array         [IN]   the array need to be sorted
*               HI_S32              s32Low            [IN]   the start position of quick sort
*               HI_S32              s32High           [IN]   the end position of quick sort
*               SAMPLE_SVP_NNIE_STACK_S *  pstStack   [IN]   the buffer used to store start positions and end positions
*
*
*
*
* Output :
* Return Value : HI_SUCCESS: Success;Error codes: Failure.
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-03-10
* Author :
* Modification : Create
*
*****************************************************************************/
HI_S32 SVP_NNIE_NonRecursiveArgQuickSort(HI_S32* ps32Array,
    HI_S32 s32Low, HI_S32 s32High, SAMPLE_SVP_NNIE_STACK_S *pstStack,HI_U32 u32MaxNum)
{
    HI_S32 i = s32Low;
    HI_S32 j = s32High;
    HI_S32 s32Top = 0;
    HI_S32 s32KeyConfidence = ps32Array[SAMPLE_SVP_NNIE_PROPOSAL_WIDTH * s32Low + 4];
    pstStack[s32Top].s32Min = s32Low;
    pstStack[s32Top].s32Max = s32High;

    while(s32Top > -1)
    {
        s32Low = pstStack[s32Top].s32Min;
        s32High = pstStack[s32Top].s32Max;
        i = s32Low;
        j = s32High;
        s32Top--;

        s32KeyConfidence = ps32Array[SAMPLE_SVP_NNIE_PROPOSAL_WIDTH * s32Low + 4];

        while(i < j)
        {
            while((i < j) && (s32KeyConfidence > ps32Array[j * SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 4]))
            {
                j--;
            }
            if(i < j)
            {
                SVP_NNIE_Argswap(&ps32Array[i*SAMPLE_SVP_NNIE_PROPOSAL_WIDTH], &ps32Array[j*SAMPLE_SVP_NNIE_PROPOSAL_WIDTH]);
                i++;
            }

            while((i < j) && (s32KeyConfidence < ps32Array[i*SAMPLE_SVP_NNIE_PROPOSAL_WIDTH + 4]))
            {
                i++;
            }
            if(i < j)
            {
                SVP_NNIE_Argswap(&ps32Array[i*SAMPLE_SVP_NNIE_PROPOSAL_WIDTH], &ps32Array[j*SAMPLE_SVP_NNIE_PROPOSAL_WIDTH]);
                j--;
            }
        }

        if(s32Low <= u32MaxNum)
        {
                if(s32Low < i-1)
                {
                    s32Top++;
                    pstStack[s32Top].s32Min = s32Low;
                    pstStack[s32Top].s32Max = i-1;
                }

                if(s32High > i+1)
                {
                    s32Top++;
                    pstStack[s32Top].s32Min = i+1;
                    pstStack[s32Top].s32Max = s32High;
                }
        }
    }
    return HI_SUCCESS;
}


/*****************************************************************************
* Prototype :   SVP_NNIE_Argswap
* Description : this function is used to exchange array data
* Input :       HI_FLOAT*           pf32Src1          [IN]   the pointer to the first array
*               HI_S32*             ps32Src2          [OUT]  the pointer to the second array
*
*
*
*
* Output :
* Return Value : void
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-03-10
* Author :
* Modification : Create
*
*****************************************************************************/
void SVP_NNIE_Argswap(HI_S32* ps32Src1, HI_S32* ps32Src2)
{
    HI_U32 i = 0;
    HI_S32 u32Tmp = 0;
    for( i = 0; i < SAMPLE_SVP_NNIE_PROPOSAL_WIDTH; i++ )
    {
        u32Tmp = ps32Src1[i];
        ps32Src1[i] = ps32Src2[i];
        ps32Src2[i] = u32Tmp;
    }
}



/*****************************************************************************
* Prototype :   SVP_NNIE_Overlap
* Description : this function is used to calculate the overlap ratio of two proposals
* Input :     HI_S32              s32XMin1          [IN]   first input proposal's minimum value of x coordinate
*               HI_S32              s32YMin1          [IN]   first input proposal's minimum value of y coordinate of first input proposal
*               HI_S32              s32XMax1          [IN]   first input proposal's maximum value of x coordinate of first input proposal
*               HI_S32              s32YMax1          [IN]   first input proposal's maximum value of y coordinate of first input proposal
*               HI_S32              s32XMin1          [IN]   second input proposal's minimum value of x coordinate
*               HI_S32              s32YMin1          [IN]   second input proposal's minimum value of y coordinate of first input proposal
*               HI_S32              s32XMax1          [IN]   second input proposal's maximum value of x coordinate of first input proposal
*               HI_S32              s32YMax1          [IN]   second input proposal's maximum value of y coordinate of first input proposal
*             HI_FLOAT            *pf32IoU          [INOUT]the pointer of the IoU value
*
*
* Output :
* Return Value : HI_FLOAT f32Iou.
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-03-10
* Author :
* Modification : Create
*
*****************************************************************************/
HI_S32 SVP_NNIE_Overlap(HI_S32 s32XMin1, HI_S32 s32YMin1, HI_S32 s32XMax1, HI_S32 s32YMax1, HI_S32 s32XMin2,
    HI_S32 s32YMin2, HI_S32 s32XMax2, HI_S32 s32YMax2,  HI_S32* s32AreaSum, HI_S32* s32AreaInter)
{
    HI_S32 s32Inter = 0;
    HI_S32 s32Total = 0;
    HI_S32 s32XMin = 0;
    HI_S32 s32YMin = 0;
    HI_S32 s32XMax = 0;
    HI_S32 s32YMax = 0;
    HI_S32 s32Area1 = 0;
    HI_S32 s32Area2 = 0;
    HI_S32 s32InterWidth = 0;
    HI_S32 s32InterHeight = 0;

    s32XMin = SAMPLE_SVP_NNIE_MAX(s32XMin1, s32XMin2);
    s32YMin = SAMPLE_SVP_NNIE_MAX(s32YMin1, s32YMin2);
    s32XMax = SAMPLE_SVP_NNIE_MIN(s32XMax1, s32XMax2);
    s32YMax = SAMPLE_SVP_NNIE_MIN(s32YMax1, s32YMax2);

    s32InterWidth = s32XMax - s32XMin + 1;
    s32InterHeight = s32YMax - s32YMin + 1;

    s32InterWidth = ( s32InterWidth >= 0 ) ? s32InterWidth : 0;
    s32InterHeight = ( s32InterHeight >= 0 ) ? s32InterHeight : 0;

    s32Inter = s32InterWidth * s32InterHeight;
    s32Area1 = (s32XMax1 - s32XMin1 + 1) * (s32YMax1 - s32YMin1 + 1);
    s32Area2 = (s32XMax2 - s32XMin2 + 1) * (s32YMax2 - s32YMin2 + 1);

    s32Total = s32Area1 + s32Area2 - s32Inter;

    *s32AreaSum = s32Total;
    *s32AreaInter = s32Inter;
    return HI_SUCCESS;
}
/*****************************************************************************
* Prototype :   SVP_NNIE_NonMaxSuppression
* Description : this function is used to do non maximum suppression
* Input :       HI_S32*           ps32Proposals     [IN]   proposals
*               HI_U32            u32AnchorsNum     [IN]   anchors num
*               HI_U32            u32NmsThresh      [IN]   non maximum suppression threshold
*               HI_U32            u32MaxRoiNum      [IN]  The max roi num for the roi pooling
*
*
*
*
* Output :
* Return Value : HI_SUCCESS: Success;Error codes: Failure.
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-03-10
* Author :
* Modification : Create
*
*****************************************************************************/
HI_S32 SVP_NNIE_NonMaxSuppression( HI_S32* ps32Proposals, HI_U32 u32AnchorsNum,
    HI_U32 u32NmsThresh,HI_U32 u32MaxRoiNum)
{
    HI_S32 s32XMin1 = 0;
    HI_S32 s32YMin1 = 0;
    HI_S32 s32XMax1 = 0;
    HI_S32 s32YMax1 = 0;
    HI_S32 s32XMin2 = 0;
    HI_S32 s32YMin2 = 0;
    HI_S32 s32XMax2 = 0;
    HI_S32 s32YMax2 = 0;
    HI_S32 s32AreaTotal = 0;
    HI_S32 s32AreaInter = 0;
    HI_U32 i = 0;
    HI_U32 j = 0;
    HI_U32 u32Num = 0;
    bool bNoOverlap  = true;
    for (i = 0; i < u32AnchorsNum && u32Num < u32MaxRoiNum; i++)
    {
        if( ps32Proposals[SAMPLE_SVP_NNIE_PROPOSAL_WIDTH*i+5] == 0 )
        {
            u32Num++;
            s32XMin1 =  ps32Proposals[SAMPLE_SVP_NNIE_PROPOSAL_WIDTH*i];
            s32YMin1 =  ps32Proposals[SAMPLE_SVP_NNIE_PROPOSAL_WIDTH*i+1];
            s32XMax1 =  ps32Proposals[SAMPLE_SVP_NNIE_PROPOSAL_WIDTH*i+2];
            s32YMax1 =  ps32Proposals[SAMPLE_SVP_NNIE_PROPOSAL_WIDTH*i+3];
            for(j= i+1;j< u32AnchorsNum; j++)
            {
                if( ps32Proposals[SAMPLE_SVP_NNIE_PROPOSAL_WIDTH*j+5] == 0 )
                {
                    s32XMin2 = ps32Proposals[SAMPLE_SVP_NNIE_PROPOSAL_WIDTH*j];
                    s32YMin2 = ps32Proposals[SAMPLE_SVP_NNIE_PROPOSAL_WIDTH*j+1];
                    s32XMax2 = ps32Proposals[SAMPLE_SVP_NNIE_PROPOSAL_WIDTH*j+2];
                    s32YMax2 = ps32Proposals[SAMPLE_SVP_NNIE_PROPOSAL_WIDTH*j+3];
                    bNoOverlap = (s32XMin2>s32XMax1)||(s32XMax2<s32XMin1)||(s32YMin2>s32YMax1)||(s32YMax2<s32YMin1);
                    if(bNoOverlap)
                    {
                        continue;
                    }
                    (void)SVP_NNIE_Overlap(s32XMin1, s32YMin1, s32XMax1, s32YMax1, s32XMin2, s32YMin2, s32XMax2, s32YMax2, &s32AreaTotal, &s32AreaInter);
                    if(s32AreaInter*SAMPLE_SVP_NNIE_QUANT_BASE > ((HI_S32)u32NmsThresh*s32AreaTotal))
                    {
                        if( ps32Proposals[SAMPLE_SVP_NNIE_PROPOSAL_WIDTH*i+4] >= ps32Proposals[SAMPLE_SVP_NNIE_PROPOSAL_WIDTH*j+4] )
                        {
                            ps32Proposals[SAMPLE_SVP_NNIE_PROPOSAL_WIDTH*j+5] = 1;
                        }
                        else
                        {
                            ps32Proposals[SAMPLE_SVP_NNIE_PROPOSAL_WIDTH*i+5] = 1;
                        }
                    }
                }
            }
        }
    }
    return HI_SUCCESS;
}


/*****************************************************************************
* Prototype :   SAMPLE_SVP_NNIE_Ssd_GetResult
* Description : this function is used to Get SSD result
* Input :     SAMPLE_SVP_NNIE_PARAM_S*               pstNnieParam     [IN]  the pointer to SSD NNIE parameter
*              SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S*   pstSoftwareParam [IN]  the pointer to SSD software parameter
*
*
*
*
* Output :
* Return Value : HI_SUCCESS: Success;Error codes: Failure.
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-11-10
* Author :
* Modification : Create
*
*****************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_Ssd_GetResult(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,
    SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam)
{
    HI_S32* aps32PermuteResult[SAMPLE_SVP_NNIE_SSD_REPORT_NODE_NUM];
    HI_S32* aps32PriorboxOutputData[SAMPLE_SVP_NNIE_SSD_PRIORBOX_NUM];
    HI_S32* aps32SoftMaxInputData[SAMPLE_SVP_NNIE_SSD_SOFTMAX_NUM];
    HI_S32* aps32DetectionLocData[SAMPLE_SVP_NNIE_SSD_SOFTMAX_NUM];
    HI_S32* ps32SoftMaxOutputData = NULL;
    HI_S32* ps32DetectionOutTmpBuf = NULL;
    HI_U32  au32SoftMaxWidth[SAMPLE_SVP_NNIE_SSD_SOFTMAX_NUM];
    HI_U32 u32Size = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0;

    /*get permut result*/
    printf("%s  %d!\n", __FILE__,__LINE__);
    for(i = 0; i < SAMPLE_SVP_NNIE_SSD_REPORT_NODE_NUM; i++)
    {
        aps32PermuteResult[i] = (HI_S32*)pstNnieParam->astSegData[0].astDst[i].u64VirAddr;
    }

    /*priorbox*/
    aps32PriorboxOutputData[0] = (HI_S32*)pstSoftwareParam->stPriorBoxTmpBuf.u64VirAddr;
    for (i = 1; i < SAMPLE_SVP_NNIE_SSD_PRIORBOX_NUM; i++)
    {
        u32Size = pstSoftwareParam->au32PriorBoxHeight[i-1]*pstSoftwareParam->au32PriorBoxWidth[i-1]*
            SAMPLE_SVP_NNIE_COORDI_NUM*2*(pstSoftwareParam->u32MaxSizeNum+pstSoftwareParam->u32MinSizeNum+
            pstSoftwareParam->au32InputAspectRatioNum[i-1]*2*pstSoftwareParam->u32MinSizeNum);
        aps32PriorboxOutputData[i] = aps32PriorboxOutputData[i - 1] + u32Size;
    }

    for (i = 0; i < SAMPLE_SVP_NNIE_SSD_PRIORBOX_NUM; i++)
    {
        s32Ret = SVP_NNIE_Ssd_PriorBoxForward(pstSoftwareParam->au32PriorBoxWidth[i],
            pstSoftwareParam->au32PriorBoxHeight[i], pstSoftwareParam->u32OriImWidth,
            pstSoftwareParam->u32OriImHeight, pstSoftwareParam->af32PriorBoxMinSize[i],
            pstSoftwareParam->u32MinSizeNum,pstSoftwareParam->af32PriorBoxMaxSize[i],
            pstSoftwareParam->u32MaxSizeNum, pstSoftwareParam->bFlip, pstSoftwareParam->bClip,
            pstSoftwareParam->au32InputAspectRatioNum[i],pstSoftwareParam->af32PriorBoxAspectRatio[i],
            pstSoftwareParam->af32PriorBoxStepWidth[i],pstSoftwareParam->af32PriorBoxStepHeight[i],
            pstSoftwareParam->f32Offset,pstSoftwareParam->as32PriorBoxVar,
            aps32PriorboxOutputData[i]);
        SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error,SVP_NNIE_Ssd_PriorBoxForward failed!\n");
    }
    printf("%s  %d!\n", __FILE__,__LINE__);

    /*softmax*/
    ps32SoftMaxOutputData = (HI_S32*)pstSoftwareParam->stSoftMaxTmpBuf.u64VirAddr;
    for(i = 0; i < SAMPLE_SVP_NNIE_SSD_SOFTMAX_NUM; i++)
    {
        aps32SoftMaxInputData[i] = aps32PermuteResult[i*2+1];
        au32SoftMaxWidth[i] = pstSoftwareParam->au32ConvChannel[i*2+1];
    }
    printf("%s  %d!\n", __FILE__,__LINE__);
    (void)SVP_NNIE_Ssd_SoftmaxForward(pstSoftwareParam->u32SoftMaxInHeight,
        pstSoftwareParam->au32SoftMaxInChn, pstSoftwareParam->u32ConcatNum,
        pstSoftwareParam->au32ConvStride, au32SoftMaxWidth,
        aps32SoftMaxInputData, ps32SoftMaxOutputData);
    printf("%s  %d!\n", __FILE__,__LINE__);
    /*detection*/
    ps32DetectionOutTmpBuf = (HI_S32*)pstSoftwareParam->stGetResultTmpBuf.u64VirAddr;
    for(i = 0; i < SAMPLE_SVP_NNIE_SSD_PRIORBOX_NUM; i++)
    {
        aps32DetectionLocData[i] = aps32PermuteResult[i*2];
    }
    printf("%s  %d!\n", __FILE__,__LINE__);
    (void)SVP_NNIE_Ssd_DetectionOutForward(pstSoftwareParam->u32ConcatNum,
        pstSoftwareParam->u32ConfThresh,pstSoftwareParam->u32ClassNum, pstSoftwareParam->u32TopK,
        pstSoftwareParam->u32KeepTopK, pstSoftwareParam->u32NmsThresh,pstSoftwareParam->au32DetectInputChn,
        aps32DetectionLocData, aps32PriorboxOutputData, ps32SoftMaxOutputData,
        ps32DetectionOutTmpBuf, (HI_S32*)pstSoftwareParam->stDstScore.u64VirAddr,
        (HI_S32*)pstSoftwareParam->stDstRoi.u64VirAddr,
        (HI_S32*)pstSoftwareParam->stClassRoiNum.u64VirAddr);
    printf("%s  %d!\n", __FILE__,__LINE__);
    return s32Ret;
}



HI_S32 SDC_SVP_NNIE_Detection_GetResult(SVP_BLOB_S *pstDstScore,
    SVP_BLOB_S *pstDstRoi, SVP_BLOB_S *pstClassRoiNum, SDC_SSD_RESULT_S *pstResult)
{
    HI_U32 i = 0, j = 0;
    HI_U32 u32RoiNumBias = 0;
    HI_U32 u32ScoreBias = 0;
    HI_U32 u32BboxBias = 0;
    HI_FLOAT f32Score = 0.0f;
    HI_S32* ps32Score = (HI_S32*)pstDstScore->u64VirAddr;
    HI_S32* ps32Roi = (HI_S32*)pstDstRoi->u64VirAddr;
    HI_S32* ps32ClassRoiNum = (HI_S32*)pstClassRoiNum->u64VirAddr;
    HI_U32 u32ClassNum = pstClassRoiNum->unShape.stWhc.u32Width;
    HI_S32 s32XMin = 0,s32YMin= 0,s32XMax = 0,s32YMax = 0;

	HI_S32 MaxObjectNum = pstResult->numOfObject;
	pstResult->numOfObject = 0;
    u32RoiNumBias += ps32ClassRoiNum[0];
    printf("%s  %d!\n", __FILE__,__LINE__);
    for (i = 1; i < u32ClassNum; i++)
    {
        u32ScoreBias = u32RoiNumBias;
        u32BboxBias = u32RoiNumBias * SAMPLE_SVP_NNIE_COORDI_NUM;
        /*if the confidence score greater than result threshold, the result will be printed*/
        // if((HI_FLOAT)ps32Score[u32ScoreBias] / SAMPLE_SVP_NNIE_QUANT_BASE >=
        //     pstResult->thresh && ps32ClassRoiNum[i]!=0)
        // {
        //    SAMPLE_SVP_TRACE_INFO("==== The %dth class box info====\n", i);
        // }
        for (j = 0; j < (HI_U32)ps32ClassRoiNum[i]; j++)
        {
            f32Score = (HI_FLOAT)ps32Score[u32ScoreBias + j] / SAMPLE_SVP_NNIE_QUANT_BASE;
            
            if (f32Score < pstResult->thresh)
            {
                break;
            }
            printf("%s  %d : f32Score %f!\n", __FILE__,__LINE__,f32Score);
            // s32XMin = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM];
            // s32YMin = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 1];
            // s32XMax = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 2];
            // s32YMax = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 3];
            // SAMPLE_SVP_TRACE_INFO("%d %d %d %d %f\n", s32XMin, s32YMin, s32XMax, s32YMax, f32Score);
//             pstResult->pObjInfo[pstResult->numOfObject].class_ = i;
// 			pstResult->pObjInfo[pstResult->numOfObject].confidence = f32Score;
// 			pstResult->pObjInfo[pstResult->numOfObject].x_left = s32XMin;
// 			pstResult->pObjInfo[pstResult->numOfObject].y_top = s32YMin;
// 			pstResult->pObjInfo[pstResult->numOfObject].x_right = s32XMax;
// 			pstResult->pObjInfo[pstResult->numOfObject].y_bottom = s32YMax;
// 			pstResult->pObjInfo[pstResult->numOfObject].w = s32XMax - s32XMin; 
// 			pstResult->pObjInfo[pstResult->numOfObject].h = s32YMax - s32YMin;
// 			pstResult->numOfObject += 1;
// 			if(pstResult->numOfObject == MaxObjectNum)
// 				return HI_SUCCESS;
        }
//         u32RoiNumBias += ps32ClassRoiNum[i];
    }
    printf("%s  %d!\n", __FILE__,__LINE__);
    return HI_SUCCESS;
}


int SDC_SVP_ForwardBGR(HI_CHAR *pcSrcBGR, SDC_SSD_RESULT_S *pstResult, SDC_SSD_INPUT_SIZE_S InputSize)
{
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};
    struct timespec time1 = {0, 0};
    struct timespec time2 = {0, 0};
    struct timespec time3 = {0, 0};
    unsigned int uiTimeCout = 0;

    /*Set configuration parameter*/
    stNnieCfg_SDC.pszPic = HI_NULL;
    stNnieCfg_SDC.pszYUV = HI_NULL;
    stNnieCfg_SDC.pszBGR = pcSrcBGR;

    /*Fill src data*/
    //  SAMPLE_SVP_TRACE_INFO("Ssd start!\n");
    stInputDataIdx.u32SegIdx = 0;
    stInputDataIdx.u32NodeIdx = 0;
    // printf("%s  %d!\n", __FILE__,__LINE__);
    s32Ret = SAMPLE_SVP_NNIE_FillSrcData(&stNnieCfg_SDC,&s_stSsdNnieParam,&stInputDataIdx);
    printf("%s  %d!\n", __FILE__,__LINE__);
    clock_gettime(CLOCK_BOOTTIME, &time1);
    stProcSegIdx.u32SegIdx = 0;
    s32Ret = SAMPLE_SVP_NNIE_Forward(&s_stSsdNnieParam,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
    printf("SAMPLE_SVP_NNIE_Forward s32Ret = 0x%x\n", s32Ret);
    printf("%s  %d!\n", __FILE__,__LINE__);
    clock_gettime(CLOCK_BOOTTIME, &time2);
    uiTimeCout = (unsigned int)(time2.tv_sec - time1.tv_sec)*1000 + (unsigned int)((time2.tv_nsec - time1.tv_nsec)/1000000);
    fprintf(stdout,"timeforward:%d\n", uiTimeCout);


    /*software process*/
    /*if user has changed net struct, please make sure SAMPLE_SVP_NNIE_Ssd_GetResult
     function's input datas are correct*/
    s32Ret = SAMPLE_SVP_NNIE_Ssd_GetResult(&s_stSsdNnieParam,&s_stSsdSoftwareParam);
    printf("SAMPLE_SVP_NNIE_Ssd_GetResult s32Ret = %d\n", s32Ret);
    printf("%s  %d!\n", __FILE__,__LINE__);
  
  
//   clock_gettime(CLOCK_BOOTTIME, &time3);
//   uiTimeCout = (unsigned int)(time3.tv_sec - time2.tv_sec)*1000 + (unsigned int)((time3.tv_nsec - time2.tv_nsec)/1000000);
//   fprintf(stdout,"timeSsd_GetResult:%d\n", uiTimeCout);


//     /*print result, this sample has 21 classes:
//      class 0:background     class 1:plane           class 2:bicycle
//      class 3:bird           class 4:boat            class 5:bottle
//      class 6:bus            class 7:car             class 8:cat
//      class 9:chair          class10:cow             class11:diningtable
//      class 12:dog           class13:horse           class14:motorbike
//      class 15:person        class16:pottedplant     class17:sheep
//      class 18:sofa          class19:train           class20:tvmonitor*/

    SDC_SVP_NNIE_Detection_GetResult(&s_stSsdSoftwareParam.stDstScore,
        &s_stSsdSoftwareParam.stDstRoi, &s_stSsdSoftwareParam.stClassRoiNum, pstResult);
    printf("%s  %d!\n", __FILE__,__LINE__);
	return s32Ret;
}

