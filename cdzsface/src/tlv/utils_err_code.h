/******************************************************************************

                  版权所有 (C), 2001-2016, 华为技术有限公司

 ******************************************************************************
  文 件 名   : utils_err_code.h
  版 本 号   : 初稿
  作    者   : 陈 赟/chenyun c00193875
  生成日期   : 2016年07月20日 12:05:57
  最近修改   :
  功能描述   : 智能模块错误码
  函数列表   :
  修改历史   :
  1.日    期   : 2016年07月20日 12:05:57
    作    者   : 陈 赟/chenyun c00193875
    修改内容   : 创建文件

******************************************************************************/
#ifndef __UTILS_ERR_CODE_H__
#define __UTILS_ERR_CODE_H__

#include <stdlib.h>
#include "sdc.h"
// #include "utils_prf.h"

/* -----------------------------------------------------------------------------
 * 智能模块错误码定义 
 *----------------------------------------------------------------------------- */
#define ITGT_SUCCESS              (0)   // 成功
#define ITGT_FAIL                 (-1)  // 失败
#define ITGT_NULL_PTR             (-2)  // 空指针
#define ITGT_OUT_OF_MEM           (-3)  // 内存不足
#define ITGT_RES_ALLOC_FAIL       (-4)  // 资源分配失败         锁、信号量等等
#define ITGT_CHAIN_PROC_NOT_START (-5)  // CHAIN          Process未开始
#define ITGT_CHAIN_INVALID_CMD    (-6)  // 非法CHAIN命令
#define ITGT_INVALID_PARAM        (-7)  // 非法参数
#define ITGT_BUF_IS_FULL          (-8)  // buf满
#define ITGT_BUF_IS_EMPTY         (-9)  // buf空
#define ITGT_ALARM_CB_FAIL        (-10) // 告警回调失败
#define ITGT_TLV_RESULT_EXIST     (-11) // TLV结果已存在
#define ITGT_OBJ_NOT_FIND         (-12) // 对象不存在
#define ITGT_BUF_TOO_SMALL        (-13) // buf太小
#define ITGT_QUE_IS_FULL          (-14) // que满
#define ITGT_QUE_IS_EMPTY         (-15) // que空
#define ITGT_FD_CREATE_FAIL       (-16) // fd创建失败
#define ITGT_MSG_SEND_FAIL        (-17) // 消息发送失败
#define ITGT_MSG_RECV_FAIL        (-18) // 消息接收失败
#define ITGT_THREAD_CREATE_FAIL   (-19) // 线程创建失败
#define ITGT_CHAIN_START_FAIL     (-20) // CHAIN线程Start失败
#define ITGT_DRV_INTERFACE_FAIL   (-21) // 调用驱动接口失败
#define ITGT_HNDL_UNINITIALIZED   (-22) // 句柄未初始化
#define ITGT_SYSCALL_FAIL         (-23) // 系统调用失败
#define ITGT_CHAIN_NUM_TOO_MUCH   (-24) // CHAIN个数太多
#define ITGT_SET_ALG_PARAM_FAIL   (-25) // 设置算法参数失败
#define ITGT_GET_ALG_PARAM_FAIL   (-26) // 获取算法参数失败
#define ITGT_REINIT_ALG_FAIL      (-27) // 算法重新初始化失败
#define ITGT_LOADFACEKEY_FAIL     (-28) // 加载人脸数据库密钥失败

/* -----------------------------------------------------------------------------
 * 错误码相关函数接口
 *----------------------------------------------------------------------------- */
/*****************************************************************************
 函 数 名  : ITGT_GetErrString
 功能描述  : 获取智能模块错误描述
 输入参数  : INT32 nErrCode  错误码
 输出参数  : VOID
 返 回 值  : const CHAR *
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年07月20日 12:27:17
    作    者   : 陈 赟/chenyun c00193875
    修改内容   : 新生成函数

*****************************************************************************/
const CHAR *ITGT_GetErrString(INT32 nErrCode);

#endif /* __UTILS_ERR_CODE_H__ */
