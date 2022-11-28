#ifndef __SDC_fd_Codec_H__
#define __SDC_fd_Codec_H__
#include <string>

#ifdef __cplusplus
extern "C"{
#endif


#include "sdc.h"


using std::string;

extern int SDC_YuvFrame2Jpeg(int fd_codec,const sdc_yuv_frame_s &yuv_frame, sdc_jpeg_frame_s &jpeg_frame);
extern int SDC_Yuv2Jpeg(int fd_codec,const sdc_yuv_frame_s &yuv_frame, const sdc_osd_region_s &osd_region, sdc_jpeg_frame_s &jpeg_frame,sdc_region_s cropregion);
extern int32_t SDC_SaveJpeg(int fd_codec,const sdc_yuv_frame_s &yuv_frame, const std::string &jpegPath);
extern int32_t SDC_FreeJpeg(int fd_codec,sdc_jpeg_frame_s &jpeg_frame);
extern int32_t SaveJpeg(const sdc_jpeg_frame_s &jpeg_frame, const string &jpegPath);
extern int SDC_YuvFrame2CropJpeg(int fd_codec,const sdc_yuv_frame_s &yuv_frame, sdc_jpeg_frame_s &jpeg_frame,sdc_region_s cropregion);
extern int32_t SDC_FreeYuv(int fd_codec,sdc_yuv_frame_s &frame);
#ifdef __cplusplus
}
#endif



#endif /* __SDC_fd_Codec__ */