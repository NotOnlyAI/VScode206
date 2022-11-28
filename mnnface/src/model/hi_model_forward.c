
#include <inttypes.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <math.h>
#include "opencv2/opencv.hpp"

#include "hi_model_forward.h"
#include "hi_model_results.h"
#include "sample_comm_nnie.h"
#include "sdc_fd_utils.h"
#include "common_defs.h"
#include "sdc.h"
extern int fd_utils;
extern int fd_algorithm;



using namespace std;

void SDC_Struct2RGB(sdc_yuv_frame_s *pstSdcRGBFrame, VW_YUV_FRAME_S *pstRGBFrameData)
{
    pstRGBFrameData->enPixelFormat = PIXEL_FORMAT_RGB_888;
    pstRGBFrameData->pYuvImgAddr = (HI_U8*)pstSdcRGBFrame->addr_virt;
    pstRGBFrameData->ulPhyAddr[0] = pstSdcRGBFrame->addr_phy;
    pstRGBFrameData->ulPhyAddr[1] = pstSdcRGBFrame->addr_phy + (pstSdcRGBFrame->height * pstSdcRGBFrame->stride);
    pstRGBFrameData->ulPhyAddr[2] = pstSdcRGBFrame->addr_phy + (pstSdcRGBFrame->height * pstSdcRGBFrame->stride)*2;

    pstRGBFrameData->ulVirAddr[0] = pstSdcRGBFrame->addr_virt;
    pstRGBFrameData->ulVirAddr[1] = pstSdcRGBFrame->addr_virt + (pstSdcRGBFrame->height * pstSdcRGBFrame->stride);
    pstRGBFrameData->ulVirAddr[2] = pstSdcRGBFrame->addr_virt + (pstSdcRGBFrame->height * pstSdcRGBFrame->stride)*2;

    pstRGBFrameData->uWidth = pstSdcRGBFrame->width;
    pstRGBFrameData->uHeight = pstSdcRGBFrame->height;
    pstRGBFrameData->uStride[0] = pstSdcRGBFrame->stride;
    pstRGBFrameData->uStride[1] = pstSdcRGBFrame->stride;
    pstRGBFrameData->uStride[2] = pstSdcRGBFrame->stride;
    pstRGBFrameData->uFrmSize = pstSdcRGBFrame->size;
    pstRGBFrameData->uPoolId = pstSdcRGBFrame->cookie[0];
    pstRGBFrameData->uVbBlk = pstSdcRGBFrame->cookie[1];
    return ;    
}



