#pragma once
#include <utils/Thread.h>
#include <IAtwClient.h>
#include "AtwTypes.h"

using namespace android;
namespace allwinner
{

// 1. 线程定期从 service 端拿回 vsync, vsync_periods
// 2. 线程做循环：
//   [1] get 当前时间
//   [2] 判断service 下一次 warp 时需要的头部姿态数据
//   [3] 通过回调函数获得前述的头部姿态数据
//   [4] 写回头部姿态数据给服务
class HeadTrackerThread : public Thread
{
public:
    HeadTrackerThread(sp<IAtwClient> client, PTRFUN_HeadTracker cb);
    virtual ~HeadTrackerThread();
    virtual status_t readyToRun();

protected:
    virtual bool threadLoop();

private:
    long long NextWarpTime(long long now);
    void Transaction(const WarpData_t &data);
    void UpdateServiceTimingState();

private:
    sp<IAtwClient> mClient = 0;
    PTRFUN_HeadTracker mTrackerCallback = NULL;

    bool mNeedResync = true;
    int mSendTimes = 0;
    long long mVsyncBase = 0;
    long long mVsyncPeriod = 0;
    long long mWarpPeriod = 0;
};

}; // namespace allwinner