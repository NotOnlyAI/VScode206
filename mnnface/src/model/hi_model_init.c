#include <iostream>
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


#include "hi_model_init.h"
#include "sdc_service.h"
#include "sdc_fd_utils.h"
#include "common_defs.h"

extern int fd_utils;

HI_S32 SAMPLE_SVP_NNIE_FillForwardInfo(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam)
{
	HI_U32 i = 0, j = 0;
	HI_U32 u32Offset = 0;
	HI_U32 u32Num = 0;

	// printf("%s  %d: ModelSegNum=%d!\n", __FILE__,__LINE__,pstNnieParam->pstModel->u32NetSegNum);
	for(i = 0; i < pstNnieParam->pstModel->u32NetSegNum; i++)
	{
        /*fill forwardCtrl info*/
		if(SVP_NNIE_NET_TYPE_ROI == pstNnieParam->pstModel->astSeg[i].enNetType)
		{
			pstNnieParam->astForwardWithBboxCtrl[i].enNnieId = pstNnieCfg->aenNnieCoreId[i];
			pstNnieParam->astForwardWithBboxCtrl[i].u32SrcNum = pstNnieParam->pstModel->astSeg[i].u16SrcNum;
			pstNnieParam->astForwardWithBboxCtrl[i].u32DstNum = pstNnieParam->pstModel->astSeg[i].u16DstNum;
			pstNnieParam->astForwardWithBboxCtrl[i].u32ProposalNum = 1;
			pstNnieParam->astForwardWithBboxCtrl[i].u32NetSegId = i;
			#if 0
			pstNnieParam->astForwardWithBboxCtrl[i].stTmpBuf = pstNnieParam->stTmpBuf;
			pstNnieParam->astForwardWithBboxCtrl[i].stTskBuf.u64PhyAddr= pstNnieParam->stTaskBuf.u64PhyAddr+u32Offset;
			pstNnieParam->astForwardWithBboxCtrl[i].stTskBuf.u64VirAddr= pstNnieParam->stTaskBuf.u64VirAddr+u32Offset;
			pstNnieParam->astForwardWithBboxCtrl[i].stTskBuf.u32Size= pstNnieParam->au32TaskBufSize[i];
			#endif
		}
		else if(SVP_NNIE_NET_TYPE_CNN == pstNnieParam->pstModel->astSeg[i].enNetType ||
            SVP_NNIE_NET_TYPE_RECURRENT== pstNnieParam->pstModel->astSeg[i].enNetType)
		{

			// printf("%s  %d!\n", __FILE__,__LINE__);
			pstNnieParam->astForwardCtrl[i].enNnieId = pstNnieCfg->aenNnieCoreId[i];
			pstNnieParam->astForwardCtrl[i].u32SrcNum = pstNnieParam->pstModel->astSeg[i].u16SrcNum;
			pstNnieParam->astForwardCtrl[i].u32DstNum = pstNnieParam->pstModel->astSeg[i].u16DstNum;
			pstNnieParam->astForwardCtrl[i].u32NetSegId = i;
			#if 0
			pstNnieParam->astForwardCtrl[i].stTmpBuf = pstNnieParam->stTmpBuf;
			pstNnieParam->astForwardCtrl[i].stTskBuf.u64PhyAddr= pstNnieParam->stTaskBuf.u64PhyAddr+u32Offset;
			pstNnieParam->astForwardCtrl[i].stTskBuf.u64VirAddr= pstNnieParam->stTaskBuf.u64VirAddr+u32Offset;
			pstNnieParam->astForwardCtrl[i].stTskBuf.u32Size= pstNnieParam->au32TaskBufSize[i];
			#endif
		}
		u32Offset += pstNnieParam->au32TaskBufSize[i];/*这里赋值要审视下，好像参数没用*/

        /*fill src blob info*/
		for(j = 0; j < pstNnieParam->pstModel->astSeg[i].u16SrcNum; j++)
	    {
            /*Recurrent blob*/
            if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->pstModel->astSeg[i].astSrcNode[j].enType)
            {
                pstNnieParam->astSegData[i].astSrc[j].enType = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].enType;
                pstNnieParam->astSegData[i].astSrc[j].unShape.stSeq.u32Dim = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.u32Dim;
                pstNnieParam->astSegData[i].astSrc[j].u32Num = pstNnieCfg->u32MaxInputNum;
                pstNnieParam->astSegData[i].astSrc[j].unShape.stSeq.u64VirAddrStep = pstNnieCfg->au64StepVirAddr[i*SAMPLE_SVP_NNIE_EACH_SEG_STEP_ADDR_NUM];
            }
            else
            {
    		    pstNnieParam->astSegData[i].astSrc[j].enType = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].enType;
    	        pstNnieParam->astSegData[i].astSrc[j].unShape.stWhc.u32Chn = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.stWhc.u32Chn;
    	        pstNnieParam->astSegData[i].astSrc[j].unShape.stWhc.u32Height = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.stWhc.u32Height;
    	        pstNnieParam->astSegData[i].astSrc[j].unShape.stWhc.u32Width = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.stWhc.u32Width;
    	        pstNnieParam->astSegData[i].astSrc[j].u32Num = pstNnieCfg->u32MaxInputNum;
				// printf("File(%s)  Line(%d): Src enType(%d)\n", __FILE__,__LINE__,pstNnieParam->pstModel->astSeg[i].astSrcNode[j].enType);
				// printf("File(%s)  Line(%d): Src Chn(%d)\n", __FILE__,__LINE__,pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.stWhc.u32Chn);
				// printf("File(%s)  Line(%d): Src Height(%d)\n", __FILE__,__LINE__,pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.stWhc.u32Height);
				// printf("File(%s)  Line(%d): Src Width(%d)\n", __FILE__,__LINE__,pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.stWhc.u32Width);
            }
	    }

        /*fill dst blob info*/
		if(SVP_NNIE_NET_TYPE_ROI == pstNnieParam->pstModel->astSeg[i].enNetType)
		{
			u32Num = pstNnieCfg->u32MaxRoiNum*pstNnieCfg->u32MaxInputNum;
		}
		else
		{
			u32Num = pstNnieCfg->u32MaxInputNum;
		}

		for(j = 0; j < pstNnieParam->pstModel->astSeg[i].u16DstNum; j++)
		{
            if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->pstModel->astSeg[i].astDstNode[j].enType)
            {
    			pstNnieParam->astSegData[i].astDst[j].enType = pstNnieParam->pstModel->astSeg[i].astDstNode[j].enType;
    			pstNnieParam->astSegData[i].astDst[j].unShape.stSeq.u32Dim =
                    pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.u32Dim;
                pstNnieParam->astSegData[i].astDst[j].u32Num = u32Num;
                pstNnieParam->astSegData[i].astDst[j].unShape.stSeq.u64VirAddrStep =
                    pstNnieCfg->au64StepVirAddr[i*SAMPLE_SVP_NNIE_EACH_SEG_STEP_ADDR_NUM+1];
            }
            else
            {
    		    pstNnieParam->astSegData[i].astDst[j].enType = pstNnieParam->pstModel->astSeg[i].astDstNode[j].enType;
    		    pstNnieParam->astSegData[i].astDst[j].unShape.stWhc.u32Chn = pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.stWhc.u32Chn;
    		    pstNnieParam->astSegData[i].astDst[j].unShape.stWhc.u32Height = pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.stWhc.u32Height;
    		    pstNnieParam->astSegData[i].astDst[j].unShape.stWhc.u32Width = pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.stWhc.u32Width;
    		    pstNnieParam->astSegData[i].astDst[j].u32Num = u32Num;
				// printf("File(%s)  Line(%d): Dst enType(%d)\n", __FILE__,__LINE__,pstNnieParam->pstModel->astSeg[i].astDstNode[j].enType);
				// printf("File(%s)  Line(%d): Dst Chn(%d)\n", __FILE__,__LINE__,pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.stWhc.u32Chn);
				// printf("File(%s)  Line(%d): Dst Height(%d)\n", __FILE__,__LINE__,pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.stWhc.u32Height);
				// printf("File(%s)  Line(%d): Dst Width(%d)\n", __FILE__,__LINE__,pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.stWhc.u32Width);
            }
		}
	}
	return HI_SUCCESS;
}


