#pragma once

#include <vector>
#include <mutex>
#include <utils/Thread.h>
#include "MessageQueue.h"
#include "vsync_thread/hwtw_vsyncthread.h"
#include "AtwTypes.h"

namespace android
{

class AtwThread;
class HwVsyncUpdater  : public VsyncListener
{
public:
    HwVsyncUpdater(sp<AtwThread> thread, int autoSyncCount);
    virtual ~HwVsyncUpdater();
    virtual void onVsync(const HwVsync_t &vsync);
private:
    sp<AtwThread> mThread; // get MessageQueue by its interface, and set update vsync state message to it.
    int mAutoSyncCount;
    uint64_t mLastTimeStamp = 0;
    int32_t mVsyncPeriod = 0;
    int32_t mHwPeriodSaved = 1e9/60; // 根据多个硬件vsync实际计算出来的 vsync period. 默认值为 60hz
    int mCurrentCount = 0;
};

class AtwService;
class AtwHal;
class AtwClient;
class AtwCalculator;
class AtwLayer;
class AtwCommitWorker;
class AtwThread : public Thread
{
public:
    AtwThread(sp<AtwHal> device, sp<AtwService> service, const DisplayInfo_t &info);
    virtual ~AtwThread();
    virtual status_t readyToRun();
    status_t initCheck();

    inline allwinner::MessageQueue &getMessageQueue()
    {
        return commitMessageQueue;
    }
protected:
    virtual bool threadLoop();

    void Command(const char* msg);
    void handleAttach(pid_t pid);
    void handleDettach(pid_t pid);
    void handleVsyncUpdate(int32_t swPeriod, int32_t hwPeriod, uint64_t vsyncBase);

private:
    sp<HwVsyncUpdater>        mVsyncSource;
    long long mSwPeriodSaved  = 1e9/60; // 传递给 client 的 sw vsync period， 有可能和 hwPeriod 不同. 用于控制应用的渲染帧率。
    long long mHwPeriodSaved  = 1e9/60; // 传递给 app 的 hw vsync period, 让 app 按照 hw period 来送头部朝向数据。

    sp<AtwService>        mService;
    sp<AtwHal>            mDevice;
    sp<AtwCommitWorker>   mWorker;
    sp<AtwCalculator>     mCalculator;

    sp<AtwClient>   mCurrentClient;

    allwinner::MessageQueue    commitMessageQueue;

    DisplayInfo_t mDispInfo;
};



};