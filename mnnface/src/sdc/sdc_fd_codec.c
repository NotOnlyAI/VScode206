

#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <securec.h>
#include "basedef.h"

#include "sdc_fd_codec.h"
#include "common_defs.h"




// int SDC_YuvFrame2RGBFrame(int fd_codec,const sdc_yuv_frame_s &yuv_frame, sdc_yuv_frame_s &rgb_frame)
// {   
//     sdc_osd_region_s osd_region;
//     memset(&osd_region, /*sizeof(osd_region),*/ 0, sizeof(osd_region));
	
// 	// 如果需要添加1个OSD，请初始化InitOsdRegion()，否在注释OSD
// 	// (void) InitOsdRegion(yuv_frame, osd_region);

// 	// 将Yuv转换成图片帧
// 	sdc_region_s cropregion={0,0,yuv_frame.width,yuv_frame.height};
// 	if (SDC_Yuv2Jpeg(fd_codec,yuv_frame, osd_region, jpeg_frame,cropregion) != OK) {
// 		LOG_ERROR("Translate Yuv to Jpeg failed");
// 		return -1;
// 	}
// 	LOG_DEBUG("Translate Yuv to Jpeg successfully");
// 	return 0;
// }




int SDC_YuvFrame2Jpeg(int fd_codec,const sdc_yuv_frame_s &yuv_frame, sdc_jpeg_frame_s &jpeg_frame)
{   
    sdc_osd_region_s osd_region;
    memset(&osd_region, /*sizeof(osd_region),*/ 0, sizeof(osd_region));
	
	// 如果需要添加1个OSD，请初始化InitOsdRegion()，否在注释OSD
	// (void) InitOsdRegion(yuv_frame, osd_region);

	// 将Yuv转换成图片帧
	sdc_region_s cropregion={0,0,yuv_frame.width,yuv_frame.height};
	if (SDC_Yuv2Jpeg(fd_codec,yuv_frame, osd_region, jpeg_frame,cropregion) != OK) {
		LOG_ERROR("Translate Yuv to Jpeg failed");
		return -1;
	}
	LOG_DEBUG("Translate Yuv to Jpeg successfully");
	return 0;
}


int SDC_YuvFrame2CropJpeg(int fd_codec,const sdc_yuv_frame_s &yuv_frame, sdc_jpeg_frame_s &jpeg_frame,sdc_region_s cropregion)
{   
    sdc_osd_region_s osd_region;
    memset(&osd_region, /*sizeof(osd_region),*/ 0, sizeof(osd_region));
	
	// 如果需要添加1个OSD，请初始化InitOsdRegion()，否在注释OSD
	// (void) InitOsdRegion(yuv_frame, osd_region);

	// 将Yuv转换成图片帧
	if (SDC_Yuv2Jpeg(fd_codec,yuv_frame, osd_region, jpeg_frame,cropregion) != OK) {
		LOG_ERROR("Translate Yuv to Jpeg failed");
		return -1;
	}
	LOG_DEBUG("Translate Yuv to Jpeg successfully");
	return 0;
}




int SDC_Yuv2Jpeg(int fd_codec,const sdc_yuv_frame_s &yuv_frame, const sdc_osd_region_s &osd_region, sdc_jpeg_frame_s &jpeg_frame,sdc_region_s cropregion)
{
    sdc_encode_jpeg_param_s param;
    memset(&param,/* sizeof(param),*/ 0, sizeof(param));
	param.qf = 99;
	param.osd_region_cnt = 1;
	param.region = cropregion;
	param.frame = yuv_frame;

    sdc_common_head_s head;
    memset(&head, /*sizeof(head),*/ 0, sizeof(head));
	head.version = SDC_VERSION; // 0x5331
	head.url = SDC_URL_ENCODED_JPEG; // 0x00
	head.method = SDC_METHOD_CREATE;
	head.head_length = sizeof(head);
	head.content_length = sizeof(param) + sizeof(osd_region);
	
	struct iovec iov[3];
	iov[0].iov_base = &head;
	iov[0].iov_len = sizeof(head);
	iov[1].iov_base = &param;
	iov[1].iov_len = sizeof(param);
	iov[2].iov_base = (void *)&osd_region;
	iov[2].iov_len = sizeof(osd_region);

	int32_t nret = writev(fd_codec, iov, 3);
	if(nret < 0) {
		LOG_ERROR("Write the iovec failed, errno: %d, errmsg: %s", errno, strerror(errno));
		return errno;
	}

	iov[1].iov_base = &jpeg_frame;
	iov[1].iov_len = sizeof(jpeg_frame);
	nret = readv(fd_codec, iov, 2);
	if(nret < 0) {
		LOG_ERROR("Read the jpeg frame failed, errno: %d, errmsg: %s", errno, strerror(errno));
		return errno;
	}

	if(head.head_length != sizeof(head) || head.content_length != sizeof(jpeg_frame)) {
		LOG_ERROR("Translate the Yuv frame to jpeg frame failed");
		return EIO;
	}
	LOG_DEBUG("Translate the Yuv frame to jpeg frame successfully");
	return OK;
}


