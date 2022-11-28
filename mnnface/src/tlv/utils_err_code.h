/******************************************************************************

                  ��Ȩ���� (C), 2001-2016, ��Ϊ�������޹�˾

 ******************************************************************************
  �� �� ��   : utils_err_code.h
  �� �� ��   : ����
  ��    ��   : �� �S/chenyun c00193875
  ��������   : 2016��07��20�� 12:05:57
  ����޸�   :
  ��������   : ����ģ�������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��07��20�� 12:05:57
    ��    ��   : �� �S/chenyun c00193875
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __UTILS_ERR_CODE_H__
#define __UTILS_ERR_CODE_H__

#include <stdlib.h>
#include "sdc.h"
// #include "utils_prf.h"

/* -----------------------------------------------------------------------------
 * ����ģ������붨�� 
 *----------------------------------------------------------------------------- */
#define ITGT_SUCCESS              (0)   // �ɹ�
#define ITGT_FAIL                 (-1)  // ʧ��
#define ITGT_NULL_PTR             (-2)  // ��ָ��
#define ITGT_OUT_OF_MEM           (-3)  // �ڴ治��
#define ITGT_RES_ALLOC_FAIL       (-4)  // ��Դ����ʧ��         �����ź����ȵ�
#define ITGT_CHAIN_PROC_NOT_START (-5)  // CHAIN          Processδ��ʼ
#define ITGT_CHAIN_INVALID_CMD    (-6)  // �Ƿ�CHAIN����
#define ITGT_INVALID_PARAM        (-7)  // �Ƿ�����
#define ITGT_BUF_IS_FULL          (-8)  // buf��
#define ITGT_BUF_IS_EMPTY         (-9)  // buf��
#define ITGT_ALARM_CB_FAIL        (-10) // �澯�ص�ʧ��
#define ITGT_TLV_RESULT_EXIST     (-11) // TLV����Ѵ���
#define ITGT_OBJ_NOT_FIND         (-12) // ���󲻴���
#define ITGT_BUF_TOO_SMALL        (-13) // buf̫С
#define ITGT_QUE_IS_FULL          (-14) // que��
#define ITGT_QUE_IS_EMPTY         (-15) // que��
#define ITGT_FD_CREATE_FAIL       (-16) // fd����ʧ��
#define ITGT_MSG_SEND_FAIL        (-17) // ��Ϣ����ʧ��
#define ITGT_MSG_RECV_FAIL        (-18) // ��Ϣ����ʧ��
#define ITGT_THREAD_CREATE_FAIL   (-19) // �̴߳���ʧ��
#define ITGT_CHAIN_START_FAIL     (-20) // CHAIN�߳�Startʧ��
#define ITGT_DRV_INTERFACE_FAIL   (-21) // ���������ӿ�ʧ��
#define ITGT_HNDL_UNINITIALIZED   (-22) // ���δ��ʼ��
#define ITGT_SYSCALL_FAIL         (-23) // ϵͳ����ʧ��
#define ITGT_CHAIN_NUM_TOO_MUCH   (-24) // CHAIN����̫��
#define ITGT_SET_ALG_PARAM_FAIL   (-25) // �����㷨����ʧ��
#define ITGT_GET_ALG_PARAM_FAIL   (-26) // ��ȡ�㷨����ʧ��
#define ITGT_REINIT_ALG_FAIL      (-27) // �㷨���³�ʼ��ʧ��
#define ITGT_LOADFACEKEY_FAIL     (-28) // �����������ݿ���Կʧ��

/* -----------------------------------------------------------------------------
 * ��������غ����ӿ�
 *----------------------------------------------------------------------------- */
/*****************************************************************************
 �� �� ��  : ITGT_GetErrString
 ��������  : ��ȡ����ģ���������
 �������  : INT32 nErrCode  ������
 �������  : VOID
 �� �� ֵ  : const CHAR *
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��07��20�� 12:27:17
    ��    ��   : �� �S/chenyun c00193875
    �޸�����   : �����ɺ���

*****************************************************************************/
const CHAR *ITGT_GetErrString(INT32 nErrCode);

#endif /* __UTILS_ERR_CODE_H__ */
