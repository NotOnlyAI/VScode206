/******************************************************************************

                  版权所有 (C), 2001-2016, 华为技术有限公司

 ******************************************************************************
  文 件 名   : intelligent_alg_metadata.h
  版 本 号   : 初稿
  作    者   : 陈 赟/chenyun c00193875
  生成日期   : 2016年08月04日 17:10:37
  最近修改   :
  功能描述   : 智能元数据TLV格式定义
  函数列表   :
  修改历史   :
  1.日    期   : 2016年08月04日 17:10:37
    作    者   : 陈 赟/chenyun c00193875
    修改内容   : 创建文件
  2.date       : 2020-02-20 
    Author     : lupengfei/l00427296
    Description: add common tlv type
******************************************************************************/
#ifndef __INTELLIGENT_ALG_METADATA_H__
#define __INTELLIGENT_ALG_METADATA_H__

// #include "generictype.h"
#include "sdc.h"

#define MAX_POINT_NUM (10)

// 元数据采用三层TLV结构嵌套，
// 具体设计参见《IPC V200R003C00 智能元数据结构设计说明书》

// 第一层元数据TYPE
typedef enum LAYER_ONE_TYPE
{
    METADATA_TYPE = 0x4154454D, // 'M''E''T''A' 小端序
} LAYER_ONE_TYPE_E;

// 第二层元数据TYPE
typedef enum LAYER_TWO_TYPE
{
    COMMON  = 0x00000001, // 通用
    TARGET  = 0x00000002, // 目标(车、人、人脸等等) 
    RULE    = 0x00000003, // 规则(设定的规则框)
    TALARM  = 0x00000004, // 智能报警
    TRECORD = 0x00000005,  // 智能触发录像
    TRAFFIC_LIGHT = 0x00000006 //信号灯状态 
} LAYER_TWO_TYPE_E;

/***** 元数据类型描述信息
第三层元数据TYPE
  类别 保留 类型    
0x 00  0000 00

高2位表示数据类别
0x01 BOOL
0x02 CHAR
0x03 UCHAR
0x04 INT16
0x05 UINT16
0x06 INT32
0x07 UINT32
0x08 INT64
0x09 UINT64
0x0A 二进制序列
0x0B 矩形 见结构体META_RECT_S
0x0C 点 见结构体META_POINT_S
0x0D 线 见结构体META_LINE_S
0x0E 多边形 见结构体META_POLYGON_S
0x0F 颜色 见结构体META_COLOR_S
0x10 人体属性 HUMAN_ATTRIBUTES 不建议使用 建议使用单属性 以0X07000300开始
0x11 人脸属性 FACE_ATTRIBUTES  不建议使用 建议使用单属性 以0X07000200开始
0x12 人脸信息（数据库中信息）
0x13 骑行人属性 RIDERMAN_ATTRIBUTES 不建议使用 建议使用单属性 以0X07000400开始

0x20 字符串STRING128 最长128
0x21 字符串对STRING128STRING128 最长128*2;'\0' 作为字符串结束标识 etc: 'glass\0yes\0'
检索到第一个'\0'为key 第二个为value

0x22 UUID(UINT128) 相机唯一值

中间4位暂时保留
低2位表示具体类型
元数据类型描述信息 ****/

