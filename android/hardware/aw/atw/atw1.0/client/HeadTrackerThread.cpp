#include <stdint.h>
#include <chrono> // for std::chrono
#include <string>
#include "HeadTrackerThread.h"
#include "AtwLog.h"
#include "AtwTimer.h"

namespace allwinner
{

const int HwVsyncOutOfTimeLimit = 16;

HeadTrackerThread::HeadTrackerThread(sp<IAtwClient> client, PTRFUN_HeadTracker cb) : Thread(false), mClient(client), mTrackerCallback(cb){}

HeadTrackerThread::~HeadTrackerThread() {}

status_t HeadTrackerThread::readyToRun()
{
    // // run itself. and will be paused in threadloop if no client available.
    // run("commit-thread", PRIORITY_URGENT_DISPLAY + PRIORITY_MORE_FAVORABLE);
    // // set commit thread to SCHED_FIFO to minimize jitter
    // struct sched_param param = {0};
    // param.sched_priority = 2;
    // if (sched_setscheduler(this->getTid(), SCHED_FIFO, &param) != 0) {
    //     ALOGE("Couldn't set SCHED_FIFO for commit-thread");
    // }
    UpdateServiceTimingState();
    return NO_ERROR;
}

long long HeadTrackerThread::NextWarpTime(long long now)
{
    long long WarpPeriod = mWarpPeriod;
    long long warpCountSinceLastVsyncBase = (now - mVsyncBase) / WarpPeriod + 1;
    long long nextWarpTime = warpCountSinceLastVsyncBase * WarpPeriod + mVsyncBase;
    return nextWarpTime;
}

void HeadTrackerThread::Transaction(const WarpData_t &warp)
{
    status_t result = mClient->setWarpData(warp);
    if(result != NO_ERROR)
    {
        _LOGE("set warp data to service failed");
    }
}

bool HeadTrackerThread::threadLoop()
{
    if(mClient == 0 || mTrackerCallback == 0)
    {
        _LOGV("stop headtracker report thread");
        return false;
    }

    const long long transactionLatencyInNano = 1e6; // 假设获取四元数+传输四元数到服务的时间总计为 1ms, TODO: 我们要根据系统负载，应用回调函数性能来动态调整这个值。
    long long now = NanoTime();
    long long nextWarp = NextWarpTime(now);
    long long deadline = nextWarp - 0.25f*mVsyncPeriod - transactionLatencyInNano; // service 提前 0.25个周期开始处理eyebuffer，因此我们必须保证能在这个时间点之前将数据成功送入。
    if(deadline < now)// 假设 deadline 不会比 now 早超过1周期
    {
        deadline = deadline + mWarpPeriod;
        nextWarp = nextWarp + mWarpPeriod;
    }

    // sleep to timepoint, then get poses from callback and send to service.
    WarpData_t warp;
    warp.timestamps[0] = nextWarp;
    warp.timestamps[1] = nextWarp + mWarpPeriod;
    SleepUntilTimePoint(deadline*1e-9, false);
    mTrackerCallback(warp.timestamps[0], warp.poses[0]);
    mTrackerCallback(warp.timestamps[1], warp.poses[1]);
    Transaction(warp);// send to service

    // update service timing state.
    mSendTimes++;
    if(mSendTimes >= HwVsyncOutOfTimeLimit || true == mNeedResync)
    {
        mNeedResync = false;
        mSendTimes = 0;
        UpdateServiceTimingState();
    }

    // sleep to warp points, then ready for next warp.
    SleepUntilTimePoint(nextWarp*1e-9, false);

    return true;
}

void HeadTrackerThread::UpdateServiceTimingState()
{
    ServiceTimingData_t timing;
    mClient->getServiceTimingState(&timing);
    if(timing.period <= 0)
    {
        _LOGV("get timing state from service = {%lld, %lld} ", timing.base, timing.period);
        const long long defaultTimingBase = NanoTime();
        const long long defaultTimingPeriod = 1e9/60;
        mVsyncBase = defaultTimingBase;
        mVsyncPeriod = defaultTimingPeriod;
        mWarpPeriod = mVsyncPeriod/2;
        mNeedResync = true;
    }
    else
    {
        mVsyncBase = timing.base;
        mVsyncPeriod = timing.period;
        mWarpPeriod = mVsyncPeriod/2;
    }

}

}; // namespace allwinner