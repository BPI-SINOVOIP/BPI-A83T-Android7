#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <utils/Trace.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if 0
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

#include <errno.h>
#include <math.h>
#include <unistd.h>            // for usleep
#include <time.h>

#include <cutils/properties.h>
#include "PortalClientLocal.h"
#include "AtwLog.h"
#include "AtwTimer.h"

namespace allwinner
{
    PortalClientLocal::PortalClientLocal():PortalClient()
    {
        uint64_t timestamps = NanoTime();
        uint32_t tid = (uint32_t) gettid();
        mClientID = ((tid & 0xff)<<16) | (timestamps & 0xff);
        _LOGV("Construct AtwClient. version=%4.2f. pid=%d, mClientID=%d", ATW_VERSION_NUM, getpid(), mClientID);
        if(NO_ERROR != openService())
        {
            mService = 0;
            mClient = 0;
            mClientID = 0;
        }
    }

    PortalClientLocal::~PortalClientLocal()
    {
        _LOGV("Destroy AtwClient. version=%4.2f. pid=%d, mClientID=%d", ATW_VERSION_NUM, getpid(), mClientID);

        if(NULL != mNativeWindow)
        {
            ANativeWindow_release(mNativeWindow);
            mNativeWindow = NULL;
        }

        if(mHeadTrackerThread != 0)
        {
            mHeadTrackerThread->requestExitAndWait();
            mHeadTrackerThread = 0;
        }
        if(mClient != 0 && mService != 0)
        {
            mService->destroyAtwConnection(mClientID); // now we can not get AtwLayer from AtwClient.
            mClient = 0;
            mService = 0;
        }
    }

    status_t PortalClientLocal::openService()
    {
        status_t result          = NO_ERROR;
        sp<IServiceManager> sm   = 0;
        sp<IBinder>      binder  = 0;

        sm = defaultServiceManager();
        binder  = sm->getService(String16(ATW_SERVICE_STRING16));

        mService = interface_cast<IAtwService>(binder);
        if(mService == 0)
        {
            _LOGE("Can not get atwservice, err=%s", strerror(errno));
            return -1;
        }

        result = mService->createAtwConnection(mClientID, &binder);
        if( NO_ERROR != result )
        {
            _LOGE("Can not create atw connection from service . ret=%d, err=%s", result, strerror(errno));
            return -1;
        }

        mClient = interface_cast<IAtwClient>(binder);// cast binder to IAtwClient.
        if(mClient == 0)
        {
            _LOGE("cast IBinder to IAtwClient object failed ");
            return -1;
        }

        return result;
    }


    int PortalClientLocal::init(ANativeWindow** nativeSurf)
    {
        status_t result          = NO_ERROR;
        if(mService == 0 || mClient == 0)
        {
            return -1;
        }

        if(nativeSurf == nullptr)
        {
            _LOGE("paramerter nativeSurf is nullptr.");
            return -1;
        }

        int format = 4;
        int flags = 0;
        result = mService->getDisplayInfo(mDisplayWidth, mDisplayHeight);
        if(result != NO_ERROR)
        {
            _LOGE("get display info failed! result=%d", result);
            return -1;
        }
        mViewportWidth = mDisplayWidth; // maybe we can do performance optimazation by scale down the viewport dims.
        mViewportHeight = mDisplayHeight;
        if(0 != createAtwSurface(nativeSurf, mViewportWidth, mViewportHeight, format, flags))
        {
            _LOGE("create atw surface failed! result=%d", result);
            return -1;
        }
        mNativeWindow = *nativeSurf;
        ANativeWindow_acquire(mNativeWindow);
        _LOGV("init client ok. returned sur=%p", mNativeWindow);


        mFpsObserver.ResetReprojectionCondition(1e9/mDefaultRefreshRate, 5, 2, 4);
        return 0;
    }

    int PortalClientLocal::setTracker(PTRFUN_HeadTracker cb, int run)
    {
        if(run == 0 && mHeadTrackerThread != 0)
        {
            _LOGV("stop headtracker thread");
            mHeadTrackerThread->requestExitAndWait();
            mHeadTrackerThread = 0;
        }
        else if(run == 1 && cb != NULL)
        {
            _LOGV("start headtracker thread");
            if(mHeadTrackerThread != 0)
            {
                _LOGV("recreate headtracker thread");
                mHeadTrackerThread->requestExitAndWait();
                mHeadTrackerThread = 0;
            }
            mHeadTrackerThread = new HeadTrackerThread(mClient, cb);
            mHeadTrackerThread->run("ht-thread", PRIORITY_URGENT_DISPLAY + PRIORITY_MORE_FAVORABLE);
        }
        else
        {
            _LOGE("cb=%p, run=%d, cur=%p", cb, run, mHeadTrackerThread.get());
        }
        return 0;
    }

