#pragma once
#include <stdint.h>
#include <android/native_window_jni.h>
#include "AtwTypesPublic.h"

namespace allwinner
{

class PortalClient;
class PortalFactory
{
public:
    static PortalClient* GetInstance();
    static void DestroyInstance(PortalClient *instance);
};

class PortalClient
{
public:
    PortalClient(){} //must call PortalFactory::GetInstance to retrive a instance.
    virtual ~PortalClient(){}

    virtual int init(ANativeWindow** nativeSur) = 0;
    virtual int prepareFrame(uint64_t frameNumber, eye_t eye) = 0;
    virtual int swapToScreen(uint64_t frameNumber, const avrQuatf &orientation, uint64_t expectedDisplayTimeInNano = 0) = 0;
    virtual int doReprojection(int mode) = 0;
    virtual int setTracker(PTRFUN_HeadTracker cb, int run) = 0;
};

};// namespace allwinner

#ifdef __cplusplus
extern "C" {
#endif
    extern int PortalInit(ANativeWindow** nativeSur);
    extern int PortalDestroy();
    extern int PortalSwap(const avrQuatf &orientation);
    extern int PortalReproj(int mode);
    extern int PortalSetTracker(PTRFUN_HeadTracker cb, int run);
#ifdef __cplusplus
}
#endif