typedef enum LAYER_THREE_TYPE {
    PTS                 = 0x09000001, // 时间戳
    ITGT_TYPE           = 0x07000011, // 智能类型
    IMG_WIDTH           = 0x07000100, // 处理图片宽
    IMG_HEIGHT          = 0x07000101, // 处理图片高
    RULE_MASK           = 0x09000007, // 规则掩码

    FACE_SCORE          = 0x04000013, // 人脸置信度
    FACE_ANGLE          = 0x04000014, // 人脸角度
    FACE_ID             = 0x07000016, // 人脸ID
    FACE_PANOPIC_SIZE   = 0x07000018, // 人脸全景图片大小
    FACE_FACEPIC_SIZE   = 0x07000019, // 人脸抠图图片大小
    FACE_PIC_TIME       = 0x08000015, // 人脸抠图产生时间
    FACE_PIC_TZONE      = 0x08000020, // 人脸抠图设备时区(单位ms 东区为+ 西区为-)
    HUMAN_FEATURE       = 0x10000002, // 人体属性
    FACE_FEATURE		= 0x11000003, // 人脸属性
    RIDERMAN_FEATURE    = 0x13000001, // 骑行人属性
    PANORAMA_PIC        = 0x0A00000A, // 全景图片
    THERMAL_PIC         = 0x0A00000D, // 热成像全景图
    FACE_PIC            = 0x0A000012, // 人脸抠图
	FACE_PIC_KPS		= 0x07000012, // 人脸抠图kps质量过滤标志位
    HUMAN_PIC           = 0x0A000013, // 人体抠图
	HUMAN_PIC_KPS		= 0x07000013, // 人体抠图kps质量过滤标志位
	HUMAN_PIC_ROI       = 0x0B000017, // 人体抠图中的人体目标框
    FACE_PANORAMA       = 0x0A000017, // 人脸全景
    FACE_PIC_POSITION   = 0x0B000011, // 人脸抠图小框位置
    FACE_POS            = 0x0B000012, // 人脸位置(实时位置框)
    HUMAN_RECT          = 0x0B000013, // 人体位置(实时位置框)
    HUMAN_RECT_POSITION = 0x0B000014, // 人体抠图小框位置
    SHOULDER_RECT       = 0x0B000018, // 头肩位置
    SHOULDER_NUM        = 0x06000001, // 头肩个数
    QUEUE_TIME          = 0x06000002, // 排队时长
    FACE_MATCH          = 0x0A000014, // 人脸数据库中匹配图片
    FACELIB_RECORDID    = 0x07000017, // 名单库中的人脸ID，用来维持特征 record的一致性
    FACE_MATCHRATE      = 0x07000020, // 人脸匹配率
    FACE_INFO           = 0x12000001, // 人脸信息,对应数据库中信息
    FACE_LIB_TYPE       = 0x07000022, // 名单库类型
    FACE_LIB_NAME       = 0x0A000015, // 名单库名字
    TARGET_TYPE         = 0x07000023, // target类型，所有智能的业务类型
    FACE_LIB_ID         = 0x07000024, // 名单库ID
    MMC_FACE_COMPARE_NUM_MAX = 0x07000025, //多机协同算法参数- 人脸比对数据
    MMC_FACE_WARNING_RECALL_RATE_MAX = 0x07000026, //多机协同算法参数- 人脸上报告警的召回率
    MMC_FACE_WARNING_RECALL_RATE_MIN = 0x07000027, //多机协同算法参数- 人脸预警率的下限值
    FACE_TEMPERATURE = 0x07000050,    // 人脸温度信息
    FACE_TEMPERATURE_UNIT = 0x07000051,    // 人脸温度单位(0摄氏度 1华氏度 2开尔文)

    OBJ_ID              = 0x07000021, // 目标ID
    GLOBAL_OBJID        = 0x09000082, // 智能目标全局ID(人脸 人体 机动车 非机动车 ITS 微卡) 
    OBJ_STATUS          = 0x06000022, // 目标状态
    OBJ_POS             = 0x0B000023, // 目标位置
    OBJ_TYPE            = 0x06000024, // 目标类型
    OBJ_SPEED           = 0x0C000025, // 目标速度
    OBJ_UPHALF_COLOR    = 0x0F000026, // 目标上半部颜色
    OBJ_DOWNHALF_COLOR  = 0x0F000027, // 目标下半部颜色
    RULE_TYPE           = 0x07000031, // 规则类型
    RULE_LINE_POS       = 0x0D000032, // 规则线位置
    RULE_LINE_DIR       = 0x07000033, // 规则线方向
    RULE_AREA_POS       = 0x0E000034, // 规则框位置
    OBJ_POS_R           = 0x0B000035, // 目标位置(相对位置)
    OBJ_SPEED_R         = 0x0C000036, // 目标速度(相对位置)
    RULE_LINE_POS_R     = 0x0D000037, // 规则线位置(相对位置)
    RULE_AREA_POS_R     = 0x0E000038, // 规则框位置(相对位置)
	HUMAN_COUNT_IN      = 0x07000709,     // 进入人数
	HUMAN_COUNT_OUT     = 0x0700070A,     // 离开人数
	HUMAN_TIME_START    = 0x09000050,    // 开始计数时间
    HUMANCOUNT_ALL_IN_NUM   = 0x07000900,        // 过线计数进入总人数
    HUMANCOUNT_ALL_OUT_NUM  = 0x07000901,        // 过线计数离开总人数
    HUMANCOUNT_EACH_IN_NUM  = 0x07000902,        // 过线计数进入增量
    HUMANCOUNT_EACH_OUT_NUM = 0x07000903,        // 过线计数离开增量

    LANE_ID             = 0x07000002, // 车道号
    LAND_DIR            = 0x07000102, // 车道方向
    PASS_COUNTER        = 0x09000079, // 过车计数
    TRAFFIC_LIGHT_COLOR_ONE  = 0x07000106, // 信号灯 1 颜色
    TRAFFIC_LIGHT_DIREC_ONE  = 0x07000107, // 信号灯1方向
    TRAFFIC_LIGHT_COLOR_TWO  = 0x07000108, // 信号灯2颜色
    TRAFFIC_LIGHT_DIREC_TWO = 0x07000109, // 信号灯2方向
    TRAFFIC_LIGHT_COLOR_THREE  = 0x07000110, //信号灯3颜色
    TRAFFIC_LIGHT_DIREC_THREE  = 0x07000111, // 信号灯3方向
    TRAFFIC_LIGHT_COLOR_FOUR  = 0x07000112, // 信号灯4颜色
    TRAFFIC_LIGHT_DIREC_FOUR  = 0x07000113, //信号灯4方向
    VEHICLE_TYPE        = 0x07000003, // 车辆类型
    VEHICLE_COLOR       = 0x07000004, // 车辆颜色
    VEHICLE_DIRECTION   = 0x07000005, // 车辆行驶方向

    VEHICLE_POS         = 0x0B000005, // 车辆位置(万分比)
    VEHICLE_POS_ABS     = 0x0B000020, // 车辆位置绝对坐标              新增
    VEHICLE_POS_COM     = 0x0B000021, // 车辆位置相对坐标万分比    新增

    PLATE_TYPE          = 0x07000006, // 车牌类型

    PLATE_POS           = 0x0B000007, // 车牌位置(M系列为绝对坐标 X系列之后为相对坐标)
    PLATE_POS_ABS       = 0x0B000026, // 车牌位置绝对坐标              新增
    PLATE_POS_COM       = 0x0B000027, // 车牌位置相对坐标万分比    新增
    VEHICLE_FACE_POS1   = 0x0B000028, // 第一张人脸位置相对坐标万分比
    VEHICLE_FACE_POS2   = 0x0B000029, // 第二张人脸位置相对坐标万分比
    VEHICLE_FACE_MAIN_INDEX   = 0x070000BA, // 主驾驶索引

    PLATE_CHAR          = 0x0A000008, // 车牌字符
    PLATE_PIC           = 0x0A000009, // 车牌抠图
    PLATE_BMP_BIT       = 0x0A00000B, // 车牌BMP图
    PLATE_BMP_BIT_SIZE  = 0x07000408, // 车牌BMP图大小        
    PLATE_BMP_BYTE      = 0x0A00000C, // 车牌BMP图    
    PLATE_BMP_BYTE_SIZE = 0x07000407, // 车牌BMP图大小            
    PLATE_CONFIDENCE    = 0x07000061, // 车牌置信度
    PLATE_COLOR         = 0x07000062, // 车牌颜色
    PLATE_CHAR_POS      = 0x0B000063, // 车牌字符位置
    PLATE_FACE_POS      = 0x0B000064, // 车脸位置
    PLATE_MOVE_DIR      = 0x07000065, // 车牌运动方向
    PLATE_SNAPSHOT_TYPE = 0x07000066, // 车牌抓拍类型
    VEHICLE_PIC         = 0x0A000067, // 车辆特写图
    FACE_FEATURE_PIC    = 0x0A000068, // 行人闯红灯人脸特写图
    VEHICLE_FACE1       = 0x0A000069, // 第一张车内人脸图
    VEHICLE_FACE2       = 0x0A00006A, // 第二张车内人脸图
    PIC_SNAPSHOT_TIMEMS = 0x09000003, // 抓拍时间(单位ms)
    PIC_SNAPSHOT_TIMEMS1 = 0x0900000B, // 第一张图抓拍时间(单位ms)
    PIC_SNAPSHOT_TIMEMS2 = 0x0900000C, // 第二张图抓拍时间(单位ms)
    PIC_SNAPSHOT_TIMEMS3 = 0x0900000D, // 第三张图抓拍时间(单位ms)
    PIC_SNAPSHOT_TIMEMS4 = 0x0900000E, // 第四张图抓拍时间(单位ms)
    PIC_SNAPSHOT_TIME   = 0x07000068, // 抓拍时间
    PIC_SNAPSHOT_TIME_US = 0x09000005,//抓拍时间(单位us)
    PIC_SNAPSHOT_TZONE  = 0x08000069, // 设备时区(单位ms 东区为+ 西区为-)
    DEVICE_ID           = 0x0A000025, // 设备编号
    ROID_ID             = 0x0A000026, // 道路编号
    DIR_ID              = 0x0A000027, // 方向编号
    DIR_INFO            = 0x0A000028, // 方向信息
    PANORAMA_PIC_SIZE   = 0x07000073, // 全景图大小
    PLATE_PIC_SIZE      = 0x07000074, // 车牌图大小
    VLPR_ALG_TYPE       = 0x07000079, // 车牌算法类型
    REGULATION_CODE     = 0x0A000029, // 违章代码字符串
    LANE_DESC           = 0x070000B2, // 车道描述
    LANE_DIR_DESC       = 0x070000B3, // 车道方向描述
    CAR_DRV_DIR         = 0x070000B6, // 车辆行驶方向描述
    RADER_CAR_DIR       = 0x070000B7, // 雷达测速方向
    CUR_SNAP_INDEX      = 0x070000B8, // 当前抓拍序列号
    FACE_NUM            = 0x12000002, // 人脸个数    
    PLATE_IDENTIFY_ID   = 0x0A00002A, // 抓拍流水号
    FACE_PED_POS        = 0x0B000065, // 行人闯红灯人脸位置
    
    ITS_TYPE            = 0x04000032, // ITS 应用模式
    VEHICLE_TL_X        = 0x0400002A, // 车辆位置左上角X坐标
    VEHICLE_TL_Y        = 0x0400002B, // 车辆位置左上角Y坐标
    VEHICLE_BR_X        = 0x0400002C, // 车辆位置右下角X坐标
    VEHICLE_BR_Y        = 0x0400002D, // 车辆位置右下角Y坐标
    PLATE_TL_X          = 0x0400002E, // 车牌位置左上角X坐标
    PLATE_TL_Y          = 0x0400002F, // 车牌位置左上角Y坐标
    PLATE_BR_X          = 0x04000030, // 车牌位置右下角X坐标
    PLATE_BR_Y          = 0x04000031, // 车牌位置右下角Y坐标
        
    ITS_COMBINE         = 0x01000003, // ITS 是否开启合成
    ITS_OSD_PIC_OFFSET  = 0x06000037, // ITS 六合一卡口osd导致的车辆位置偏移量,正值表示叠加外侧上边缘,负值表示叠加外侧下边缘
	
	BLINKAGE_SNAP       = 0x01000005, // 联动抓拍功能使能
	BLINKAGE_SNAP_HOST_MACHINE = 0x01000006, // 联动抓拍抓拍主机
	BLINKAGE_SNAP_SLAVE_MACHINE = 0x01000007, // 联动抓拍抓拍从机
	LINKAGESNAP_PIC_1   = 0x0A000031, // 联动抓拍第一张
	LINKAGESNAP_PIC_2   = 0x0A000032, // 联动抓拍第二张
	LINKAGESNAP_PIC_3   = 0x0A000033, // 联动抓拍第三张
	IMG_SNAP_WIDTH      = 0x07000114, // 设置的抓拍图片宽
	IMG_SNAP_HEIGHT     = 0x07000115, // 设置的抓拍图片高
	

    MICRO_PORT_TRAFFIC_STATISTICS = 0x070000A0,    //车流量统计参数
    STATISTICS_LANE_COUNT = 0x070000A1, //微卡口车流量统计车道数量
    STATISTICS_LANE_INDEX = 0x070000A2, //微卡口车流量统计当前车道
    STATISTICS_VEHICLE_COUNT = 0x070000A3,//车辆计数
    STATISTICS_AVG_SPEED = 0x070000A4,//平均速度
    STATISTICS_LANE_TIME_USED_RATIO = 0x070000A5,//车道时间占有率
    STATISTICS_VEHICLE_DENSITY = 0x070000A6,//车流密度
    STATISTICS_VEHICLE_HEAD_INTERVAL = 0x070000A7,//车头时间间隔
    STATISTICS_VEHICLE_HEAD_SPACE_INTERVAL = 0x070000A8,//车头间隔
    STATISTICS_CONGESTION_DEGREE = 0x070000A9,//交通状态
    STATISTICS_VEHICLE_CAR_LARGE_COUNT = 0x070000AA,//大型车数量
    STATISTICS_VEHICLE_CAR_MED_COUNT = 0x070000AB,//中型车数量
    STATISTICS_VEHICLE_CAR_SMALL_COUNT = 0x070000AC,//小型车数量
    STATISTICS_QUEUE_LENGTH = 0x070000AD,//排队长度
    STATISTICS_LANE_SPACE_USED_RATIO = 0x070000AE,//车道空间占有率
    ITS_TRAFFIC_LEFT_VEHICLE_COUNT = 0x070000AF,    //its左转车数量
    ITS_TRAFFIC_STRAIGHT_VHEICLE_COUNT = 0x070000B0,    //its直行车数量
    ITS_TRAFFIC_RIGHT_VHEICLE_COUNT = 0x070000B1,   //its右转车数量
    TRAFFIC_STATISTICS_CYCLE        = 0x070000B9,   //车流量统计周期
    TRAFFIC_STATISTICS_NONMOTOR_COUNT = 0x070000C0, //非机动车数量
    TRAFFIC_STATISTICS_PEDESTRIAN_COUNT = 0x070000C1, //行人数量
    TRAFFIC_STATISTICS_TOTAL_VEHICLE_COUNT = 0x070000C2, //断面流量
    
    LANE_NUMBER = 0x07000045, // 车道号(万集称重系统返回的车道号)
    WEIGH_ANALYSIS_ID = 0x07000046, //解析的ID (万集称重系统解析的ID)

    VEHICLE_SPEED       = 0x07000075,           // 车辆速度
    REGULATION_TYPE     = 0x07000076,           // 违章类型
    ITS_FLOWRATE_FEATURE = 0x07000077,          // its流量统计属性
    VEHICLE_MFR_TYPE     = 0x07000078,          // 机动车二次特征
    ITS_VEHICLE_FLOWRATE_FEATURE = 0x070000B5,  // its车流量统计属性

    MFR_MAIN_CALL       = 0x06000025,  // 主驾驶打电话
    MFR_MAIN_BELT       = 0x06000026,  // 主驾驶安全带
    MFR_VICE_EXIST      = 0x06000027,  // 是否有副驾驶
    MFR_VICE_BELT       = 0x06000035,  // 副驾驶安全带
    MFR_YEAR_LOG        = 0x06000036,  // 年检标
    MFR_MAIN_SUN_VISOR  = 0x06000030,  // 主驾驶遮阳板
    MFR_VICE_SUN_VISOR  = 0x06000031,  // 副驾驶遮阳板
    MFR_NAP_KIN_BOX     = 0x06000032,  // 纸巾盒
    MFR_CAR_PENDANT     = 0x06000034,  // 挂件

    ITS_TRAFFIC_STATE    = 0x070000B4, // 车道交通状态
    
    CARDETECTION_POS    = 0x0B000015, // 停车位置

    VEHICLE_BODY_RECT   = 0x0B000008, // 车身位置
    NOMOTOR_BODY_RECT   = 0x0B000009, // 非机动车车身位置

    CAR_PRE_BRAND       = 0x0A000007, // 品牌字符 （大  众）
    CAR_SUB_BRAND       = 0x0A000022, // 子款字符 （桑塔纳）
    CAR_TYPE_BRAND      = 0x0A000023, // 车型字符 （轿  车）
    CAR_YEAR_BRAND      = 0x0A000024, // 年款字符 （2011）
    VHD_OBJ_ID          = 0x09000006, // 机非人ID
    CAR_PRE_BRAND_INDEX = 0x06000028, // 品牌字符索引 （大  众）
    CAR_SUB_BRAND_INDEX = 0x06000029, // 子款字符索引 （桑塔纳）
	SHELTER_PLATE       = 0x0600002a, // 遮挡车牌

	PICTURE_TYPE        = 0x0600003a, // 图片类型
    
    DEV_CNT				= 0x03000070, // 设备数
    CHAN_ID				= 0x03000071, // 通道号
    PTZ_CMD_TYPE		= 0x03000072, // 
    SNAP_TYPE			= 0x03000073,
    TIME				= 0x09000074, // 智能内部使用
    ANGLE				= 0x09000075, // 智能内部使用
    SCORE				= 0x09000076, // 智能内部使用
    IMGSIZE				= 0x09000077,
    IMGBUF				= 0x0A000078,
    REGION_CNT			= 0x07000080, // 智能内部使用
    REGION_MDID			= 0x0700007A, // 智能内部使用
    TALARM_TYPE			= 0x0700007B, // 智能内部使用
    ALARM_TIME			= 0x0700007C, // 智能内部使用
    EVENT_NUM			= 0x0700007D, // 智能内部使用
    TARGET_ID			= 0x0700007E, // 智能内部使用
    STATUS				= 0x0700007F, // 智能内部使用
    EVENT_VAL			= 0x07000081, // 智能内部使用
    TARGETLT_X			= 0x08000081, // 目标左上角X，智能内部使用
    TARGETLT_Y			= 0x08000082, // 目标左上角Y，智能内部使用
    TARGETRB_X			= 0x08000083, // 目标右下角X，智能内部使用
    TARGETRB_Y			= 0x08000084, // 目标右下角Y，智能内部使用
    TRECORD_TYPE        = 0x07000085, // 智能内部使用
    PEOPLE_NUM          = 0X07000087, // 人群密度检测算法人数
    HEADSHOULDER_POS    = 0X0B000088, // 人群密度检测算法返回框的地址
    AREARATIO           = 0X07000089, // 人群密度检测算法人群密度
    TRACK_OBJECT        = 0x07000028, // 跟踪目标id
	ONLINE_LEARNING_INFER = 0x07000029, // 在线自学习算法推理状态
    ALARM_AREA_ID       = 0x07000030, // 告警区域id
    
    //人脸属性类 以FACE开头 0 表示未知 1~n依次对应注释的属性
    FACE_GLASS          = 0X07000200, // 眼镜{无，有} 
    FACE_GENDER         = 0X07000201, // 性别{女，男} 
    FACE_AGE            = 0X07000202, // 年龄，具体的年龄值1~99 
    FACE_MOUTHMASK      = 0X07000203, // 遮档(口罩) {无，是} 
    FACE_EXPRESSION     = 0X07000204, // 人脸表情{微笑、愤怒、悲伤、正常、惊讶}
    FACE_HAT            = 0X07000205, // 帽子{无, 有}
    FACE_MUSTACHE       = 0X07000206, // 胡子{无, 有}
    FACE_HAIR           = 0X07000207, // 发型{长, 短}
    FACE_GLASS_TYPE     = 0X07000208, // 眼镜{无，普通眼镜，太阳眼镜} 801版本算法更改

    //人体属性类 以HUMAN开头 0 表示未知 1~n依次对应注释的属性
    HUMAN_AGE           = 0X07000300, // 年龄 {少年,青年,老年} 
    HUMAN_GENDER        = 0X07000301, // 性别{男，女}
    HUMAN_UPPERSTYLE    = 0X07000302, // 上衣款式 {长袖，短袖} 
    HUMAN_UPPERCOLOR    = 0X07000303, // 上衣颜色 {黑，蓝，绿，白/灰，黄/橙/棕，红/粉/紫}
    HUMAN_UPPERTEXTURE  = 0X07000304, // 上衣纹理 {纯色，条纹，格子} 
    HUMAN_LOWSTYLE      = 0X07000305, // 下衣款式 {长裤,短裤，裙子}      
    HUMAN_LOWERCOLOR    = 0X07000306, // 下衣颜色 {黑，蓝，绿，白/灰，黄/橙/棕，红/粉/紫}  
    HUMAN_SHAPE         = 0X07000307, // 体型{standard, fat, thin}
    HUMAN_MOUTHMASK     = 0X07000308, // 口罩{no,yes}  
    HUMAN_HAIR          = 0X07000309, // 发型{ long, short }
    HUMAN_BACKPACK      = 0X0700030A, // 背包{no,yes} 
    HUMAN_CARRY         = 0X0700030B, // 是否拎东西{no,yes}
    HUMAN_SATCHEL       = 0X0700030C, // 斜挎包{no,yes} 
    HUMAN_UMBRELLA      = 0X0700030D, // 雨伞{no,yes}
    HUMAN_FRONTPACK     = 0X0700030E, // 前面背包{no,yes}
    HUMAN_LUGGAGE       = 0X0700030F, // 行李箱{no,yes} 
    HUMAN_DIRECT        = 0X07000310, // 行进方向{forward,backward}
    HUMAN_SPEED         = 0X07000311, // 行进速度{slow,fast}
    HUMAN_VIEW          = 0X07000312, // 朝向{frontal, back, leftprofiled, rightprofiled}
    HUMAN_GLASS         = 0X07000313, // 眼镜{no,glass, sunglass}
    HUMAN_HAT           = 0X07000314, // 戴帽子{no, yes}

    //非机动车属性类 以RIDERMAN开头 0 表示未知 1~n依次对应注释的属性
    RIDERMAN_AGE        = 0X07000400, // 年龄 {少年,青年,老年}   
    RIDERMAN_GENDER     = 0X07000401, // 性别{男，女}     
    RIDERMAN_UPPERSTYLE = 0X07000402, // 上衣款式 {长袖，短袖}        
    RIDERMAN_UPPERCOLOR = 0X07000403, // 上衣颜色 {黑，蓝，绿，白/灰，黄/橙/棕，红/粉/紫}        
    RIDERMAN_HELMET     = 0X07000404, // 是否戴头盔 {no, yes} 
    RIDERMAN_HELMETCOLOR= 0X07000405, // 头盔颜色 {黑，蓝，绿，白/灰，黄/橙/棕，红/粉/紫} 
    VEHICLE_TYPE_EXT    = 0x07000406, // C50车辆类型
    APPROACH_LANE_ID    = 0x07000605, //临近车道号

    //未使用在线自学习人体属性类 以HUMAN开头 0 表示未知 1~n依次对应注释的属性
    ORIGINAL_HUMAN_AGE           = 0X07000500, // 年龄 {少年,青年,老年} 
    ORIGINAL_HUMAN_GENDER        = 0X07000501, // 性别{男，女}
    ORIGINAL_HUMAN_UPPERSTYLE    = 0X07000502, // 上衣款式 {长袖，短袖} 
    ORIGINAL_HUMAN_UPPERCOLOR    = 0X07000503, // 上衣颜色 {黑，蓝，绿，白/灰，黄/橙/棕，红/粉/紫}
    ORIGINAL_HUMAN_UPPERTEXTURE  = 0X07000504, // 上衣纹理 {纯色，条纹，格子} 
    ORIGINAL_HUMAN_LOWSTYLE      = 0X07000505, // 下衣款式 {长裤,短裤，裙子}      
    ORIGINAL_HUMAN_LOWERCOLOR    = 0X07000506, // 下衣颜色 {黑，蓝，绿，白/灰，黄/橙/棕，红/粉/紫}  
    ORIGINAL_HUMAN_SHAPE         = 0X07000507, // 体型{standard, fat, thin}
    ORIGINAL_HUMAN_MOUTHMASK     = 0X07000508, // 口罩{no,yes}  
    ORIGINAL_HUMAN_HAIR          = 0X07000509, // 发型{ long, short }
    ORIGINAL_HUMAN_BACKPACK      = 0X0700050A, // 背包{no,yes} 
    ORIGINAL_HUMAN_CARRY         = 0X0700050B, // 是否拎东西{no,yes}
    ORIGINAL_HUMAN_SATCHEL       = 0X0700050C, // 斜挎包{no,yes} 
    ORIGINAL_HUMAN_UMBRELLA      = 0X0700050D, // 雨伞{no,yes}
    ORIGINAL_HUMAN_FRONTPACK     = 0X0700050E, // 前面背包{no,yes}
    ORIGINAL_HUMAN_LUGGAGE       = 0X0700050F, // 行李箱{no,yes} 
    ORIGINAL_HUMAN_DIRECT        = 0X07000510, // 行进方向{forward,backward}
    ORIGINAL_HUMAN_SPEED         = 0X07000511, // 行进速度{slow,fast}
    ORIGINAL_HUMAN_VIEW          = 0X07000512, // 朝向{frontal, back, leftprofiled, rightprofiled}
    ORIGINAL_HUMAN_GLASS         = 0X07000513, // 眼镜{glass, sunglass, no}
    ORIGINAL_HUMAN_HAT           = 0X07000514, // 戴帽子{no, yes}

    //骑行工具属性项
    MOTOR_COLOR                  = 0X07000600, // 非机动车颜色 {黑，蓝，绿，白/灰，黄/橙/棕，红/粉/紫}     
    MOTOR_SUNSHADE               = 0X07000601, // 是否有遮阳伞{no,yes}
    MOTOR_SUNSHADE_COLOR         = 0X07000602, // 遮阳伞颜色 {黑，蓝，绿，白/灰，黄/橙/棕，红/粉/紫}
    MOTOR_MOTOR_CARRY            = 0X07000603, // 是否有携带物 {no, yes} 
    MOTOR_LICENSE_PLATE          = 0X07000604, // 是否有车牌{no,yes}  
    RIDERMAN_NUM                 = 0X07000606, // 骑行人数，具体的人数 
    MOTOR_TYPE                   = 0X07000607, // 非机动车类型 {自行车、三轮车、电瓶车、摩托车}

    //未使用在线自学习人脸属性类  0表示未知 1~n依次对应注释的属性
    ORIGINAL_FACE_GLASS          = 0X07000700, // 眼镜{无，普通眼镜，太阳眼镜} 
    ORIGINAL_FACE_GENDER         = 0X07000701, // 性别{女，男} 
    ORIGINAL_FACE_AGE            = 0X07000702, // 年龄，具体的年龄值1~99 
    ORIGINAL_FACE_MOUTHMASK      = 0X07000703, // 遮档(口罩) {无，是} 
    ORIGINAL_FACE_EXPRESSION     = 0X07000704, // 人脸表情{微笑、愤怒、悲伤、正常、惊讶}
    ORIGINAL_FACE_HAT            = 0X07000705, // 帽子{无, 有}
    ORIGINAL_FACE_MUSTACHE       = 0X07000706, // 胡子{无, 有}
    ORIGINAL_FACE_HAIR           = 0X07000707, // 发型{长, 短}

	METADATATYPE        = 0X0700009F, // 图片类型(行为分析下人体FTP)   智能内部使用 
	FTPPICTYPEDIVID     = 0X07000010, // human ride vhd(机非人区分)
    CHANNEL_ID          = 0x09000078, // 相机通道号 智能内部使用
	
    CHANNEL_ID_EX          = 0x09000092, // 相机通道号,，对外，用于区分元数据归属

	HOTMAP_WIDTH        = 0x05000001, // 热度图宽
	HOTMAP_HEIGHT       = 0x05000002, // 热度图高
	HOTMAP_ACCUM_IMG    = 0x05000003, // 热度图

	PIC_SEND_NUM        = 0x09000080, // 抠图发送序号

    FACE_CAP_FEATURE    = 0x0A000020,   // 人脸抓拍特征值
    FACE_RECOG_BOX_COLOUR = 0x09000081,//人脸识别框颜色

	VOICE_PROMPT        =  0x0A000079,      // 音频提示 
    FACEPIC_UUID        =  0x0A00007A,      // 图片UUID 
    ID_CARDMD5          = 0x0A00007B,       // 身份证MD5 
    VISITOR_TYPE        = 0x0200007C,       // 访客类型 0 : 社区居民 1:访客 
    SNAPFACENUM         = 0x07000103,       // 当前相机抓拍人数统计
    TOTALSNAPNUM        = 0x07000104,       // 整个服务抓拍人数 
    SNAP_MATCHRATE_MILLION = 0x07000105,    // 发送给元数据网关的匹配率，6位有效数字
    MATCH_TYPE          = 0x01000004,       // 人脸识别是否比对成功 
	
	SYS_LANGUAGE_TYPE   = 0x07000515,// 后台系统语言类型  
	
	MSL_AUTO_CALIBRATION_POINT = 0x0C000037, // 枪球联动自动标定点
	
    PIC_SNAPSHOT_DSTOFFSET = 0x08000085,     //夏令时偏移时间(s)

    COMM_LABEL_ADD_TYPE = 0x0A00007C,       // ADD第三方数据
    COMM_LABEL_DEL_TYPE = 0x0A00007D,       // delete第三方数据

    FACE_FEATURE_VAL        = 0x0A000030,       // 人脸特征，智能内部使用
    
    COUNTRY_CODE_CHAR = 0x0A000080, // 车牌国家码字符
    STATE_CODE_CHAR      = 0x0A000081, // 车牌省份码字符
    ARH_COUNTRY_CODE_CHAR = 0x0A000082, // ARH车牌国家码字符
    ARH_STATE_CODE_CHAR      = 0x0A000083, // ARH车牌省份码字符
    GBT_COUNTRY_CODE_CHAR = 0x0A000086, // GBT2659-2000世界各国和地区名称代码

    HOTMAP_IMG = 0x0A000084, // 热度图二进制序列

    TEMPERATURE_CONTROL_ALARM_AREA_ID = 0x07000040, // 温控告警区域ID
    TEMPERATURE_CONTROL_ALARM_TYPE = 0x06000041, // 温控告警类型，0代表预警，1代表告警
    FACE_REC_TASK_ID = 0x09000093,              // NVR800 图片特征提取 任务ID
    FACE_REC_TASK_STATE = 0x08000086,            // NVR800 图片特征提取 任务状态
    FACE_REC_VERSION = 0x0A000085,              // NVR800 图片特征提取 人脸识别算法版本号
    
    CLOSEUP_PIC = 0x0A00006B,           // 对象特写图 所有的非全景图 包含抠图 拼图

    ALG_TYPE = 0x20000001,              // 算法分类
    META_NAME = 0x20000002,             // 元数据名
    PRODUCER_NAME = 0x20000003,         // 数据生成者名字，即APP名

    DESCRIBE_INFO = 0x20000004,         // 象描述信息

    SDC_UUID = 0x22000001,              // 摄像机UUID
    OBJ_ATTR = 0x21000001,              // 属性信息，表示对象的扩展属性,属性对规则 0X21

    PIC_ATTR = 0x21000002,              // 图片的扩展属性信息,属性对规则 0X21

    COMMON_ALARM_TYPE = 0x07000041,     // 通用告警类型
    ALARM_NAME      = 0x20000005,       // 告警名称     
    ALARM_SOURCE    = 0x20000006,       // 告警源
    PANORAMA_ID     = 0X0900000A,       // 背景图ID
    CLIP_VIDEO  = 0x0A0000A0, // 短视频
    SCENE_ID             = 0x07000099 //场景ID
} LAYER_THREE_TYPE_E;