    static bool CheckNativeWindow(Surface* surf, ANativeWindow *win)
    {
        int fenceFd;
        android_native_buffer_t *dequeued = nullptr;

        if(surf == nullptr || win == nullptr)
        {
            _LOGE("found win=%p", win);
            return false;
        }

        status_t result = NO_ERROR;

        // Check Surface api
        int rWidth, rHeight, rFormat;
        result = surf->query(NATIVE_WINDOW_WIDTH, &rWidth);
        if(result != NO_ERROR)
        {
            _LOGE("get native window width return %d", result);
            return false;
        }
        result = surf->query(NATIVE_WINDOW_HEIGHT, &rHeight);
        if(result != NO_ERROR)
        {
            _LOGE("get native window height return %d", result);
            return false;
        }
        result = surf->query(NATIVE_WINDOW_FORMAT, &rFormat);
        if(result != NO_ERROR)
        {
            _LOGE("get native window format return %d", result);
            return false;
        }
        _LOGV("### create surf=%p, width=%d, height=%d, format=%d", surf, rWidth, rHeight, rFormat);

        // Check ANativeWindow api.
        result = win->perform(win, NATIVE_WINDOW_API_CONNECT, NATIVE_WINDOW_API_CPU);
        if(result != NO_ERROR)
        {
            _LOGE(" do api connect return=%d", result);
            return false;
        }

        result = win->dequeueBuffer(win, &dequeued, &fenceFd);
        if(result != NO_ERROR)
        {
            _LOGE(" do dequeueBuffer return=%d", result);
            return false;
        }

        result = win->cancelBuffer(win, dequeued, fenceFd);
        if(result != NO_ERROR)
        {
            _LOGE(" do cancelBuffer return=%d", result);
            return false;
        }

        result = win->perform(win, NATIVE_WINDOW_API_DISCONNECT, NATIVE_WINDOW_API_CPU);
        if(result != NO_ERROR)
        {
            _LOGE(" do api disconnect return=%d", result);
            return false;
        }
        _LOGV("CheckNativeWindow(%p, %p) return NO_ERROR.", surf, win);
        return true;
    }

    int PortalClientLocal::createAtwSurface(ANativeWindow** nativeSurf, uint32_t width, uint32_t height, int format, int flags)
    {
        status_t result = NO_ERROR;

        if(mService == 0 || mClient == 0)
        {
            return -1;
        }

        Surface* surf = nullptr;
        sp<IGraphicBufferProducer> gbp  = 0;
        result = mClient->createSurface(width, height, format, flags, &gbp);
        if(NO_ERROR !=  result)
        {
            _LOGE("create surface from client(%p) failed! result=%d ", mClient.get(), result);
            return -1;
        }
        surf = new Surface(gbp, false);
        if(false == CheckNativeWindow(surf, (ANativeWindow*)surf))
        {
            _FATAL("Check native window failed on pid=%d mClientID=%d, abort...", getpid(), mClientID);
        }

        *nativeSurf = (ANativeWindow*)surf;
        return 0;
    }

    int PortalClientLocal::prepareFrame(uint64_t frameNumber, eye_t eye)
    {
        // Do nothing here, we keep this interface for future use. eg: performance informations, FBO operations.
        UN_USED(frameNumber);
        UN_USED(eye);
        return NO_ERROR;
    }

    static long long       NanoTime()
    {
        // This should be the same as java's system.nanoTime(), which is what the
        // choreographer vsync timestamp is based on.
        struct timespec tp;
        const int status = clock_gettime(CLOCK_MONOTONIC, &tp);
        if ( status != 0 )
        {
            _LOG( "clock_gettime status=%i", status );
        }
        const long long result = (long long)tp.tv_sec * (long long)(1000 * 1000 * 1000) + (long long)(tp.tv_nsec);
        return result;
    }

    static double TimeInSeconds()
    {
            return NanoTime() * 1e-9;
    }

