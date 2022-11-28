/******************************************************************************

                  版权所有 (C), 2019-2029, SDC OS 开源软件小组所有

 ******************************************************************************
  文 件 名   : sdc_data_src.h
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
#ifndef __SDC_DATA_SRC_H__
#define __SDC_DATA_SRC_H__

#include <memory>
#include "sdc.h"



// 创建数据源（VideoStream, ImageFile）
class IYuvDataSrc;
std::shared_ptr<IYuvDataSrc> CreateDataSrcFromVideoStream(uint32_t yuvChnId);           // 从Yuv通道获取YuvFrame
std::shared_ptr<IYuvDataSrc> CreateDataSrcFromImageFile(const std::string &filePath);

class IYuvDataSrc
{
public:
    IYuvDataSrc(void);
    virtual ~IYuvDataSrc(void) = 0;
    virtual bool GetYuvData(sdc_yuv_data_s &data) = 0;
    virtual void FreeYuvData(sdc_yuv_data_s &data) = 0;

private:
    IYuvDataSrc(const IYuvDataSrc&);
    IYuvDataSrc& operator=(const IYuvDataSrc&);
    IYuvDataSrc(IYuvDataSrc&&);
    IYuvDataSrc& operator=(IYuvDataSrc&&);
};

class VideoStream : public IYuvDataSrc
{
public:
    VideoStream(void);
    virtual ~VideoStream(void);
    bool SubscYuvChannel(uint32_t yuvChnId);
    virtual bool GetYuvData(sdc_yuv_data_s &data);
    virtual void FreeYuvData(sdc_yuv_data_s &data);

private:
    static const int MAX_CACHED_COUNT = 10; // 最大缓存YUV帧数量为10(SDC_APP开发指南默认填10)
    int32_t m_fd;
};


// 针对于JPEG图片
class ImageFile : public IYuvDataSrc
{
public:
    ImageFile(void);
    virtual ~ImageFile(void);
    bool ReadFile(const std::string &filePath);
    virtual bool GetYuvData(sdc_yuv_data_s &data);
    virtual void FreeYuvData(sdc_yuv_data_s &data);

private:
    uint8_t *m_arr;     // YUV帧数据地址
    uint32_t m_size;     // YUV帧的大小
    int32_t m_width;    // YUV帧的宽
    int32_t m_height;   // YUV帧的高
    bool    m_eof;      // 视频帧是否读完
};



#endif /* __SDC_DATA_SRC_H__ */