#define METATYPE_MASK LAYER_THREE_TYPE::RULE_MASK 

// TARGET 目标类型
typedef enum ITGT_TARGET_TYPE {
    TARGET_FACE_HUMAN_RECT      = 0x00,  // 人脸人体检测框
    TARGET_FACE_DT_PROCESS      = 0x01,  // 人脸后处理数据，发送抠图
    TARGET_FACE_RECOGNITION     = 0x02,  // 人脸识别
    TARGET_MMC_FACE_PRE_PROCESS = 0x03,  // 多机协同人脸检测到的抠图和算法配置参数 对内使用
    TARGET_MMC_FACE_RECOG       = 0x04,  // 多机协同人脸识别 对内使用
    TARGET_IBALL_VEHICLE_DT     = 0x05,  // 违章球车辆检测

    TARGET_HUMANBODY            = 0x06,  // 机非人业务人体信息
    TARGET_VHD_VEHICLE          = 0x07,  // 机非人业务机动车信息
    TARGET_VHD_NOMOTOR          = 0x08,  // 机非人非机动车信息

    TARGET_VEHICLE_RECT         = 0x09,  // 车检测框
    TARGET_NOMOTOR_RECT         = 0x0a,  // 非机动车检测框

    TARGET_HOTMAP               = 0x0b,  // 热度图
    TARGET_CROWD_DENSITY        = 0x0c,  // 人群密度
    TARGET_QUEUING_LENGTH       = 0x0d,  // 排队长度
    TARGET_BEHAVIOR             = 0x0e,  // 行为分析
    TARGET_HUMANCOUNT           = 0x0f,  // 过线计数

    TARGET_AUTOTRACK            = 0x10,  //自动跟踪
    TARGET_CARDETECTION         = 0x11,  //停车侦测
	TARGET_MSL_AUTO_CALIBRATION = 0x12,  //枪球联动自动标定点显示

    TARGET_HBA                  = 0x13,  // 复杂行为分析
    TARGET_PIC_FEATURE          = 0x14,  // NVR800 人脸特征元数据

    TARGET_BEHAVIOR_SNAP        =0x15,   // 行为分析抓图
    TARGET_PIC_FEATURE_SYNERGY  = 0x16,  // NVR特征协同元数据
	
    TARGET_ITS_PROCESS          = 0x30,  // ITS
    TARGET_ITS_STATISTICS       = 0x31,  // ITS车流量统计
    TARGET_ITS_OBJ_DT           = 0x32,  // ITS目标检测框

    TARGET_VLPR_PROCESS         = 0x33,  // 微卡口
    TARGET_VLPR_STATISTICS      = 0x34,  // 微卡口车流量统计

    TARGET_VHD_HUMAN_ON_NOMOTOR = 0x35,  // 混行业务 非机动车上人脸人体

    TARGET_ITS_PEDRUNRED        = 0x36,  // ITS行人闯红灯业务
    TARGET_ITS_HOLOCAMERA       = 0x37,   //ITS电警全息相机
    TARGET_ITS_TRRFICLIGHT      = 0x38,  //ITS 信号灯状态
    TARGET_ITS_LINKAGE_DATA     = 0x39,  // 鞍山不礼让行人联动数据上传 

    TARGET_TEMPERATURE_ALARM    = 0x40,     // 热成像温控告警
    TARGET_FIRE_RESULT          = 0x41,     // 火点检测结果

    TARGET_ITS_NOMOTOR = 0x42,      //ITS非机动车
    TARGET_ITS_HUMAN = 0x43,      //ITS行人
    TARGET_TDOME_ILLEGAL_PARKING_ALARM = 0x44, // 违停球镜头第一次拉近抓拍告警

    TARGET_RECT                 = 0x60,     // 目标框信息，用于实时显示

    TARGET_KEEPALIVE            = 0x80,     // 元数据保活
    TARGET_MAX,
} ITGT_TARGET_TYPE_E;

