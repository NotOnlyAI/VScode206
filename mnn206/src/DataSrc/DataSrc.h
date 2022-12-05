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
#ifndef __SDC_DATA_SRC1_H__
#define __SDC_DATA_SRC1_H__

#include <memory>
#include <opencv2/opencv.hpp>
#include "MNN/MNNDefine.h"

typedef struct sdc_frame_stru
{
    cv::Mat cvimage;
}sdc_frame_s;

typedef struct sdc_data_stru
{
	uint64_t pts;
	sdc_frame_s frame;
}sdc_data_s;


// 创建数据源（VideoStream, ImageFile）
class DataSrc;
std::shared_ptr<DataSrc> CreateDataSrcFromVideoStream(uint32_t yuvChnId);           
std::shared_ptr<DataSrc> CreateDataSrcFromImageFile(const std::string &filePath);

class DataSrc
{
public:
    DataSrc(void);
    virtual ~DataSrc(void) = 0;
    virtual bool GetBgrData(sdc_data_s &data) = 0;
    virtual void FreeBgrData(sdc_data_s &data) = 0;

private:
    DataSrc(const DataSrc&);
    DataSrc& operator=(const DataSrc&);
    DataSrc(DataSrc&&);
    DataSrc& operator=(DataSrc&&);
};


// 针对于JPEG图片
class ImageFile : public DataSrc
{
public:
    ImageFile(void);
    virtual ~ImageFile(void);
    bool ReadFile(const std::string &filePath);
    virtual bool GetData(sdc_data_s &data);
    virtual void FreeData(sdc_data_s &data);

private:
    bool    m_eof;      // 视频帧是否读完
    cv::Mat m_img;
};



#endif /* __SDC_DATA_SRC_H__ */