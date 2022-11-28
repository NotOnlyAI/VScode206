#ifndef __MEMORY_H__
#define __MEMORY_H__
/******************************************************************************

                  版权所有 (C), 2019-2020, SDC OS 开源软件小组所有

 ******************************************************************************
  文 件 名   : sdc_sys_res.h
  版 本 号   : 初稿
  作    者   : athina
  生成日期   : 2020年8月15日
  最近修改   :
  功能描述   : 获取系统资源
  函数列表   :
  修改历史   :
  1.日    期   : 2020年8月15日
    作    者   : athina
    修改内容   : 创建文件

******************************************************************************/
#include <stdint.h>

#define MAX_MEMINFO_SIZE    64

typedef struct tagMemoryData {
    uint32_t uiMemTotal;   // kB
    uint32_t uiBuffer;     // kB
    uint32_t uiMemfree;    // kB
    uint32_t uiCache;      // kB
    uint32_t uiUsedMem;    // kB
} MemoryData;

typedef struct tagCpuData
{
    uint32_t uiUserTime;
    uint32_t uiNiceTime;
    uint32_t uiSysTime;
    uint32_t uiIdleTime;
    uint32_t uiIowaitTime;
    uint32_t uiIrqTime;
    uint32_t uiSoftirqTime;
} CpuData;

class MemoryInfo
{
public:
    MemoryInfo(void);
    ~MemoryInfo(void);
    int32_t GetMemory(MemoryData &data);
    int32_t GetProcMem(uint32_t &usedMem) const;
};

class CpuInfo
{
public:
    CpuInfo(void);
    ~CpuInfo(void);
    int32_t CalcUsage(uint32_t &cpuUsage);

private:
    int32_t SamplingData(CpuData &data);
    CpuData m_lastSampleData;
};

#endif /* __MEMORY_H__ */