//不同颜色的目标检测框
typedef enum _OBJ_FRAME_TYPE_E {
    OBJ_FRAME_TYPE_NONE    = 0x00,     // 未分类[t00200240@9/24/2014]
    OBJ_FRAME_TYPE_VEHICLE = 0x01,     // 车[t00200240@9/24/2014]
    OBJ_FRAME_TYPE_HUMAN   = 0x02,     // 人[t00200240@9/24/2014]
    OBJ_FRAME_TYPE_VEH     = 0x60,     // 机非人的机动车
    OBJ_FRAME_TYPE_NMV     = 0x61,     // 机非人的非机动车
    OBJ_FRAME_TYPE_PED     = 0x62,     // 机非人的行人

    OBJ_FRAME_TYPE_ZERO    = 0x80,     // 区域显示框类型0:人脸框
    OBJ_FRAME_TYPE_ONE     = 0x81,     // 区域显示框类型1:人框
    OBJ_FRAME_TYPE_TWO     = 0x82,     // 区域显示框类型2:非机动车框
    OBJ_FRAME_TYPE_THREE   = 0x83,     // 区域显示框类型3:机动车框
    OBJ_FRAME_TYPE_FOUR    = 0x84,     // 区域显示框类型4:在人脸业务中是人身框、在交通业务中是车牌框
    OBJ_FRAME_TYPE_FIVE    = 0x85,     // 区域显示框类型5:停车侦测使用框
    OBJ_FRAME_TYPE_SIX     = 0x86,     // 区域显示框类型6:人群密度使用框
    OBJ_FRAME_TYPE_SEVEN   = 0x87,     // 区域显示框类型7:排队长度使用框
    OBJ_FRAME_TYPE_EIGHT   = 0x88,     // 区域显示框类型8:违停球使用框
    OBJ_FRAME_TYPE_NINE    = 0x89,     // 区域显示框类型9:自动跟踪使用框
    OBJ_FRAME_TYPE_TEN     = 0x8a,     // 区域显示框类型10:复杂行为分析使用框
    OBJ_FRAME_TYPE_ELEVEN  = 0x8b,     // 区域显示框类型11:火点检测使用框
    
    OBJ_FRAME_TYPE_OTHER   = 0xFF,     // 其他
    OBJ_FRAME_TYPE_MAX,
} ITGT_OBJ_FRAME_TYPE_E;

