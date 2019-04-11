#pragma once

#include <utils/Errors.h>  // for status_t
#include <utils/RefBase.h>
#include <system/window.h> // for buffer_handle_t
#include <ui/GraphicBuffer.h>

#include "vsync_thread/hwtw_vsyncthread.h"
#include "vsync_thread/hwtw_committhread.h"
#include "AtwTypes.h"

struct hwtimewarp_device_1; // namespace was not in android.
namespace android
{

typedef struct
{
    int fd;
    int value;
}FenceLocal_t;

class AtwHal : public virtual RefBase
{
public:
    AtwHal();
    virtual ~AtwHal();
    status_t initCheck();

    status_t setBlank();

    FenceLocal_t setAtwLayer(uint64_t frameNumber, buffer_handle_t pixelsHnd, buffer_handle_t coefficentsHnd);

    status_t setVsyncListener(sp<VsyncListener> listener);
    status_t enableVsync(bool enable);
    status_t resetDebugCtl(int value);

    status_t getSwapProgram(swapProgram_t &config);
    status_t getDisplayInfo(DisplayInfo_t &info);

    status_t timelineAdd2(int targetValue);

protected:
    FenceLocal_t createFence();

private:
    struct hwtimewarp_device_1 *mAtwDevice = nullptr;
    sp<VsyncThread> mVsyncThread = 0;

#if ENABLE_ASYNC_COMMIT
    sp<AtwHalCommit> mCommit = 0;
#else
    // 重复帧统计机制
    void ReportRepeatRatio(uint64_t frameNumber);
    uint64_t mLastFrameNumber = 0;
    uint32_t mRepeatCount = 0;
    uint32_t mCommitCount = 0;
#endif

    int mSyncDev = -1; // fence 设备节点。
    int mSyncPtVal = 0;
    int mTimeValue = 0;

    bool mDebugCtl = false;

    sp<GraphicBuffer> mBlankBuffer = 0;
};

}; // namespace android