

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


#include "sdc_fd_algorithm.h"
#include "sdc_fd_utils.h"
#include "sample_comm_nnie.h"



int SDC_ModelDecript(sdc_mmz_alloc_s *pstMmzAddr)
{
    if(pstMmzAddr == NULL)
    {
        printf("Err in SDC_ModelDecript, pstMmzAddr is null\n");
        return ERR;
    }    
    return OK;
}

int SDC_UnLoadModel(int fd_algorithm,SVP_NNIE_MODEL_S *pstModel)
{
    int nRet = -1;
    if (NULL != pstModel)
    {
        sdc_common_head_s head;
        struct iovec iov[2] = {
        [0] = {.iov_base = &head , .iov_len = sizeof(head)},
        [1] = {.iov_base = pstModel, .iov_len = 
        sizeof(SVP_NNIE_MODEL_S)}};
        // fill head struct 
        memset(&head, 0, sizeof(head));
        head.version = SDC_VERSION;
        head.url = SDC_URL_NNIE_MODEL;
        head.method = SDC_METHOD_DELETE;
        head.head_length = sizeof(head);
        head.content_length = sizeof(SVP_NNIE_MODEL_S);
        nRet = writev(fd_algorithm, iov, sizeof(iov)/sizeof(iov[0]));
        if (nRet < 0)
        {
            fprintf(stdout,"Errin SDC_UnLoadModel:failed to unload nnie module!\n");
        } 
    }
    else
    {
        fprintf(stdout,"Err in SDC_UnLoadModel:module pointer is NULL!\n");
    }
    
    return 0;
}




int SDC_LoadModel(int fd_algorithm,int fd_utils,unsigned int uiLoadMode, char *pucModelFileName, SVP_NNIE_MODEL_S *pstModel,SAMPLE_SVP_NNIE_MODEL_S *ps_stModel)
{
    int s32Ret = 0;
    int ret = 0;
    sdc_extend_head_s* extend_head;
    unsigned int uFileSize;
    sdc_mmz_alloc_s stMmzAddr;
    if ((NULL == pstModel) || (NULL == pucModelFileName))
    {
        fprintf(stdout,"Err in SDC_LoadModel, pstModel or pucModelFileName is null\n");
        return -1;
    }
    
    // fprintf(stdout,"Start Load model, pucModelFileName:%s!\n", pucModelFileName);

    char buf[1024] = {0};
    sdc_common_head_s *phead = (sdc_common_head_s *)buf;
    phead->version = SDC_VERSION;
    phead->url = SDC_URL_NNIE_MODEL;
    phead->method = SDC_METHOD_CREATE;
    phead->head_length = sizeof(sdc_common_head_s);
    phead->content_length = MAX_MODULE_PATH;


    /*模式 0，不带扩展头，默认内存方式加载*/
    if (uiLoadMode == 1)/*模式 1，带扩展头，扩展头参数指定为内存方式加载*/
    {
        FILE *fp = fopen(pucModelFileName, "rb");
        if(fp == NULL)
        {
            fprintf(stdout,"modelfile fopen %s fail!\n", pucModelFileName);
            return -1;
        }
        ret = fseek(fp,0L,SEEK_END);
        if(ret != 0)
        {
            fprintf(stdout,"check nnie file SEEK_END, fseek fail.");
            fclose(fp);
            return -1;
        }
        
        uFileSize = ftell(fp);
        ret = fseek(fp,0L,SEEK_SET);
        if(0 != ret)
        {
            fprintf(stdout,"check nnie file SEEK_SET, fseek fail.");
            fclose(fp);
            return -1;
        }

        stMmzAddr.size = uFileSize;
        ret = SDC_MemAlloc(fd_utils, uFileSize, 0, &stMmzAddr); // param 2: 0 no cache, 1 cache
        if(ret != stMmzAddr.size)
        {
            fprintf(stdout,"SDC_MmzAlloc ret %d, readsize %d", ret, stMmzAddr.size);
            return -1;
        }
        ret = fread((HI_VOID*)(uintptr_t)stMmzAddr.addr_virt, 1, stMmzAddr.size, fp);
        if(ret != stMmzAddr.size)
        {
            fprintf(stdout,"filesize %d, readsize %d", ret, stMmzAddr.size);
            return -1;
        }
        /*用户执行调用算法程序对传入文件进行解码*/
        if(SDC_ModelDecript(&stMmzAddr))
        {
            fprintf(stdout,"SDC_ModelDecript Fail!");
            return -1;
        }
        extend_head = (sdc_extend_head_s *)&buf[phead->head_length];
        extend_head->type = 1;//NNIE_NNIE_MODEL_OP
        extend_head->length = sizeof(*extend_head);
        extend_head->reserve = 0;/*0 或者不带是内存方式，1 是文件名方式*/
        phead->head_length += sizeof(sdc_extend_head_s);
        // iov[1].iov_base = &stMmzAddr; 
    }
    else
    {
        fprintf(stdout,"Not Implement uiLoadMode! \n");
        return -1;
    }


    struct iovec iov[2] = 
    {
        [0] = { .iov_base = buf, .iov_len = sizeof(sdc_common_head_s)+ sizeof(sdc_extend_head_s)},
        [1] = { .iov_base = &stMmzAddr,.iov_len = MAX_MODULE_PATH}
    };


    s32Ret = writev(fd_algorithm, iov, 2);
    if (s32Ret < 0)
    {
        fprintf(stdout,"creat nnie,write to algorithm.iaas.sdc fail: %m\n");
        return -1;
    }
    
    /*模型加载后立即释放*/
    // if (uiLoadMode < 2)mmz_free(fd_config, &stMmzAddr);
    struct rsp_strcut 
    {
        sdc_common_head_s head;
        SVP_NNIE_MODEL_S model;
    }rsp_strcut_tmp;

    s32Ret = read(fd_algorithm, &rsp_strcut_tmp, sizeof(rsp_strcut_tmp));
    if(s32Ret == -1)
    {
        fprintf(stdout,"get_channel_data fail: %m\n");
        return -1;
    }
    if(s32Ret > sizeof(rsp_strcut_tmp))
    {
        fprintf(stdout,"get_channel_data truncated, data len: %d > %zu\n", s32Ret, sizeof(rsp_strcut_tmp));
        return -1;
    }
    if (s32Ret < 0 || rsp_strcut_tmp.head.code != SDC_CODE_200 || rsp_strcut_tmp.head.content_length <= 0)
    {
        fprintf(stdout,"get nnie create response, read from algorithm.iaas.sdc fail,s32Ret:%d, code=%d,length=%d\n", 
            s32Ret, rsp_strcut_tmp.head.code, rsp_strcut_tmp.head.content_length);
        return -1;
    }
    else 
    {
        ps_stModel->stModel = rsp_strcut_tmp.model;
        memcpy(pstModel, &rsp_strcut_tmp.model,sizeof(SVP_NNIE_MODEL_S));
        // fprintf(stdout, "Load model Suscess!\n"); 
    }
    
    return OK;
}