//智能内部元数据抓图使用
typedef enum ITGT_LAYER_THREE_TYPE_E {
   SNAP_TYPE_FTP        = 0x06000033,
   REGULA_PIC_QTY       = 0x04000015,   // 一组违章的图片总数
   REGULA_PIC_ORDER     = 0x04000016,   // 当前违章的图片序号
   SNAP_OBJ_ENDTIME     = 0x09000002,   // 抓拍违章最后时刻
   SNAP_ASSOCIATERECORD = 0x01000001,   // 抓拍是否关联录像使能
} ITGT_LAYER_THREE_TYPE_E;

typedef enum _PLATE_SNAPSHOT_TYPE_E {
    MANUAL_TRIGGER = 0,
    AUTO_TRIGGER,
    PLATE_SNAPSHOT_TYPE_MAX
} PLATE_SNAPSHOT_TYPE_E;

typedef enum _RULE_TYPE_E {
    TRIPWIRE_RULE = 0,
    WANDER_RULE,
    ABANDON_RULE,
    REMOVE_RULE,
    INVASION_RULE,
    ENTER_RULE,
    EXIT_RULE,
    FASTMOVE_RULE,
    CARDETECTION_RULE,
    HUMANCOUNT_RULE,
    CROWDDENSITY_DETECT_RULE,
    QUEUE_DETECT_RULE,
    IBALL_DETECT_RULE,
    MSL_AUTO_CALIB_RULE,
    MSL_AUTO_CALIB_RULE_CLEAR,
    FIGHTING_RULE,
    CLIMBING_RULE,
    FALL_DOWN_RULE,
    RUNNING_RULE,
    FIRE_DETECT_RULE,
	CALIB_RULE_RULE,
	
    RULE_TYPE_MAX
} RULE_TYPE_E;

