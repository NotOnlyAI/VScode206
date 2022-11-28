
/******************************************************************************
                  版权所有 (C), 2019-2029, SDC OS 开源软件小组所有

 ******************************************************************************
  文 件 名   : sdc_def_ext.h
  版 本 号   : 初稿
  作    者   :  s30001871
  生成日期   : 2020年7月4日
  最近修改   :
  功能描述   : 常用宏定义
  函数列表   :
  修改历史   :
  1.日    期   : 2020年7月4日
    作    者   :  s30001871
    修改内容   : 创建文件

******************************************************************************/
#ifndef __SDC_DEF_EXT_H__
#define __SDC_DEF_EXT_H__
#include "sample_comm_nnie.h"



#define         DEMOYOLOLOG                    "DEMOYOLOLOG#"         // 元数据转换模块

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_DEBUG(fmt, arg...) do { \
    fprintf(stdout, "[%s][%04d][%s]" fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##arg); \
} while(0)

#define LOG_ERROR(fmt, arg...) do { \
    fprintf(stderr, "[%s][%04d][%s]" fmt "\n",  __FILE__, __LINE__, __FUNCTION__, ##arg); \
} while(0)

#define LOG_INFO_PRINT(model, fmt, args...) do { \
    fprintf(stdout,"ERRO (%s|%s|%d)[%s]:" fmt, __FILE__, __FUNCTION__, __LINE__, model, ##args); \
} while(0)

#define LOG_ASSERT(fmt, arg...)


#define ITGT_RETURN_DO_IF_FAIL(model, expr, func, msgfmt, args...) \
    /*lint -save -e717*/ do {                                        \
        if (!(expr)) {                                             \
            LOG_ERROR(msgfmt, ##args);               \
            func();                                                \
            return;                                                \
        }                                                          \
    } while (0) /*lint -restore*/
#define ITGT_RETURN_IF_FAIL(model, expr, msgfmt, ...)     \
    /*lint -save -e717*/ do {                               \
        if (!(expr)) {                                    \
            LOG_INFO_PRINT(model, msgfmt, __VA_ARGS__); \
            return;                                       \
        }                                                 \
    } while (0) /*lint -restore*/

#define ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(model, expr, ret, msgfmt, args...)                 \
    /*lint -save -e717*/ do {                                                                 \
        if (!(expr)) {                                                                      \
           fprintf(stderr, "[%s] [%s][%04d][%s]" msgfmt "\n",model,  __FILE__, __LINE__, __FUNCTION__, ##args); \
            return ret;                                                                     \
        }                                                                                   \
    } while (0) /*lint -restore*/


#define ITGT_ERROR_PRINT(model, dbgflag, fmt, ...) do { \
    printf("error ITGT_ERROR_PRINT");\
} while(0)

#define ITGT_INFO_PRINT(model, fmt, args...) do { \
    printf("error ITGT_INFO_PRINT");\
} while(0)

#define ITGT_LOG_IF_FAIL(model, expr, msgfmt, args...) \
    /*lint -save -e717*/ do {                            \
        if (!(expr)) {                                 \
            ITGT_ERROR_PRINT(model, msgfmt, ##args);   \
        }                                              \
    } while (0) /*lint -restore*/

#define ITGT_RETURN_VAL_IF_FAIL(model, expr, ret, msgfmt, ...) \
    /*lint -save -e717*/ do {                                    \
        if (!(expr)) {                                         \
            ITGT_ERROR_PRINT(model, msgfmt, __VA_ARGS__);      \
            return ret;                                        \
        }                                                      \
    } while (0) /*lint -restore*/




#ifdef __cplusplus
}
#endif
#endif /* __SDC_DEF_EXT_H__ */