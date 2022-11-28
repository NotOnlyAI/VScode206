#ifndef __Hi_model_init_H__
#define __Hi_model_init_H__


#ifdef __cplusplus
extern "C"{
#endif

#include "sdc.h"
#include "sample_comm_nnie.h"

extern HI_S32 SAMPLE_SVP_NNIE_FillForwardInfo(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam);
extern HI_S32 SAMPLE_SVP_NNIE_ParamInit(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam);
extern HI_S32 SDC_NNIE_ParamInit(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam);
extern HI_S32 SAMPLE_SVP_NNIE_HardWare_and_SoftWare_ParamInit(SAMPLE_SVP_NNIE_CFG_S* pstCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftWareParam);

extern HI_U32 SAMPLE_SVP_NNIE_GetResultTmpBuf(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,
    SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam);
extern HI_S32 SAMPLE_SVP_NNIE_SoftwareInit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
    SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam);


#ifdef __cplusplus
}
#endif

#endif /* __Hi_model_init_H__ */