// 元数据类型掩码
enum META_TYPE_MASK {
    META_RECT = 0x00000001,         // 框数据   000....0001
    META_PIC  = 0x00000002,         // 图数据   000....0010
    META_KEEPALIVE = 0x00000008,    // 保活数据   000....1000

    META_MAX
};

// 矩形
typedef struct _META_RECT_S {
    UINT16 usX;      // 矩形左上顶点的x坐标
    UINT16 usY;      // 矩形左上顶点的y坐标
    UINT16 usWidth;  // 矩形宽
    UINT16 usHeight; // 矩形高
} META_RECT_S;

// 点
typedef struct _META_POINT_S {
    UINT16 usX; // x
    UINT16 usY; // y
} META_POINT_S;

// 线
typedef struct _META_LINE_S {
    META_POINT_S stStartPoint; // 起始点
    META_POINT_S stEndPoint;   // 终止点
} META_LINE_S;

// 多边形
typedef struct _META_POLYGON_S {
    UINT32 uPointNum;         // 点个数
    META_POINT_S astPts[MAX_POINT_NUM]; // 
} META_POLYGON_S;

// 颜色
typedef struct _META_COLOR_S {
    UCHAR auc_r[3];
    UCHAR auc_g[3];
    UCHAR auc_b[3];
    UCHAR auc_ConfLev[3]; // 置信度
    UCHAR auc_ColorID[3]; // ID
} META_COLOR_S;

