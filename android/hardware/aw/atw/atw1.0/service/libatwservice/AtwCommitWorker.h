#pragma once
#include <utils/RefBase.h>
#include <pthread.h>
#include <vector>
#include <deque>
#include <mutex>

#include "AtwVsync.h"
#include "vsync_thread/hwtw_vsyncthread.h"
#include <gui/BufferItem.h>
#include "AtwHal.h"

#include "AtwTypes.h"

#include "AW_Math.h"

#include "AtwCalculator.h"

#include <ui/Fence.h>

const int32_t VSYNC_CALLBACK_ID = 0;

namespace android
{

class AtwHal;
class AtwCalculator;
class IHeadTracker;
class AtwClient;
class AtwLayer;

class AtwCommitWorker : public VsyncListener
{
public:
    AtwCommitWorker(sp<AtwHal> device, sp<AtwCalculator> calculator);
    virtual ~AtwCommitWorker();

    status_t initCheck();
    void resetClient(sp<AtwClient> newer);
    void resetVsyncBase(uint64_t vsyncBase, int32_t swPeriod, int32_t vsyncPeriod);

    void resetDebugCtl(int value);

    virtual void onVsync(const HwVsync_t &vsync);
    void LoopOnce();
protected:
    // return false if vsync is not initialized by vsync thread.
    bool GetVsyncState(double &fractionalVsyncNow, uint64_t &hwVsyncBaseInNano, uint64_t &hwVsyncPeriodsInNano);

    void buildDistortionMatrices(WarpInput_t &input);
    void buildTimeWarpMatrices(const avrQuatf &cur, WarpInput_t &input, avrQuatf *pose);
    bool Commit();
    void releaseLastBuffer(const BufferItem& buffer, const FenceLocal_t& fence); // call by commit thread.
    void signalLastBuffer(const HwVsync_t &vsync);// call by vsync thread.

    void freeAllBuffers();

    //Debug Functions
    void ScreenCap(sp<GraphicBuffer> sgb);
private:
    void ShowFps(const BufferItem& current);
    uint64_t mBeginFrameNumber = 0; // 用于统计应用渲染帧率
    typedef struct
    {
        uint64_t frameNumber;
        uint64_t timestamp;

        int fenceval;
    }CommitFrameInfo_t;
    std::vector<CommitFrameInfo_t> mCommitFrames; // 用于统计atw commit 帧率

    uint64_t mLastHwVsyncBaseInNano = 0;
    double mLastSwVsyncBase = 0.0;

private:
    sp<AtwHal> mDevice = 0;
    sp<AtwCalculator> mCalculator = 0;

    BufferItem mLastFrameBuffer = BufferItem();
    sp<Fence> mLastReleaseFence = 0;

    swapProgram_t mSwapProgram;

#if ATW_CONFIG_FORCE_RENDER_LEFT_EYE
    Eye_t mWarpOrder[2] = {EyeTargetLeft, EyeTargetRight}; // 应用先绘制左眼，atw 同样先 warp 左眼
#else
    Eye_t mWarpOrder[2] = {EyeTargetRight, EyeTargetLeft}; // 应用先绘制右眼，atw 同样先 warp 右眼
#endif

#if DEBUG_ONLY_CORRECT_DISTORTION
    bool mDiscardAcquireFence = true;
#else
    bool mDiscardAcquireFence = false;
#endif
    std::mutex mCommitLock;
    sp<AtwClient> mClient = 0;
    std::deque<CommitFrameInfo_t> mCommitted2;

    double mVsyncCount = 0;
    VsyncState_t mState;

    int32_t mCaptureFrames = 0;

    bool mDebugEnabled = false;
    bool mDebugDynamicRenderFps = false;
};

}; // namespace android
