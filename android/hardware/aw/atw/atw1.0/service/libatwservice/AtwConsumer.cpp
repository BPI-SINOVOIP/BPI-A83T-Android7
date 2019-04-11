#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <utils/Trace.h>

#include "AtwLayer.h"
#include "AtwLog.h"

namespace android{

AtwConsumer::AtwConsumer(const sp<IGraphicBufferConsumer>& consumer, wp<FrameAvailableListener> listener, bool controlledByApp):
    ConsumerBase(consumer, controlledByApp) 
{
    this->setFrameAvailableListener(listener);
}

AtwConsumer::~AtwConsumer()
{
    _LOGV("### consumer destroyed");
    this->setFrameAvailableListener(0);
    if(isAbandoned() == false)
    {
        _FATAL("consumer is not not abanded");
    }
}

status_t AtwConsumer::acquireBuffer(BufferItem *item)
{
    return acquireBufferLocked(item, 0);
}

status_t AtwConsumer::releaseBuffer(int slot, const sp<GraphicBuffer> graphicBuffer, uint64_t frameNumber)
{
    ATRACE_INT64("Buffer-Released", frameNumber);
    return releaseBufferLocked(slot, graphicBuffer, EGL_NO_DISPLAY, EGL_NO_SYNC_KHR);
}

status_t AtwConsumer::addDisplayReleaseFence(int slot, const sp<GraphicBuffer> graphicBuffer, const sp<Fence>& fence)
{
    return addReleaseFence(slot, graphicBuffer, fence);
}


} // namespace android