
#include "IveUtils.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <securec.h>
#include "sdc_service.h"

// extern int fd_codec;

IveUtils::IveUtils(void) : m_fdCodec(-1)
{
    LOG_DEBUG("Create the IveUtils Object");
}

IveUtils::~IveUtils(void)
{
    LOG_DEBUG("Destroy the IveUtils Object");
    Close();
}

int32_t IveUtils::Init(int fd_codec)
{
    m_fdCodec = fd_codec;
    if(m_fdCodec < 0) {
        LOG_ERROR("Open codec.iaas.sdc fail, errno: %d, errmsg: %s", errno, strerror(errno));
        return ERR;
    }
    LOG_DEBUG("Init the IveUtils successfully, m_fdCodec: %d", m_fdCodec);

    return OK;
}

void IveUtils::Close(void)
{
    LOG_DEBUG("Close the file handles of IVE Manager");   
    if(m_fdCodec != -1) {
        close(m_fdCodec);
        m_fdCodec = -1;
    }
}

int32_t IveUtils::FreeYuvFrame(sdc_yuv_frame_s &frame) const
{
    uint8_t buf[1024] = {0};
    sdc_common_head_s *head = (sdc_common_head_s *)buf;
    head->version = SDC_VERSION;
    head->url = SDC_URL_COMBINED_IMAGE;
    head->method = SDC_METHOD_DELETE;
    head->head_length = sizeof(*head);
    head->content_length = sizeof(frame);
    memcpy_s(&buf[head->head_length], sizeof(frame), &frame, sizeof(frame));

    int32_t nret = write(m_fdCodec, buf, head->head_length + head->content_length);
    if (nret < 0) {
        LOG_ERROR("writev fail combine image,response:%d,url:%d,code:%d,method:%d\n", head->response, head->url, head->code, head->method);
    }
    memset_s(&frame, sizeof(frame), 0, sizeof(frame));

    return OK;
}

CropUtils::CropUtils(void) : IveUtils()
{
    LOG_DEBUG("Create the CropUtils Object");
}

CropUtils::~CropUtils(void)
{
    LOG_DEBUG("Destroy the CropUtils Object");
}

int32_t CropUtils::HandleRegion(const sdc_yuv_frame_s &origin, const sdc_region_s &region, sdc_yuv_frame_s &dst) const
{
    memset_s(&dst, sizeof(dst), 0, sizeof(dst));
    return CropRegion(origin, region, dst);
}

int32_t CropUtils::CropRegion(const sdc_yuv_frame_s &origin, const sdc_region_s &region, sdc_yuv_frame_s &dst) const
{
    sdc_combined_yuv_param_s param = { 0 };
    param.width = region.w;
    param.height = region.h;
    param.yuv_cnt = 1;

    sdc_combined_yuv_s combined_yuv = { 0 };
    combined_yuv.origin_region = { region.x, region.y, region.w, region.h };
    combined_yuv.combined_region = { 0, 0, region.w, region.h };
    combined_yuv.frame = origin;

    uint8_t buf[1024] = { 0 };
    sdc_common_head_s* head = (sdc_common_head_s*)buf;
	head->version = SDC_VERSION;
	head->url = SDC_URL_COMBINED_IMAGE;
	head->method = SDC_METHOD_CREATE;
	head->head_length = sizeof(*head);
	head->content_length = sizeof(param) + sizeof(combined_yuv);
 
    struct iovec iov[3] = {
        {.iov_base = head, .iov_len = sizeof(*head)},
        {.iov_base = &param, .iov_len = sizeof(param)},
        {.iov_base = &combined_yuv, .iov_len = sizeof(combined_yuv)}
    };

	int32_t retn = writev(m_fdCodec, iov, 3);
	if(retn < 0) {
		LOG_ERROR("Write the iovec failed, m_fdCodec: %d, errno: %d, errmsg: %s", m_fdCodec, errno, strerror(errno));
		return ERR;
	}
	
	retn = read(m_fdCodec, buf, sizeof(buf));
    head = (sdc_common_head_s *)buf;
	if(head->code != SDC_CODE_200 || retn < 0) {
		LOG_ERROR("read from codec.iaas.sdc failed, code: %d", head->code);
		return ERR;
	}

    LOG_DEBUG("Crop the given region successfully, m_fdCodec: %d", m_fdCodec);
    memcpy(&dst, (sdc_yuv_frame_s *)(buf + head->head_length), sizeof(dst));

	return OK;
}

ZoomUtils::ZoomUtils(void) : IveUtils()
{
    LOG_DEBUG("Create the ZoomUtils Object");
}

ZoomUtils::~ZoomUtils(void)
{
    LOG_DEBUG("Destroy the ZoomUtils Object");
}

int32_t ZoomUtils::HandleRegion(const sdc_yuv_frame_s &origin, const sdc_region_s &region, sdc_yuv_frame_s &dst) const
{
    memset_s(&dst, sizeof(dst), 0, sizeof(dst));
    return ZoomRegion(origin, region, dst);
}

