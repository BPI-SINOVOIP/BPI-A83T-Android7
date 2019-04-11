#ifndef _EGL_AW_H_
#define _EGL_AW_H_

#include <EGL/egl.h>
#include <mutex>
#include "AtwTypesPublic.h"

namespace android
{

extern EGLBoolean eglCustomEnablePortal(int mode);
extern EGLBoolean eglCustomPrepareFrame(avrQuatf pose);
extern EGLBoolean eglCustomStartHeadTracker(PTRFUN_HeadTracker cb);
extern EGLBoolean eglCustomStopHeadTracker();

struct portal_driver_hooks_t
{
    void                  *libatwclient;
    PFUNC_PortalInit       PortalInit;
    PFUNC_PortalDestroy    PortalDestroy;
    PFUNC_PortalSwap       PortalSwap;
    PFUNC_PortalReproj     PortalReproj;
    PFUNC_PortalSetTracker PortalSetTracker;

    inline portal_driver_hooks_t()
    {
        libatwclient = NULL;
        PortalInit = NULL;
        PortalDestroy = NULL;
        PortalSwap = NULL;
        PortalReproj = NULL;
        PortalSetTracker = NULL;
    }
    inline ~portal_driver_hooks_t()
    {
        if(libatwclient)
        {
            dlclose(libatwclient);
        }
        libatwclient = NULL;
        PortalInit = NULL;
        PortalDestroy = NULL;
        PortalSwap = NULL;
        PortalReproj = NULL;
        PortalSetTracker = NULL;
    }
};


typedef enum
{
    Portal_Unknown = 0,
    Portal_Enabled,
    Portal_Created,
    Portal_Destroyed
}portal_status_t;

class PortalDevice
{
public:
    PortalDevice();
    ~PortalDevice();

    // get window,
    EGLSurface  GetPortalSurface();
    int         SavePortalSurface(EGLSurface surf);
    int         Reprojection();

    int  SetPortalEnabled(int mode);
    bool IsPortalEnabled();
    int  CreatePortal(ANativeWindow **win);
    int  DestroyPortal();

    int PrepareFrame(avrQuatf p);

    int CreateTracker(PTRFUN_HeadTracker cb);
    int DestroyTracker();

protected:
    int _CheckDriverApi();
    int _CheckTidName();

    int _CreatePortal();
    int _DestroyPortal();

    int _CreateTracker(PTRFUN_HeadTracker cb);
    int _DestroyTracker();

private:
    std::mutex     mMutex;
    bool mFirstCall    = true;
    bool mDriverStatus = false;
    portal_driver_hooks_t  *Hooks = NULL;

    ANativeWindow *mWindow = NULL;
    EGLSurface     mSurface = EGL_NO_SURFACE;

    portal_status_t mPortalStatus = Portal_Unknown;
    int mPortalMode = 0;

    // portal status.
    int  mReprojMode   = 0;

    bool mTrackerEnabled = false;
    PTRFUN_HeadTracker mTrackerCallback = NULL;
};

extern PortalDevice *GetPortalDevice();

}; // namespace android

#endif
