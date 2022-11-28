#ifndef __MULTI_OSD_H__
#define __MULTI_OSD_H__

#include <string>
#include <stdint.h>
#include <stdio.h>
#include "sdc.h"



using std::string;
class JpegFile
{
public:
    JpegFile(void);
    virtual ~JpegFile(void);
    int32_t Init(void);
    int32_t SaveJpeg(const sdc_yuv_frame_s &yuv_frame, const string &jpegPath) const;
    int32_t SaveRGB2Jpeg(const sdc_yuv_frame_s &yuv_frame, const string &jpegPath) const;

protected:
    int32_t m_codecFd;  // 图片解码文件描述符

private:
    int32_t InitOsdRegion(const sdc_yuv_frame_s &yuv_frame, sdc_osd_region_s &osd_region) const;
    int32_t Yuv2Jpeg(const sdc_yuv_frame_s &yuv_frame, const sdc_osd_region_s &osd_region, sdc_jpeg_frame_s &jpeg_frame) const;
    int32_t RGB2Jpeg(const sdc_yuv_frame_s &yuv_frame, const sdc_osd_region_s &osd_region, sdc_jpeg_frame_s &jpeg_frame) const;
    int32_t FreeJpeg(sdc_jpeg_frame_s &jpeg_frame) const;
    int32_t SaveJpeg(const sdc_jpeg_frame_s &jpeg_frame, const string &jpegPath) const;
};

#endif