/*********************************************************************
                                人体属性
*********************************************************************/
//行进方向，前后
enum MOVE_DIRECT {
    DIRECT_UNKNOWN = 0,
    FORWARD        = 1,
    BACKWARD       = 2,
};

//行进速度，慢、快
enum MOVE_SPEED {
    SPEED_UNKNOWN = 0,
    SLOW          = 1,
    FAST          = 2,
};

// 人体属性结果 
typedef struct _HUMAN_ATTRIBUTES {
    BOOL isVaild;

    // INT32 定义的属性 0 代表 未知，1-n依次代表后面的属性具体含义
    INT32 age;                              // 年龄 {少年,青年,老年} 
    INT32 gender;                           // 性别{男，女}
    INT32 upperStyle;                       // 上衣款式 {长袖，短袖} 
    INT32 upperColor;                       // 上衣颜色 {黑，蓝，绿，白/灰，黄/橙/棕，红/粉/紫}  
    INT32 upperTexture;                     // 上衣纹理 {纯色，条纹, 格子} 
    INT32 lowerStyle;                   	// 下衣款式 {长裤, 短裤, 裙子}      
    INT32 lowerColor;                  	    // 下衣颜色 {黑，蓝，绿，白/灰，黄/橙/棕，红/粉/紫}  
    INT32 shape;                        	// 体型{standard, fat, thin}
    INT32 mouthmask;                    	// 口罩{no,yes}  
    INT32 hair;                             // 发型{ long, short }
    INT32 backpack;                     	// 背包{no,yes} 
    INT32 carry;                        	// 是否拎东西{no,yes}
    INT32 satchel;                      	// 斜挎包{no,yes} 
    INT32 umbrella;                     	// 雨伞{no,yes}
    INT32 frontpack;                    	// 前面背包{no,yes}
    INT32 luggage;                      	// 行李箱{no,yes} 

    MOVE_DIRECT moveDirect;				    // 行进方向{forward,backward}
    MOVE_SPEED  moveSpeed;				    // 行进速度{slow,fast}

    INT64 view;                             // 朝向{frontal, back, leftprofiled, rightprofiled}，非产品需求，只解决有无,C30无此属性
    INT64 glass;                            // 眼镜{no， glass, sunglass}，非产品需求，只解决有无,C30无此属性    
    INT64 hat;                              // 戴帽子{no, yes}，非产品需求，只解决有无,C30无此属性
} HUMAN_ATTRIBUTES;

