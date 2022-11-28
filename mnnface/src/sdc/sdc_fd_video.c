

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



#include "sdc_fd_video.h"



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
int SDC_YuvChnAttrGetIdleYuvChn(int fd_video, unsigned int *puiChnId)
{
	int nret,i;
	char buf[4096] = { 0 };
	sdc_yuv_channel_info_s* info;

    sdc_common_head_s* head = (sdc_common_head_s*)buf;
	head->version = SDC_VERSION; //0x5331
	head->url = SDC_URL_YUV_CHANNEL; //0x00
	head->method = SDC_METHOD_GET; //0x02
	head->head_length = sizeof(*head);
    head->content_length = 0;
     //head->content_length = sizeof(sdc_chan_query_s);

    sdc_chan_query_s* param;
	param = (sdc_chan_query_s*)(buf+head->head_length);
    param->channel = 0;/*通道ID为0时查询的是所有的逻辑通道*/
   

	nret = write(fd_video, head, head->head_length);
	if(nret < 0)
	{
	   fprintf(stdout,"writev fail yuv_channel_get,response:%d,url:%d,code:%d,method:%d\n",head->response,head->url,head->code, head->method);
	   return errno;
	}
	//fprintf(stdout,"yuv_channel_get write succeed \n");

	nret = read(fd_video,buf,sizeof(buf));
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



int SDC_YuvChnAttrSetDefault(int fd_video, int uiYuvChnId)
{
	sdc_yuv_channel_param_s param = 
    {
		.channel = uiYuvChnId,
        .width = 1920,
        .height = 1080,
		.fps = 10,
        .on_off = 1,
		.format = SDC_YVU_420SP,
	};

	// sdc_yuv_channel_param_s param = { 0 }; // 获取原始图默认分辨率大小

	sdc_common_head_s head;
	memset(&head, 0, sizeof(head));
	head.version = SDC_VERSION;
    head.url = SDC_URL_YUV_CHANNEL;
    head.method = SDC_METHOD_UPDATE;
    head.head_length = sizeof(head);
    head.content_length = sizeof(param);


    
	struct iovec iov[2] = { {.iov_base = &head, .iov_len = sizeof(head)},
		{.iov_base = &param, .iov_len = sizeof(param) }};
	int nret;

	nret = writev(fd_video, iov, 2);
	if(nret < 0)
	{
	   fprintf(stdout,"writev fail yuv_channel_set,response:%d,url:%d,code:%d,method:%d\n",head.response,head.url,head.code, head.method);
	   return errno;
	}

	nret = read(fd_video,&head, sizeof(head));
	if(nret < 0 || head.code != SDC_CODE_200)
	{
	   fprintf(stdout,"read fail response:%d,url:%d,code:%d, method:%d\n",head.response,head.url,head.code, head.method);
	   return errno;
	}

	// fprintf(stdout,"yuv_channel_set read succeed content_length:%d \n", head.content_length);
	
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
int SDC_YuvChnAttrSet(int fd_video, int uiYuvChnId,unsigned int w, unsigned int h,unsigned int fps)
{
	sdc_yuv_channel_param_s param = 
    {
		.channel = uiYuvChnId,
        .width = w,
        .height = h,
		.fps = fps,
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


    
	struct iovec iov[2] = { {.iov_base = &head, .iov_len = sizeof(head)},
		{.iov_base = &param, .iov_len = sizeof(param) }};
	int nret;

	nret = writev(fd_video, iov, 2);
	if(nret < 0)
	{
	   fprintf(stdout,"writev fail yuv_channel_set,response:%d,url:%d,code:%d,method:%d\n",head.response,head.url,head.code, head.method);
	   return errno;
	}

	nret = read(fd_video,&head, sizeof(head));
	if(nret < 0 || head.code != SDC_CODE_200)
	{
	   fprintf(stdout,"read fail response:%d,url:%d,code:%d, method:%d\n",head.response,head.url,head.code, head.method);
	   return errno;
	}

	// fprintf(stdout,"yuv_channel_set read succeed content_length:%d \n", head.content_length);
	
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
int SDC_YuvChnAttrGet(int fd_video)
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
    
	nret = write(fd_video, head, head->head_length);
	if(nret < 0)
	{
	   fprintf(stdout,"writev fail yuv_channel_get,response:%d,url:%d,code:%d,method:%d\n",head->response,head->url,head->code, head->method);
	   return errno;
	}
	fprintf(stdout,"yuv_channel_get write succeed \n");

	nret = read(fd_video,buf,sizeof(buf));
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


int SDC_YuvDataReq(int fd_video, int extendheadflag, unsigned int uiChnId, unsigned int uiMaxUsedBufNum)
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

	// fprintf(stdout,"yuv_frame_get:extendheadflag:%d\n",extendheadflag);

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
        
	nret = write(fd_video, head, head->head_length + head->content_length);
	if(nret < 0)
	{
	   fprintf(stdout,"writev fail yuv_frame_get,response:%d,url:%d,code:%d,method:%d\n",
            head->response,head->url,head->code, head->method);
	   return ERR;
	}

	nret = read(fd_video, buf,sizeof(buf));
	if(nret < 0)
	{
	   fprintf(stdout,"read fail yuv_frame_get,response:%d,url:%d,code:%d,method:%d\n",
            head->response,head->url,head->code, head->method);
	   return ERR;
	}
    // fprintf(stdout,"YuvDataReq Succeed,response:%d,url:%d,code:%d,method:%d\n",
    //         head->response,head->url,head->code, head->method);
    return OK;    
}

int32_t SDC_GetYuvData(int fd_video,sdc_yuv_data_s &yuv_data)
{
    uint8_t cMsgReadBuf[1024];
    sdc_common_head_s *pstSdcMsgHead = (sdc_common_head_s *)cMsgReadBuf;
	int iReadLen = 0;
	// printf("%s  %d!\n", __FILE__,__LINE__);
	iReadLen = read(fd_video, (void *)cMsgReadBuf, sizeof(cMsgReadBuf));
	// printf("%s  %d!\n", __FILE__,__LINE__);

	
    if (iReadLen < 0) {
        fprintf(stdout,"Read the Yuv frame fail response:%d,url:%d,code:%d, method:%d\n",
                pstSdcMsgHead->response,pstSdcMsgHead->url,pstSdcMsgHead->code, pstSdcMsgHead->method);
        return ERR;
    }
    fprintf(stdout,"Read the Yuv frame succeed response:%d,url:%d,code:%d, method:%d, iReadLen:%d\n",
            pstSdcMsgHead->response,pstSdcMsgHead->url,pstSdcMsgHead->code, pstSdcMsgHead->method, iReadLen);


    if (pstSdcMsgHead->url == SDC_URL_YUV_DATA) {
        if (pstSdcMsgHead->content_length != 0) {
            (void)memcpy(&yuv_data, /*sizeof(yuv_data),*/ cMsgReadBuf + pstSdcMsgHead->head_length, sizeof(yuv_data));
            return OK;
        }
    }
	    // fprintf(stdout,"Read the Yuv frame fail response:%d,url:%d,code:%d, method:%d\n",
        //         pstSdcMsgHead->response,pstSdcMsgHead->url,pstSdcMsgHead->code, pstSdcMsgHead->method);
    return ERR;
}

void SDC_YuvDataFree(int fd_video,  sdc_yuv_data_s *yuv_data)
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
    
    nret = write(fd_video, buf, head->head_length + sizeof(sdc_yuv_data_s)); 
    if(nret < 0)
    {
        fprintf(stdout,"writev fail yuv_frame_get,response:%d,url:%d,code:%d,method:%d\n",
        head->response,head->url,head->code, head->method);
    }

    return;
}






void SDC_DisplayYuvData(sdc_yuv_data_s* yuv_data)
{
    printf("yuv_data,channel:%d,reserve:%d,pts:%lld,pts_sys:%lld,addr_phy:0x%lx,addr_virt:0x%lx\n",
		yuv_data->channel, yuv_data->reserve,yuv_data->pts,yuv_data->pts_sys,
		yuv_data->frame.addr_phy,yuv_data->frame.addr_virt);
    printf("yuv_data  width=%d height=%d stride=%d size=%d\n",yuv_data->frame.width,yuv_data->frame.height,yuv_data->frame.stride,yuv_data->frame.size);
    return;
}


void SDC_DisplayYuvFrame(sdc_yuv_frame_s* yuv_frame)
{
    printf("yuv_frame,addr_phy:0x%lx,addr_virt:0x%lx", yuv_frame->addr_phy, yuv_frame->addr_virt);
    printf("yuv_frame  width=%d height=%d stride=%d size=%d \n", yuv_frame->width, yuv_frame->height, yuv_frame->stride, yuv_frame->size);
    return;
}


void SDC_DisplayJpegFrame(sdc_jpeg_frame_s *jpeg_frame)
{
    printf("jpeg_frame,addr_phy:0x%lx,addr_virt:0x%lx", jpeg_frame->addr_phy, jpeg_frame->addr_virt);
	printf("jpeg_frame,size:%d\n", jpeg_frame->size);
}