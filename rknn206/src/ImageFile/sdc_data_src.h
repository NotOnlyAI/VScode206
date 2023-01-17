
#ifndef __SDC_DATA_SRC_H__
#define __SDC_DATA_SRC_H__

#include <memory>




// 创建数据源（VideoStream, ImageFile）
class DataSrc;
std::shared_ptr<DataSrc> CreateDataSrcFromVideoStream(uint32_t yuvChnId);           // 从Yuv通道获取YuvFrame
std::shared_ptr<DataSrc> CreateDataSrcFromImageFile(const std::string &filePath);

class DataSrc
{
public:
    DataSrc(void);
    virtual ~DataSrc(void) = 0;
    virtual bool GetYuvData(sdc_yuv_data_s &data) = 0;
    virtual void FreeYuvData(sdc_yuv_data_s &data) = 0;

private:
    DataSrc(const DataSrc&);
    DataSrc& operator=(const DataSrc&);
    DataSrc(DataSrc&&);
    DataSrc& operator=(DataSrc&&);
};

class VideoStream : public DataSrc
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
class ImageFile : public DataSrc
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