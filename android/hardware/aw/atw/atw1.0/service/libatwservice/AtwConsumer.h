#pragma once
#include <gui/ConsumerBase.h>
#include <gui/IGraphicBufferConsumer.h>

namespace android{

class AtwLayer;
class AtwConsumer : public ConsumerBase
{
public:
    AtwConsumer(const sp<IGraphicBufferConsumer>& consumer, wp<FrameAvailableListener> listener, bool controlledByApp);
    virtual ~AtwConsumer();
    status_t acquireBuffer(BufferItem *item);
    status_t releaseBuffer(int slot, const sp<GraphicBuffer> graphicBuffer, uint64_t frameNumber = 0);
    status_t addDisplayReleaseFence(int slot, const sp<GraphicBuffer> graphicBuffer, const sp<Fence>& fence);
};

} // namespace android