void SAMPLE_SVP_NNIE_GetBlobMemSize(SVP_NNIE_NODE_S astNnieNode[], HI_U32 u32NodeNum,
	HI_U32 u32TotalStep,SVP_BLOB_S astBlob[], HI_U32 u32Align, HI_U32* pu32TotalSize,HI_U32 au32BlobSize[])
{
	HI_U32 i = 0;
	HI_U32 u32Size = 0;
	HI_U32 u32Stride = 0;

	for(i = 0; i < u32NodeNum; i++)
	{
		if(SVP_BLOB_TYPE_S32== astNnieNode[i].enType||SVP_BLOB_TYPE_VEC_S32== astNnieNode[i].enType||
            SVP_BLOB_TYPE_SEQ_S32== astNnieNode[i].enType)
		{
			u32Size = sizeof(HI_U32);
			// printf("File(%s)  Line(%d): u32Size(%d)\n", __FILE__,__LINE__,u32Size);
		}
		else
		{
			u32Size = sizeof(HI_U8);
			// printf("File(%s)  Line(%d): u32Size(%d)\n", __FILE__,__LINE__,u32Size);
		}
        if(SVP_BLOB_TYPE_SEQ_S32 == astNnieNode[i].enType)
        {
            if(SAMPLE_SVP_NNIE_ALIGN_16 == u32Align)
    		{
    			u32Stride = SAMPLE_SVP_NNIE_ALIGN16(astNnieNode[i].unShape.u32Dim*u32Size);
    		}
    		else
    		{
    			u32Stride = SAMPLE_SVP_NNIE_ALIGN32(astNnieNode[i].unShape.u32Dim*u32Size);
    		}
            au32BlobSize[i] = u32TotalStep*u32Stride;
        }
        else
        {
            if(SAMPLE_SVP_NNIE_ALIGN_16 == u32Align)
    		{
    			u32Stride = SAMPLE_SVP_NNIE_ALIGN16(astNnieNode[i].unShape.stWhc.u32Width*u32Size);
				// printf("File(%s)  Line(%d): u32Width(%d)\n", __FILE__,__LINE__,astNnieNode[i].unShape.stWhc.u32Width);
				// printf("File(%s)  Line(%d): u32Stride(%d)\n", __FILE__,__LINE__,u32Stride);
    		}
    		else
    		{
    			u32Stride = SAMPLE_SVP_NNIE_ALIGN32(astNnieNode[i].unShape.stWhc.u32Width*u32Size);
				
				// printf("File(%s)  Line(%d): u32Stride(%d)\n", __FILE__,__LINE__,u32Stride);
    		}
    		au32BlobSize[i] = astBlob[i].u32Num*u32Stride*astNnieNode[i].unShape.stWhc.u32Height*
    			astNnieNode[i].unShape.stWhc.u32Chn;
        }
		*pu32TotalSize += au32BlobSize[i];
	    astBlob[i].u32Stride = u32Stride;
	}
}