int32_t ZoomUtils::ZoomRegion(const sdc_yuv_frame_s &origin, const sdc_region_s &region, sdc_yuv_frame_s &dst) const
{
	sdc_combined_yuv_param_s param = { 0 };
    param.width = region.w;
    param.height = region.h;
    param.yuv_cnt = 1;

    sdc_combined_yuv_s combined_yuv = { 0 };
    combined_yuv.origin_region = { 0, 0, origin.width, origin.height };
    combined_yuv.combined_region = { 0, 0, region.w, region.h };
    combined_yuv.frame = origin;

    uint8_t buf[1024] = { 0 };
    sdc_common_head_s* head = (sdc_common_head_s*)buf;
	head->version = SDC_VERSION;
	head->url = SDC_URL_COMBINED_IMAGE;
	head->method = SDC_METHOD_CREATE;
	head->head_length = sizeof(*head);
	head->content_length = sizeof(param) + sizeof(combined_yuv);
 
    struct iovec iov[3] = {
        {.iov_base = head, .iov_len = sizeof(*head)},
        {.iov_base = &param, .iov_len = sizeof(param)},
        {.iov_base = &combined_yuv, .iov_len = sizeof(combined_yuv)}
    };

	int32_t retn = writev(m_fdCodec, iov, 3);
	if(retn < 0) {
		LOG_ERROR("Write the iovec failed, fd_codec: %d, errno: %d, errmsg: %s", m_fdCodec, errno, strerror(errno));
		return ERR;
	}
	
	retn = read(m_fdCodec, buf, sizeof(buf));
    head = (sdc_common_head_s*)buf;
	if(head->code != SDC_CODE_200 || retn < 0) {
		LOG_ERROR("Read from codec.iaas.sdc failed, fd_codec: %d, code: %d", m_fdCodec, head->code);
		return ERR;
	}

    LOG_DEBUG("Zoom the given region successfully, m_fdCodec: %d", m_fdCodec);
    memcpy(&dst, (sdc_yuv_frame_s *)(buf + head->head_length), sizeof(dst));

	return OK;
}

CropAndZoomUtils::CropAndZoomUtils(void) : IveUtils()
{
    LOG_DEBUG("Create the CropAndZoomUtils Object");
}

CropAndZoomUtils::~CropAndZoomUtils(void)
{
    LOG_DEBUG("Destroy the CropAndZoomUtils Object");
}

int32_t CropAndZoomUtils::HandleRegion(const sdc_yuv_frame_s &origin, const sdc_region_s &region, sdc_yuv_frame_s &dst) const
{
    int32_t width = dst.width;
    int32_t height = dst.height;
    memset_s(&dst, sizeof(dst), 0, sizeof(dst));
    dst.width = width;
    dst.height = height;
    return CropAndZoomRegion(origin, region, dst);
}

int32_t CropAndZoomUtils::CropAndZoomRegion(const sdc_yuv_frame_s &origin, const sdc_region_s &region, sdc_yuv_frame_s &dst) const
{
    sdc_combined_yuv_param_s param = { 0 };
    param.width = dst.width;
    param.height = dst.height;
    param.yuv_cnt = 1;

    sdc_combined_yuv_s combined_yuv = { 0 };
    combined_yuv.origin_region = { region.x, region.y, region.w, region.h };
    combined_yuv.combined_region = { 0, 0, dst.width, dst.height };
    combined_yuv.frame = origin;

    uint8_t buf[1024] = { 0 };
    sdc_common_head_s* head = (sdc_common_head_s*)buf;
	head->version = SDC_VERSION;
	head->url = SDC_URL_COMBINED_IMAGE;
	head->method = SDC_METHOD_CREATE;
	head->head_length = sizeof(*head);
	head->content_length = sizeof(param) + sizeof(combined_yuv);
 
    struct iovec iov[3] = {
        {.iov_base = head, .iov_len = sizeof(*head)},
        {.iov_base = &param, .iov_len = sizeof(param)},
        {.iov_base = &combined_yuv, .iov_len = sizeof(combined_yuv)}
    };

	int32_t retn = writev(m_fdCodec, iov, 3);
	if(retn < 0) {
		LOG_ERROR("Write the iovec failed, m_fdCodec: %d, errno: %d, errmsg: %s", m_fdCodec, errno, strerror(errno));
		return ERR;
	}
	
	retn = read(m_fdCodec, buf, sizeof(buf));
    head = (sdc_common_head_s *)buf;
	if(head->code != SDC_CODE_200 || retn < 0) {
		LOG_ERROR("read from codec.iaas.sdc failed, code: %d", head->code);
		return ERR;
	}

    LOG_DEBUG("Crop the given region successfully, m_fdCodec: %d", m_fdCodec);
    memcpy(&dst, (sdc_yuv_frame_s *)(buf + head->head_length), sizeof(dst));

	return OK;
}
