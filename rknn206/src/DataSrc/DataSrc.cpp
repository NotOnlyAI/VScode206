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
#include "DataSrc.h"

// std::shared_ptr<DataSrc> CreateDataSrcFromVideoStream(uint32_t yuvChnId)
// {
//     // 从视频码流中读取Yuv数据
//     std::shared_ptr<VideoStream> videoSrc = std::make_shared<VideoStream>();
//     if (videoSrc == nullptr) {
//         LOG_ERROR("Create the data source from video failed, yuv_channel: %d", yuvChnId);
//         return nullptr;
//     }

//     // 订阅Yuv通道
//     if (!videoSrc->SubscYuvChannel(yuvChnId)) {
//         LOG_ERROR("The yuv channel is invalid, yuv_channel: %d", yuvChnId);
//         videoSrc.reset();   // 释放智能指针
//         return nullptr;
//     }
//     return videoSrc;
// }


// std::shared_ptr<DataSrc> CreateDataSrcFromImageFile(const std::string &filePath)
// {
//     // 从Image文件读取Yuv数据
//     std::shared_ptr<ImageFile> imageSrc = std::make_shared<ImageFile>();
//     if (imageSrc == nullptr) {
//         LOG_ERROR("Create the data source from image file failed, file_path: %s", filePath.c_str());
//         return nullptr;
//     }
//     if (!imageSrc->ReadFile(filePath)) {
//         LOG_ERROR("The Read the Image file failed, image_path: %s", filePath.c_str());
//         imageSrc.reset();   // 释放智能指针
//         return nullptr;
//     }
//     return imageSrc;
// }



DataSrc::DataSrc(void)
{
    MNN_PRINT("Construct the DataSrc object");
}

DataSrc::~DataSrc(void)
{
    MNN_PRINT("Distruct the DataSrc object");
}




ImageFile::ImageFile(void) : m_eof(false)
{
    MNN_PRINT("Construct the ImageFile object");
}

ImageFile::~ImageFile(void)
{
    MNN_PRINT("Distruct the ImageFile object");
}

bool ImageFile::ReadFile(const std::string &imgPath)
{
    
   	m_img = cv::imread(imgPath);
    if (m_img.empty()) {
        return false;
    }
    MNN_PRINT("Open the image file successfully, file_name: %s", imgPath.c_str());
    return true;
}

bool ImageFile::GetData(sdc_data_s &data)
{
    
    if (!m_eof) {
        data.pts = 10;   // 默认3帧/秒
        data.frame.cvimage=m_img.clone();
        m_eof = false;
        return true;
    }
    
    return false;
}

void ImageFile::FreeData(sdc_data_s &data)
{
    // 清空YuvData内容
}

