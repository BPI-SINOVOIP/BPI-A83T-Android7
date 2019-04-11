#pragma once


#include <stdint.h>

#include <utils/Errors.h>  // for status_t
#include <utils/RefBase.h>

#include <binder/IInterface.h>

#include <ui/PixelFormat.h>

#include <gui/Surface.h>

#include "AtwTypes.h"

namespace android
{
// 1. IAtwClient 提供接口创建和销毁 surface,
// 2. IAtwClient 提供接口设置 callback
// 3. IAtwClient 提供接口设置 frame pose

class IAtwClient  : public IInterface
{
public:
    DECLARE_META_INTERFACE(AtwClient);

    virtual status_t createSurface(uint32_t w, uint32_t h, PixelFormat format, uint32_t flags, sp<IGraphicBufferProducer>* gbp) = 0;
    virtual status_t destroySurface() = 0;

    virtual status_t setHeadTrackerThread(pid_t tid) = 0;

    virtual status_t setFramePose(const uint64_t frameNumber, const avrQuatf &pose, const uint64_t expectedDisplayTimeInNano = 0) = 0;
    virtual status_t getFramePresentTime(const uint64_t frameNumber, uint64_t *actualPresentTimeInNano) = 0;

    virtual status_t setWarpData(const WarpData_t &warp) = 0;
    virtual status_t getServiceTimingState(ServiceTimingData_t *timing) = 0;
};


class BnAtwClient : public BnInterface<IAtwClient> {
public:
    virtual status_t onTransact(uint32_t code, const Parcel& data,
            Parcel* reply, uint32_t flags = 0);
};

}; // namespace android