/******************************************************************************
* function : Fill Src Data
******************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_FillSrcData(SAMPLE_SVP_NNIE_CFG_S* pstNnieCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S* pstInputDataIdx)
{
    FILE* fp = NULL;
    HI_U32 i =0, j = 0, n = 0;
    HI_U32 u32Height = 0, u32Width = 0, u32Chn = 0, u32Stride = 0, u32Dim = 0;
    HI_U32 u32VarSize = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8*pu8PicAddr = NULL;
    HI_U32*pu32StepAddr = NULL;
    HI_U32 u32SegIdx = pstInputDataIdx->u32SegIdx;
    HI_U32 u32NodeIdx = pstInputDataIdx->u32NodeIdx;
    HI_U32 u32TotalStepNum = 0;
	HI_U8 *pu8BGR = HI_NULL;
	HI_U8 *pu8YUV = HI_NULL;
	HI_BOOL bRBG2BGR = HI_TRUE;
	
	// RBG => BGR
	#define B_BASE_OFFSET (1*u32Stride*u32Height)
	#define G_BASE_OFFSET (2*u32Stride*u32Height)
	#define R_BASE_OFFSET (0*u32Stride*u32Height)
	HI_U8 *p_B_data = HI_NULL; 
	HI_U8 *p_G_data = HI_NULL;
	HI_U8 *p_R_data = HI_NULL;
			
    /*open file*/
    if (NULL != pstNnieCfg->pszPic)
    {
        fp = fopen(pstNnieCfg->pszPic,"rb");
        SAMPLE_SVP_CHECK_EXPR_RET(NULL == fp,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error, open file failed!\n");
    }
	else if(NULL != pstNnieCfg->pszBGR)
    {
	    pu8BGR = (HI_U8*)pstNnieCfg->pszBGR; 
	    //printf("pu8BGR = %p\n", pu8BGR);	
	}
	else if(NULL != pstNnieCfg->pszYUV)
    {
	    pu8YUV = (HI_U8*)pstNnieCfg->pszYUV; 
	    // printf("pu8YUV = %p\n", pu8YUV);
	}

    /*get data size*/
    if(SVP_BLOB_TYPE_U8 <= pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType &&
        SVP_BLOB_TYPE_YVU422SP >= pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType)
    {
        u32VarSize = sizeof(HI_U8);
    }
    else
    {
        u32VarSize = sizeof(HI_U32);
    }

    /*fill src data*/
    if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType)
    {
        u32Dim = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stSeq.u32Dim;
        u32Stride = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Stride;
        pu32StepAddr = (HI_U32*)(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stSeq.u64VirAddrStep);
        pu8PicAddr = (HI_U8*)(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr);
        for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++)
        {
            for(i = 0;i < *(pu32StepAddr+n); i++)
            {
                s32Ret = fread(pu8PicAddr,u32Dim*u32VarSize,1,fp);
                SAMPLE_SVP_CHECK_EXPR_GOTO(1 != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,Read image file failed!\n");
                pu8PicAddr += u32Stride;
            }
            u32TotalStepNum += *(pu32StepAddr+n);
        }
        SDC_FlushCache(fd_utils,pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64PhyAddr,
            (HI_VOID *) pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr,
            u32TotalStepNum*u32Stride);
    }
    else
    {
        u32Height = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stWhc.u32Height;
        u32Width = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stWhc.u32Width;
        u32Chn = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stWhc.u32Chn;
        u32Stride = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Stride;
        pu8PicAddr = (HI_U8*)(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr);
        if(SVP_BLOB_TYPE_YVU420SP== pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType)
        {
            for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++)
            {
                
                UINT32 uiDataSize = u32Width * u32Height * 3 / 2;
                memset_s(pu8PicAddr, uiDataSize, 0, uiDataSize);
                memcpy_s(pu8PicAddr, uiDataSize,  pu8YUV, uiDataSize);
            }

            // printf("Input Type :SVP_BLOB_TYPE_YVU420SP\n");
        }
        else if(SVP_BLOB_TYPE_YVU422SP== pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType)
        {
            for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++)
            {
                for(i = 0; i < u32Height*2; i++)
                {
                    s32Ret = fread(pu8PicAddr,u32Width*u32VarSize,1,fp);
                    SAMPLE_SVP_CHECK_EXPR_GOTO(1 != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,Read image file failed!\n");
                    pu8PicAddr += u32Stride;
                }
            }
        }
        else
        {
			if(bRBG2BGR) // RBG => BGR
			{
				p_B_data = pu8BGR + B_BASE_OFFSET;
				p_G_data = pu8BGR + G_BASE_OFFSET;
				p_R_data = pu8BGR + R_BASE_OFFSET;
			}
            for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++)
            {
                for(i = 0;i < u32Chn; i++)
                {
                    for(j = 0; j < u32Height; j++)
                    {
						if(HI_NULL != fp)
						{  
							s32Ret = fread(pu8PicAddr,u32Width*u32VarSize,1,fp);
							SAMPLE_SVP_CHECK_EXPR_GOTO(1 != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,Read image file failed!\n");
						}
						else if(HI_NULL != pstNnieCfg->pszBGR)
						{
							//printf("u32Width*u32VarSize = %d\n", u32Width*u32VarSize);
							if(bRBG2BGR) // RBG => BGR
							{
							    if(u32Chn == 0) //copy B
								{
									memcpy_s(pu8PicAddr, u32Width*u32VarSize, p_B_data, u32Width*u32VarSize);  
									p_B_data += u32Stride;
								}
								else if(u32Chn == 1) //copy G
								{
									memcpy_s(pu8PicAddr, u32Width*u32VarSize, p_G_data, u32Width*u32VarSize);  
									p_G_data += u32Stride;
								}
								else // copy R
								{
									memcpy_s(pu8PicAddr, u32Width*u32VarSize, p_R_data, u32Width*u32VarSize);  
									p_R_data += u32Stride;
								}
							}
							else
							{
								memcpy_s(pu8PicAddr, u32Width*u32VarSize, pu8BGR, u32Width*u32VarSize);  
								pu8BGR += u32Stride;
							}
						}
						//pu8BGR += u32Width*u32VarSize;
						//printf("u32Stride = %d\n", u32Stride);
						pu8PicAddr += u32Stride;
                    }
                }
            }
        }
        SDC_FlushCache(fd_utils,pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64PhyAddr,
            (HI_VOID *) pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr,
            pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num*u32Chn*u32Height*u32Stride);
    }
    if(fp != HI_NULL)
        fclose(fp);
    return HI_SUCCESS;

