#ifndef __SDC_FD_VEDIO_H__
#define __SDC_FD_VEDIO_H__


#ifdef __cplusplus
extern "C"{
#endif


#include "sdc.h"


extern int SDC_YuvChnAttrGetIdleYuvChn(int fd_video, unsigned int *puiChnId);
extern int SDC_YuvChnAttrSetDefault(int fd_video, int uiYuvChnId);
extern int SDC_YuvChnAttrSet(int fd_video, int uiYuvChnId,unsigned int w, unsigned int h,unsigned int fps);
extern int SDC_YuvChnAttrGet(int fd_video);


extern int SDC_YuvDataReq(int fd_video, int extendheadflag, unsigned int uiChnId, unsigned int uiMaxUsedBufNum);
extern int32_t SDC_GetYuvData(int fd_video,sdc_yuv_data_s &yuv_data);
extern void SDC_YuvDataFree(int fd_video,  sdc_yuv_data_s *yuv_data);
extern void SDC_DisplayYuvData(sdc_yuv_data_s* yuv_data);
extern void SDC_DisplayYuvFrame(sdc_yuv_frame_s* yuv_frame);
extern void SDC_DisplayJpegFrame(sdc_jpeg_frame_s *jpeg_frame);
#ifdef __cplusplus
}
#endif

#endif /* __SDC_FD_VEDIO_H__ */