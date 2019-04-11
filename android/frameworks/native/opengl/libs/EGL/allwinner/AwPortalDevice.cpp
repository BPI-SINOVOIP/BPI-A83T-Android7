#include <array>
#include <chrono>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <limits.h>
#include <dirent.h>

#include <android/dlext.h>
#include <cutils/log.h>
#include <cutils/properties.h>

#include "AwPortalDevice.h"

#if defined(__LP64__)
#define ATW_WRAPPER_DIR "/system/lib64"
#else
#define ATW_WRAPPER_DIR "/system/lib"
#endif

namespace android{

static PortalDevice sPortalDevice;
PortalDevice *GetPortalDevice()
{
#if 1
    return &sPortalDevice;
#else
    return NULL;
#endif
}

// ------------------------------------------------------------------------ local C functions for conveniont.
static long long       NanoTime()
{
    struct timespec tp;
    const int status = clock_gettime(CLOCK_MONOTONIC, &tp);
    if ( status != 0 )
    {
        ALOGE( "clock_gettime status=%i", status );
    }
    const long long result = (long long)tp.tv_sec * (long long)(1000 * 1000 * 1000) + (long long)(tp.tv_nsec);
    return result;
}

static void* GetPortalApiByName(void *hnd, const char* symbol)
{
    void *ptr = dlsym(hnd, symbol);
    ALOGD("Get %s=%p from libatw_2.0.so", symbol, ptr);
    return ptr;
}

static bool __loadPortalDriver(portal_driver_hooks_t *dev)
{
    const char* portal_driver_path = ATW_WRAPPER_DIR "/libatw_2.0.so";
    void *so = dlopen(portal_driver_path, RTLD_NOW | RTLD_LOCAL);
    if(so == NULL)
    {
        ALOGE("open %s failed. errno=%d,%s", portal_driver_path, errno, strerror(errno));
        return false;
    }

    dev->libatwclient = so;
    dev->PortalInit = (PFUNC_PortalInit) GetPortalApiByName(so, "PortalInit");
    dev->PortalDestroy = (PFUNC_PortalDestroy) GetPortalApiByName(so, "PortalDestroy");
    dev->PortalSwap = (PFUNC_PortalSwap) GetPortalApiByName(so, "PortalSwap");
    dev->PortalReproj = (PFUNC_PortalReproj) GetPortalApiByName(so, "PortalReproj");
    dev->PortalSetTracker = (PFUNC_PortalSetTracker) GetPortalApiByName(so, "PortalSetTracker");
    return (NULL != dev->PortalInit && NULL != dev->PortalDestroy && NULL != dev->PortalSwap && NULL != dev->PortalReproj && NULL != dev->PortalSetTracker);
}

// ------------------------------------------------------------------- class PortalDevice implementation.
PortalDevice::PortalDevice(){}
PortalDevice::~PortalDevice()
{
    if(Hooks)
    {
        delete Hooks;
        Hooks = NULL;
    }
}

static int __GetThreadName(int tid, char *name)
{
    char fPath[128];
    sprintf(fPath, "/proc/%d/comm", tid);
    FILE *fp = fopen(fPath, "r");
    if(fp == NULL)
    {
        ALOGE("open %s failed, errno=%d, %s", fPath, errno, strerror(errno));
        return -1;
    }
    fgets(name, 128, fp);
    fclose(fp);
    return 0;
}

int PortalDevice::_CheckTidName()
{
    char thread_name[128];
    int tid = (int)gettid();
    if(__GetThreadName(tid, thread_name) < 0)
    {
        return -1;
    }

    const char *black_list[] = {"RenderThread"};
    int cnt = sizeof(black_list)/sizeof(const char *);
    int s0 = strlen(thread_name);
    for(int i=0; i<cnt; i++)
    {
        const char* str = black_list[i];
        int s1 = strlen(str);
        if(strncmp(thread_name, str, s1) == 0)
        {
            //ALOGD("tid=%d, name=%s(%d) is detected", tid, thread_name, s0);
            return -1;
        }
        //ALOGD("tid=%d, name=%s(%d) is passed", tid, thread_name, s0);
    }
    return 0;
}

int PortalDevice::SavePortalSurface(EGLSurface surf)
{
    std::lock_guard<std::mutex> guard(mMutex);
    mSurface = surf;
    return 0;
}

EGLSurface PortalDevice::GetPortalSurface()
{
    std::lock_guard<std::mutex> guard(mMutex);
    return mSurface;
}

int PortalDevice::Reprojection()
{
    std::lock_guard<std::mutex> guard(mMutex);
    if(0 != _CheckDriverApi())
    {
        ALOGE("PortalDevice: get portal driver apis failed");
        return -1;
    }

    return Hooks->PortalReproj(mReprojMode);
}

int PortalDevice::_CheckDriverApi()
{
    if(mFirstCall == true)
    {
        ALOGD("PortalDevice: load portal driver");
        mFirstCall = false; // only load first time.
        Hooks = new portal_driver_hooks_t();
        if(NULL == Hooks)
        {
            ALOGE("PortalDevice: out of memory");
            return -1;
        }
        mDriverStatus = __loadPortalDriver(Hooks);
    }
    return mDriverStatus ? 0 : -1; // true is 0, false is -1.
}

int PortalDevice::SetPortalEnabled(int mode)
{
    std::lock_guard<std::mutex> guard(mMutex);
    if(0 != _CheckDriverApi())
    {
        ALOGE("PortalDevice: get portal driver apis failed");
        return -1;
    }

    if(mPortalStatus == Portal_Enabled || mPortalStatus == Portal_Created)
    {
        ALOGD("PortalDevice: current Portal is %d", (int)mPortalStatus);
        return 0;
    }

    mPortalMode = mode;
    mPortalStatus = Portal_Enabled;
    return 0;
}

int  PortalDevice::CreatePortal(ANativeWindow **win)
{
    std::lock_guard<std::mutex> guard(mMutex);
    if(0 != _CreatePortal())
    {
        ALOGE("PortalDevice: portal create failed");
        return -1;
    }
    ALOGD("PortalDevice: create portal ok, return window = %p", mWindow);
    *win = mWindow;
    mReprojMode = mPortalMode & 0xff;
    mPortalStatus = Portal_Created;
    return 0;
}

int PortalDevice::_CreatePortal()
{
    if(Portal_Enabled != mPortalStatus)
    {
        ALOGE("create portal before enable it");
        return -1;
    }
    return Hooks->PortalInit(&mWindow);
}

int PortalDevice::DestroyPortal()
{
    std::lock_guard<std::mutex> guard(mMutex);
    if(0 != _DestroyPortal())
    {
        ALOGE("PortalDevice: portal destroyed failed");
        return -1;
    }
    mPortalStatus = Portal_Enabled;
    ALOGD("PortalDevice: portal destroyed");
    return 0;
}

int PortalDevice::_DestroyPortal()
{
    if(Portal_Created != mPortalStatus)
    {
        ALOGE("PortalDevice: can not destroy portal, current status=%d", (int)mPortalStatus);
        return -1;
    }
    return Hooks->PortalDestroy();
}

bool PortalDevice::IsPortalEnabled()
{
    std::lock_guard<std::mutex> guard(mMutex);

    if(mPortalStatus != Portal_Enabled)
    {
        ALOGD("PortalDevice: check portal found status=%d", (int)mPortalStatus);
        return false;
    }

    if(_CheckTidName() != 0)
    {
        ALOGD("PortalDevice: tid=%d should not use hw surface", gettid());
        return false;
    }

    return true;
}

int PortalDevice::CreateTracker(PTRFUN_HeadTracker cb)
{
    std::lock_guard<std::mutex> guard(mMutex);
    if(0 != _CheckDriverApi())
    {
        ALOGE("PortalDevice: get portal driver apis failed");
        return -1;
    }

    if(0 != _CreateTracker(cb))
    {
        ALOGE("PortalDevice: create tracker failed");
        return -1;
    }
    mTrackerEnabled = true;
    mTrackerCallback = cb;
    return 0;
}

int PortalDevice::_CreateTracker(PTRFUN_HeadTracker cb)
{
    if(mTrackerEnabled == true)
    {
        ALOGE("PortalDevice: tracker has been created");
        return -1;
    }
    ALOGD("PortalDevice: create tracker");
    return Hooks->PortalSetTracker(cb, 1);
}

int PortalDevice::DestroyTracker()
{
    std::lock_guard<std::mutex> guard(mMutex);
    if(0 != _CheckDriverApi())
    {
        ALOGE("PortalDevice: get portal driver apis failed");
        return -1;
    }

    if(0 != _DestroyTracker())
    {
        ALOGE("PortalDevice: destroy tracker failed");
        return -1;
    }
    mTrackerEnabled = false;
    mTrackerCallback = NULL;
    return 0;
}

int PortalDevice::_DestroyTracker()
{
    if(mTrackerEnabled == false)
    {
        ALOGE("PortalDevice: tracker has been destroyed");
        return -1;
    }
    ALOGD("PortalDevice: destroy tracker");
    return Hooks->PortalSetTracker(mTrackerCallback, 0);
}

int PortalDevice::PrepareFrame(avrQuatf p)
{
    std::lock_guard<std::mutex> guard(mMutex);
    if(Portal_Created != mPortalStatus)
    {
        ALOGE("PortalDevice: call Prepare without create portal device");
        return -1;
    }

    if(0 != _CheckDriverApi())
    {
        ALOGE("PortalDevice: get portal driver apis failed");
        return -1;
    }
    return Hooks->PortalSwap(p);
}

// -------------------------------------------------------------------------- custom egl apis

EGLBoolean eglCustomEnablePortal(int flags)
{
    ALOGD("call %s. time=%f", __func__, NanoTime()*1e-9);

    PortalDevice* dev = GetPortalDevice();
    if( NULL == dev || 0 != dev->SetPortalEnabled(flags) )
    {
        return EGL_FALSE;
    }
    return EGL_TRUE;
}

EGLBoolean eglCustomPrepareFrame(avrQuatf pose)
{
    PortalDevice* dev = GetPortalDevice();
    if(!dev)
    {
        ALOGE("%s: found NULl dev", __func__);
        return EGL_FALSE;
    }

    dev->PrepareFrame(pose);
    return EGL_TRUE;
}

EGLBoolean eglCustomStartHeadTracker(PTRFUN_HeadTracker cb)
{
    ALOGD("call %s. time=%f", __func__, NanoTime()*1e-9);

    PortalDevice* dev = GetPortalDevice();
    if(!dev)
    {
        ALOGE("%s: found NULl dev", __func__);
        return EGL_FALSE;
    }

    if(0 != dev->CreateTracker(cb))
    {
        ALOGE("%s: create tracker failed", __func__);
        return EGL_FALSE;
    }
    return EGL_TRUE;
}

EGLBoolean eglCustomStopHeadTracker()
{
    ALOGD("call %s. time=%f", __func__, NanoTime()*1e-9);

    PortalDevice* dev = GetPortalDevice();
    if(!dev)
    {
        ALOGE("%s: found NULl dev", __func__);
        return EGL_FALSE;
    }

    if(0 != dev->DestroyTracker())
    {
        ALOGE("%s: destroy tracker failed", __func__);
        return EGL_FALSE;
    }
    return EGL_TRUE;
}
}; // namespace android
