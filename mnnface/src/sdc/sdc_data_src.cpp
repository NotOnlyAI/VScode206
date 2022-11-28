/******************************************************************************

                  版权所有 (C), 2019-2029, SDC OS 开源软件小组所有

 ******************************************************************************
  文 件 名   : sdc_data_src.cpp
  版 本 号   : 初稿
  作    者   : athina
  生成日期   : 2020年7月27日
  最近修改   :
  功能描述   : 数据源文件
  函数列表   :
  修改历史   :
  1.日    期   : 2020年7月4日
    作    者   : athina
    修改内容   : 创建文件

******************************************************************************/

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "common_defs.h"

#include "sdc_data_src.h"
#include "sdc_opencv_api.h"
#include "sdc_fd_video.h"

extern int fd_video;
extern int fd_utils;

std::shared_ptr<IYuvDataSrc> CreateDataSrcFromVideoStream(uint32_t yuvChnId)
{
    // 从视频码流中读取Yuv数据
    std::shared_ptr<VideoStream> videoSrc = std::make_shared<VideoStream>();
    if (videoSrc == nullptr) {
        LOG_ERROR("Create the data source from video failed, yuv_channel: %d", yuvChnId);
        return nullptr;
    }

    // 订阅Yuv通道
    if (!videoSrc->SubscYuvChannel(yuvChnId)) {
        LOG_ERROR("The yuv channel is invalid, yuv_channel: %d", yuvChnId);
        videoSrc.reset();   // 释放智能指针
        return nullptr;
    }
    return videoSrc;
}


std::shared_ptr<IYuvDataSrc> CreateDataSrcFromImageFile(const std::string &filePath)
{
    // 从Image文件读取Yuv数据
    std::shared_ptr<ImageFile> imageSrc = std::make_shared<ImageFile>();
    if (imageSrc == nullptr) {
        LOG_ERROR("Create the data source from image file failed, file_path: %s", filePath.c_str());
        return nullptr;
    }
    if (!imageSrc->ReadFile(filePath)) {
        LOG_ERROR("The Read the Image file failed, image_path: %s", filePath.c_str());
        imageSrc.reset();   // 释放智能指针
        return nullptr;
    }
    return imageSrc;
}



IYuvDataSrc::IYuvDataSrc(void)
{
    LOG_DEBUG("Construct the IYuvDataSrc object");
}

IYuvDataSrc::~IYuvDataSrc(void)
{
    LOG_DEBUG("Distruct the IYuvDataSrc object");
}

VideoStream::VideoStream(void) : IYuvDataSrc(), m_fd(-1)
{
    LOG_DEBUG("Construct the VideoStream object");
}

VideoStream::~VideoStream(void)
{
    LOG_DEBUG("Distruct the VideoStream object");
}

bool VideoStream::SubscYuvChannel(uint32_t yuvChnId)
{
    // 将全局变量fd_video赋值个m_fd
    m_fd = fd_video;

    // 订阅Yuv数据
    if (SDC_YuvDataReq(m_fd, 2, yuvChnId, VideoStream::MAX_CACHED_COUNT) != OK) {
        LOG_ERROR("Subscribe the Yuv data failed, channel_id: %u", yuvChnId);
        return false;
    }

    LOG_DEBUG("Subscribe the Yuv data successfully, channel_id: %u", yuvChnId);
    return true;
}

bool VideoStream::GetYuvData(sdc_yuv_data_s &data)
{
    char cMsgReadBuf[2048];
    sdc_common_head_s *pstSdcMsgHead = (sdc_common_head_s *)cMsgReadBuf;
    if (read(m_fd, (void *)cMsgReadBuf, sizeof(cMsgReadBuf)) < 0) {
        LOG_ERROR("Read the Yuv frame from OS failed, response:%d,url:%d,code:%d, method:%d, errno: %d, errmsg: %s",
            pstSdcMsgHead->response, pstSdcMsgHead->url, pstSdcMsgHead->code, pstSdcMsgHead->method, errno, strerror(errno));
        return false;
    }

    if (pstSdcMsgHead->url == SDC_URL_YUV_DATA) {
        if (pstSdcMsgHead->content_length != 0) {
            (void)memcpy_s(&data, sizeof(data), cMsgReadBuf + pstSdcMsgHead->head_length, sizeof(data));
            return true;
        }
    }
    return false;
}

void VideoStream::FreeYuvData(sdc_yuv_data_s &data)
{
    if (m_fd != -1) {
        SDC_YuvDataFree(m_fd, &data);
        memset_s(&data, sizeof(data), 0, sizeof(data));
    }
}


ImageFile::ImageFile(void) : m_arr(nullptr), m_size(0), m_width(0), m_height(0), m_eof(false)
{
    LOG_DEBUG("Construct the ImageFile object");
}

ImageFile::~ImageFile(void)
{
    if (m_arr != nullptr) {
        free(m_arr);
        m_arr = nullptr;
    }
    LOG_DEBUG("Distruct the ImageFile object");
}

bool ImageFile::ReadFile(const std::string &filePath)
{
    // 将JPEG图片转换YuV数据帧
    if (ProcessImgPath(filePath.c_str(), m_width, m_height, &m_arr) < 0) {
        LOG_ERROR("Open the image file: %s failed", filePath.c_str());
        return false;
    }
    
    m_size = sizeof(uint8_t) * m_width * m_height * 3 / 2;
    LOG_DEBUG("Open the image file successfully, file_name: %s, file_size: %u", filePath.c_str(), m_size);
    return true;
}

bool ImageFile::GetYuvData(sdc_yuv_data_s &data)
{
    // 针对于JPEG图片而言，只读取一帧YUV视频帧进行推理分析
    if (!m_eof) {
        // 将JPEG Image数据，赋值给YuvData
        data.pts = 10;   // 默认3帧/秒
        data.frame.addr_virt = (uintptr_t)m_arr;
        data.frame.size = (uint32_t)m_size;
        data.frame.width = (uint32_t)m_width;
        data.frame.height = (uint32_t)m_height;
        data.frame.stride= (uint32_t)m_width;
        m_eof = true;
        return true;
    }
    
    return false;
}

void ImageFile::FreeYuvData(sdc_yuv_data_s &data)
{
    // 清空YuvData内容
    (void)memset_s(&data, sizeof(data), 0, sizeof(data));
}