    static float SleepUntilTimePoint( const double targetSeconds, const bool busyWait )
    {
        const char* sTraceTags[2] = {"SleepUntilNextVsync", "BusyWaitUntilNextVsync"};
        ATRACE_NAME(busyWait ? sTraceTags[1]:sTraceTags[0]);

        const float currentSeconds = TimeInSeconds();
        float sleepSeconds = targetSeconds - currentSeconds;
        if ( sleepSeconds > 0 )
        {
            if ( busyWait )
            {
                while( targetSeconds - TimeInSeconds() > 0 )
                {
                }
            }
            else
            {
                // I'm assuming we will never sleep more than one full second.
                timespec    t, rem;
                t.tv_sec = 0;
                t.tv_nsec = sleepSeconds * 1e9;
                nanosleep( &t, &rem );

                //TODO 1: 做插帧的时候，这里会打印比较多， 需要查一下
                // const double overSleep = TimeInSeconds() - targetSeconds;
                // if ( overSleep > 0.001 )// 1 ms
                // {
                //     _LOGV( "Overslept %f seconds", overSleep );
                // }
            }
        }
        else
        {
            // TODO 2: 做插帧的时候，这里会打印比较多， 需要查一下
            // const double overSleep = currentSeconds - targetSeconds;
            // _LOGV( "targetSeconds(%f) < currentSeconds(%f). overSlept=%f", targetSeconds, currentSeconds, overSleep);
            // sleepSeconds = 0.0f;
        }
        return sleepSeconds;
    }

    int PortalClientLocal::swapToScreen(uint64_t frameNumber, const avrQuatf &pose, uint64_t expectedPresentTimeInNano)
    {
        (void) frameNumber;
        (void) expectedPresentTimeInNano;
        ATRACE_NAME("SwapToScreen");
        if(mClient == 0)
        {
            _LOGE("invalid client instance.");
            return -1;
        }

        status_t result = NO_ERROR;
        result = mClient->setFramePose(mFrameNumber, pose, 0);
        mFrameNumber++;

        return 0;
    }

    int PortalClientLocal::doReprojection(int mode)
    {
        float sleptSeconds = 0.0f;
        uint64_t actualPresentTimeInNano = 0;
        status_t result = mClient->getFramePresentTime(0, &actualPresentTimeInNano);
        if(result == NO_ERROR)
        {
            double actualPresentTimeInSeconds = 1.0f * actualPresentTimeInNano * 1e-9;
            sleptSeconds = SleepUntilTimePoint(actualPresentTimeInSeconds, false);
        }

        switch(mode)
        {
            case 1:
            {
                bool enable = mFpsObserver.CollectTimestamp(NanoTime(), sleptSeconds*1e9);
                enableReprojection(enable);
                break;
            }
            case 2:
            {
                enableReprojection(true);
                break;
            }
            case 0:
            default:
            {
                break;
            }
        }
        return NO_ERROR;
    }

    status_t PortalClientLocal::enableReprojection(bool enable)
    {
        //_LOGV("observer suggest %s reprojection", enable ? "enable":"disable");
        const char* strServiceNodeName = "debug.atw.renderfps";
        if(enable != mReprojectionEnabled)
        {
            int reprojectionAttribute = property_get_int32(ATW_SERVICE_DEBUG_ENABLE_REPROJECTION, 0);
            if(reprojectionAttribute == 0)
            {
                //_LOGV("Reprojection is disabled");
                return -1;
            }

            _LOGV("change reprojection from %s to %s", mReprojectionEnabled ? "enable":"disable", enable ? "enable":"disable");
            property_set(strServiceNodeName, enable ? mReprojectionRenderFps : mDefaultRenderFps);
            mReprojectionEnabled = enable;
            if(mReprojectionEnabled == true)
            {
                // 从插帧模式退出需要连续 10s 丢帧不严重。
                mFpsObserver.ResetReprojectionCondition(1e9/mDefaultRefreshRate, 10, 2, 4);
            }
            else
            {
                // 进入非插帧模式， 只需要连续2s丢帧严重，或者5秒内有2秒丢帧严重。
                mFpsObserver.ResetReprojectionCondition(1e9/mDefaultRefreshRate, 5, 2, 4);
            }
        }
        return NO_ERROR;
    }
}; // namespace allwinner
