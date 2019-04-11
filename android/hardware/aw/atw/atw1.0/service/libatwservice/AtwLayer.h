#pragma once

#include <stdint.h>
#include <deque>
#include <gui/ConsumerBase.h>
#include <gui/BufferQueue.h>

#include <utils/RefBase.h>

#include <ui/PixelFormat.h>
#include "AtwConsumer.h"
#include "AtwLockless.h"

#include <ui/Fence.h>

namespace android{

class AtwLayer : public ConsumerBase::FrameAvailableListener
{
public:
    AtwLayer(pid_t pid);
    virtual ~AtwLayer();

    status_t setBuffers(uint32_t w, uint32_t h, PixelFormat format);

    bool getLatestAvailableBuffer(BufferItem *find, bool discardAcquireFence);
    void releaseBuffer(sp<Fence> fence, const BufferItem& item);

    // override ConsumerBase::FrameAvailableListener
    virtual void onFrameAvailable(const BufferItem& item);
    virtual void onFrameReplaced(const BufferItem&  item);

    inline sp<IGraphicBufferProducer> getProducer()
    {
        return mProducer;
    }

    inline pid_t getPid()
    {
        return mPid;
    }
private:
    pid_t                      mPid;

    PixelFormat                mFormat;

    sp<IGraphicBufferProducer> mProducer;
    sp<IGraphicBufferConsumer> mConsumer;
    sp<AtwConsumer>            mAtwConsumer;

    pthread_mutex_t             mLock;
    int32_t                     mQueuedNums = 0;
    std::deque<BufferItem>      mBuffersAcquired;

    BufferItem                  mLastReleasedBuffer;

    std::vector<BufferItem>     mLocalBuffers;
};

} // namespace android