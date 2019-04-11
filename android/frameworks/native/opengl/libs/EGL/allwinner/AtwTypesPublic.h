#pragma once

#include <stdint.h>
#include "AW_CAPI.h"

typedef enum
{
   left_eye = 0,
   right_eye
}eye_t;

typedef int (*PTRFUN_HeadTracker)(uint64_t timestamp, avrQuatf &pose);

typedef int (*PFUNC_PortalInit)(ANativeWindow** nativeSur);
typedef int (*PFUNC_PortalDestroy)();
typedef int (*PFUNC_PortalSwap)(const avrQuatf &pose);
typedef int (*PFUNC_PortalReproj)(int mode);
typedef int (*PFUNC_PortalSetTracker)(PTRFUN_HeadTracker cb, int run);