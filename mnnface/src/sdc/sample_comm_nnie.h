#ifndef __SAMPLE_COMM_NNIE_H__
#define __SAMPLE_COMM_NNIE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "hi_nnie.h"
#include "mpi_nnie.h"
#include "sample_comm_svp.h"



#define SAMPLE_SVP_NNIE_MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define SAMPLE_SVP_NNIE_MIN(a,b)    (((a) < (b)) ? (a) : (b))

/*16Byte align*/
#define SAMPLE_SVP_NNIE_ALIGN_16 16
#define SAMPLE_SVP_NNIE_ALIGN16(u32Num) ((u32Num + SAMPLE_SVP_NNIE_ALIGN_16-1) / SAMPLE_SVP_NNIE_ALIGN_16*SAMPLE_SVP_NNIE_ALIGN_16)
/*32Byte align*/
#define SAMPLE_SVP_NNIE_ALIGN_32 32
#define SAMPLE_SVP_NNIE_ALIGN32(u32Num) ((u32Num + SAMPLE_SVP_NNIE_ALIGN_32-1) / SAMPLE_SVP_NNIE_ALIGN_32*SAMPLE_SVP_NNIE_ALIGN_32)


#define SAMPLE_SVP_COORDI_NUM                   4        /*num of coordinates*/

#define SAMPLE_SVP_NNIE_COORDI_NUM  			4      /*coordinate numbers*/
#define SAMPLE_SVP_NNIE_EACH_SEG_STEP_ADDR_NUM  2
#define SAMPLE_SVP_NNIE_QUANT_BASE              4096    /*the base value*/

#define SAMPLE_SVP_NNIE_SSD_REPORT_NODE_NUM     12
#define SAMPLE_SVP_NNIE_SSD_BOX_NUM         	4
#define SAMPLE_SVP_NNIE_SSD_SCORE_NUM         	4
#define SAMPLE_SVP_NNIE_SSD_LANDMARK_NUM       	4


#define SAMPLE_SVP_NNIE_EACH_BBOX_INFER_RESULT_NUM  4
#define SAMPLE_SVP_NNIE_EACH_CLASS_INFER_RESULT_NUM 2
#define SAMPLE_SVP_NNIE_EACH_LANDMARK_INFER_RESULT_NUM  10

#define SDC_IN
#define SDC_OUT
#define SDC_IN_OUT



typedef struct hiSAMPLE_SVP_NNIE_MODEL_S
{
    SVP_NNIE_MODEL_S    stModel;
    SVP_MEM_INFO_S      stModelBuf;//store Model file
}SAMPLE_SVP_NNIE_MODEL_S;


/*each seg input and output memory*/
typedef struct hiSAMPLE_SVP_NNIE_SEG_DATA_S
{
	SVP_SRC_BLOB_S astSrc[SVP_NNIE_MAX_INPUT_NUM];
	SVP_DST_BLOB_S astDst[SVP_NNIE_MAX_OUTPUT_NUM];
}SAMPLE_SVP_NNIE_SEG_DATA_S;



/*NNIE Execution parameters */
typedef struct hiSAMPLE_SVP_NNIE_PARAM_S
{
    SVP_NNIE_MODEL_S*    pstModel;
    HI_U32 u32TmpBufSize;
    HI_U32 au32TaskBufSize[SVP_NNIE_MAX_NET_SEG_NUM];
    //SVP_MEM_INFO_S      stTaskBuf;
	//SVP_MEM_INFO_S      stTmpBuf;
    SVP_MEM_INFO_S      stStepBuf;//store Lstm step info
    SAMPLE_SVP_NNIE_SEG_DATA_S astSegData[SVP_NNIE_MAX_NET_SEG_NUM];//each seg's input and output blob
    SVP_NNIE_FORWARD_CTRL_S astForwardCtrl[SVP_NNIE_MAX_NET_SEG_NUM];
	SVP_NNIE_FORWARD_WITHBBOX_CTRL_S astForwardWithBboxCtrl[SVP_NNIE_MAX_NET_SEG_NUM];
}SAMPLE_SVP_NNIE_PARAM_S;


/*each seg input and output data memory size*/
typedef struct hiSAMPLE_SVP_NNIE_BLOB_SIZE_S
{
	HI_U32 au32SrcSize[SVP_NNIE_MAX_INPUT_NUM];
	HI_U32 au32DstSize[SVP_NNIE_MAX_OUTPUT_NUM];
}SAMPLE_SVP_NNIE_BLOB_SIZE_S;