HI_S32 SAMPLE_SVP_NNIE_GetTaskAndBlobBufSize(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,HI_U32*pu32TotalTaskBufSize, HI_U32*pu32TmpBufSize,
    SAMPLE_SVP_NNIE_BLOB_SIZE_S astBlobSize[],HI_U32*pu32TotalSize)
{
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 i = 0, j = 0;
    HI_U32 u32TotalStep = 0;

	#if 0
	/*Get each seg's task buf size*/
	s32Ret = HI_MPI_SVP_NNIE_GetTskBufSize(pstNnieCfg->u32MaxInputNum, pstNnieCfg->u32MaxRoiNum,
		pstNnieParam->pstModel, pstNnieParam->au32TaskBufSize,pstNnieParam->pstModel->u32NetSegNum);
	SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error,HI_MPI_SVP_NNIE_GetTaskSize failed!\n");

    /*Get total task buf size*/
	*pu32TotalTaskBufSize = 0;
	for(i = 0; i < pstNnieParam->pstModel->u32NetSegNum; i++)
	{
		*pu32TotalTaskBufSize += pstNnieParam->au32TaskBufSize[i];
	}

	/*Get tmp buf size*/
	*pu32TmpBufSize = pstNnieParam->pstModel->u32TmpBufSize;
	*pu32TotalSize += *pu32TotalTaskBufSize + *pu32TmpBufSize;
	#endif

	/*calculate Blob mem size*/
	for(i = 0; i < pstNnieParam->pstModel->u32NetSegNum; i++)
	{
        if(SVP_NNIE_NET_TYPE_RECURRENT == pstNnieParam->pstModel->astSeg[i].enNetType)
        {
            for(j = 0; j < pstNnieParam->astSegData[i].astSrc[0].u32Num; j++)
            {
                u32TotalStep += *((HI_S32*)pstNnieParam->astSegData[i].astSrc[0].unShape.stSeq.u64VirAddrStep+j);
            }
        }
		/*the first seg's Src Blob mem size, other seg's src blobs from the output blobs of
		those segs before it or from software output results*/
		if(i == 0)
		{
			SAMPLE_SVP_NNIE_GetBlobMemSize(&(pstNnieParam->pstModel->astSeg[i].astSrcNode[0]),
				pstNnieParam->pstModel->astSeg[i].u16SrcNum,u32TotalStep,&(pstNnieParam->astSegData[i].astSrc[0]),
				SAMPLE_SVP_NNIE_ALIGN_16, pu32TotalSize, &(astBlobSize[i].au32SrcSize[0]));
		}

		/*Get each seg's Dst Blob mem size*/
		SAMPLE_SVP_NNIE_GetBlobMemSize(&(pstNnieParam->pstModel->astSeg[i].astDstNode[0]),
			pstNnieParam->pstModel->astSeg[i].u16DstNum,u32TotalStep,&(pstNnieParam->astSegData[i].astDst[0]),
			SAMPLE_SVP_NNIE_ALIGN_16, pu32TotalSize, &(astBlobSize[i].au32DstSize[0]));
	}
	return s32Ret;
}

