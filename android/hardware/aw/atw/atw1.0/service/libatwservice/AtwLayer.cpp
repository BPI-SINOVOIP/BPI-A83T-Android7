#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <utils/Trace.h>
#include <cutils/properties.h>
#include "AtwLayer.h"
#include "AtwLog.h"

#include <gui/GraphicBufferAlloc.h>

namespace android{

const int MAX_BUFFER_COUNT = 3;
//const int MAX_ACQUIRE_BUFFER_COUNT = 1;
//const int MAX_DEQUEUED_BUFFER_COUNT = 1;
const int MAX_ACQUIRE_BUFFER_COUNT = 2;
const int MAX_DEQUEUED_BUFFER_COUNT = 2;
const bool controlledByApp = false;
const bool enabledAsyncMode = false;
AtwLayer::AtwLayer(pid_t pid):
    mPid(pid),
    mFormat(PIXEL_FORMAT_NONE),
    mProducer(0),
    mConsumer(0)
{
    sp<IGraphicBufferProducer> producer;
    sp<IGraphicBufferConsumer> consumer;

    sp<IGraphicBufferAlloc> alloc = new GraphicBufferAlloc();
    BufferQueue::createBufferQueue(&producer, &consumer, alloc);

    mProducer = producer;
    mConsumer = consumer;

    char value[2];
    property_get("ro.sys.vr.forcelandscape", value, "1");
    if(atoi(value) == 2){
        mConsumer->setTransformHint(NATIVE_WINDOW_TRANSFORM_ROT_270);
    }else{
        mConsumer->setTransformHint(NATIVE_WINDOW_TRANSFORM_ROT_90);
    }
    mConsumer->setMaxBufferCount(MAX_BUFFER_COUNT);
    mConsumer->setMaxAcquiredBufferCount(MAX_ACQUIRE_BUFFER_COUNT);
    mProducer->setMaxDequeuedBufferCount(MAX_DEQUEUED_BUFFER_COUNT);
    mProducer->setAsyncMode(enabledAsyncMode);

    pthread_mutex_init(&mLock, 0);
    mBuffersAcquired.clear();
    mLocalBuffers.clear();
    mLocalBuffers.reserve(64);
    mLocalBuffers.resize(64);

    // NOTE: class pointer this must be very carefull when work with sp.
    mAtwConsumer = new AtwConsumer(consumer, this, controlledByApp);
}

AtwLayer::~AtwLayer()
{
    _LOGV("#### destroy layer %d", mPid);
}

status_t AtwLayer::setBuffers(uint32_t w, uint32_t h, PixelFormat format)
{
    //_LOGV("setBuffers: %d %d %d", w,h,format);
    mFormat = format;
    mAtwConsumer->setDefaultBufferSize(w, h);
    mAtwConsumer->setDefaultBufferFormat(format);
    return 0;
}

void AtwLayer::onFrameAvailable(const BufferItem& item)
{
    //_LOGV("LAYER: QUEUED frame=%" PRIu64 ".", (uint64_t)item.mFrameNumber);
    ATRACE_INT64("Buffer-Queued", item.mFrameNumber);
    if(item.mFrameNumber <= 3)
    { // for debug
        _LOGV("onFrameAvailable frame=%" PRIu64 ".", (uint64_t)item.mFrameNumber);
    }

    pthread_mutex_lock(&mLock);
    mQueuedNums++;
    pthread_mutex_unlock(&mLock);
}

bool AtwLayer::getLatestAvailableBuffer(BufferItem *find, bool discardAcquireFence)
{
    ATRACE_CALL();
    if(mBuffersAcquired.size() <= MAX_ACQUIRE_BUFFER_COUNT) // bufferqueue allow consumer allow acquire MAX_ACQUIRE_BUFFER_COUNT+1 buffers for replace the old buffer.
    {
        pthread_mutex_lock(&mLock);
        auto acquireNum = mQueuedNums;
        for(auto i=0; i<acquireNum; i++)
        {
            BufferItem item;
            status_t result = mAtwConsumer->acquireBuffer(&item);
            if(result != NO_ERROR)
            {
                break;
            }
            if(item.mSlot == -1)
            {
                _FATAL("FATAL: acquire buffer return invalid slot");
            }
            mQueuedNums--;

            //_LOGV("ACQUIRED: frame(%" PRIu64 "), slot(%d), buffer(%p)", (uint64_t)item.mFrameNumber, item.mSlot, item.mGraphicBuffer.get());
            ATRACE_INT64("Buffer-Acquired", item.mFrameNumber);

            if(mLocalBuffers[item.mSlot].mSlot == -1)
            {
                mLocalBuffers[item.mSlot] = item;
            }

            if(item.mGraphicBuffer.get() == nullptr)
            {
                if(mLocalBuffers[item.mSlot].mGraphicBuffer.get() == nullptr)
                {
                    _FATAL("%s %d",__func__, __LINE__);
                }
                item.mGraphicBuffer = mLocalBuffers[item.mSlot].mGraphicBuffer;
            }

            mBuffersAcquired.push_front(item);
        }
        if(mQueuedNums<0)
        {
            mQueuedNums = 0;
        }
        pthread_mutex_unlock(&mLock);
    }
    // else
    // {
    //     _LOGV("LAYER: already acquired %d, max acquired num = %d", mBuffersAcquired.size(), MAX_ACQUIRE_BUFFER_COUNT);
    // }

    bool hasFound = false;
    std::deque<BufferItem>::iterator itr = mBuffersAcquired.begin();
    while(itr != mBuffersAcquired.end())
    {
        BufferItem &item = *itr;
        if(hasFound == true)
        {
            // release the older frame.
            //_LOGV("LAYER: drop frame(%" PRIu64 "), slot(%d), buffer(%p)", (uint64_t)item.mFrameNumber, item.mSlot, item.mGraphicBuffer.get());
            mAtwConsumer->addDisplayReleaseFence(item.mSlot, item.mGraphicBuffer, Fence::NO_FENCE);
            mAtwConsumer->releaseBuffer(item.mSlot, item.mGraphicBuffer, item.mFrameNumber);
            itr = mBuffersAcquired.erase(itr);
            continue;
        }

        sp<Fence> acquireFence = item.mFence;
        if(acquireFence->isValid() == false)
        {
            // release the error frame.
            _LOGE("LAYER:: found frame(%" PRIu64 ") fence invalid", (uint64_t)item.mFrameNumber);
            mAtwConsumer->addDisplayReleaseFence(item.mSlot, item.mGraphicBuffer, Fence::NO_FENCE);
            mAtwConsumer->releaseBuffer(item.mSlot, item.mGraphicBuffer, item.mFrameNumber);
            itr = mBuffersAcquired.erase(itr);
            continue;
        }

        nsecs_t status = acquireFence->getSignalTime();
        if(status == -1)
        {
            // release the error frame.
            _LOGE("LAYER: frame(%" PRIu64 "), slot(%d), buffer(%p) fence error", item.mFrameNumber, item.mSlot, item.mGraphicBuffer.get());
            mAtwConsumer->addDisplayReleaseFence(item.mSlot, item.mGraphicBuffer, Fence::NO_FENCE);
            mAtwConsumer->releaseBuffer(item.mSlot, item.mGraphicBuffer, item.mFrameNumber);
            itr = mBuffersAcquired.erase(itr);
            continue;
        }
        if(status != INT64_MAX || discardAcquireFence == true)
        {
            // if this buffer already signaled, we can use it without tearing risk. if not, tearing maybe occurred.
            hasFound = true;
            *find = item;
            itr = mBuffersAcquired.erase(itr);
            continue;
        }
        //_LOGV("LAYER: frame(%" PRIu64 "), slot(%d), buffer(%p) not ready", (uint64_t)item.mFrameNumber, item.mSlot, item.mGraphicBuffer.get());
        itr++;
    }
    return hasFound;
}

void AtwLayer::releaseBuffer(sp<Fence> fence, const BufferItem& item)
{
    status_t result = NO_ERROR;
    result = mAtwConsumer->addDisplayReleaseFence(item.mSlot, item.mGraphicBuffer, fence);
    if(result != NO_ERROR)
    {
        _FATAL("FATAL: set release fence to slot[%d] buffer[%p] failed", item.mSlot, item.mGraphicBuffer.get());
    }

    result = mAtwConsumer->releaseBuffer(item.mSlot, item.mGraphicBuffer, item.mFrameNumber);
    if(result != NO_ERROR)
    {
        _LOGE("ERROR: release slot[%d] buffer[%p] frame=%" PRIu64 " failed. result=%d", item.mSlot, item.mGraphicBuffer.get(), item.mFrameNumber, result);
    }
    //_LOGV("### LAYER: frame=%" PRIu64 ", slot[%d] buffer[%p] released", item.mFrameNumber, item.mSlot, item.mGraphicBuffer.get());
}

void AtwLayer::onFrameReplaced(const BufferItem&  item)
{
    _LOGV("LAYER: replaced frame=%" PRIu64 "", item.mFrameNumber);
}

} // namespace android