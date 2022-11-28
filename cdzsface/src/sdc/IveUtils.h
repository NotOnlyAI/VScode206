#ifndef __IVE_MANAGER_H__
#define __IVE_MANAGER_H__

#include <memory>
#include <utility>
#include "sdc.h"
#include "basedef.h"

#define ALIGN_UPTO_16BIT(x) (((x) + 16 - 1) & ~(16 - 1))

class IveUtils
{
public:
    IveUtils(void);
    virtual ~IveUtils(void);
    virtual int32_t Init(int fd_codec);
    virtual void Close(void);
    virtual int32_t HandleRegion(const sdc_yuv_frame_s &origin, const sdc_region_s &region, sdc_yuv_frame_s &dst) const { return ERR; }
    virtual int32_t FreeYuvFrame(sdc_yuv_frame_s &frame) const;

protected:
    int32_t m_fdCodec;
};

// 对YuvFrame进行裁剪，不缩放处理
class CropUtils : public IveUtils
{
public:
    CropUtils(void);
    virtual ~CropUtils(void);
    virtual int32_t HandleRegion(const sdc_yuv_frame_s &origin, const sdc_region_s &region, sdc_yuv_frame_s &dst) const;

protected:
    int32_t CropRegion(const sdc_yuv_frame_s &origin, const sdc_region_s &region, sdc_yuv_frame_s &dst) const;
};

// 对YuvFrame进行缩放，支持放大/缩小
class ZoomUtils : public IveUtils
{
public:
    ZoomUtils(void);
    virtual ~ZoomUtils(void);
    virtual int32_t HandleRegion(const sdc_yuv_frame_s &origin, const sdc_region_s &region, sdc_yuv_frame_s &dst) const;

protected:
    int32_t ZoomRegion(const sdc_yuv_frame_s &origin, const sdc_region_s &region, sdc_yuv_frame_s &dst) const;
};

// 对YuvFrame先裁剪指定区域再缩放
class CropAndZoomUtils : public IveUtils
{
public:
    CropAndZoomUtils(void);
    virtual ~CropAndZoomUtils(void);
    virtual int32_t HandleRegion(const sdc_yuv_frame_s &origin, const sdc_region_s &region, sdc_yuv_frame_s &dst) const;

protected:
    int32_t CropAndZoomRegion(const sdc_yuv_frame_s &origin, const sdc_region_s &region, sdc_yuv_frame_s &dst) const;
};

template <typename T>
class IveUtilsFactory
{
public:
    static std::shared_ptr<IveUtils> Create(void)
    {
        return std::make_shared<T>();
    }

private:
    IveUtilsFactory(void) {}
    ~IveUtilsFactory(void) {}
};

#endif /* __IVE_MANAGER_H__ */

