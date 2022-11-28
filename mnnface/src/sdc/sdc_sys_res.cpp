/******************************************************************************

                  版权所有 (C), 2019-2020, SDC OS 开源软件小组所有

 ******************************************************************************
  文 件 名   : sdc_sys_res.cpp
  版 本 号   : 初稿
  作    者   : athina
  生成日期   : 2020年8月15日
  最近修改   :
  功能描述   : sdc_sys_res.cpp函数集
  函数列表   :
  修改历史   :
  1.日    期   : 2020年8月15日
    作    者   : athina
    修改内容   : 创建文件

******************************************************************************/
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <memory.h>
#include "sdc_sys_res.h"

MemoryInfo::MemoryInfo(void)
{
}

MemoryInfo::~MemoryInfo(void)
{
}

int32_t MemoryInfo::GetMemory(MemoryData &data)
{
    FILE* pFile = fopen("/proc/meminfo", "r");
    if (pFile == NULL) {
        return -1;
    }

    char buffer[MAX_MEMINFO_SIZE] = {0};
    while (fgets(buffer, sizeof(buffer), pFile) != NULL) {
        if (strncmp(buffer, "MemTotal:", sizeof("MemTotal:") - 1) == 0) {
            (void)sscanf(buffer, "MemTotal: %u kB", &(data.uiMemTotal));
        } else if (strncmp(buffer, "MemFree:", sizeof("MemFree:") - 1) == 0) {
            (void)sscanf(buffer, "MemFree: %u kB", &(data.uiMemfree));
        } else if (strncmp(buffer, "Buffers:", sizeof("Buffers:") - 1) == 0) {
            (void)sscanf(buffer, "Buffers: %u kB", &(data.uiBuffer));
        } else if (strncmp(buffer, "Cached:", sizeof("Cached:") - 1) == 0) {
            (void)sscanf(buffer, "Cached: %u kB", &(data.uiCache));
        }
    }
    fclose(pFile);
    pFile = NULL;

    data.uiUsedMem = data.uiMemTotal - (data.uiMemfree + data.uiBuffer + data.uiCache);
    return 0;
}

int32_t MemoryInfo::GetProcMem(uint32_t &usedMem) const
{
    char filePath[256] = {0};
    pid_t pid = getpid();
    (void) snprintf(filePath, sizeof(filePath), "/proc/%d/status", pid);

    FILE* pFile = fopen(filePath, "r");
    if (pFile == NULL) {
        return -1;
    }

    char buffer[MAX_MEMINFO_SIZE] = {0};
    while(fgets(buffer, sizeof(buffer), pFile) != NULL) {
        if (strncmp(buffer, "VmRSS:", sizeof("VmRSS:") - 1) == 0) {
            (void)sscanf(buffer, "VmRSS: %u kB", &usedMem);
            break;
        } 
    }
    fclose(pFile);
    pFile = NULL;
    return 0;
}

CpuInfo::CpuInfo(void)
{
    memset(&m_lastSampleData, 0, sizeof(m_lastSampleData));
}

CpuInfo::~CpuInfo(void)
{
}

int32_t CpuInfo::SamplingData(CpuData &data)
{
    int32_t iRet = -1;
    FILE* pFile = fopen("/proc/stat", "r");
    if (NULL == pFile) {
        return iRet;
    }

    if (7 == fscanf(pFile, "cpu %u %u %u %u %u %u %u", &(data.uiUserTime), \
        &(data.uiNiceTime), \
        &(data.uiSysTime), \
        &(data.uiIdleTime), \
        &(data.uiIowaitTime), \
        &(data.uiIrqTime), \
        &(data.uiSoftirqTime)))
    {
        iRet = 0;
    }
    fclose(pFile);
    pFile = NULL;

    return iRet;
}

int32_t CpuInfo::CalcUsage(uint32_t &cpuUsage)
{
    CpuData data;
    if (SamplingData(data) < 0) {
        return 0;
    }

    // Compute the usage rate of the CPU
    uint32_t totalIdleTime = data.uiIdleTime - m_lastSampleData.uiIdleTime;
    uint32_t totalUsedTime = (data.uiUserTime - m_lastSampleData.uiUserTime) \
        + (data.uiNiceTime - m_lastSampleData.uiNiceTime) \
        + (data.uiSysTime - m_lastSampleData.uiSysTime) \
        + (data.uiIdleTime - m_lastSampleData.uiIdleTime) \
        + (data.uiIowaitTime - m_lastSampleData.uiIowaitTime) \
        + (data.uiIrqTime - m_lastSampleData.uiIrqTime) \
        + (data.uiSoftirqTime - m_lastSampleData.uiSoftirqTime);

    // If sampling cpu data frequently, than the retVal is not reasonable,
    // Use the last retVal as the sampling value
    if (totalUsedTime < 1000) {
        return -1;
    }

    // Save the last sample cpu data
    memcpy(&m_lastSampleData, &data, sizeof(data));

    // compute cpu usage rate
    cpuUsage = ((totalUsedTime - totalIdleTime) * 100) / totalUsedTime;
	return 0;
}