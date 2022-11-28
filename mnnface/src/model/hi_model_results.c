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
#include "common_defs.h"

#include "hi_model_results.h"
#include "sample_comm_nnie.h"
#include "sdc_fd_utils.h"

extern int fd_utils;
extern int fd_algorithm;

using namespace std;


float l2norm(float *data,int N)
{
    float c=0;
    for(int i=0;i<N;i++)
        c=c+data[i]*data[i];
    c=sqrt(c);
    return c;
}



static HI_DOUBLE SVP_NNIE_Iou(SAMPLE_SVP_NNIE_BBOX_S *pstBbox1,
    SAMPLE_SVP_NNIE_BBOX_S *pstBbox2)
{
    HI_FLOAT InterWidth = 0.0;
    HI_FLOAT InterHeight = 0.0;
    HI_DOUBLE f64InterArea = 0.0;
    HI_DOUBLE f64Box1Area = 0.0;
    HI_DOUBLE f64Box2Area = 0.0;
    HI_DOUBLE f64UnionArea = 0.0;

    InterWidth =  SAMPLE_SVP_NNIE_MIN(pstBbox1->f32Xmax*100.0f, pstBbox2->f32Xmax*100.0f) - SAMPLE_SVP_NNIE_MAX(pstBbox1->f32Xmin*100.0f,pstBbox2->f32Xmin*100.0f);
    InterHeight = SAMPLE_SVP_NNIE_MIN(pstBbox1->f32Ymax*100.0f, pstBbox2->f32Ymax*100.0f) - SAMPLE_SVP_NNIE_MAX(pstBbox1->f32Ymin*100.0f,pstBbox2->f32Ymin*100.0f);

    if(InterWidth <= 0 || InterHeight <= 0) return 0;

    f64InterArea = InterWidth * InterHeight;
    f64Box1Area = (pstBbox1->f32Xmax*100.0 - pstBbox1->f32Xmin*100.0)* (pstBbox1->f32Ymax*100.0 - pstBbox1->f32Ymin*100.0);
    f64Box2Area = (pstBbox2->f32Xmax*100.0 - pstBbox2->f32Xmin*100.0)* (pstBbox2->f32Ymax*100.0 - pstBbox2->f32Ymin*100.0);
    f64UnionArea = f64Box1Area + f64Box2Area - f64InterArea;

    return f64InterArea/f64UnionArea;
}

static HI_S32 SVP_NNIE_NonMaxSuppression( SAMPLE_SVP_NNIE_BBOX_S* pstBbox,
    HI_U32 u32BboxNum, HI_DOUBLE f64NmsThresh,HI_U32 u32MaxRoiNum)
{
    HI_U32 i,j;
    HI_U32 u32Num = 0;
    HI_DOUBLE f64Iou = 0.0;

    for (i = 0; i < u32BboxNum && u32Num < u32MaxRoiNum; i++)
    {
        if(pstBbox[i].u32Mask == 0 )  //u32Mask进行nms的标记
        {
            u32Num++;
            for(j= i+1;j< u32BboxNum; j++)
            {
                if( pstBbox[j].u32Mask == 0 )
                {
                    f64Iou = SVP_NNIE_Iou(&pstBbox[i],&pstBbox[j]);
                    if(f64Iou >= f64NmsThresh)  
                    {
                        pstBbox[j].u32Mask = 1;
                    }
                }
            }
        }
    }

    return HI_SUCCESS;
}


static void SVP_NNIE_Argswap(HI_S32* ps32Src1, HI_S32* ps32Src2,
    HI_U32 u32ArraySize)
{
    HI_U32 i = 0;
    HI_S32 s32Tmp = 0;
    for( i = 0; i < u32ArraySize; i++ )
    {
        s32Tmp = ps32Src1[i];
        ps32Src1[i] = ps32Src2[i];
        ps32Src2[i] = s32Tmp;
    }
}
static HI_S32 SVP_NNIE_NonRecursiveArgQuickSort(HI_S32* ps32Array,
    HI_S32 s32Low, HI_S32 s32High, HI_U32 u32ArraySize,HI_U32 u32ScoreIdx,
    SAMPLE_SVP_NNIE_STACK_S *pstStack)
{
    HI_S32 i = s32Low;
    HI_S32 j = s32High;
    HI_S32 s32Top = 0;
    HI_S32 s32KeyConfidence = ps32Array[u32ArraySize * s32Low + u32ScoreIdx];
    pstStack[s32Top].s32Min = s32Low;
    pstStack[s32Top].s32Max = s32High;

    while(s32Top > -1)
    {
        s32Low = pstStack[s32Top].s32Min;
        s32High = pstStack[s32Top].s32Max;
        i = s32Low;
        j = s32High;
        s32Top--;

        s32KeyConfidence = ps32Array[u32ArraySize * s32Low + u32ScoreIdx];

        while(i < j)
        {
            while((i < j) && (s32KeyConfidence > ps32Array[j * u32ArraySize + u32ScoreIdx]))
            {
                j--;
            }
            if(i < j)
            {
                SVP_NNIE_Argswap(&ps32Array[i*u32ArraySize], &ps32Array[j*u32ArraySize],u32ArraySize);
                i++;
            }

            while((i < j) && (s32KeyConfidence < ps32Array[i*u32ArraySize + u32ScoreIdx]))
            {
                i++;
            }
            if(i < j)
            {
                SVP_NNIE_Argswap(&ps32Array[i*u32ArraySize], &ps32Array[j*u32ArraySize],u32ArraySize);
                j--;
            }
        }

        if(s32Low < i-1)
        {
            s32Top++;
            pstStack[s32Top].s32Min = s32Low;
            pstStack[s32Top].s32Max = i-1;
        }

        if(s32High > i+1)
        {
            s32Top++;
            pstStack[s32Top].s32Min = i+1;
            pstStack[s32Top].s32Max = s32High;
        }
    }
    return HI_SUCCESS;
}

static HI_S32 SVP_NNIE_NonRecursiveArgQuickSort2(HI_S32* ps32Array,HI_S32* ps32Array2,
    HI_S32 s32Low, HI_S32 s32High, HI_U32 u32ArraySize,HI_U32 u32ArraySize2,HI_U32 u32ScoreIdx,
    SAMPLE_SVP_NNIE_STACK_S *pstStack)
{
    HI_S32 i = s32Low;
    HI_S32 j = s32High;
    HI_S32 s32Top = 0;
    HI_S32 s32KeyConfidence = ps32Array[u32ArraySize * s32Low + u32ScoreIdx];
    pstStack[s32Top].s32Min = s32Low;
    pstStack[s32Top].s32Max = s32High;

    while(s32Top > -1)
    {
        s32Low = pstStack[s32Top].s32Min;
        s32High = pstStack[s32Top].s32Max;
        i = s32Low;
        j = s32High;
        s32Top--;

        s32KeyConfidence = ps32Array[u32ArraySize * s32Low + u32ScoreIdx];

        while(i < j)
        {
            while((i < j) && (s32KeyConfidence > ps32Array[j * u32ArraySize + u32ScoreIdx]))
            {
                j--;
            }
            if(i < j)
            {
                SVP_NNIE_Argswap(&ps32Array[i*u32ArraySize], &ps32Array[j*u32ArraySize],u32ArraySize);
                SVP_NNIE_Argswap(&ps32Array2[i*u32ArraySize2], &ps32Array2[j*u32ArraySize2],u32ArraySize2);
                i++;
            }

            while((i < j) && (s32KeyConfidence < ps32Array[i*u32ArraySize + u32ScoreIdx]))
            {
                i++;
            }
            if(i < j)
            {
                SVP_NNIE_Argswap(&ps32Array[i*u32ArraySize], &ps32Array[j*u32ArraySize],u32ArraySize);
                SVP_NNIE_Argswap(&ps32Array2[i*u32ArraySize2], &ps32Array2[j*u32ArraySize2],u32ArraySize2);
                j--;
            }
        }

        if(s32Low < i-1)
        {
            s32Top++;
            pstStack[s32Top].s32Min = s32Low;
            pstStack[s32Top].s32Max = i-1;
        }

        if(s32High > i+1)
        {
            s32Top++;
            pstStack[s32Top].s32Min = i+1;
            pstStack[s32Top].s32Max = s32High;
        }
    }
    return HI_SUCCESS;
}

