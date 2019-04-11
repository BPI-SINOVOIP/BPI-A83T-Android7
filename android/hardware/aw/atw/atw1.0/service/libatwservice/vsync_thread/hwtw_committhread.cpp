#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <utils/Trace.h>

#include "hwtw_committhread.h"
#include "hwtimewarp.h"

namespace android
{

AtwHalCommit::AtwHalCommit(struct hwtimewarp_device_1 *device) : Thread(false), mDevice(device), mMessageQueue(100){}

AtwHalCommit::~AtwHalCommit(){}

status_t AtwHalCommit::readyToRun()
{
    return NO_ERROR;
}

bool AtwHalCommit::threadLoop()
{
    for(;;)
    {
        const char * msg = mMessageQueue.GetNextMessage();
        if(!msg)
        {
            break;
        }
        Command( msg );
        free( (void *)msg );
    }

    mMessageQueue.SleepUntilMessage();
    return true;
}

void AtwHalCommit::PostCommit(uint64_t frameNumber, buffer_handle_t color, buffer_handle_t coefficent)
{
    std::lock_guard<std::mutex> guard(mMutex);
    int idx = frameNumber % 3;
    if(mCommitInfos[idx].frameNumber != 0)
    {
        _LOGE("WARNING: frame=%" PRIu64 " still in commit thread local caches, we will drop this frame definitly !", mCommitInfos[idx].frameNumber);
    }

    // override it.
    mCommitInfos[idx].frameNumber = frameNumber;
    mCommitInfos[idx].color = color;
    mCommitInfos[idx].coefficent = coefficent;

    // post a message to our thread loop, this will unblock our thread.
    mMessageQueue.PostPrintf("commit %lld", frameNumber);

    // 统计帧重复显示的概率
    if(mDebugCtl)
    {
        ReportRepeatRatio(frameNumber);
    }
}

void AtwHalCommit::resetDebugCtl(bool ctrl)
{
    if(mDebugCtl != ctrl)
    {
        _LOGV("hal commit debug %s", ctrl ? "enabled":"disabled");
        mDebugCtl = ctrl;
    }
}

void AtwHalCommit::ReportRepeatRatio(uint64_t frameNumber)
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

void AtwHalCommit::Command(const char* msg)
{
    if( false == allwinner::MatchesHead( "commit",msg ) )
    {
        _LOGE("hal commit thread meets unknown message: %s", msg);
        return;
    }

    AtwData_t commit;
    uint64_t frameNumber = 0;
    sscanf(msg, "commit %lld", &frameNumber);
    {
        std::lock_guard<std::mutex> guard(mMutex);
        int idx = frameNumber % 3;
        if(mCommitInfos[idx].frameNumber != frameNumber)
        {
            _LOGE("WARNING: frame=%" PRIu64 " not match to commit thread local caches which is %" PRIu64 ".", frameNumber, mCommitInfos[idx].frameNumber);
            //return;
        }

        mCommitInfos[idx].frameNumber = 0; // clear it.
        commit.hnd_pixel = mCommitInfos[idx].color;
        commit.hnd_coeffcient = mCommitInfos[idx].coefficent;
    }

    // set to display driver
    ATRACE_NAME("Commit");
    mDevice->setAtwLayer(mDevice, &commit);
    return;
}

};// namespace android