#pragma once
#include "AtwLockless.h"
#include "AtwTypes.h"
#include "AtwTimer.h"

typedef struct VsyncState
{
    // 转换 NanoTime() 距离 latestHwVsyncBaseInNano 过去多少个 latestHwVsyncPeriodInNano
    void GetLastestVsyncState(double &vsyncNow, uint64_t &latestHwVsyncBaseInNano, uint64_t &latestHwVsyncPeriodInNano);

    // 返回对应 framePoint 所代表的系统显示时间.  framePoint 通常是 X.5 或者 X.0 之类的时间点，代表第几个 vsync 的某个时刻点
    double FramePointTimeInSeconds( const double framePoint );

    void UpdateVsync(const struct HardwareVsync& vsync);
private:
    LocklessUpdater<struct HardwareVsync> mHwVsync;  // 使用基于 c++11 atomic_flag 实现的 spinlock， 因此可以安全的读写这些值而不用担心多线程并发问题。
                                        // 同时因为不用加锁， warp thread 和 render thread 不会被 blocked 住。
                                        // oculus 这样设计的原因是因为它的 vsync 是由 ui thread 通过 jvm 的方式设置下来，因此会出现 race condition
                                        // 我们似乎不需要这样设计 ？
}VsyncState_t;