static HI_FLOAT s_af32ExpCoef[10][16] = {
    {1.0f, 1.00024f, 1.00049f, 1.00073f, 1.00098f, 1.00122f, 1.00147f, 1.00171f, 1.00196f, 1.0022f, 1.00244f, 1.00269f, 1.00293f, 1.00318f, 1.00342f, 1.00367f},
    {1.0f, 1.00391f, 1.00784f, 1.01179f, 1.01575f, 1.01972f, 1.02371f, 1.02772f, 1.03174f, 1.03578f, 1.03984f, 1.04391f, 1.04799f, 1.05209f, 1.05621f, 1.06034f},
    {1.0f, 1.06449f, 1.13315f, 1.20623f, 1.28403f, 1.36684f, 1.45499f, 1.54883f, 1.64872f, 1.75505f, 1.86825f, 1.98874f, 2.117f, 2.25353f, 2.39888f, 2.55359f},
    {1.0f, 2.71828f, 7.38906f, 20.0855f, 54.5981f, 148.413f, 403.429f, 1096.63f, 2980.96f, 8103.08f, 22026.5f, 59874.1f, 162755.0f, 442413.0f, 1.2026e+006f, 3.26902e+006f},
    {1.0f, 8.88611e+006f, 7.8963e+013f, 7.01674e+020f, 6.23515e+027f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f},
    {1.0f, 0.999756f, 0.999512f, 0.999268f, 0.999024f, 0.99878f, 0.998536f, 0.998292f, 0.998049f, 0.997805f, 0.997562f, 0.997318f, 0.997075f, 0.996831f, 0.996588f, 0.996345f},
    {1.0f, 0.996101f, 0.992218f, 0.98835f, 0.984496f, 0.980658f, 0.976835f, 0.973027f, 0.969233f, 0.965455f, 0.961691f, 0.957941f, 0.954207f, 0.950487f, 0.946781f, 0.94309f},
    {1.0f, 0.939413f, 0.882497f, 0.829029f, 0.778801f, 0.731616f, 0.687289f, 0.645649f, 0.606531f, 0.569783f, 0.535261f, 0.502832f, 0.472367f, 0.443747f, 0.416862f, 0.391606f},
    {1.0f, 0.367879f, 0.135335f, 0.0497871f, 0.0183156f, 0.00673795f, 0.00247875f, 0.000911882f, 0.000335463f, 0.00012341f, 4.53999e-005f, 1.67017e-005f, 6.14421e-006f, 2.26033e-006f, 8.31529e-007f, 3.05902e-007f},
    {1.0f, 1.12535e-007f, 1.26642e-014f, 1.42516e-021f, 1.60381e-028f, 1.80485e-035f, 2.03048e-042f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
};

static HI_FLOAT SVP_NNIE_QuickExp( HI_S32 s32Value )
{
    if( s32Value & 0x80000000 )
    {
        s32Value = ~s32Value + 0x00000001;
        return s_af32ExpCoef[5][s32Value & 0x0000000F] * s_af32ExpCoef[6][(s32Value>>4) & 0x0000000F] * s_af32ExpCoef[7][(s32Value>>8) & 0x0000000F] * s_af32ExpCoef[8][(s32Value>>12) & 0x0000000F] * s_af32ExpCoef[9][(s32Value>>16) & 0x0000000F ];
    }
    else
    {
        return s_af32ExpCoef[0][s32Value & 0x0000000F] * s_af32ExpCoef[1][(s32Value>>4) & 0x0000000F] * s_af32ExpCoef[2][(s32Value>>8) & 0x0000000F] * s_af32ExpCoef[3][(s32Value>>12) & 0x0000000F] * s_af32ExpCoef[4][(s32Value>>16) & 0x0000000F ];
    }
}

static HI_S32 SVP_NNIE_SoftMax( HI_FLOAT* pf32Src, HI_U32 u32Num)
{
    HI_FLOAT f32Max = 0;
    HI_FLOAT f32Sum = 0;
    HI_U32 i = 0;

    for(i = 0; i < u32Num; ++i)
    {
        if(f32Max < pf32Src[i])
        {
            f32Max = pf32Src[i];
        }
    }

    for(i = 0; i < u32Num; ++i)
    {
        pf32Src[i] = (HI_FLOAT)SVP_NNIE_QuickExp((HI_S32)((pf32Src[i] - f32Max)*SAMPLE_SVP_NNIE_QUANT_BASE));
        f32Sum += pf32Src[i];
    }

    for(i = 0; i < u32Num; ++i)
    {
        pf32Src[i] /= f32Sum;
    }
    return HI_SUCCESS;
}

HI_S32 SVP_NNIE_SSD_SoftMax(HI_S32* ps32Src, HI_S32 s32ArraySize, HI_S32* ps32Dst)
{
    HI_S32 s32Max = 0;
    HI_S32 s32Sum = 0;
    HI_S32 i = 0;
    for (i = 0; i < s32ArraySize; ++i)
    {
        if (s32Max < ps32Src[i])
        {
            s32Max = ps32Src[i];
        }
    }
    for (i = 0; i < s32ArraySize; ++i)
    {
        ps32Dst[i] = (HI_S32)(SAMPLE_SVP_NNIE_QUANT_BASE* exp((HI_FLOAT)(ps32Src[i] - s32Max) / SAMPLE_SVP_NNIE_QUANT_BASE));
        s32Sum += ps32Dst[i];
    }
    for (i = 0; i < s32ArraySize; ++i)
    {
        ps32Dst[i] = (HI_S32)(((HI_FLOAT)ps32Dst[i] / (HI_FLOAT)s32Sum) * SAMPLE_SVP_NNIE_QUANT_BASE);
    }
    return HI_SUCCESS;
}


HI_S32 SVP_NNIE_Ssd_SoftmaxForward(HI_U32 u32SoftMaxInHeight,
    HI_U32 au32SoftMaxInChn[], HI_U32 u32ConcatNum, HI_U32 au32ConvStride[],
    HI_U32 au32SoftMaxWidth[],HI_S32* aps32SoftMaxInputData[], HI_S32* ps32SoftMaxOutputData)
{
    HI_S32* ps32InputData = NULL;
    HI_S32* ps32OutputTmp = NULL;
    HI_U32 u32OuterNum = 0;
    HI_U32 u32InnerNum = 0;
    HI_U32 u32InputChannel = 0;
    HI_U32 i = 0;
    HI_U32 u32ConcatCnt = 0;
    HI_S32 s32Ret = 0;
    HI_U32 u32Stride = 0;
    HI_U32 u32Skip = 0;
    HI_U32 u32Left = 0;
    ps32OutputTmp = ps32SoftMaxOutputData;
    for (u32ConcatCnt = 0; u32ConcatCnt < u32ConcatNum; u32ConcatCnt++)
    {
        ps32InputData = aps32SoftMaxInputData[u32ConcatCnt];
        u32Stride = au32ConvStride[u32ConcatCnt];
        u32InputChannel = au32SoftMaxInChn[u32ConcatCnt];
        u32OuterNum = u32InputChannel / u32SoftMaxInHeight;
        u32InnerNum = u32SoftMaxInHeight;
        u32Skip = au32SoftMaxWidth[u32ConcatCnt] / u32InnerNum;
        u32Left = u32Stride - au32SoftMaxWidth[u32ConcatCnt];
        for (i = 0; i < u32OuterNum; i++)
        {
            s32Ret = SVP_NNIE_SSD_SoftMax(ps32InputData, (HI_S32)u32InnerNum,ps32OutputTmp);
            if ((i + 1) % u32Skip == 0)
            {
                ps32InputData += u32Left;
            }
            ps32InputData += u32InnerNum;
            ps32OutputTmp += u32InnerNum;
        }
    }
    return s32Ret;
}


// HI_S32 FCOSFace_GetResult(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam,SDC_SSD_RESULT_S *pstResult)
// {


//     // printf("File(%s)  Line(%d): Src0 enType(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].enType);
//     // printf("File(%s)  Line(%d): Src0 Chn(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Chn);
//     // printf("File(%s)  Line(%d): Src0 Height(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Height);
//     // printf("File(%s)  Line(%d): Src0 Width(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Width);
//         // cv::Mat image(cv::Size(256, 114), CV_8UC3);
// 	// image.data =(HI_U8*)(sdc_nnie_forward.astSrc[0].u64VirAddr);
//     // cv::imwrite("rgb_opencv_test.jpg",image);

//     HI_S32 s32Ret = HI_SUCCESS;
//     HI_U32 i = 0, j = 0, k = 0, c = 0, h = 0, w = 0;
//     HI_FLOAT *pf32Permute = NULL;
//     HI_FLOAT *pf32PermuteScore = NULL;
//     HI_FLOAT *pf32PermuteLandmark = NULL;
//     SAMPLE_SVP_NNIE_BBOX_S *pstBbox = NULL;
//     SAMPLE_SVP_NNIE_LANDMARK_S *pstLandmark = NULL;
//     HI_S32 *ps32AssistBuf = NULL;



//     HI_FLOAT f32Xmin;
//     HI_FLOAT f32Ymin;
//     HI_FLOAT f32Xmax;
//     HI_FLOAT f32Ymax;


//     HI_FLOAT f32Score;
//     HI_FLOAT f32X0;
//     HI_FLOAT f32Y0;
//     HI_FLOAT f32X1;
//     HI_FLOAT f32Y1;
//     HI_FLOAT f32X2;
//     HI_FLOAT f32Y2;
//     HI_FLOAT f32X3;
//     HI_FLOAT f32Y3;
//     HI_FLOAT f32X4;
//     HI_FLOAT f32Y4;
//     HI_U32 u32BboxNum = 0;


//     HI_U32 au32Stride[10] = {0};
//     HI_S32 *aps32InputBlob[10] = {0};
//     HI_S32 *aps32BoxBlob[5] = {0};
//     HI_S32 *aps32ScoreBlob[5] = {0};


//     for(i = 0; i < pstSoftwareParam->au32NodeNum; i++)
//     {   
//         aps32InputBlob[i] = (HI_S32*)pstNnieParam->astSegData[0].astDst[i].u64VirAddr;
//         au32Stride[i] = pstNnieParam->astSegData[0].astDst[i].u32Stride;
//         // printf("Dst_number_i=%d\n",i);
//         // printf("File(%s)  Line(%d): Dst0 enType(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astDst[i].enType);
//         // printf("File(%s)  Line(%d): Dst0 Chn(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astDst[i].unShape.stWhc.u32Chn);
//         // printf("File(%s)  Line(%d): Dst0 Height(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astDst[i].unShape.stWhc.u32Height);
//         // printf("File(%s)  Line(%d): Dst0 Width(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astDst[i].unShape.stWhc.u32Width);
//         // printf("File(%s)  Line(%d): Dst0 Stride(%d)\n", __FILE__,__LINE__,au32Stride[i]);
//         // printf("\n");
//     }

//     for(i = 0; i < pstSoftwareParam->au32BoxNum; i++)
//     {
//         aps32BoxBlob[i] = (HI_S32*)pstNnieParam->astSegData[0].astDst[i*2].u64VirAddr;
//         aps32ScoreBlob[i] = (HI_S32*)pstNnieParam->astSegData[0].astDst[i*2+1].u64VirAddr;
//     }

//     pf32Permute = (HI_FLOAT*)(HI_S32*)pstSoftwareParam->stGetResultTmpBuf.u64VirAddr;
//     pf32PermuteScore=(HI_FLOAT*)(pf32Permute+pstSoftwareParam->u32MaxBoxBlobSize/sizeof(HI_S32));
//     pstBbox = (SAMPLE_SVP_NNIE_BBOX_S*)(pf32PermuteScore+pstSoftwareParam->u32MaxScoreBlobSize/sizeof(HI_S32));
//     ps32AssistBuf = (HI_S32*)(pstBbox+pstSoftwareParam->u32TotalBboxNum);


//     int nn=0;

//     for(i = 0; i < pstSoftwareParam->au32BoxNum; i++)
//     {
//         HI_U32 u32Offset=0;
//         HI_U32 u32OffsetScore=0;

//         HI_S32* ps32Box = aps32BoxBlob[i];
//         HI_S32* ps32Score = aps32ScoreBlob[i];

//         HI_U32  grid_h = pstNnieParam->astSegData[0].astDst[2*i].unShape.stWhc.u32Chn;
//         HI_U32  grid_w = pstNnieParam->astSegData[0].astDst[2*i].unShape.stWhc.u32Height;
//         HI_U32  grid_stride_bbox= pstNnieParam->astSegData[0].astDst[2*i].u32Stride/sizeof(HI_S32);
//         HI_U32  grid_c_bbox =pstNnieParam->astSegData[0].astDst[2*i].unShape.stWhc.u32Width;


//         HI_U32  grid_stride_score= pstNnieParam->astSegData[0].astDst[2*i+1].u32Stride/sizeof(HI_S32);
//         HI_U32  grid_c_score =pstNnieParam->astSegData[0].astDst[2*i+1].unShape.stWhc.u32Width;

//         // printf("grid_h=%d,grid_w=%d,grid_stride_bbox=%d,grid_c_bbox=%d\n",grid_h,grid_w,grid_stride_bbox,grid_c_bbox);
//         // printf("grid_h=%d,grid_w=%d,grid_stride_score=%d,grid_c_score=%d\n",grid_h,grid_w,grid_stride_score,grid_c_score);

//         for (h = 0; h <grid_h ; h++)
//         {    
//             for (w = 0; w < grid_w; w++)
//             {
//                 for (c = 0; c < grid_c_bbox; c++)
//                 {
//                     pf32Permute[u32Offset]= (HI_FLOAT)(ps32Box[c+w*grid_stride_bbox+h*(grid_w*grid_stride_bbox)]) / SAMPLE_SVP_NNIE_QUANT_BASE;
//                     // printf("(%d):%f\n",u32Offset,pf32Permute[u32Offset]);
//                     u32Offset=u32Offset+1;
//                 }

//                 for (c = 0; c < grid_c_score; c++)
//                 {
//                     pf32PermuteScore[u32OffsetScore]= (HI_FLOAT)(ps32Score[c+w*grid_stride_score+h*(grid_w*grid_stride_score)]) / SAMPLE_SVP_NNIE_QUANT_BASE;
//                     // printf("(%d):%f\n",u32OffsetScore,pf32PermuteScore[u32OffsetScore]);
//                     u32OffsetScore=u32OffsetScore+1;   
//                 }
//             }
//         }


//         for(j = 0; j < grid_h*grid_w; j++)
//         {
//             HI_U32 x = j % grid_w;
//             HI_U32 y = j / grid_w;
//             HI_FLOAT dense_cx = (x+0.5) * pstSoftwareParam->af32BoxStep[i] / pstSoftwareParam->u32OriImWidth;
//             HI_FLOAT dense_cy = (y+0.5) * pstSoftwareParam->af32BoxStep[i] / pstSoftwareParam->u32OriImHeight;

//             for(k = 0; k < pstSoftwareParam->u32BboxNumEachGrid[i]; k++)
//             {
                

                
                
//                 u32OffsetScore = (j * pstSoftwareParam->u32BboxNumEachGrid[i] + k) * SAMPLE_SVP_NNIE_EACH_CLASS_INFER_RESULT_NUM;
//                 (void)SVP_NNIE_SoftMax(&pf32PermuteScore[u32OffsetScore], SAMPLE_SVP_NNIE_EACH_CLASS_INFER_RESULT_NUM);
//                 f32Score=pf32PermuteScore[u32OffsetScore+1];


//                 if (f32Score > pstSoftwareParam->f32ConfThresh) //即conf大于0.5
//                 {
//                     //将预测的中心点坐标变换成对角线坐标

//                     // printf("nn(%d): %f,%f,%f,%f,%f\n",nn,f32Xmin,f32Ymin,f32Xmax,f32Ymax,f32Score);

//                     HI_FLOAT s_kx = pstSoftwareParam->af32BoxMinSize[i][k] / pstSoftwareParam->u32OriImWidth;
//                     HI_FLOAT s_ky = pstSoftwareParam->af32BoxMinSize[i][k] / pstSoftwareParam->u32OriImHeight;
//                     u32Offset = (j * pstSoftwareParam->u32BboxNumEachGrid[i] + k) * SAMPLE_SVP_NNIE_EACH_BBOX_INFER_RESULT_NUM;
//                     f32Xmin = pf32Permute[u32Offset + 0]*0.2*s_kx+dense_cx;
//                     f32Ymin = pf32Permute[u32Offset + 1]*0.2*s_ky+dense_cy;
//                     f32Xmax = pf32Permute[u32Offset + 2]*0.2*s_kx+dense_cx;
//                     f32Ymax = pf32Permute[u32Offset + 3]*0.2*s_ky+dense_cy;


//                     pstBbox[u32BboxNum].f32Xmin= (HI_FLOAT)(f32Xmin);
//                     pstBbox[u32BboxNum].f32Ymin= (HI_FLOAT)(f32Ymin);
//                     pstBbox[u32BboxNum].f32Xmax= (HI_FLOAT)(f32Xmax);
//                     pstBbox[u32BboxNum].f32Ymax= (HI_FLOAT)(f32Ymax);
//                     pstBbox[u32BboxNum].f32Score = f32Score;
//                     pstBbox[u32BboxNum].u32Mask = 0;                       
                        
//                     u32BboxNum++;
//                 }
//             }

//         }
//     }  

//     (void)SVP_NNIE_NonRecursiveArgQuickSort((HI_S32*)pstBbox, 0, u32BboxNum - 1,
//     sizeof(SAMPLE_SVP_NNIE_BBOX_S)/sizeof(HI_U32),4,(SAMPLE_SVP_NNIE_STACK_S*)ps32AssistBuf);

//     // (void)SVP_NNIE_NonRecursiveArgQuickSort2((HI_S32*)pstBbox,(HI_S32*)pstLandmark, 0, u32BboxNum - 1,
//     // sizeof(SAMPLE_SVP_NNIE_BBOX_S)/sizeof(HI_U32),sizeof(SAMPLE_SVP_NNIE_LANDMARK_S)/sizeof(HI_U32),
//     // 4,(SAMPLE_SVP_NNIE_STACK_S*)ps32AssistBuf);
//     // printf("nn(%d)\n",pstSoftwareParam->u32maxNumOfFace);
//     (void)SVP_NNIE_NonMaxSuppression(pstBbox, u32BboxNum, pstSoftwareParam->f64NmsThresh, pstSoftwareParam->u32maxNumOfFace);
//     pstSoftwareParam->u32BboxNum=u32BboxNum;
        
//     int n=0;
//     for(i = 0; i < u32BboxNum; i++)
//     {
//         // printf("\nu32BboxNum==%d\n",i);
//         // printf("f32Score(%f), f32Xmin(%f),f32Ymin(%f),f32Xmax(%f),f32Ymax(%f) ,u32Mask(%d) \n",
//         // pstBbox[i].f32Score,
//         // pstBbox[i].f32Xmin,
//         // pstBbox[i].f32Ymin,
//         // pstBbox[i].f32Xmax,
//         // pstBbox[i].f32Ymax,
//         // pstBbox[i].u32Mask);
//         if (n>=pstResult->numOfObject){break;}
//         if (pstBbox[i].u32Mask==0)
//         {
//             pstResult->pObjInfo[n].f32Xmin=pstBbox[i].f32Xmin;
//             pstResult->pObjInfo[n].f32Ymin=pstBbox[i].f32Ymin;
//             pstResult->pObjInfo[n].f32Xmax=pstBbox[i].f32Xmax;
//             pstResult->pObjInfo[n].f32Ymax=pstBbox[i].f32Ymax;
//             pstResult->pObjInfo[n].f32Score=pstBbox[i].f32Score;
//             n=n+1;
//         }
//     }
//     pstResult->numOfObject=n;

//     return s32Ret;

// }


HI_S32 FCOSFace_GetResult(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam,SDC_SSD_RESULT_S *pstResult)
{


  
    struct timespec time1 = {0, 0};
    struct timespec time2 = {0, 0};



    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0, j = 0, k = 0, c = 0, h = 0, w = 0;
    HI_FLOAT *pf32Permute = NULL;
    HI_FLOAT *pf32PermuteScore = NULL;
    SAMPLE_SVP_NNIE_BBOX_S *pstBbox = NULL;
    HI_S32 *ps32AssistBuf = NULL;



    HI_FLOAT f32Xmin;
    HI_FLOAT f32Ymin;
    HI_FLOAT f32Xmax;
    HI_FLOAT f32Ymax;


    HI_FLOAT f32Score;

    HI_U32 u32BboxNum = 0;


    // HI_U32 au32Stride[10] = {0};
    // HI_S32 *aps32InputBlob[10] = {0};
    HI_S32 *aps32BoxBlob[5] = {0};
    HI_S32 *aps32ScoreBlob[5] = {0};


    for(i = 0; i < pstSoftwareParam->au32BoxNum; i++)
    {
        aps32BoxBlob[i] = (HI_S32*)pstNnieParam->astSegData[0].astDst[i*2].u64VirAddr;
        aps32ScoreBlob[i] = (HI_S32*)pstNnieParam->astSegData[0].astDst[i*2+1].u64VirAddr;
    }

    pf32Permute = (HI_FLOAT*)(HI_S32*)pstSoftwareParam->stGetResultTmpBuf.u64VirAddr;
    pf32PermuteScore=(HI_FLOAT*)(pf32Permute+pstSoftwareParam->u32MaxBoxBlobSize/sizeof(HI_S32));
    pstBbox = (SAMPLE_SVP_NNIE_BBOX_S*)(pf32PermuteScore+pstSoftwareParam->u32MaxScoreBlobSize/sizeof(HI_S32));
    ps32AssistBuf = (HI_S32*)(pstBbox+pstSoftwareParam->u32TotalBboxNum);


    int nn=0;

    for(i = 0; i < pstSoftwareParam->au32BoxNum; i++)
    {
        HI_U32 u32Offset=0;
        HI_U32 u32OffsetScore=0;

        HI_S32* ps32Box = aps32BoxBlob[i];
        HI_S32* ps32Score = aps32ScoreBlob[i];

        HI_U32  grid_h = pstNnieParam->astSegData[0].astDst[2*i].unShape.stWhc.u32Chn;
        HI_U32  grid_w = pstNnieParam->astSegData[0].astDst[2*i].unShape.stWhc.u32Height;
        HI_U32  grid_stride_bbox= pstNnieParam->astSegData[0].astDst[2*i].u32Stride/sizeof(HI_S32);
        HI_U32  grid_c_bbox =pstNnieParam->astSegData[0].astDst[2*i].unShape.stWhc.u32Width;


        HI_U32  grid_stride_score= pstNnieParam->astSegData[0].astDst[2*i+1].u32Stride/sizeof(HI_S32);
        HI_U32  grid_c_score =pstNnieParam->astSegData[0].astDst[2*i+1].unShape.stWhc.u32Width;

        // printf("grid_h=%d,grid_w=%d,grid_stride_bbox=%d,grid_c_bbox=%d\n",grid_h,grid_w,grid_stride_bbox,grid_c_bbox);
        // printf("grid_h=%d,grid_w=%d,grid_stride_score=%d,grid_c_score=%d\n",grid_h,grid_w,grid_stride_score,grid_c_score);
        // clock_gettime(CLOCK_BOOTTIME, &time1);
        for (h = 0; h <grid_h ; h++)
        {    
            for (w = 0; w < grid_w; w++)
            {
                // for (c = 0; c < grid_c_bbox; c++)
                // {
                //     pf32Permute[u32Offset]= (HI_FLOAT)(ps32Box[c+w*grid_stride_bbox+h*(grid_w*grid_stride_bbox)]) / SAMPLE_SVP_NNIE_QUANT_BASE;
                //     // printf("(%d):%f\n",u32Offset,pf32Permute[u32Offset]);
                //     u32Offset=u32Offset+1;
                // }

                for (c = 0; c < grid_c_score; c++)
                {
                    pf32PermuteScore[u32OffsetScore]= (HI_FLOAT)(ps32Score[c+w*grid_stride_score+h*(grid_w*grid_stride_score)]) / SAMPLE_SVP_NNIE_QUANT_BASE;
                    // printf("(%d):%f\n",u32OffsetScore,pf32PermuteScore[u32OffsetScore]);
                    u32OffsetScore=u32OffsetScore+1;   
                }
            }
        }
        // clock_gettime(CLOCK_BOOTTIME, &time2);
        // LOG_DEBUG("Fill_DIST_Data_time: %03lu ms", (time2.tv_sec - time1.tv_sec) *1000 + (time2.tv_nsec - time1.tv_nsec)/1000000); 

        for(j = 0; j < grid_h*grid_w; j++)
        {

            for(k = 0; k < pstSoftwareParam->u32BboxNumEachGrid[i]; k++)
            {
                              
                u32OffsetScore = (j * pstSoftwareParam->u32BboxNumEachGrid[i] + k) * 2;
                (void)SVP_NNIE_SoftMax(&pf32PermuteScore[u32OffsetScore], SAMPLE_SVP_NNIE_EACH_CLASS_INFER_RESULT_NUM);
                f32Score=pf32PermuteScore[u32OffsetScore+1];

                if (f32Score > pstSoftwareParam->f32ConfThresh) //即conf大于0.5
                {
                    //将预测的中心点坐标变换成对角线坐标

                    // printf("nn(%d): %f,%f,%f,%f,%f\n",nn,f32Xmin,f32Ymin,f32Xmax,f32Ymax,f32Score);
                    HI_U32 x = j % grid_w;
                    HI_U32 y = j / grid_w;
                    HI_FLOAT dense_cx = (x+0.5) * pstSoftwareParam->af32BoxStep[i] / pstSoftwareParam->u32OriImWidth;
                    HI_FLOAT dense_cy = (y+0.5) * pstSoftwareParam->af32BoxStep[i] / pstSoftwareParam->u32OriImHeight;
                    HI_FLOAT s_kx = pstSoftwareParam->af32BoxMinSize[i][k] / pstSoftwareParam->u32OriImWidth;
                    HI_FLOAT s_ky = pstSoftwareParam->af32BoxMinSize[i][k] / pstSoftwareParam->u32OriImHeight;

                    // u32Offset = (j * pstSoftwareParam->u32BboxNumEachGrid[i] + k) * 14;
                    // f32Xmin = pf32Permute[u32Offset + 0]*0.1*s_kx+dense_cx;
                    // f32Ymin = pf32Permute[u32Offset + 1]*0.1*s_ky+dense_cy;
                    // f32Xmax = pf32Permute[u32Offset + 2]*0.1*s_kx+dense_cx;
                    // f32Ymax = pf32Permute[u32Offset + 3]*0.1*s_ky+dense_cy;

                    // f32X0 = pf32Permute[u32Offset + 4]*0.1*s_kx+dense_cx;
                    // f32Y0 = pf32Permute[u32Offset + 5]*0.1*s_ky+dense_cy;
                    // f32X1 = pf32Permute[u32Offset + 6]*0.1*s_kx+dense_cx;
                    // f32Y1 = pf32Permute[u32Offset + 7]*0.1*s_ky+dense_cy;
                    // f32X2 = pf32Permute[u32Offset + 8]*0.1*s_kx+dense_cx;
                    // f32Y2 = pf32Permute[u32Offset + 9]*0.1*s_ky+dense_cy;
                    // f32X3 = pf32Permute[u32Offset + 10]*0.1*s_kx+dense_cx;
                    // f32Y3 = pf32Permute[u32Offset + 11]*0.1*s_ky+dense_cy;
                    // f32X4 = pf32Permute[u32Offset + 12]*0.1*s_kx+dense_cx;
                    // f32Y4 = pf32Permute[u32Offset + 13]*0.1*s_ky+dense_cy;


                    u32Offset = x*grid_stride_bbox+y*(grid_w*grid_stride_bbox);
                    f32Xmin = (HI_FLOAT)(ps32Box[u32Offset+0]) / SAMPLE_SVP_NNIE_QUANT_BASE*0.2*s_kx+dense_cx;
                    f32Ymin = (HI_FLOAT)(ps32Box[u32Offset + 1]) / SAMPLE_SVP_NNIE_QUANT_BASE*0.2*s_ky+dense_cy;
                    f32Xmax =(HI_FLOAT)(ps32Box[u32Offset + 2]) / SAMPLE_SVP_NNIE_QUANT_BASE*0.2*s_kx+dense_cx;
                    f32Ymax = (HI_FLOAT)(ps32Box[u32Offset + 3]) / SAMPLE_SVP_NNIE_QUANT_BASE*0.2*s_ky+dense_cy;




                    pstBbox[u32BboxNum].f32Xmin= (HI_FLOAT)(f32Xmin);
                    pstBbox[u32BboxNum].f32Ymin= (HI_FLOAT)(f32Ymin);
                    pstBbox[u32BboxNum].f32Xmax= (HI_FLOAT)(f32Xmax);
                    pstBbox[u32BboxNum].f32Ymax= (HI_FLOAT)(f32Ymax);


                    pstBbox[u32BboxNum].f32Score = f32Score;
                    pstBbox[u32BboxNum].u32Mask = 0;                       
                        
                    u32BboxNum++;
                }
            }

        }
    }  

    (void)SVP_NNIE_NonRecursiveArgQuickSort((HI_S32*)pstBbox, 0, u32BboxNum - 1,
    sizeof(SAMPLE_SVP_NNIE_BBOX_S)/sizeof(HI_U32),4,(SAMPLE_SVP_NNIE_STACK_S*)ps32AssistBuf);

    // (void)SVP_NNIE_NonRecursiveArgQuickSort2((HI_S32*)pstBbox,(HI_S32*)pstLandmark, 0, u32BboxNum - 1,
    // sizeof(SAMPLE_SVP_NNIE_BBOX_S)/sizeof(HI_U32),sizeof(SAMPLE_SVP_NNIE_LANDMARK_S)/sizeof(HI_U32),
    // 4,(SAMPLE_SVP_NNIE_STACK_S*)ps32AssistBuf);
    // printf("nn(%d)\n",pstSoftwareParam->u32maxNumOfFace);
    (void)SVP_NNIE_NonMaxSuppression(pstBbox, u32BboxNum, pstSoftwareParam->f64NmsThresh, pstSoftwareParam->u32maxNumOfFace);
    pstSoftwareParam->u32BboxNum=u32BboxNum;
        
    int n=0;
    for(i = 0; i < u32BboxNum; i++)
    {
        // printf("\nu32BboxNum==%d\n",i);
        // printf("f32Score(%f), f32Xmin(%f),f32Ymin(%f),f32Xmax(%f),f32Ymax(%f) ,u32Mask(%d) \n",
        // pstBbox[i].f32Score,
        // pstBbox[i].f32Xmin,
        // pstBbox[i].f32Ymin,
        // pstBbox[i].f32Xmax,
        // pstBbox[i].f32Ymax,
        // pstBbox[i].u32Mask);
        if (n>=pstResult->numOfObject){break;}
        if (pstBbox[i].u32Mask==0)
        {
            pstResult->pObjInfo[n].f32Xmin=pstBbox[i].f32Xmin;
            pstResult->pObjInfo[n].f32Ymin=pstBbox[i].f32Ymin;
            pstResult->pObjInfo[n].f32Xmax=pstBbox[i].f32Xmax;
            pstResult->pObjInfo[n].f32Ymax=pstBbox[i].f32Ymax;
            pstResult->pObjInfo[n].f32Score=pstBbox[i].f32Score;
            n=n+1;
        }
    }
    pstResult->numOfObject=n;

    return s32Ret;

}



HI_S32 FCOSFaceLandmark_GetResult(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam,SDC_SSD_RESULT_S *pstResult)
{


    // printf("File(%s)  Line(%d): Src0 enType(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].enType);
    // printf("File(%s)  Line(%d): Src0 Chn(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Chn);
    // printf("File(%s)  Line(%d): Src0 Height(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Height);
    // printf("File(%s)  Line(%d): Src0 Width(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Width);
    // cv::Mat image(cv::Size(640, 640), CV_8UC3);
	// image.data =(HI_U8*)(pstNnieParam->astSegData[0].astSrc[0].u64VirAddr);
    // cv::imwrite("rgb_opencv_test.jpg",image);

    struct timespec time1 = {0, 0};
    struct timespec time2 = {0, 0};



    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0, j = 0, k = 0, c = 0, h = 0, w = 0;
    HI_FLOAT *pf32Permute = NULL;
    HI_FLOAT *pf32PermuteScore = NULL;
    HI_FLOAT *pf32PermuteLandmark = NULL;
    SAMPLE_SVP_NNIE_BBOX_S *pstBbox = NULL;
    SAMPLE_SVP_NNIE_LANDMARK_S *pstLandmark = NULL;
    HI_S32 *ps32AssistBuf = NULL;



    HI_FLOAT f32Xmin;
    HI_FLOAT f32Ymin;
    HI_FLOAT f32Xmax;
    HI_FLOAT f32Ymax;


    HI_FLOAT f32Score;
    HI_FLOAT f32X0;
    HI_FLOAT f32Y0;
    HI_FLOAT f32X1;
    HI_FLOAT f32Y1;
    HI_FLOAT f32X2;
    HI_FLOAT f32Y2;
    HI_FLOAT f32X3;
    HI_FLOAT f32Y3;
    HI_FLOAT f32X4;
    HI_FLOAT f32Y4;
    HI_U32 u32BboxNum = 0;


    // HI_U32 au32Stride[10] = {0};
    // HI_S32 *aps32InputBlob[10] = {0};
    HI_S32 *aps32BoxBlob[5] = {0};
    HI_S32 *aps32ScoreBlob[5] = {0};


    // for(i = 0; i < pstSoftwareParam->au32NodeNum; i++)
    // {   
    //     aps32InputBlob[i] = (HI_S32*)pstNnieParam->astSegData[0].astDst[i].u64VirAddr;
    //     au32Stride[i] = pstNnieParam->astSegData[0].astDst[i].u32Stride;
    //     // printf("Dst_number_i=%d\n",i);
    //     // printf("File(%s)  Line(%d): Dst0 enType(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astDst[i].enType);
    //     // printf("File(%s)  Line(%d): Dst0 Chn(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astDst[i].unShape.stWhc.u32Chn);
    //     // printf("File(%s)  Line(%d): Dst0 Height(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astDst[i].unShape.stWhc.u32Height);
    //     // printf("File(%s)  Line(%d): Dst0 Width(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astDst[i].unShape.stWhc.u32Width);
    //     // printf("File(%s)  Line(%d): Dst0 Stride(%d)\n", __FILE__,__LINE__,au32Stride[i]);
    //     // printf("\n");
    // }

    for(i = 0; i < pstSoftwareParam->au32BoxNum; i++)
    {
        aps32BoxBlob[i] = (HI_S32*)pstNnieParam->astSegData[0].astDst[i*2].u64VirAddr;
        aps32ScoreBlob[i] = (HI_S32*)pstNnieParam->astSegData[0].astDst[i*2+1].u64VirAddr;
    }

    pf32Permute = (HI_FLOAT*)(HI_S32*)pstSoftwareParam->stGetResultTmpBuf.u64VirAddr;
    pf32PermuteScore=(HI_FLOAT*)(pf32Permute+pstSoftwareParam->u32MaxBoxBlobSize/sizeof(HI_S32));
    pstBbox = (SAMPLE_SVP_NNIE_BBOX_S*)(pf32PermuteScore+pstSoftwareParam->u32MaxScoreBlobSize/sizeof(HI_S32));
    ps32AssistBuf = (HI_S32*)(pstBbox+pstSoftwareParam->u32TotalBboxNum);


    int nn=0;

    for(i = 0; i < pstSoftwareParam->au32BoxNum; i++)
    {
        HI_U32 u32Offset=0;
        HI_U32 u32OffsetScore=0;

        HI_S32* ps32Box = aps32BoxBlob[i];
        HI_S32* ps32Score = aps32ScoreBlob[i];

        HI_U32  grid_h = pstNnieParam->astSegData[0].astDst[2*i].unShape.stWhc.u32Chn;
        HI_U32  grid_w = pstNnieParam->astSegData[0].astDst[2*i].unShape.stWhc.u32Height;
        HI_U32  grid_stride_bbox= pstNnieParam->astSegData[0].astDst[2*i].u32Stride/sizeof(HI_S32);
        HI_U32  grid_c_bbox =pstNnieParam->astSegData[0].astDst[2*i].unShape.stWhc.u32Width;


        HI_U32  grid_stride_score= pstNnieParam->astSegData[0].astDst[2*i+1].u32Stride/sizeof(HI_S32);
        HI_U32  grid_c_score =pstNnieParam->astSegData[0].astDst[2*i+1].unShape.stWhc.u32Width;

        // printf("grid_h=%d,grid_w=%d,grid_stride_bbox=%d,grid_c_bbox=%d\n",grid_h,grid_w,grid_stride_bbox,grid_c_bbox);
        // printf("grid_h=%d,grid_w=%d,grid_stride_score=%d,grid_c_score=%d\n",grid_h,grid_w,grid_stride_score,grid_c_score);
        // clock_gettime(CLOCK_BOOTTIME, &time1);
        for (h = 0; h <grid_h ; h++)
        {    
            for (w = 0; w < grid_w; w++)
            {
                // for (c = 0; c < grid_c_bbox; c++)
                // {
                //     pf32Permute[u32Offset]= (HI_FLOAT)(ps32Box[c+w*grid_stride_bbox+h*(grid_w*grid_stride_bbox)]) / SAMPLE_SVP_NNIE_QUANT_BASE;
                //     // printf("(%d):%f\n",u32Offset,pf32Permute[u32Offset]);
                //     u32Offset=u32Offset+1;
                // }

                for (c = 0; c < grid_c_score; c++)
                {
                    pf32PermuteScore[u32OffsetScore]= (HI_FLOAT)(ps32Score[c+w*grid_stride_score+h*(grid_w*grid_stride_score)]) / SAMPLE_SVP_NNIE_QUANT_BASE;
                    // printf("(%d):%f\n",u32OffsetScore,pf32PermuteScore[u32OffsetScore]);
                    u32OffsetScore=u32OffsetScore+1;   
                }
            }
        }
        // clock_gettime(CLOCK_BOOTTIME, &time2);
        // LOG_DEBUG("Fill_DIST_Data_time: %03lu ms", (time2.tv_sec - time1.tv_sec) *1000 + (time2.tv_nsec - time1.tv_nsec)/1000000); 

        for(j = 0; j < grid_h*grid_w; j++)
        {

            for(k = 0; k < pstSoftwareParam->u32BboxNumEachGrid[i]; k++)
            {
                              
                u32OffsetScore = (j * pstSoftwareParam->u32BboxNumEachGrid[i] + k) * 2;
                (void)SVP_NNIE_SoftMax(&pf32PermuteScore[u32OffsetScore], SAMPLE_SVP_NNIE_EACH_CLASS_INFER_RESULT_NUM);
                f32Score=pf32PermuteScore[u32OffsetScore+1];

                if (f32Score > pstSoftwareParam->f32ConfThresh) //即conf大于0.5
                {
                    //将预测的中心点坐标变换成对角线坐标

                    // printf("nn(%d): %f,%f,%f,%f,%f\n",nn,f32Xmin,f32Ymin,f32Xmax,f32Ymax,f32Score);
                    HI_U32 x = j % grid_w;
                    HI_U32 y = j / grid_w;
                    HI_FLOAT dense_cx = (x+0.5) * pstSoftwareParam->af32BoxStep[i] / pstSoftwareParam->u32OriImWidth;
                    HI_FLOAT dense_cy = (y+0.5) * pstSoftwareParam->af32BoxStep[i] / pstSoftwareParam->u32OriImHeight;
                    HI_FLOAT s_kx = pstSoftwareParam->af32BoxMinSize[i][k] / pstSoftwareParam->u32OriImWidth;
                    HI_FLOAT s_ky = pstSoftwareParam->af32BoxMinSize[i][k] / pstSoftwareParam->u32OriImHeight;

                    // u32Offset = (j * pstSoftwareParam->u32BboxNumEachGrid[i] + k) * 14;
                    // f32Xmin = pf32Permute[u32Offset + 0]*0.1*s_kx+dense_cx;
                    // f32Ymin = pf32Permute[u32Offset + 1]*0.1*s_ky+dense_cy;
                    // f32Xmax = pf32Permute[u32Offset + 2]*0.1*s_kx+dense_cx;
                    // f32Ymax = pf32Permute[u32Offset + 3]*0.1*s_ky+dense_cy;

                    // f32X0 = pf32Permute[u32Offset + 4]*0.1*s_kx+dense_cx;
                    // f32Y0 = pf32Permute[u32Offset + 5]*0.1*s_ky+dense_cy;
                    // f32X1 = pf32Permute[u32Offset + 6]*0.1*s_kx+dense_cx;
                    // f32Y1 = pf32Permute[u32Offset + 7]*0.1*s_ky+dense_cy;
                    // f32X2 = pf32Permute[u32Offset + 8]*0.1*s_kx+dense_cx;
                    // f32Y2 = pf32Permute[u32Offset + 9]*0.1*s_ky+dense_cy;
                    // f32X3 = pf32Permute[u32Offset + 10]*0.1*s_kx+dense_cx;
                    // f32Y3 = pf32Permute[u32Offset + 11]*0.1*s_ky+dense_cy;
                    // f32X4 = pf32Permute[u32Offset + 12]*0.1*s_kx+dense_cx;
                    // f32Y4 = pf32Permute[u32Offset + 13]*0.1*s_ky+dense_cy;


                    u32Offset = x*grid_stride_bbox+y*(grid_w*grid_stride_bbox);
                    f32Xmin = (HI_FLOAT)(ps32Box[u32Offset+0]) / SAMPLE_SVP_NNIE_QUANT_BASE*0.1*s_kx+dense_cx;
                    f32Ymin = (HI_FLOAT)(ps32Box[u32Offset + 1]) / SAMPLE_SVP_NNIE_QUANT_BASE*0.1*s_ky+dense_cy;
                    f32Xmax =(HI_FLOAT)(ps32Box[u32Offset + 2]) / SAMPLE_SVP_NNIE_QUANT_BASE*0.1*s_kx+dense_cx;
                    f32Ymax = (HI_FLOAT)(ps32Box[u32Offset + 3]) / SAMPLE_SVP_NNIE_QUANT_BASE*0.1*s_ky+dense_cy;

                    f32X0 = (HI_FLOAT)(ps32Box[u32Offset + 4]) / SAMPLE_SVP_NNIE_QUANT_BASE*0.1*s_kx+dense_cx;
                    f32Y0 = (HI_FLOAT)(ps32Box[u32Offset + 5]) / SAMPLE_SVP_NNIE_QUANT_BASE*0.1*s_ky+dense_cy;
                    f32X1 = (HI_FLOAT)(ps32Box[u32Offset + 6]) / SAMPLE_SVP_NNIE_QUANT_BASE*0.1*s_kx+dense_cx;
                    f32Y1 = (HI_FLOAT)(ps32Box[u32Offset + 7]) / SAMPLE_SVP_NNIE_QUANT_BASE*0.1*s_ky+dense_cy;
                    f32X2 = (HI_FLOAT)(ps32Box[u32Offset + 8]) / SAMPLE_SVP_NNIE_QUANT_BASE*0.1*s_kx+dense_cx;
                    f32Y2 = (HI_FLOAT)(ps32Box[u32Offset + 9]) / SAMPLE_SVP_NNIE_QUANT_BASE*0.1*s_ky+dense_cy;
                    f32X3 = (HI_FLOAT)(ps32Box[u32Offset +10]) / SAMPLE_SVP_NNIE_QUANT_BASE*0.1*s_kx+dense_cx;
                    f32Y3 = (HI_FLOAT)(ps32Box[u32Offset + 11]) / SAMPLE_SVP_NNIE_QUANT_BASE*0.1*s_ky+dense_cy;
                    f32X4 = (HI_FLOAT)(ps32Box[u32Offset + 12]) / SAMPLE_SVP_NNIE_QUANT_BASE*0.1*s_kx+dense_cx;
                    f32Y4 = (HI_FLOAT)(ps32Box[u32Offset + 13]) / SAMPLE_SVP_NNIE_QUANT_BASE*0.1*s_ky+dense_cy;



                    pstBbox[u32BboxNum].f32Xmin= (HI_FLOAT)(f32Xmin);
                    pstBbox[u32BboxNum].f32Ymin= (HI_FLOAT)(f32Ymin);
                    pstBbox[u32BboxNum].f32Xmax= (HI_FLOAT)(f32Xmax);
                    pstBbox[u32BboxNum].f32Ymax= (HI_FLOAT)(f32Ymax);
                    pstBbox[u32BboxNum].f32X0= (HI_FLOAT)(f32X0);
                    pstBbox[u32BboxNum].f32Y0= (HI_FLOAT)(f32Y0);
                    pstBbox[u32BboxNum].f32X1= (HI_FLOAT)(f32X1);
                    pstBbox[u32BboxNum].f32Y1= (HI_FLOAT)(f32Y1);
                    pstBbox[u32BboxNum].f32X2= (HI_FLOAT)(f32X2);
                    pstBbox[u32BboxNum].f32Y2= (HI_FLOAT)(f32Y2);
                    pstBbox[u32BboxNum].f32X3= (HI_FLOAT)(f32X3);
                    pstBbox[u32BboxNum].f32Y3= (HI_FLOAT)(f32Y3);
                    pstBbox[u32BboxNum].f32X4= (HI_FLOAT)(f32X4);
                    pstBbox[u32BboxNum].f32Y4= (HI_FLOAT)(f32Y4);


                    pstBbox[u32BboxNum].f32Score = f32Score;
                    pstBbox[u32BboxNum].u32Mask = 0;                       
                        
                    u32BboxNum++;
                }
            }

        }
    }  

    (void)SVP_NNIE_NonRecursiveArgQuickSort((HI_S32*)pstBbox, 0, u32BboxNum - 1,
    sizeof(SAMPLE_SVP_NNIE_BBOX_S)/sizeof(HI_U32),4,(SAMPLE_SVP_NNIE_STACK_S*)ps32AssistBuf);

    // (void)SVP_NNIE_NonRecursiveArgQuickSort2((HI_S32*)pstBbox,(HI_S32*)pstLandmark, 0, u32BboxNum - 1,
    // sizeof(SAMPLE_SVP_NNIE_BBOX_S)/sizeof(HI_U32),sizeof(SAMPLE_SVP_NNIE_LANDMARK_S)/sizeof(HI_U32),
    // 4,(SAMPLE_SVP_NNIE_STACK_S*)ps32AssistBuf);
    // printf("nn(%d)\n",pstSoftwareParam->u32maxNumOfFace);
    (void)SVP_NNIE_NonMaxSuppression(pstBbox, u32BboxNum, pstSoftwareParam->f64NmsThresh, pstSoftwareParam->u32maxNumOfFace);
    pstSoftwareParam->u32BboxNum=u32BboxNum;
        
    int n=0;
    for(i = 0; i < u32BboxNum; i++)
    {
        // printf("\nu32BboxNum==%d\n",i);
        // printf("f32Score(%f), f32Xmin(%f),f32Ymin(%f),f32Xmax(%f),f32Ymax(%f) ,u32Mask(%d) \n",
        // pstBbox[i].f32Score,
        // pstBbox[i].f32Xmin,
        // pstBbox[i].f32Ymin,
        // pstBbox[i].f32Xmax,
        // pstBbox[i].f32Ymax,
        // pstBbox[i].u32Mask);
        if (n>=pstResult->numOfObject){break;}
        if (pstBbox[i].u32Mask==0)
        {
            pstResult->pObjInfo[n].f32Xmin=pstBbox[i].f32Xmin;
            pstResult->pObjInfo[n].f32Ymin=pstBbox[i].f32Ymin;
            pstResult->pObjInfo[n].f32Xmax=pstBbox[i].f32Xmax;
            pstResult->pObjInfo[n].f32Ymax=pstBbox[i].f32Ymax;
            pstResult->pObjInfo[n].f32X0=pstBbox[i].f32X0;
            pstResult->pObjInfo[n].f32Y0=pstBbox[i].f32Y0;
            pstResult->pObjInfo[n].f32X1=pstBbox[i].f32X1;
            pstResult->pObjInfo[n].f32Y1=pstBbox[i].f32Y1;
            pstResult->pObjInfo[n].f32X2=pstBbox[i].f32X2;
            pstResult->pObjInfo[n].f32Y2=pstBbox[i].f32Y2;
            pstResult->pObjInfo[n].f32X3=pstBbox[i].f32X3;
            pstResult->pObjInfo[n].f32Y3=pstBbox[i].f32Y3;
            pstResult->pObjInfo[n].f32X4=pstBbox[i].f32X4;
            pstResult->pObjInfo[n].f32Y4=pstBbox[i].f32Y4;
            pstResult->pObjInfo[n].f32Score=pstBbox[i].f32Score;
            n=n+1;
        }
    }
    pstResult->numOfObject=n;

    return s32Ret;

}


HI_S32 DenseNet_GetResult(SAMPLE_SVP_NNIE_PARAM_S* pstNnieParam,SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam,SDC_FEATURE_INFO_S *pstFeatureInfo)
{


    // printf("File(%s)  Line(%d): Src0 enType(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].enType);
    // printf("File(%s)  Line(%d): Src0 Chn(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Chn);
    // printf("File(%s)  Line(%d): Src0 Height(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Height);
    // printf("File(%s)  Line(%d): Src0 Width(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Width);
    // cv::Mat image(cv::Size(112, 112), CV_8UC3);
	// image.data =(HI_U8*)(pstNnieParam->astSegData[0].astSrc[0].u64VirAddr);
    // cv::imwrite("rgb_opencv_test.jpg",image);

    struct timespec time1 = {0, 0};
    struct timespec time2 = {0, 0};


    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0, j = 0, k = 0, c = 0, h = 0, w = 0;

    HI_S32 *aps32FeatureBlob;


    // for(i = 0; i < pstSoftwareParam->au32NodeNum; i++)
    // {   
    //     aps32FeatureBlob= (HI_S32*)pstNnieParam->astSegData[0].astDst[i].u64VirAddr;
    //     // printf("Dst_number_i=%d\n",i);
    //     // printf("File(%s)  Line(%d): Dst0 enType(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astDst[i].enType);
    //     // printf("File(%s)  Line(%d): Dst0 Chn(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astDst[i].unShape.stWhc.u32Chn);
    //     // printf("File(%s)  Line(%d): Dst0 Height(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astDst[i].unShape.stWhc.u32Height);
    //     // printf("File(%s)  Line(%d): Dst0 Width(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astDst[i].unShape.stWhc.u32Width);
    //     // printf("File(%s)  Line(%d): Dst0 Stride(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astDst[i].u32Stride);
    //     // printf("\n");
    // }







    HI_FLOAT *pf32Permute = NULL;


   
    aps32FeatureBlob = (HI_S32*)pstNnieParam->astSegData[0].astDst[0].u64VirAddr;

    pf32Permute = (HI_FLOAT*)(HI_S32*)pstSoftwareParam->stGetResultTmpBuf.u64VirAddr;
    HI_U32  grid_h = pstNnieParam->astSegData[0].astDst[0].unShape.stWhc.u32Chn;
    HI_U32  grid_w = pstNnieParam->astSegData[0].astDst[0].unShape.stWhc.u32Height;
    HI_U32  grid_stride= pstNnieParam->astSegData[0].astDst[0].u32Stride/sizeof(HI_S32);
    HI_U32  grid_c =pstNnieParam->astSegData[0].astDst[0].unShape.stWhc.u32Width;
    // printf("grid_h=%d,grid_w=%d,grid_stride=%d,grid_c=%d\n",grid_h,grid_w,grid_stride,grid_c);
    HI_U32 u32Offset=0;
    for (h = 0; h <grid_h ; h++)
    {    
        for (w = 0; w < grid_w; w++)
        {
            for (c = 0; c < grid_c; c++)
            {
                pf32Permute[u32Offset]= (HI_FLOAT)(aps32FeatureBlob[c+w*grid_stride+h*(grid_w*grid_stride)]) / SAMPLE_SVP_NNIE_QUANT_BASE;
                
                // pstFeatureInfo->af32feature[u32Offset]=pf32Permute[u32Offset];
                u32Offset=u32Offset+1;
            }       
        }
    }
    int N=512;
    float norm=l2norm(pf32Permute,N);
    for (i = 0; i <N ; i++)
    {
        pstFeatureInfo->af32feature[i]=pf32Permute[i]/norm;
    }


    // for (i = 0; i <N ; i++)
    // {
    //     printf("(%d):%f\n",i,pstFeatureInfo->af32feature[i]);
    // }


    return s32Ret;
}


HI_S32 SAMPLE_SVP_NNIE_GetResult(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam,SDC_SSD_RESULT_S *pstResult,SDC_SSD_OBJECT_INFO_S *pstObject)
{


    // printf("File(%s)  Line(%d): Src0 enType(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].enType);
    // printf("File(%s)  Line(%d): Src0 Chn(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Chn);
    // printf("File(%s)  Line(%d): Src0 Height(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Height);
    // printf("File(%s)  Line(%d): Src0 Width(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Width);
        // cv::Mat image(cv::Size(256, 114), CV_8UC3);
	// image.data =(HI_U8*)(sdc_nnie_forward.astSrc[0].u64VirAddr);
    // cv::imwrite("rgb_opencv_test.jpg",image);

    HI_S32 s32Ret;
    if (pstSoftwareParam->u32ModelType==0)
    {
        // printf("FCOSFace_GetResult pstSoftwareParam->u32ModelType=%d\n",pstSoftwareParam->u32ModelType);
        s32Ret =FCOSFace_GetResult(pstNnieParam,pstSoftwareParam,pstResult);
        return s32Ret;
    }
    else if (pstSoftwareParam->u32ModelType==1)
    {
        // printf("FCOSFaceLandmark_GetResult pstSoftwareParam->u32ModelType=%d\n",pstSoftwareParam->u32ModelType);
        s32Ret =FCOSFaceLandmark_GetResult(pstNnieParam,pstSoftwareParam,pstResult);
        return s32Ret;
    }
    else
    {
        printf("pstSoftwareParam->au32ModelType:%d NotImplemented Error\n",pstSoftwareParam->u32ModelType);
        return s32Ret;
    }

}



HI_S32 SAMPLE_FEATURE_NNIE_GetResult(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftwareParam,SDC_FEATURE_INFO_S *pstFeatureInfo)
{


    // printf("File(%s)  Line(%d): Src0 enType(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].enType);
    // printf("File(%s)  Line(%d): Src0 Chn(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Chn);
    // printf("File(%s)  Line(%d): Src0 Height(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Height);
    // printf("File(%s)  Line(%d): Src0 Width(%d)\n", __FILE__,__LINE__,pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Width);
        // cv::Mat image(cv::Size(256, 114), CV_8UC3);
	// image.data =(HI_U8*)(sdc_nnie_forward.astSrc[0].u64VirAddr);
    // cv::imwrite("rgb_opencv_test.jpg",image);
    HI_S32 s32Ret;
    if (pstSoftwareParam->u32ModelType==100)
    {
        s32Ret = DenseNet_GetResult(pstNnieParam,pstSoftwareParam,pstFeatureInfo);
        return s32Ret;
    }
    else
    {
        printf("pstSoftwareParam->au32ModelType:%d NotImplemented Error\n",pstSoftwareParam->u32ModelType);
        return s32Ret;
    }

    
}
