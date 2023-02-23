

#ifndef __MAIN_H__
#define __MAIN_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

enum SYSTEM_STATE_E
{
    SYS_STATE_IDLE,
    SYS_STATE_STARTING,
    SYS_STATE_NORMAL,
    SYS_STATE_ABORT
};

typedef struct SYSTEM_MANAGE_PARAS_STRU
{
    unsigned int uiSystemState;
    unsigned int uiMaxUsedBufNum;/*��󻺴�buf����*/
    
}SYSTEM_MANAGE_PARAS_S;

typedef struct META_INFO_STRU
{
    unsigned short usX;      
    unsigned short usY;      
    unsigned short usWidth;  
    unsigned short usHeight; 
    unsigned int uclass;
    float confidence;
}META_INFO_S;
 
/* �����ṹ�� */
struct member
{
    int num;
    char *name;
};     


extern int main(int argc,char* argv[]);
extern void * SDC_ReadFromVideoService(void *arg);
extern void * SDC_YuvDataProc(void *arg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __MAIN_H__ */