FAIL:
    if(fp != HI_NULL)
        fclose(fp);
    fclose(fp);
    return HI_FAILURE;
}








int SDC_Nnie_Forward(sdc_nnie_forward_s *p_sdc_nnie_forward)
{
    int nRet;
    sdc_common_head_s rsp_head;
    sdc_common_head_s head;
    struct iovec iov[2] = 
    {
        [0] = {.iov_base = &head, .iov_len = sizeof(head)},
        [1] = {.iov_base = p_sdc_nnie_forward, .iov_len = sizeof(*p_sdc_nnie_forward)}
    };
    // fill head struct 
    memset(&head, 0, sizeof(head));
    head.version = SDC_VERSION;
    head.url = SDC_URL_NNIE_FORWARD;
    head.method = SDC_METHOD_GET;
    head.head_length = sizeof(head);
    head.content_length = sizeof(*p_sdc_nnie_forward);

    // write request
    nRet = writev(fd_algorithm, iov, sizeof(iov)/sizeof(iov[0]));
    if (nRet < 0)
    {
        fprintf(stdout,"Error:failed to write info to NNIE Forward,nRet:%d!\n",nRet);
        return ERR;
    }
    // read response
    iov[0].iov_base = &rsp_head;
    iov[0].iov_len = sizeof(rsp_head);
    nRet = readv(fd_algorithm, iov, 1);
    if (rsp_head.code != SDC_CODE_200 || nRet < 0)
    {
        fprintf(stdout,"Error:failed to read info from NNIE Forward,nRet:%d,rsp_head.code:%d!\n",
            nRet, rsp_head.code);
        return ERR;
    } 
    return OK;
}