HI_S32 SAMPLE_SVP_NNIE_ParamInit(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam)
{
	HI_U32 i = 0, j = 0;
	HI_U32 u32TotalSize = 0;
	HI_U32 u32TotalTaskBufSize = 0;
	HI_U32 u32TmpBufSize = 0;
	HI_S32 s32Ret = HI_SUCCESS;
	//HI_U32 u32Offset = 0;
	HI_U64 u64PhyAddr = 0;
	HI_U8 *pu8VirAddr = NULL;
	SAMPLE_SVP_NNIE_BLOB_SIZE_S astBlobSize[SVP_NNIE_MAX_NET_SEG_NUM] = {0};
    sdc_mmz_alloc_s stMemParas;

	/*fill forward info*/
	s32Ret = SAMPLE_SVP_NNIE_FillForwardInfo(pstNnieCfg,pstNnieParam);
	SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error,SAMPLE_SVP_NNIE_FillForwardCtrl failed!\n");


	/*Get taskInfo and Blob mem size*/
	s32Ret = SAMPLE_SVP_NNIE_GetTaskAndBlobBufSize(pstNnieCfg,pstNnieParam,&u32TotalTaskBufSize,
		&u32TmpBufSize,astBlobSize,&u32TotalSize);
	SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error,SAMPLE_SVP_NNIE_GetTaskAndBlobBufSize failed!\n");


	/*Malloc mem*/
	fprintf(stdout,"SDC_MemAlloc u32TotalSize %d", u32TotalSize);
    s32Ret = SDC_MemAlloc(fd_utils,u32TotalSize,1,&stMemParas);
    u64PhyAddr = stMemParas.addr_phy;
    pu8VirAddr = (HI_U8 *)stMemParas.addr_virt;

	fprintf(stdout,"SDC_MemAlloc u32TotalSize %d, size %d\n", u32TotalSize, stMemParas.size);

	
    u32TotalSize = stMemParas.size;
    SAMPLE_SVP_CHECK_EXPR_RET(u32TotalSize != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error,Malloc memory failed,u32TotalSize:%d,s32Ret:%d!\n", u32TotalSize, s32Ret);
	memset(pu8VirAddr, 0, u32TotalSize);
	SDC_FlushCache(fd_utils,u64PhyAddr,(void*)pu8VirAddr,u32TotalSize);


	
	/*fill each blob's mem addr*/
	u64PhyAddr =  u64PhyAddr;//+u32TotalTaskBufSize+u32TmpBufSize;
	pu8VirAddr = pu8VirAddr;//+u32TotalTaskBufSize+u32TmpBufSize;
	for(i = 0; i < pstNnieParam->pstModel->u32NetSegNum; i++)
	{
		/*first seg has src blobs, other seg's src blobs from the output blobs of
		those segs before it or from software output results*/
		if(0 == i)
		{
			for(j = 0; j < pstNnieParam->pstModel->astSeg[i].u16SrcNum; j++)
			{
				if(j!=0)
				{
					u64PhyAddr += astBlobSize[i].au32SrcSize[j-1];
					pu8VirAddr += astBlobSize[i].au32SrcSize[j-1];
				}
				pstNnieParam->astSegData[i].astSrc[j].u64PhyAddr = u64PhyAddr;
				pstNnieParam->astSegData[i].astSrc[j].u64VirAddr = (HI_U64)pu8VirAddr;
			}
			u64PhyAddr += astBlobSize[i].au32SrcSize[j-1];
			pu8VirAddr += astBlobSize[i].au32SrcSize[j-1];
		}

		/*fill the mem addrs of each seg's output blobs*/
		for(j = 0; j < pstNnieParam->pstModel->astSeg[i].u16DstNum; j++)
		{
			if(j!=0)
			{
				u64PhyAddr += astBlobSize[i].au32DstSize[j-1];
				pu8VirAddr += astBlobSize[i].au32DstSize[j-1];
			}
			pstNnieParam->astSegData[i].astDst[j].u64PhyAddr = u64PhyAddr;
			pstNnieParam->astSegData[i].astDst[j].u64VirAddr = (HI_U64)pu8VirAddr;
		}
		u64PhyAddr += astBlobSize[i].au32DstSize[j-1];
		pu8VirAddr += astBlobSize[i].au32DstSize[j-1];
	}
	return HI_SUCCESS;
}


HI_S32 SDC_NNIE_ParamInit(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam)
{
	HI_S32 s32Ret = HI_SUCCESS;
    // fprintf(stdout,"START SDC_NNIE_ParamInit!\n");
    /*check*/
    SAMPLE_SVP_CHECK_EXPR_RET((NULL == pstNnieCfg || NULL == pstNnieParam),HI_ERR_SVP_NNIE_ILLEGAL_PARAM,
        SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,pstNnieCfg and pstNnieParam can't be NULL!\n");
    SAMPLE_SVP_CHECK_EXPR_RET((NULL == pstNnieParam->pstModel),HI_ERR_SVP_NNIE_ILLEGAL_PARAM,
        SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,pstNnieParam->pstModel can't be NULL!\n");

	/*NNIE parameter initialization */
	s32Ret = SAMPLE_SVP_NNIE_ParamInit(pstNnieCfg,pstNnieParam);
	SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error, SAMPLE_SVP_NNIE_ParamInit failed!\n");
    // fprintf(stdout,"END SDC_NNIE_ParamInit!\n");
	return s32Ret;
FAIL:
	// s32Ret = SDC_NNIE_ParamDeinit(pstNnieParam, fd);
	s32Ret = HI_FAILURE;
	SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error, SDC_NNIE_ParamDeinit failed!\n");
	return HI_FAILURE;
}



HI_U32 FCOSFace_GetResultTmpBuf(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,
    SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam)
{


    HI_U32 u32TotalSize = 0;
    HI_U32 u32AssistStackSize = 0;
    HI_U32 u32TotalBboxNum = 0;
    HI_U32 u32TotalBboxSize = 0;
	HI_U32 u32TotalLandmarkSize = 0;

    HI_U32 u32DstBoxBlobSize = 0;
    HI_U32 u32MaxBoxBlobSize = 0;

	HI_U32 u32DstScoreBlobSize = 0;
    HI_U32 u32MaxScoreBlobSize = 0;

	HI_U32 u32DstLandmarkBlobSize = 0;
    HI_U32 u32MaxLandmarkBlobSize = 0;

    HI_U32 i = 0;

    for(i = 0; i < pstSoftwareParam->au32BoxNum; i++)
    {
        u32DstBoxBlobSize = (pstSoftwareParam->au32GridWidth[2*i])*(pstSoftwareParam->au32GridHeight[2*i])*sizeof(HI_U32)*
        SAMPLE_SVP_NNIE_EACH_BBOX_INFER_RESULT_NUM*(pstSoftwareParam->u32BboxNumEachGrid[i]);
        // printf("File(%s)  Line(%d): u32DstBoxBlobSize(%d)\n", __FILE__,__LINE__,u32DstBoxBlobSize);
        if(u32MaxBoxBlobSize < u32DstBoxBlobSize)
        {
            u32MaxBoxBlobSize = u32DstBoxBlobSize;
        }

        u32DstScoreBlobSize = (pstSoftwareParam->au32GridWidth[2*i+1])*(pstSoftwareParam->au32GridHeight[2*i+1])*sizeof(HI_U32)*
            SAMPLE_SVP_NNIE_EACH_CLASS_INFER_RESULT_NUM*(pstSoftwareParam->u32BboxNumEachGrid[i]);
        // printf("File(%s)  Line(%d): u32DstScoreBlobSize(%d)\n", __FILE__,__LINE__,u32DstScoreBlobSize);
        if(u32MaxScoreBlobSize < u32DstScoreBlobSize)
        {
            u32MaxScoreBlobSize = u32DstScoreBlobSize;
        }

    }

    for(i = 0; i < pstSoftwareParam->au32BoxNum; i++)
    {
        u32TotalBboxNum += (pstSoftwareParam->au32GridWidth[2*i])*(pstSoftwareParam->au32GridHeight[2*i])*(pstSoftwareParam->u32BboxNumEachGrid[i]);
        // printf("File(%s)  Line(%d): u32TotalBboxNum(%d)\n", __FILE__,__LINE__,u32TotalBboxNum);
    }

    u32AssistStackSize = u32TotalBboxNum*sizeof(SAMPLE_SVP_NNIE_STACK_S);
    u32TotalBboxSize = u32TotalBboxNum*sizeof(SAMPLE_SVP_NNIE_BBOX_S);
    u32TotalSize += (u32MaxBoxBlobSize+u32MaxScoreBlobSize+u32AssistStackSize+u32TotalBboxSize);

    pstSoftwareParam->u32MaxBoxBlobSize=u32MaxBoxBlobSize;
    pstSoftwareParam->u32MaxScoreBlobSize=u32MaxScoreBlobSize;
    pstSoftwareParam->u32TotalBboxNum=u32TotalBboxNum;
    pstSoftwareParam->u32TotalBboxSize=u32TotalBboxSize;
    return u32TotalSize;
	
}

HI_U32 FCOSFaceLandmark_GetResultTmpBuf(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam)
{

    HI_U32 u32TotalSize = 0;
    HI_U32 u32AssistStackSize = 0;
    HI_U32 u32TotalBboxNum = 0;
    HI_U32 u32TotalBboxSize = 0;
	HI_U32 u32TotalLandmarkSize = 0;

    HI_U32 u32DstBoxBlobSize = 0;
    HI_U32 u32MaxBoxBlobSize = 0;

	HI_U32 u32DstScoreBlobSize = 0;
    HI_U32 u32MaxScoreBlobSize = 0;

	HI_U32 u32DstLandmarkBlobSize = 0;
    HI_U32 u32MaxLandmarkBlobSize = 0;

    HI_U32 i = 0;

    for(i = 0; i < pstSoftwareParam->au32BoxNum; i++)
    {
        u32DstBoxBlobSize = pstSoftwareParam->au32GridWidth[2*i]*pstSoftwareParam->au32GridHeight[2*i]*pstSoftwareParam->au32GridChannel[2*i]*sizeof(HI_U32);
        // printf("File(%s)  Line(%d): u32DstBoxBlobSize(%d)\n", __FILE__,__LINE__,u32DstBoxBlobSize);
        if(u32MaxBoxBlobSize < u32DstBoxBlobSize)
        {
            u32MaxBoxBlobSize = u32DstBoxBlobSize;
        }

        u32DstScoreBlobSize = pstSoftwareParam->au32GridWidth[2*i+1]*pstSoftwareParam->au32GridHeight[2*i+1]*pstSoftwareParam->au32GridChannel[2*i+1]*sizeof(HI_U32);;
        // printf("File(%s)  Line(%d): u32DstScoreBlobSize(%d)\n", __FILE__,__LINE__,u32DstScoreBlobSize);
        if(u32MaxScoreBlobSize < u32DstScoreBlobSize)
        {
            u32MaxScoreBlobSize = u32DstScoreBlobSize;
        }
    }

    for(i = 0; i < pstSoftwareParam->au32BoxNum; i++)
    {
        u32TotalBboxNum += pstSoftwareParam->au32GridWidth[2*i]*pstSoftwareParam->au32GridHeight[2*i];
        // printf("File(%s)  Line(%d): u32TotalBboxNum(%d)\n", __FILE__,__LINE__,u32TotalBboxNum);
    }

    u32AssistStackSize = u32TotalBboxNum*sizeof(SAMPLE_SVP_NNIE_STACK_S);
    u32TotalBboxSize = u32TotalBboxNum*sizeof(SAMPLE_SVP_NNIE_BBOX_S);
    u32TotalSize += (u32MaxBoxBlobSize+u32MaxScoreBlobSize+u32AssistStackSize+u32TotalBboxSize);

    pstSoftwareParam->u32MaxBoxBlobSize=u32MaxBoxBlobSize;
    pstSoftwareParam->u32MaxScoreBlobSize=u32MaxScoreBlobSize;
    pstSoftwareParam->u32TotalBboxNum=u32TotalBboxNum;
    pstSoftwareParam->u32TotalBboxSize=u32TotalBboxSize;
    return u32TotalSize;
	
}

HI_U32 DenseNet_GetResultTmpBuf(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam)
{

    HI_U32 u32TotalSize = 0;
    u32TotalSize = pstSoftwareParam->au32GridWidth[0]*pstSoftwareParam->au32GridHeight[0]*pstSoftwareParam->au32GridChannel[0]*sizeof(HI_U32);
    // printf("File(%s)  Line(%d): u32DstBoxBlobSize(%d)\n", __FILE__,__LINE__,u32DstBoxBlobSize);
    return u32TotalSize;
	
}




HI_U32 FCOSFace_SoftwareInit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftWareParam)
{   

    HI_U32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    HI_U64 u64PhyAddr = 0;
    HI_U8* pu8VirAddr = NULL;
    sdc_mmz_alloc_s stMemParas;

    HI_U32 u32TmpBufTotalSize = 0;
	HI_U32 u32TotalSize = 0;

    pstSoftWareParam->au32NodeNum=10;
    pstSoftWareParam->au32BoxNum=5;
    pstSoftWareParam->au32ScoreNum=5;

    pstSoftWareParam->u32OriImHeight = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Height;
    pstSoftWareParam->u32OriImWidth = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Width;
    printf("inputs=%d\n",i);
    printf("File(%s)  Line(%d): inputs Chn(%d)\n", __FILE__,__LINE__,pstSoftWareParam->au32GridHeight[i]);
    printf("File(%s)  Line(%d): inputs Height(%d)\n", __FILE__,__LINE__,pstSoftWareParam->u32OriImHeight);
    printf("File(%s)  Line(%d): inputs Width(%d)\n", __FILE__,__LINE__,pstSoftWareParam->u32OriImWidth);
    printf("\n");

    for(i = 0; i < pstSoftWareParam->au32NodeNum; i++)
    {
        pstSoftWareParam->au32GridHeight[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Chn;
        pstSoftWareParam->au32GridWidth[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Height;
        pstSoftWareParam->au32GridChannel[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Width;
		printf("output_number_i=%d\n",i);
        printf("File(%s)  Line(%d): output Chn(%d)\n", __FILE__,__LINE__,pstSoftWareParam->au32GridHeight[i]);
        printf("File(%s)  Line(%d): output Height(%d)\n", __FILE__,__LINE__,pstSoftWareParam->au32GridWidth[i]);
        printf("File(%s)  Line(%d): output Width(%d)\n", __FILE__,__LINE__,pstSoftWareParam->au32GridChannel[i]);
        printf("\n");
    }

    pstSoftWareParam->u32BboxNumEachGrid[0] = 1;
	pstSoftWareParam->u32BboxNumEachGrid[1] = 1;
	pstSoftWareParam->u32BboxNumEachGrid[2] = 1;
    pstSoftWareParam->u32BboxNumEachGrid[3] = 1;
	pstSoftWareParam->u32BboxNumEachGrid[4] = 1;

	pstSoftWareParam->af32BoxStep[0] = 8;
    pstSoftWareParam->af32BoxStep[1] = 16;
    pstSoftWareParam->af32BoxStep[2] = 32;
    pstSoftWareParam->af32BoxStep[3] = 64;
    pstSoftWareParam->af32BoxStep[4] = 128;


	pstSoftWareParam->af32BoxMinSize[0][0] = 16.0f;
    pstSoftWareParam->af32BoxMinSize[1][0] = 32.0f;
    pstSoftWareParam->af32BoxMinSize[2][0] = 64.0f;
    pstSoftWareParam->af32BoxMinSize[3][0] = 128.0f;
    pstSoftWareParam->af32BoxMinSize[4][0] = 256.0f;

	pstSoftWareParam->f64NmsThresh=0.5;

	u32TmpBufTotalSize = FCOSFace_GetResultTmpBuf(pstNnieParam,pstSoftWareParam);
	u32TotalSize = u32TmpBufTotalSize;
	// printf("File(%s)  Line(%d): u32TotalSize_NEED(%d)\n", __FILE__,__LINE__,u32TotalSize);
	s32Ret = SDC_MemAlloc(fd_utils,u32TotalSize,1,&stMemParas);
	u64PhyAddr = stMemParas.addr_phy;
    pu8VirAddr = (HI_U8 *)stMemParas.addr_virt;
    u32TotalSize = stMemParas.size;
	// printf("File(%s)  Line(%d): u32TotalSize_ALLOC(%d)\n", __FILE__,__LINE__,u32TotalSize);
	
	SAMPLE_SVP_CHECK_EXPR_RET(u32TotalSize != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error,Malloc memory failed!\n");
	pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr = u64PhyAddr;
    pstSoftWareParam->stGetResultTmpBuf.u64VirAddr = (HI_U64)(pu8VirAddr);


	if (s32Ret != u32TotalSize) {
        // LOG_ERROR("FaceDetetectModel SoftWare_ParamInit failed");
        return HI_FAILURE;
    } else {
        // LOG_DEBUG("FaceDetetectModel SoftWare_ParamInit successfully ");
        return HI_SUCCESS;
    }

}


HI_U32 FCOSFaceLandmark_SoftwareInit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftWareParam)
{   

    HI_U32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    HI_U64 u64PhyAddr = 0;
    HI_U8* pu8VirAddr = NULL;
    sdc_mmz_alloc_s stMemParas;

    HI_U32 u32TmpBufTotalSize = 0;
	HI_U32 u32TotalSize = 0;

    pstSoftWareParam->au32NodeNum=10;
    pstSoftWareParam->au32BoxNum=5;
    pstSoftWareParam->au32ScoreNum=5;

    pstSoftWareParam->u32OriImHeight = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Height;
    pstSoftWareParam->u32OriImWidth = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Width;
    printf("inputs=%d\n",i);
    printf("File(%s)  Line(%d): inputs Height(%d)\n", __FILE__,__LINE__,pstSoftWareParam->u32OriImHeight);
    printf("File(%s)  Line(%d): inputs Width(%d)\n", __FILE__,__LINE__,pstSoftWareParam->u32OriImWidth);
    printf("\n");

    for(i = 0; i < pstSoftWareParam->au32NodeNum; i++)
    {
        pstSoftWareParam->au32GridHeight[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Chn;
        pstSoftWareParam->au32GridWidth[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Height;
        pstSoftWareParam->au32GridChannel[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Width;
		printf("output_number_i=%d\n",i);
        printf("File(%s)  Line(%d): output Chn(%d)\n", __FILE__,__LINE__,pstSoftWareParam->au32GridHeight[i]);
        printf("File(%s)  Line(%d): output Height(%d)\n", __FILE__,__LINE__,pstSoftWareParam->au32GridWidth[i]);
        printf("File(%s)  Line(%d): output Width(%d)\n", __FILE__,__LINE__,pstSoftWareParam->au32GridChannel[i]);
        printf("\n");
    }

    pstSoftWareParam->u32BboxNumEachGrid[0] = 1;
	pstSoftWareParam->u32BboxNumEachGrid[1] = 1;
	pstSoftWareParam->u32BboxNumEachGrid[2] = 1;
    pstSoftWareParam->u32BboxNumEachGrid[3] = 1;
	pstSoftWareParam->u32BboxNumEachGrid[4] = 1;

	pstSoftWareParam->af32BoxStep[0] = 8;
    pstSoftWareParam->af32BoxStep[1] = 16;
    pstSoftWareParam->af32BoxStep[2] = 32;
    pstSoftWareParam->af32BoxStep[3] = 64;
    pstSoftWareParam->af32BoxStep[4] = 128;


	pstSoftWareParam->af32BoxMinSize[0][0] = 16.0f;
    pstSoftWareParam->af32BoxMinSize[1][0] = 32.0f;
    pstSoftWareParam->af32BoxMinSize[2][0] = 64.0f;
    pstSoftWareParam->af32BoxMinSize[3][0] = 128.0f;
    pstSoftWareParam->af32BoxMinSize[4][0] = 256.0f;

	pstSoftWareParam->f64NmsThresh=0.5;

	u32TmpBufTotalSize = FCOSFaceLandmark_GetResultTmpBuf(pstNnieParam,pstSoftWareParam);
	u32TotalSize = u32TmpBufTotalSize;
	// printf("File(%s)  Line(%d): u32TotalSize_NEED(%d)\n", __FILE__,__LINE__,u32TotalSize);
	s32Ret = SDC_MemAlloc(fd_utils,u32TotalSize,1,&stMemParas);
	u64PhyAddr = stMemParas.addr_phy;
    pu8VirAddr = (HI_U8 *)stMemParas.addr_virt;
    u32TotalSize = stMemParas.size;
	// printf("File(%s)  Line(%d): u32TotalSize_ALLOC(%d)\n", __FILE__,__LINE__,u32TotalSize);
	
	SAMPLE_SVP_CHECK_EXPR_RET(u32TotalSize != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error,Malloc memory failed!\n");
	pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr = u64PhyAddr;
    pstSoftWareParam->stGetResultTmpBuf.u64VirAddr = (HI_U64)(pu8VirAddr);


	if (s32Ret != u32TotalSize) {
        // LOG_ERROR("FaceDetetectModel SoftWare_ParamInit failed");
        return HI_FAILURE;
    } else {
        // LOG_DEBUG("FaceDetetectModel SoftWare_ParamInit successfully ");
        return HI_SUCCESS;
    }

}


HI_U32 DenseNet_SoftwareInit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftWareParam)
{   

    HI_U32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    HI_U64 u64PhyAddr = 0;
    HI_U8* pu8VirAddr = NULL;
    sdc_mmz_alloc_s stMemParas;

    HI_U32 u32TmpBufTotalSize = 0;
	HI_U32 u32TotalSize = 0;

    pstSoftWareParam->au32NodeNum=1;


    pstSoftWareParam->u32OriImHeight = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Height;
    pstSoftWareParam->u32OriImWidth = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Width;
    printf("inputs=%d\n",i);
    printf("File(%s)  Line(%d): inputs Height(%d)\n", __FILE__,__LINE__,pstSoftWareParam->u32OriImHeight);
    printf("File(%s)  Line(%d): inputs Width(%d)\n", __FILE__,__LINE__,pstSoftWareParam->u32OriImWidth);
    printf("\n");

    for(i = 0; i < pstSoftWareParam->au32NodeNum; i++)
    {
        pstSoftWareParam->au32GridHeight[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Chn;
        pstSoftWareParam->au32GridWidth[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Height;
        pstSoftWareParam->au32GridChannel[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Width;
		printf("output_number_i=%d\n",i);
        printf("File(%s)  Line(%d): output Chn(%d)\n", __FILE__,__LINE__,pstSoftWareParam->au32GridHeight[i]);
        printf("File(%s)  Line(%d): output Height(%d)\n", __FILE__,__LINE__,pstSoftWareParam->au32GridWidth[i]);
        printf("File(%s)  Line(%d): output Width(%d)\n", __FILE__,__LINE__,pstSoftWareParam->au32GridChannel[i]);
        printf("\n");
    }


	u32TmpBufTotalSize = DenseNet_GetResultTmpBuf(pstNnieParam,pstSoftWareParam);
	u32TotalSize = u32TmpBufTotalSize;
	// printf("File(%s)  Line(%d): u32TotalSize_NEED(%d)\n", __FILE__,__LINE__,u32TotalSize);
	s32Ret = SDC_MemAlloc(fd_utils,u32TotalSize,1,&stMemParas);
	u64PhyAddr = stMemParas.addr_phy;
    pu8VirAddr = (HI_U8 *)stMemParas.addr_virt;
    u32TotalSize = stMemParas.size;
	// printf("File(%s)  Line(%d): u32TotalSize_ALLOC(%d)\n", __FILE__,__LINE__,u32TotalSize);
	
	SAMPLE_SVP_CHECK_EXPR_RET(u32TotalSize != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error,Malloc memory failed!\n");
	pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr = u64PhyAddr;
    pstSoftWareParam->stGetResultTmpBuf.u64VirAddr = (HI_U64)(pu8VirAddr);


	if (s32Ret != u32TotalSize) {
        // LOG_ERROR("FaceDetetectModel SoftWare_ParamInit failed");
        return HI_FAILURE;
    } else {
        // LOG_DEBUG("FaceDetetectModel SoftWare_ParamInit successfully ");
        return HI_SUCCESS;
    }

}





HI_S32 SAMPLE_SVP_NNIE_HardWare_and_SoftWare_ParamInit(SAMPLE_SVP_NNIE_CFG_S* pstCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SDC_NNIE_ParamInit(pstCfg,pstNnieParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),SDC_NNIE_ParamInit failed!\n",s32Ret);

    /*init software para*/
    s32Ret = SAMPLE_SVP_NNIE_SoftwareInit(pstNnieParam,pstSoftWareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),SAMPLE_SVP_NNIE_SoftwareInit failed!\n",s32Ret);

    return s32Ret;
INIT_FAIL_0:
    // s32Ret = SAMPLE_SVP_NNIE_Ssd_Deinit(pstNnieParam,pstSoftWareParam,NULL, fd);
	s32Ret = HI_FAILURE;
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error(%#x),SAMPLE_SVP_NNIE_Ssd_Deinit failed!\n",s32Ret);
    return HI_FAILURE;
}


HI_S32 SAMPLE_SVP_NNIE_SoftwareInit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam)
{
	HI_S32 s32Ret;

    // std::cout<<"u32ModelType="<<pstSoftwareParam->u32ModelType<<std::endl;
    if (pstSoftwareParam->u32ModelType==0)
    {
        printf("init0\n");
        s32Ret =FCOSFace_SoftwareInit(pstNnieParam,pstSoftwareParam);
        return s32Ret;
    }
    else if (pstSoftwareParam->u32ModelType==1)
    {
        printf("init1\n");
        s32Ret =FCOSFaceLandmark_SoftwareInit(pstNnieParam,pstSoftwareParam);
        return s32Ret;
    }
    else if (pstSoftwareParam->u32ModelType==100)
    {
        printf("init100\n");
        s32Ret =DenseNet_SoftwareInit(pstNnieParam,pstSoftwareParam);
        return s32Ret;
    }
    else{
        printf("pstSoftwareParam->au32ModelType:%d NotImplemented Error\n",pstSoftwareParam->u32ModelType);
        return s32Ret;
    }

}