int SDC_TransYUV2RGB(int fd_algorithm, sdc_yuv_frame_s *yuv, sdc_yuv_frame_s *rgb)
{
    sdc_common_head_s head;
    int nRet;
    struct iovec iov[2];

        
    // fill head struct 
    memset(&head, 0, sizeof(head));
    head.version = SDC_VERSION;
    head.url = 3;
    head.method = SDC_METHOD_CREATE;
    head.head_length = sizeof(head);
    head.content_length = sizeof(sdc_yuv_frame_s);

    iov[0].iov_base = &head;
    iov[0].iov_len = sizeof(head);
    iov[1].iov_base = yuv;
    iov[1].iov_len = sizeof(sdc_yuv_frame_s);

    // write request
    nRet = writev(fd_algorithm, iov, sizeof(iov)/sizeof(iov[0]));
    if (nRet < 0)
    {
        fprintf(stdout,"Error:failed SDC_TransYUV2RGB,nRet:%d!\n",nRet);
        return ERR;
    }
    // read response
    iov[1].iov_base = rgb;
    iov[1].iov_len = sizeof(sdc_yuv_frame_s);
    nRet = readv(fd_algorithm, iov, 2);
    if (head.code != SDC_CODE_200 || nRet < 0 || head.content_length != sizeof(sdc_yuv_frame_s))
    {
        fprintf(stdout,"Err:SDC_TransYUV2RGB,nRet:%d,rsp_head.code:%d!\n",
            nRet, head.code);
        return ERR;
    } 
    return OK;

}


int SDC_TransYUV2RGBRelease(int fd_algorithm, sdc_yuv_frame_s *rgb)
{
    sdc_common_head_s head;
    int nRet;
    struct iovec iov[2];

    // fill head struct 
    memset(&head, 0, sizeof(head));
    head.version = SDC_VERSION;
    head.url = 3;
    head.method = SDC_METHOD_DELETE;
    head.head_length = sizeof(head);
    head.content_length = sizeof(sdc_yuv_frame_s);

    iov[0].iov_base = &head;
    iov[0].iov_len = sizeof(head);
    iov[1].iov_base = rgb;
    iov[1].iov_len = sizeof(sdc_yuv_frame_s);

    // write request
    nRet = writev(fd_algorithm, iov, sizeof(iov)/sizeof(iov[0]));
    if (nRet < 0)
    {
        fprintf(stdout,"Error:failed SDC_TransYUV2RGBRelease,nRet:%d!\n",nRet);
        return ERR;
    }
    
    return OK;
}