/******************************************************************************
* function : NNIE Forward
******************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_Forward(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S* pstInputDataIdx,
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S* pstProcSegIdx,HI_BOOL bInstant)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0, j = 0;
    //HI_BOOL bFinish = HI_FALSE;
    //SVP_NNIE_HANDLE hSvpNnieHandle = 0;
    // HI_U32 u32TotalStepNum = 0;
    sdc_nnie_forward_s sdc_nnie_forward;

	#if 0
    SDC_FlushCache(pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u64PhyAddr,
        (HI_VOID *) pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u64VirAddr,
        pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u32Size);
	#endif

    /*set input blob according to node name*/
    if(pstInputDataIdx->u32SegIdx != pstProcSegIdx->u32SegIdx)
    {
        for(i = 0; i < pstNnieParam->pstModel->astSeg[pstProcSegIdx->u32SegIdx].u16SrcNum; i++)
        {
            for(j = 0; j < pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].u16DstNum; j++)
            {
                if(0 == strncmp(pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].astDstNode[j].szName,
                    pstNnieParam->pstModel->astSeg[pstProcSegIdx->u32SegIdx].astSrcNode[i].szName,
                    SVP_NNIE_NODE_NAME_LEN))
                {
                    pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc[i] =
                        pstNnieParam->astSegData[pstInputDataIdx->u32SegIdx].astDst[j];
                    break;
                }
            }
            SAMPLE_SVP_CHECK_EXPR_RET((j == pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].u16DstNum),
                HI_FAILURE,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,can't find %d-th seg's %d-th src blob!\n",
                pstProcSegIdx->u32SegIdx,i);
        }
    }

    /*NNIE_Forward*/


    
    memcpy(&sdc_nnie_forward.model, pstNnieParam->pstModel,  sizeof(SVP_NNIE_MODEL_S));
    sdc_nnie_forward.forward_ctrl.max_batch_num = 1;
	sdc_nnie_forward.forward_ctrl.max_bbox_num = 0;/*此处需要根据算法模型的ROI个数决定，max_bbox_num = max_roi_num(ROI个数)*/
    sdc_nnie_forward.forward_ctrl.netseg_id = pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].u32NetSegId;
    

    memcpy(sdc_nnie_forward.astSrc, pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc,  16*sizeof(SVP_DST_BLOB_S));
    memcpy(sdc_nnie_forward.astDst, pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst,  16*sizeof(SVP_DST_BLOB_S));


/*目前NNIE的服务化接口有一个BUG，最大max_bbox_num有限制，最新版本会修复使用服务化接口*/
#if 1
    s32Ret = SDC_Nnie_Forward(&sdc_nnie_forward);
    if(s32Ret != OK) 
    {   
        // printf("%s  %d!\n", __FILE__,__LINE__);
		fprintf(stderr, "Err in SDC_Nnie_Forward, s32Ret: %d\n", s32Ret);
		return ERR;
	}
    // printf("%s  %d!\n", __FILE__,__LINE__);
#else
    
    /*NNIE_Forward*/
    s32Ret = HI_MPI_SVP_NNIE_Forward(&hSvpNnieHandle,
        pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc,
        pstNnieParam->pstModel, pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst,
        &pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx], bInstant);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error,HI_MPI_SVP_NNIE_Forward failed!\n");

    if(bInstant)
    {
        /*Wait NNIE finish*/
        while(HI_ERR_SVP_NNIE_QUERY_TIMEOUT == (s32Ret = HI_MPI_SVP_NNIE_Query(pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].enNnieId,
            hSvpNnieHandle, &bFinish, HI_TRUE)))
        {
            usleep(100);
            SAMPLE_SVP_TRACE(SAMPLE_SVP_ERR_LEVEL_INFO,
                "HI_MPI_SVP_NNIE_Query Query timeout!\n");
        }
    }


    //bFinish = HI_FALSE;
    for(i = 0; i < pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].u32DstNum; i++)
    {
        if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].enType)
        {
            for(j = 0; j < pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num; j++)
            {
                u32TotalStepNum += *((HI_U32*)(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stSeq.u64VirAddrStep)+j);
            }
            SDC_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                (HI_VOID *) pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr,
                u32TotalStepNum*pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);

        }
        else
        {

            SDC_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                (HI_VOID *) pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr,
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Chn*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Height*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);
        }
    }

#endif

    return s32Ret;
}





