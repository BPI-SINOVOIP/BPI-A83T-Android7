#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <stdio.h>
#include <cutils/properties.h>

#include "AtwCommitThread.h"
#include "AtwHal.h"
#include "AtwClient.h"
#include "AtwCalculator.h"
#include "AtwLayer.h"
#include "AtwCommitWorker.h"
#include "AtwService.h"

namespace android
{

static int MinSyncCount = 16;
HwVsyncUpdater::HwVsyncUpdater(sp<AtwThread> thread, int autoSyncCount):VsyncListener(1001),mThread(thread),mAutoSyncCount(autoSyncCount<MinSyncCount ? MinSyncCount:autoSyncCount)
{}

HwVsyncUpdater::~HwVsyncUpdater()
{}

void HwVsyncUpdater::onVsync(const HwVsync_t &vsync)
{
    bool bNeedReportVsync = false;

    if(mLastTimeStamp == 0)
    {
        // first sample.
        bNeedReportVsync = true;// tell listener as soon as possible to avoid jitter.
        mLastTimeStamp = vsync.timestamp;
    }
    else
    {
        int32_t period = vsync.timestamp - mLastTimeStamp;
        mLastTimeStamp = vsync.timestamp;
        if(mVsyncPeriod != 0)
        {
            // check the stability of this period.
            const int32_t tolerance = mVsyncPeriod*0.1f;
            int32_t error = abs(mVsyncPeriod-period);
            if(error > tolerance)
            {
                _LOGV("average_period=%d, current_period=%d, which error=%d beyond the tolerance=%d. ", mVsyncPeriod, period, error, tolerance);
                mHwPeriodSaved = mVsyncPeriod;
                mLastTimeStamp = 0; // 丢弃这个 vsync sample
                // 重新开始 vsync 周期计算
                mCurrentCount = 0;
                mVsyncPeriod = 0;
            }
            else
            {
                mVsyncPeriod = (mVsyncPeriod+period) / 2;
                mCurrentCount++;
            }
        }
        else
        {
            // we are handling the second sample.
            mVsyncPeriod = period;
        }

        if(mCurrentCount >= mAutoSyncCount)
        {
            bNeedReportVsync = true;
            mHwPeriodSaved = mVsyncPeriod;
            // 重新开始计算 vsync 周期。
            mCurrentCount = 0;
            mVsyncPeriod = 0;
        }
    }

    if(true == bNeedReportVsync)
    {
        int hwPeriod = mHwPeriodSaved; // commit thread 的commit帧率 必须符合 hw.
        int swPeriod = hwPeriod;          // 软件 app 的 render fps, 默认值是等于硬件 commit 的帧率
        long long base = mLastTimeStamp;

        // atw debug 属性 处理
        if(1)
        {
            int value = 0;

            // 动态 render fps
            value = property_get_int32(ATW_SERVICE_DEBUG_DYNAMIC_RENDER_FPS,0);
            if(value > 0)
            {
                // 修改软件 app 的渲染帧率， 根据用户设置的全局属性来设置app render fps.
                swPeriod = 1e9/value;
                //_LOGV("set render fps to %d, change vsync_period from %d to %d", value, hwPeriod, swPeriod);
            }

            // check should we need to enable log
            value = property_get_int32(ATW_SERVICE_DEBUG_SYS_ATTRIBUTE, 0);
            mThread->getMessageQueue().PostPrintf("debug-ctl %d", value);
        }

        // send update message to commit thread.
        mThread->getMessageQueue().PostPrintf("vsync-update %d, %d, %lld", swPeriod, hwPeriod, base);
    }
}

AtwThread::AtwThread(sp<AtwHal> device, sp<AtwService> service, const DisplayInfo_t &dispInfo) : Thread(false), mService(service), mDevice(device), commitMessageQueue(1000)
{
    mDispInfo = dispInfo;
}

AtwThread::~AtwThread(){}

status_t AtwThread::readyToRun()
{
    status_t status = mCalculator->loadResource(); // 加载系数网格，创建系数计算器(加载compute shader)
    if(status != NO_ERROR)
    {
        _LOGE("calculator load resource failed");
        return -1;
    }
    _LOGV("commit thread started");
    return NO_ERROR;
}

status_t AtwThread::initCheck()
{
    // 根据 mDispInfo 的信息， 计算 eyebuffer 的宽和高
    // eyebuffer 的宽就是行扫描的宽.
    // eyebuffer 的高就是行扫描的行数的一半.
    int eye_w = 0;
    int eye_h = 0;
    if(mDispInfo.scn_num == 1)
    {
        // 肯定是竖屏。
        eye_w = mDispInfo.rendertarget_h;
        eye_h = mDispInfo.rendertarget_w / 2;
    }
    else
    {
        eye_w = mDispInfo.scn_width;
        eye_h = mDispInfo.scn_height;
    }

    _LOGV("set eye={%d %d}, disp={%d %d} to calculator", eye_w, eye_h, mDispInfo.scn_width, mDispInfo.scn_height);
    sp<AtwCalculator> calculator = new AtwCalculator(eye_w, eye_h, mDispInfo.scn_width, mDispInfo.scn_height);
    if(calculator->initCheck() != NO_ERROR)
    {
        _LOGE("init check calculator failed");
        return -1;
    }

    sp<AtwCommitWorker> worker = new AtwCommitWorker(mDevice, calculator);
    if(worker->initCheck() != NO_ERROR)
    {
        _LOGE("init check commit worker failed");
        return -1;
    }

    if( NO_ERROR != mDevice->setVsyncListener(worker)) // 启动 vsync thread, 并将 worker 设置为监听者
    {
        _LOGE("init check set vsync listener failed");
        return -1;
    }

    sp<HwVsyncUpdater> vsync = new HwVsyncUpdater(this, 32);
    if( NO_ERROR != mDevice->setVsyncListener(vsync))
    {
        _LOGE("init check set vsync updater failed");
        return -1;
    }

    mCalculator = calculator; 
    mWorker = worker;
    mVsyncSource = vsync;

    // run itself. and will be paused in threadloop if no client available.
    run("commit-thread", PRIORITY_URGENT_DISPLAY + PRIORITY_MORE_FAVORABLE);
    // set commit thread to SCHED_FIFO to minimize jitter
    struct sched_param param = {0};
    param.sched_priority = 2;
    if (sched_setscheduler(this->getTid(), SCHED_FIFO, &param) != 0) {
        ALOGE("Couldn't set SCHED_FIFO for commit-thread");
    }
    return NO_ERROR;
}

void AtwThread::Command(const char* msg)
{
    if( allwinner::MatchesHead( "attach",msg ) )
    {
        pid_t pid = 0;
        sscanf(msg, "attach %d", &pid);
        handleAttach(pid);
        return;
    }

    if( allwinner::MatchesHead( "dettach",msg ) )
    {
        pid_t pid = 0;
        sscanf(msg, "dettach %d", &pid);
        handleDettach(pid);
        return;
    }

    if( allwinner::MatchesHead( "vsync-update",msg ) )
    {
        int32_t swPeriod = 0;
        int32_t hwPeriod = 0;
        uint64_t vsyncBase = 0;
        sscanf(msg, "vsync-update %d, %d, %" PRIu64 ".", &swPeriod, &hwPeriod, &vsyncBase);
        handleVsyncUpdate(swPeriod, hwPeriod, vsyncBase);
        return;
    }

    if( allwinner::MatchesHead( "debug-ctl", msg ))
    {
        //mThread->getMessageQueue().PostPrintf("debug-ctl %d", value);
        int32_t value;
        sscanf(msg, "debug-ctl %d", &value);
        mWorker->resetDebugCtl(value);
        mDevice->resetDebugCtl(value);
        return;
    }

    _LOGE("unknown message: %s", msg);
}

void AtwThread::handleVsyncUpdate(int32_t swPeriod, int32_t hwPeriod, uint64_t vsyncBase)
{
    //_LOGV("vsync update: %d, %" PRIu64 ".", vsyncPeriod, vsyncBase);
    //_LOGV("vsync update. sw=%d, hw=%d", swPeriod, hwPeriod);
    mWorker->resetVsyncBase(vsyncBase, swPeriod, hwPeriod);
    if(mCurrentClient != 0)
    {
        mCurrentClient->UpdateVsyncState(vsyncBase, swPeriod, hwPeriod);
    }
    mSwPeriodSaved = swPeriod;
    mHwPeriodSaved = hwPeriod;
}

void AtwThread::handleAttach(pid_t pid)
{
    _LOGV("handleAttach : pid=%d", pid);

    if(mCurrentClient != 0 && mCurrentClient->getPid() == pid)
    {
        _LOGE("CHECK: retach same pid to thread");
        return;
    }

    sp<AtwClient> client = 0;
    client = mService->getClient(pid);
    if(client.get() == nullptr)
    {
        _LOGE("get client (%d) from service return null", pid);
        return;
    }

    _LOGV("set current client {pid,idx}={%d, %d}", client->getPid(), client->getIndex());
    mCurrentClient = client;
    mWorker->resetClient(client);

    mDevice->enableVsync(true);
    mCurrentClient->UpdateVsyncState(NanoTime(), mSwPeriodSaved, mHwPeriodSaved);

    _LOGV("notify enable atw service");
    const char* atwServiceEnable = "service.atw.enable";
    property_set(atwServiceEnable, "1");
}

void AtwThread::handleDettach(pid_t pid)
{
    _LOGV("handleDettach : pid=%d", pid);

    if(mCurrentClient == 0)
    {
        _LOGE("CHECK: detach null client");
        return;
    }
    if(mCurrentClient->getPid() != pid)
    {
        _LOGV("request dettach %d, but current is %d", pid, mCurrentClient->getPid());
        return;
    }

    mCurrentClient = 0;
    mWorker->resetClient(0);

    //Do not disable the vsync, because it will make the surfaceflinger's frame block as the fence cannot
    //be release in vsync thread. 
    //mDevice->enableVsync(false);

    _LOGV("notify disable atw service");
    const char* atwServiceEnable = "service.atw.enable";
    property_set(atwServiceEnable, "0");
}

bool AtwThread::threadLoop()
{
    // handle message queue.
    for(;;)
    {
        const char * msg = commitMessageQueue.GetNextMessage();
        if(!msg)
        {
            break;
        }
        Command( msg ); // change client in this function.
        free( (void *)msg );
    }

    // if no client, wait next message.
    if(mCurrentClient == 0)
    {
        //_LOGV("no client in commit thread. wait next message");
        commitMessageQueue.SleepUntilMessage();
        return true;
    }

    mWorker->LoopOnce();
    return true;
}

};