/****************************************************************		
                                人脸属性
****************************************************************/
typedef struct _FACE_ATTRIBUTES {
    BOOL isVaild;

    // INT32 定义的属性 0 代表 未知，1-n依次代表后面的属性具体含义
    INT32 glasses;              // 眼镜{无,普通眼镜,墨镜} 
    INT32 gender;               // 性别{女，男} 
    INT32 age;                  // 年龄，具体的年龄值1~99 
    INT32 mouthmask;            // 遮档 {无，是} 
    INT32 expression;           // 人脸表情{微笑、愤怒、悲伤、正常、惊讶}
    INT32 hat;                  // 帽子{无，有}
    INT32 mustache;             // 胡须{无，有}
    INT32 hair;                 // 发型{长，短}
} FACE_ATTRIBUTES;

/****************************************************************		
                                骑行人属性
****************************************************************/
typedef struct _RIDERMAN_ATTRIBUTES {
    BOOL isVaild;   //是否有效

    // INT32 定义的属性 0 代表 未知，1-n依次代表后面的属性具体含义
    INT32 age;                          // 年龄 {少年,青年,老年}   
    INT32 gender;                       // 性别{男，女}     
    INT32 upperStyle;                   // 上衣款式 {长袖，短袖} 
    INT32 upperColor;                   // 上衣颜色 {黑，蓝，绿，白/灰，黄/橙/棕，红/粉/紫} 
    INT32 helmet;                       // 是否戴头盔 {no, yes} 
    INT32 helmetColor;                  // 头盔颜色 {黑，蓝，绿，白/灰，黄/橙/棕，红/粉/紫}
} RIDERMAN_ATTRIBUTES;

/****************************************************************		
                             人脸信息
****************************************************************/
typedef struct _FACE_INFO_S {
    CHAR name[64];
    INT32  iGender;
    CHAR birthday[32];
    CHAR province[32];
    CHAR city[48];
    INT32  iCardType;
    CHAR cardID[32];
} FACE_INFO_S;

/****************************************************************		
                             ITS流量统计信息
****************************************************************/
#pragma pack(1)
typedef struct _ITS_FLOWRATE_S {
    INT64 llTimeZone;       // 时区
    UINT32 uSnapTime;       // 抓拍时间
    UINT32 ulLane;          // 车道号
    UINT32 ulVechileCount;  // 机动车数量
    UINT32 uNonVehicleCount; // 非机动车数量
    UINT32 uHumanCount;     // 行人数量
    
} ITS_FLOWRATE_S;
#pragma pack()

#pragma pack(1)
typedef struct _ITS_FLOWRATE_STATISTICS_S {
    UINT32  uLaneNum;                   // 车道总数
    UINT32  uLaneID;                    // 车道号
    INT64   llTimeZone;                 // 时区
    UINT32  uSnapTime;                  // 抓拍时间
    UINT32  uVehicleCount;              // 车流量
    UINT32  uLargeVehicleCount;         // 大型车数量
    UINT32  uMediumVehicleCount;        // 中型车数量
    UINT32  uSmallVehicleCount;         // 小型车数量
    UINT32  uLeftVehicleCount;          // 左转车数量
    UINT32  uStraightVehicleCount;      // 直行车数量
    UINT32  uRightVehicleCount;         // 右转车数量
    UINT32  uQueueLength;               // 排队长度          
    FLOAT   fAverageVelocity;           // 平均速度
    FLOAT   fVehicleHeadTimeInterval;   // 车头时距
    FLOAT   fVehicleHeadSpaceInterval;  // 车头间距
    FLOAT   fLaneTimeUseRatio;          // 车道时间占有率
    FLOAT   fLaneSpaceUseRatio;         // 车道空间占有率
    FLOAT   fVehileDenisy;              // 车辆密度
    UINT32  uTrafficState;              // 交通情况 1-畅通  2-缓行 3-拥堵
} ITS_FLOWRATE_STATISTICS_S;
#pragma pack()

typedef enum ItgtHbaStatus {
    HBA_STATUS_NONE = 0x00000000,       // No state
    HBA_STATUS_FIGHT = 0x00000001,      // 打架
    HBA_STATUS_CLIMB = 0x00000002,      // 攀爬
    HBA_STATUS_FALL_DOWN = 0x00000004,  // 跌倒
    HBA_STATUS_RUN = 0x00000008,        // 奔跑
} ItgtHbaStatus;

#endif /* __INTELLIGENT_ALG_METADATA_H__ */
