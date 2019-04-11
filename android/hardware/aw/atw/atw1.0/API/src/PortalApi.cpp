#include "PortalApi.h"
#include <EGL/egl.h>
#include <android/log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "PortalApi"

#ifndef ALOGD
#define ALOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#endif

#ifndef ALOGE
#define ALOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#endif

typedef EGLBoolean (*PFUNC_eglCustomEnablePortal)(int mode);
typedef EGLBoolean (*PFUNC_eglCustomPrepareFrame)(avrQuatf p);
typedef EGLBoolean (*PFUNC_eglCustomStartHeadTracker)(PTRFUN_HeadTracker cb);
typedef EGLBoolean (*PFUNC_eglCustomStopHeadTracker)();

static PFUNC_eglCustomEnablePortal     _eglCustomEnablePortal = NULL;
static PFUNC_eglCustomPrepareFrame     _eglCustomPrepareFrame = NULL;
static PFUNC_eglCustomStartHeadTracker _eglCustomStartHeadTracker = NULL;
static PFUNC_eglCustomStopHeadTracker  _eglCustomStopHeadTracker = NULL;
static bool CheckCustomEglDriver()
{
    static bool first = true;
    static bool sDriverStatus = false;
    if(first)
    {
        first = false;
        _eglCustomEnablePortal = (PFUNC_eglCustomEnablePortal) eglGetProcAddress("eglCustomEnablePortal");
        _eglCustomPrepareFrame = (PFUNC_eglCustomPrepareFrame) eglGetProcAddress("eglCustomPrepareFrame");
        _eglCustomStartHeadTracker = (PFUNC_eglCustomStartHeadTracker) eglGetProcAddress("eglCustomStartHeadTracker");
        _eglCustomStopHeadTracker = (PFUNC_eglCustomStopHeadTracker) eglGetProcAddress("eglCustomStopHeadTracker");
        sDriverStatus = _eglCustomEnablePortal && _eglCustomStartHeadTracker && _eglCustomStopHeadTracker && _eglCustomPrepareFrame;
    }
    return sDriverStatus;
}

int awInitPortalSdk(int mode)
{
    ALOGD("call %s",__func__);
    if(true != CheckCustomEglDriver())
    {
        ALOGE("call %s found driver: enable=%p prepare=%p startTracker=%p stopTracker=%p",
                __func__, _eglCustomEnablePortal, _eglCustomPrepareFrame, _eglCustomStartHeadTracker, _eglCustomStopHeadTracker);
        return -1;
    }

    if(EGL_FALSE == _eglCustomEnablePortal(mode))
    {
        ALOGE("ERROR: init portal sdk failed");
        return -1;
    }
    ALOGD("init portal sdk success");
    return 0;
}

int awPrepareFrame(avrQuatf pose)
{
    //ALOGD("call %s",__func__);
    if(true != CheckCustomEglDriver())
    {
        ALOGE("call %s found driver: enable=%p prepare=%p startTracker=%p stopTracker=%p",
                __func__, _eglCustomEnablePortal, _eglCustomPrepareFrame, _eglCustomStartHeadTracker, _eglCustomStopHeadTracker);
        return -1;
    }

    _eglCustomPrepareFrame(pose);
    return 0;
}

int awStartHeadTracker(PTRFUN_HeadTracker cb)
{
    ALOGD("call %s",__func__);
    if(true != CheckCustomEglDriver())
    {
        ALOGE("call %s found driver: enable=%p prepare=%p startTracker=%p stopTracker=%p",
                __func__, _eglCustomEnablePortal, _eglCustomPrepareFrame, _eglCustomStartHeadTracker, _eglCustomStopHeadTracker);
        return -1;
    }

    _eglCustomStartHeadTracker(cb);
    return -1;
}

int awStopHeadTracker()
{
    ALOGD("call %s",__func__);
    if(true != CheckCustomEglDriver())
    {
        ALOGE("call %s found driver: enable=%p prepare=%p startTracker=%p stopTracker=%p",
                __func__, _eglCustomEnablePortal, _eglCustomPrepareFrame, _eglCustomStartHeadTracker, _eglCustomStopHeadTracker);
        return -1;
    }

    _eglCustomStopHeadTracker();
    return 0;
}