int32_t SaveJpeg(const sdc_jpeg_frame_s &jpeg_frame, const string &jpegPath)
{
	int32_t fd = -1;
	if ((fd=open(jpegPath.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0) {
		LOG_ERROR("Open the jpeg file failed, file_path: %s, errno: %d, errmsg: %s", jpegPath.c_str(), errno, strerror(errno));
		return ERR;
	}

	ssize_t writeLen= 0;
	if ((writeLen=write(fd, (void*)jpeg_frame.addr_virt, jpeg_frame.size)) < 0) {
		LOG_ERROR("Write the jpeg file failed, errno: %d, errmsg: %s", errno, strerror(errno));
	} else if(writeLen != (ssize_t)jpeg_frame.size) {
		LOG_ERROR("Write the jpeg file truncted, write_size: %ld, real_size: %u", writeLen, jpeg_frame.size);
	}else {
		LOG_DEBUG("Save the jpeg file successfully, file_path: %s, real_size: %u", jpegPath.c_str(), jpeg_frame.size);
	}
	close(fd);

	return OK;
}


int32_t SDC_SaveJpeg(int fd_codec,const sdc_yuv_frame_s &yuv_frame, const string &jpegPath)
{
    sdc_osd_region_s osd_region;
    memset(&osd_region, /*sizeof(osd_region),*/ 0, sizeof(osd_region));
	
	// 如果需要添加1个OSD，请初始化InitOsdRegion()，否在注释OSD
	// (void) InitOsdRegion(yuv_frame, osd_region);

	// 将Yuv转换成图片帧
    sdc_jpeg_frame_s jpeg_frame;
	sdc_region_s cropregion={0,0,yuv_frame.width,yuv_frame.height};
	if (SDC_Yuv2Jpeg(fd_codec,yuv_frame, osd_region, jpeg_frame,cropregion) != OK) {
		LOG_ERROR("Translate Yuv to Jpeg failed");
		return ERR;
	}
	LOG_DEBUG("Translate Yuv to Jpeg successfully");

	// 用于测试，保存Jpeg图片帧至本地，如果客户将jpeg_frame上传至平台，不需要保存本地,
	// 只需要将jpeg_frame.addr_virt, jpeg_frame.size的内容封装成TLV数据内容，并上报至平台；
	(void) SaveJpeg(jpeg_frame, jpegPath);

	// 释放Jpeg图片帧
	(void) SDC_FreeJpeg(fd_codec,jpeg_frame);

	return OK;
}





int32_t SDC_FreeJpeg(int fd_codec,sdc_jpeg_frame_s &jpeg_frame)
{
    sdc_common_head_s head;
    memset(&head, /*sizeof(head),*/ 0, sizeof(head));
	head.version = SDC_VERSION; // 0x5331
	head.url = SDC_URL_ENCODED_JPEG; // 0x00
	head.method = SDC_METHOD_DELETE;
	head.head_length = sizeof(head);
	head.content_length = sizeof(jpeg_frame);

	struct iovec iov[2];
	iov[0].iov_base = (void *)&head;
	iov[0].iov_len = sizeof(head);
	iov[1].iov_base = (void *)&jpeg_frame;
	iov[1].iov_len = sizeof(jpeg_frame);
	
	int32_t nret = writev(fd_codec, iov, 2);
	if (nret < 0) {
		LOG_ERROR("Write the iovec failed, errno: %d, errmsg: %s", errno, strerror(errno));
	}
	LOG_DEBUG("Write the iovec successfully");

	return OK;
}


int32_t SDC_FreeYuv(int fd_codec,sdc_yuv_frame_s &frame)
{
    uint8_t buf[1024] = {0};
    sdc_common_head_s *head = (sdc_common_head_s *)buf;
    head->version = SDC_VERSION;
    head->url = SDC_URL_COMBINED_IMAGE;
    head->method = SDC_METHOD_DELETE;
    head->head_length = sizeof(*head);
    head->content_length = sizeof(frame);
    memcpy_s(&buf[head->head_length], sizeof(frame), &frame, sizeof(frame));

    int32_t nret = write(fd_codec, buf, head->head_length + head->content_length);
    if (nret < 0) {
        LOG_ERROR("writev fail combine image,response:%d,url:%d,code:%d,method:%d\n", head->response, head->url, head->code, head->method);
    }
    memset_s(&frame, sizeof(frame), 0, sizeof(frame));

    return OK;
}