int SDC_SVP_ForwardYUV(HI_CHAR *pcSrcYUV, SAMPLE_SVP_NNIE_CFG_S* pstNnieCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam,SDC_SSD_RESULT_S *pstResult,SDC_SSD_OBJECT_INFO_S *pstObject)
{
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};

    /*Set configuration parameter*/
    pstNnieCfg->pszPic = HI_NULL;
    pstNnieCfg->pszYUV = (HI_U8 *)pcSrcYUV;
    pstNnieCfg->pszBGR = HI_NULL;


    // cv::Mat image(cv::Size(112, 112), CV_8UC3);
	// image.data =(HI_U8*)pstNnieCfg->pszYUV;
    // cv::imwrite("ForwardYUV1.jpg",image);


    /*Fill src data*/
  //  SAMPLE_SVP_TRACE_INFO("Ssd start!\n");

    struct timespec time1 = {0, 0};
    struct timespec time2 = {0, 0};
    struct timespec time3 = {0, 0};
    struct timespec time4 = {0, 0};

    clock_gettime(CLOCK_BOOTTIME, &time1);
    stInputDataIdx.u32SegIdx = 0;
    stInputDataIdx.u32NodeIdx = 0;
    s32Ret = SAMPLE_SVP_NNIE_FillSrcData(pstNnieCfg,pstNnieParam,&stInputDataIdx);
    clock_gettime(CLOCK_BOOTTIME, &time2);
    LOG_DEBUG("FillSrcData_time: %03lu ms", (time2.tv_sec - time1.tv_sec) *1000 + (time2.tv_nsec - time1.tv_nsec)/1000000); 

    stProcSegIdx.u32SegIdx = 0;
    s32Ret = SAMPLE_SVP_NNIE_Forward(pstNnieParam,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
    clock_gettime(CLOCK_BOOTTIME, &time3);
    LOG_DEBUG("Forward_time: %03lu ms", (time3.tv_sec - time2.tv_sec) *1000 + (time3.tv_nsec - time2.tv_nsec)/1000000); 

    s32Ret = SAMPLE_SVP_NNIE_GetResult(pstNnieParam,pstSoftwareParam,pstResult,pstObject);
    clock_gettime(CLOCK_BOOTTIME, &time4);
    LOG_DEBUG("GetResult_time: %03lu ms", (time4.tv_sec - time3.tv_sec) *1000 + (time4.tv_nsec - time3.tv_nsec)/1000000); 
    LOG_DEBUG("\n"); 
	return s32Ret;
}


int SDC_FEATURE_ForwardYUV(HI_CHAR *pcSrcYUV, SAMPLE_SVP_NNIE_CFG_S* pstNnieCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam,SDC_FEATURE_INFO_S *pstFeatureInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};

    /*Set configuration parameter*/
    pstNnieCfg->pszPic = HI_NULL;
    pstNnieCfg->pszYUV = (HI_U8 *)pcSrcYUV;
    pstNnieCfg->pszBGR = HI_NULL;

    /*Fill src data*/
  //  SAMPLE_SVP_TRACE_INFO("Ssd start!\n");

    struct timespec time1 = {0, 0};
    struct timespec time2 = {0, 0};
    struct timespec time3 = {0, 0};
    struct timespec time4 = {0, 0};

    clock_gettime(CLOCK_BOOTTIME, &time1);
    stInputDataIdx.u32SegIdx = 0;
    stInputDataIdx.u32NodeIdx = 0;


    s32Ret = SAMPLE_SVP_NNIE_FillSrcData(pstNnieCfg,pstNnieParam,&stInputDataIdx);
    clock_gettime(CLOCK_BOOTTIME, &time2);
    LOG_DEBUG("FillSrcData_time: %03lu ms", (time2.tv_sec - time1.tv_sec) *1000 + (time2.tv_nsec - time1.tv_nsec)/1000000); 

    stProcSegIdx.u32SegIdx = 0;
    s32Ret = SAMPLE_SVP_NNIE_Forward(pstNnieParam,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
    clock_gettime(CLOCK_BOOTTIME, &time3);
    LOG_DEBUG("Forward_time: %03lu ms", (time3.tv_sec - time2.tv_sec) *1000 + (time3.tv_nsec - time2.tv_nsec)/1000000); 

    s32Ret = SAMPLE_FEATURE_NNIE_GetResult(pstNnieParam,pstSoftwareParam,pstFeatureInfo);
    clock_gettime(CLOCK_BOOTTIME, &time4);
    LOG_DEBUG("GetResult_time: %03lu ms", (time4.tv_sec - time3.tv_sec) *1000 + (time4.tv_nsec - time3.tv_nsec)/1000000); 
    LOG_DEBUG("\n"); 
	return s32Ret;
}