/*stack for sort*/
typedef struct hiSAMPLE_SVP_NNIE_STACK
{
    HI_S32 s32Min;
    HI_S32 s32Max;
} SAMPLE_SVP_NNIE_STACK_S;


/*NNIE configuration parameter*/
typedef struct hiSAMPLE_SVP_NNIE_CFG_S
{
	HI_U8 *pszBGR;
	HI_U8 *pszYUV;
    HI_CHAR *pszPic;
    HI_U32 u32MaxInputNum;
    HI_U32 u32MaxRoiNum;
    HI_U64 au64StepVirAddr[SAMPLE_SVP_NNIE_EACH_SEG_STEP_ADDR_NUM*SVP_NNIE_MAX_NET_SEG_NUM];//virtual addr of LSTM's or RNN's step buffer
	SVP_NNIE_ID_E	aenNnieCoreId[SVP_NNIE_MAX_NET_SEG_NUM];
}SAMPLE_SVP_NNIE_CFG_S;

/*SSD software parameter*/
typedef struct hiSAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S
{
	/*----------------- Model Parameters ---------------*/
	int u32ModelType;

	HI_U32 au32NodeNum;
	HI_U32 au32BoxNum;
	HI_U32 au32ScoreNum;
	HI_U32 au32LandmarkNum;


	HI_U32 au32GridHeight[12];
	HI_U32 au32GridWidth[12];
	HI_U32 au32GridChannel[12];

	HI_U32 u32BboxNumEachGrid[5];
	HI_FLOAT af32BoxStep[5];
	HI_FLOAT af32BoxMinSize[5][4];
	
	HI_FLOAT af32BoxMinSizeX[5][4];
	HI_FLOAT af32BoxMinSizeY[5][4];

	HI_U32 u32MaxBoxBlobSize;
	HI_U32 u32MaxScoreBlobSize;
	HI_U32 u32MaxLandmarkBlobSize;
	HI_U32 u32TotalBboxSize;
	HI_U32 u32TotalLandmarkSize;
	HI_U32 u32TotalBboxNum;


	HI_U32 u32OriImHeight;
	HI_U32 u32OriImWidth;

	HI_FLOAT f32ConfThresh;
	HI_DOUBLE f64NmsThresh;
	HI_U32 u32BboxNum;
	HI_U32 u32maxNumOfFace;

	SVP_MEM_INFO_S stGetResultTmpBuf;
	
}SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S;






/*NNIE input or output data index*/
typedef struct hiSAMPLE_SVP_NNIE_DATA_INDEX_S
{
	HI_U32 u32SegIdx;
	HI_U32 u32NodeIdx;
}SAMPLE_SVP_NNIE_DATA_INDEX_S;

/*this struct is used to indicate the input data from which seg's input or report node*/
typedef SAMPLE_SVP_NNIE_DATA_INDEX_S  SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S;
/*this struct is used to indicate which seg will be executed*/
typedef SAMPLE_SVP_NNIE_DATA_INDEX_S  SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S;


typedef struct SDC_SSD_INPUT_SIZE_S
{
	HI_U32 ImageWidth;
    HI_U32 ImageHeight;
}SDC_SSD_INPUT_SIZE_S;

typedef struct hiSAMPLE_SVP_NNIE_BBOX
{
    HI_FLOAT f32Xmin;
    HI_FLOAT f32Xmax;
    HI_FLOAT f32Ymin;
    HI_FLOAT f32Ymax;
    HI_FLOAT f32Score;
    // HI_U32 u32ClassIdx;
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
	
    HI_U32 u32Mask;
}SAMPLE_SVP_NNIE_BBOX_S;

typedef struct hiSAMPLE_SVP_NNIE_LANDMARK
{
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

}SAMPLE_SVP_NNIE_LANDMARK_S;

typedef struct SDC_SSD_OBJECT_INFO_S
{
	HI_FLOAT f32Xmin;
    HI_FLOAT f32Xmax;
    HI_FLOAT f32Ymin;
    HI_FLOAT f32Ymax;
	HI_U32   u32Mask;
	HI_U32   u32Quality;
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


}SDC_SSD_OBJECT_INFO_S;


typedef struct SDC_FEATURE_INFO_S
{
	HI_U32 N=512;
	HI_FLOAT af32feature[512];
}SDC_SSD_FEATURE_INFO_S;




typedef struct SDC_SSD_RESULT_S
{
	SDC_IN_OUT HI_U32 numOfObject;
	SDC_IN HI_FLOAT thresh;
	SDC_OUT SDC_SSD_OBJECT_INFO_S *pObjInfo;
}SDC_SSD_RESULT_S;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SAMPLE_COMM_SVP_H__ */

