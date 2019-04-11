#pragma once
#include "PortalClient.h"
#include "IAtwService.h"
#include "IAtwClient.h"

#include "HeadTrackerThread.h"
#include "HeadTracker.h"

#include "AtwRenderFpsObserver.h"

using namespace android;

namespace allwinner
{

class PortalClientLocal : public PortalClient
{
public:
    PortalClientLocal();
    virtual ~PortalClientLocal();

    virtual int init(ANativeWindow** nativeSur);
    virtual int prepareFrame(uint64_t frameNumber, eye_t eye);
    virtual int swapToScreen(uint64_t frameNumber, const avrQuatf &orientation, uint64_t expectedDisplayTimeInNano = 0);
    virtual int doReprojection(int mode);
    virtual int setTracker(PTRFUN_HeadTracker cb, int run);

protected:
    virtual int createAtwSurface(ANativeWindow** nativeSur, uint32_t width, uint32_t height, int format, int flags);
    status_t openService();
    status_t enableReprojection(bool enable);

private:
    sp<IAtwService> mService = 0;
    sp<IAtwClient> mClient = 0;

    ANativeWindow *mNativeWindow = NULL;

    int mViewportWidth = -1;
    int mViewportHeight = -1;
    int mDisplayWidth = -1;
    int mDisplayHeight = -1;

    sp<HeadTrackerThread> mHeadTrackerThread = 0;

    uint64_t mFrameNumber = 1;
#if 0
    EGLDisplay mDisplay = EGL_NO_DISPLAY;
    EGLSurface mSurface = EGL_NO_SURFACE;
#endif

    AtwRenderFpsObserver mFpsObserver;
    bool mReprojectionEnabled = false;
    int32_t mDefaultRefreshRate = 60;
    const char* mDefaultRenderFps = "60";
    const char* mReprojectionRenderFps = "30";

    int mClientID;
private:
    friend class PortalFactory;
};

}; // namespace allwinner
