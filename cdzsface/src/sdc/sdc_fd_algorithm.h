#ifndef __SDC_fd_algorithm_H__
#define __SDC_fd_algorithm_H__


#ifdef __cplusplus
extern "C"{
#endif

#include "sdc.h"
#include "sample_comm_nnie.h"

extern int SDC_ModelDecript(sdc_mmz_alloc_s *pstMmzAddr);
extern int SDC_LoadModel(int fd_algorithm,int fd_utils,unsigned int uiLoadMode, char *pucModelFileName, SVP_NNIE_MODEL_S *pstModel,SAMPLE_SVP_NNIE_MODEL_S *ps_stModel);
extern int SDC_TransYUV2RGB(int fd_algorithm, sdc_yuv_frame_s *yuv, sdc_yuv_frame_s *rgb);
extern int SDC_UnLoadModel(int fd_algorithm,SVP_NNIE_MODEL_S *pstModel);
extern int SDC_TransYUV2RGBRelease(int fd_algorithm, sdc_yuv_frame_s *rgb);
#ifdef __cplusplus
}
#endif

#endif /* __SDC_fd_algorithm_H__ */