#pragma once

#include <system/window.h> // for buffer_handle_t
#include <utils/Errors.h>  // for status_t
#include <utils/RefBase.h>
#include <utils/Thread.h>
#include <vector>
#include <mutex>
#include "AtwLog.h"
#include "MessageQueue.h"

struct hwtimewarp_device_1;
namespace android
{

typedef struct
{
    uint64_t        frameNumber;
    buffer_handle_t color;
    buffer_handle_t coefficent;
}CommitInfoLocal_t;

class AtwHalCommit : public Thread
{
public:
    AtwHalCommit(struct hwtimewarp_device_1 *device);
    virtual ~AtwHalCommit();
    virtual status_t readyToRun();
    void PostCommit(uint64_t frameNumber, buffer_handle_t color, buffer_handle_t coefficent);

    void resetDebugCtl(bool ctrl);

protected:
    virtual bool threadLoop();
    void Command(const char* msg);

private:
    struct hwtimewarp_device_1 *mDevice = nullptr;
    allwinner::MessageQueue    mMessageQueue;

    std::mutex mMutex;
    CommitInfoLocal_t mCommitInfos[3];

    bool mDebugCtl = false;

    void ReportRepeatRatio(uint64_t frameNumber);
    uint64_t mLastFrameNumber = 0;
    uint32_t mRepeatCount = 0;
    uint32_t mCommitCount = 0;
};

}; // namespace android