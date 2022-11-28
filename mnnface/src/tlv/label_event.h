/******************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   : label_event.h
  版 本 号   : 初稿
  作    者   : athina
  生成日期   : 2020年7月4日
  最近修改   :
  功能描述   : TLV消息格式定义
  函数列表   :
  修改历史   :
  1.日    期   : 2020年7月4日
    作    者   : athina
    修改内容   : 创建文件
******************************************************************************/
#ifndef __LABEL_EVENT_H__
#define __LABEL_EVENT_H__

#include <stdint.h>
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */
//坐标
typedef struct {
    uint32_t x;
    uint32_t y;
} point;

//多边形
typedef struct {
    int32_t color;//rgb
    int32_t edge_width;//0-default
    uint32_t attr;//0x01-是否填充底色
    int32_t bottom_color; //底色颜色
    int32_t transparency; //底色透明度
    int32_t iPointcnt;//点的个数，起点只需包含一次
    point points[0];
} polygon;

//文字标注
typedef struct {
    int32_t color;//rgb
    char font[32];//按照渲染库能力定义
    int32_t size;//文字大小
    point pos;//文字左上角位置
    int32_t len;
    char str[0];
} tag;

//标注事件
typedef struct {
    uint32_t add_flag; //添加 || 删除
    char app_name[32];
    uint64_t id;
    uint16_t polygon_cnt;
    polygon polygons[0];
    uint8_t tag_cnt;
    tag tags[0];
    uint8_t ttl;//default 1 second
} label;

// 发送元数据
extern int SDC_LabelEventDel(int fp, unsigned int baseid, unsigned int id, char *cAppName);
extern int SDC_LabelEventPublish(int fp, unsigned int baseid, int iDataLen, char *cEventMsg, uint64_t pts);

#ifdef __cplusplus
#if __cplusplus
}
#endif 
#endif/* End of #ifdef __cplusplus */

#endif  /* __LABEL_EVENT_H__ */