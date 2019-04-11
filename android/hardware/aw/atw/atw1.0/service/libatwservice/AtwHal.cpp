#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <utils/Trace.h>
#include "AtwHal.h"
#include "hwtimewarp.h"
#include "AtwLog.h"

#include "sw_sync/sw_sync.h"

namespace android
{
static hw_module_t* module = nullptr;

AtwHal::AtwHal(){}

AtwHal::~AtwHal()
{
#if ENABLE_ASYNC_COMMIT
    mCommit->requestExitAndWait();
#endif
    mVsyncThread->requestExitAndWait();
    if(mAtwDevice != nullptr)
    {
        hwtimewarp_close_1(mAtwDevice);
        mAtwDevice = nullptr;
    }
}

status_t AtwHal::initCheck()
{
    hw_module_t const* module;
    hwtimewarp_device_1_t *hwtimewarp;
    status_t result = NO_ERROR;
    int fenceDevFd = -1;

    result = hw_get_module(HWTIMEWARP_HARDWARE_MODULE_ID, &module);
    if(result != 0)
    {
        _LOGE("could not open module %s. err=%s", HWTIMEWARP_HARDWARE_MODULE_ID, strerror(errno));
        return -1;
    }
    result = hwtimewarp_open_1(module, &hwtimewarp);
    if(result != 0)
    {
        _LOGE("could not open atw device. err=%s", strerror(errno));
        return -1;
    }

    fenceDevFd = sw_sync_timeline_create();
    if(fenceDevFd <= 0)
    {
        hwtimewarp_close_1(hwtimewarp);
        _LOGE("could not open sw_sync device. err(%s)", strerror(errno));
        return -1;
    }

    uint32_t width = 64;
    uint32_t height = 64;
    PixelFormat format = PIXEL_FORMAT_RGBA_8888;
    uint32_t usage = GraphicBuffer::USAGE_HW_COMPOSER | GraphicBuffer::USAGE_SOFTWARE_MASK;
    _LOGV("create GraphicBuffer(%d, %d, %d, 0x%x) for blank layer.", width, height, format, usage);
    sp<GraphicBuffer> sgb = new GraphicBuffer(width, height, format, usage);
    if(NO_ERROR != sgb->initCheck())
    {
        _LOGE("could not create GraphicBuffer(%d, %d, %d, 0x%x) for blank layer.", width, height, format, usage);
        return -1;
    }

    mBlankBuffer = sgb;
    mAtwDevice = hwtimewarp;
    mSyncDev = fenceDevFd;

    mVsyncThread = new VsyncThread();
#if ENABLE_ASYNC_COMMIT
    mCommit = new AtwHalCommit(mAtwDevice);
    mCommit->run("hal_commit", PRIORITY_URGENT_DISPLAY);
#endif
    return NO_ERROR;
}

status_t AtwHal::setBlank()
{
    _LOGV("set display to blank.");
#if ENABLE_ASYNC_COMMIT
    mCommit->PostCommit(0, mBlankBuffer->getNativeBuffer()->handle, 0);
#else
    AtwData_t commit;
    commit.hnd_pixel = mBlankBuffer->getNativeBuffer()->handle;
    commit.hnd_coeffcient = 0;
    mAtwDevice->setAtwLayer(mAtwDevice, &commit);
#endif
    return NO_ERROR;
}

status_t AtwHal::timelineAdd2(int targetValue)
{
    if(targetValue <= mTimeValue)
    {
        return -1;
    }
    int add = targetValue - mTimeValue;
    int err = sw_sync_timeline_inc(mSyncDev, add);
    mTimeValue = targetValue;
    return err < 0 ? -errno : status_t(NO_ERROR);
}

FenceLocal_t AtwHal::createFence()
{
    int fd = -1;
    int val = ++mSyncPtVal;// 第一个 fence 的 pt = 1, 此时 timeline = 0
    char token[128] = {'\0'};
    sprintf(token, "atw_rel_%d", val);
    fd = sw_sync_fence_create(mSyncDev, token, val);
    //_LOGV("DEBUG: Release Fence Created: token=%s, val(%d), fd(%d)", token, val, fd);
    FenceLocal_t fence = {fd,val};
    return fence;
}

FenceLocal_t AtwHal::setAtwLayer(uint64_t frameNumber, buffer_handle_t pixelsHnd, buffer_handle_t coefficentsHnd)
{
    ATRACE_CALL();
#if ENABLE_ASYNC_COMMIT
    mCommit->PostCommit(frameNumber, pixelsHnd, coefficentsHnd);
#else
    AtwData_t commit;
    commit.hnd_pixel = pixelsHnd;
    commit.hnd_coeffcient = coefficentsHnd;
    mAtwDevice->setAtwLayer(mAtwDevice, &commit);

    if(mDebugCtl)
    {
        ReportRepeatRatio(frameNumber);
    }
#endif
    return createFence();
}

#if !ENABLE_ASYNC_COMMIT
void AtwHal::ReportRepeatRatio(uint64_t frameNumber)
{
    mCommitCount++;
    if(mLastFrameNumber == frameNumber && mLastFrameNumber != 0)
    {
        //_LOGV("WARNING: set frame=%" PRIu64 " to diaplay again.", frameNumber);
        mRepeatCount++;
    }
    mLastFrameNumber = frameNumber;

    const uint32_t maxCount = 100;
    if(mCommitCount >= maxCount)
    {
        float ratio = mRepeatCount * 1.0f / mCommitCount;
        _LOGV("Repeat frame : %d in %d, ratio = %8.4f", mRepeatCount, mCommitCount, ratio);
        mRepeatCount = 0;
        mCommitCount = 0;
    }
}
#endif

status_t AtwHal::setVsyncListener(sp<VsyncListener> listener)
{
    mVsyncThread->registerVsyncListener(listener);
    if(mVsyncThread->isRunning() == false)
    {
        _LOGV("start atw vsync thread");
        mVsyncThread->run("atw-vsync", HAL_PRIORITY_URGENT_DISPLAY);
    }
    return NO_ERROR;
}

status_t AtwHal::enableVsync(bool enable)
{
    _LOGV("%s vsync thread", enable ? "enable" : "disable");
    mAtwDevice->enable(mAtwDevice, enable);
    return NO_ERROR;
}

status_t AtwHal::resetDebugCtl(int value)
{
    bool ctl = value != 0 ? true:false;
    if(mDebugCtl != ctl)
    {
        mDebugCtl = ctl;
        _LOGV("hal debug %s", mDebugCtl ? "enabled":"disabled");
#if ENABLE_ASYNC_COMMIT
        mCommit->resetDebugCtl(mDebugCtl);
#endif
    }
    return NO_ERROR;
}

status_t AtwHal::getSwapProgram(swapProgram_t &config)
{
    config.deltaVsync[0] = 0.75f; // 在 vsyncBase 的基础上， vsyncBase + config.deltaVsync[0] 的时刻才开始 准备/warp first eye
    config.deltaVsync[1] = 1.25f;
    config.predictionPoints[0][0] = 1.0f; // 在 vsyncBase 的基础上， 预测 vsyncBase + 1.0f 是 first eye begin scan 的时间点
    config.predictionPoints[0][1] = 1.5f; // 在 vsyncBase 的基础上， 预测 vsyncBase + 1.5f 是 first eye finish scan 的时间点
    config.predictionPoints[1][0] = 1.5f; // 在 vsyncBase 的基础上， 预测 vsyncBase + 1.5f 是 second eye begin scan 的时间点
    config.predictionPoints[1][1] = 2.0f; // 在 vsyncBase 的基础上， 预测 vsyncBase + 2.0f 是 second eye finish scan 的时间点
    return NO_ERROR;
}

status_t AtwHal::getDisplayInfo(DisplayInfo_t &info)
{
    mAtwDevice->getDisplayInfo(mAtwDevice, &info);
    return NO_ERROR;
}

};