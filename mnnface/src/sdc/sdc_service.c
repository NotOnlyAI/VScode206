

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



#include "sdc_service.h"




int fd_video = -1;
int fd_codec = -1;
int fd_codec2 = -1;
int fd_utils = -1;
int fd_algorithm = -1;
int fd_algorithm2 = -1;
int fd_event = -1;
int fd_cache = -1;
// int fd_tproxy = -1;


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

	fd_codec2 = open("/mnt/srvfs/codec.iaas.sdc",O_RDWR);
	if(fd_codec2 < 0) goto codec2_fail;


	fd_utils = open("/mnt/srvfs/utils.iaas.sdc",O_RDWR);
	if(fd_utils < 0) goto config_fail;
	fd_algorithm = open("/mnt/srvfs/algorithm.iaas.sdc",O_RDWR);
	if(fd_algorithm < 0) goto algorithm_fail;

	fd_algorithm2 = open("/mnt/srvfs/algorithm.iaas.sdc",O_RDWR);
	if(fd_algorithm2 < 0) goto algorithm2_fail;

	fd_event = open("/mnt/srvfs/event.paas.sdc",O_RDWR);
	if(fd_event < 0) goto event_fail;
	fd_cache = open("/dev/cache",O_RDWR);
	if(fd_cache < 0) goto cache_fail;
	// fd_tproxy = open("/mnt/srvfs/tproxy.paas.sdc", O_RDWR);
	// if(fd_tproxy < 0) goto tproxy_fail;
	
	return 0;

// tproxy_fail:
// 	fprintf(stderr, "open /dev/tproxy fail in SDC_ServiceCreate!\r\n");
// 	close(fd_cache);
// 	fd_cache = -1;
cache_fail:
	fprintf(stderr, "open /dev/cache fail in SDC_ServiceCreate!\r\n");
	close(fd_event);
	fd_event = -1;
event_fail:
	fprintf(stderr, "open event fail in SDC_ServiceCreate!\r\n");
	close(fd_algorithm2);
	fd_algorithm2 = -1;
algorithm2_fail:
	fprintf(stderr, "errno:%d open algorithm.iaas.sdc fail！\r\n", errno);
	close(fd_algorithm);
	fd_algorithm = -1;
algorithm_fail:
	fprintf(stderr, "errno:%d open algorithm2.iaas.sdc fail！\r\n", errno);
	close(fd_utils);
	fd_utils = -1;
config_fail:
	close(fd_codec2);
	fprintf(stderr, "errno:%d open utils.iaas.sdc fail！\r\n", errno);
	fd_codec2 = -1;

codec2_fail:
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




void SDC_ServiceDestroy(void)
{
	close(fd_video);
	close(fd_codec);
	close(fd_codec2);
	close(fd_utils);
	close(fd_algorithm);
	close(fd_algorithm2);
	close(fd_event);
	close(fd_cache);

    return;
}

