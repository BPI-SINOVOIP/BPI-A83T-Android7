#ifndef _PORTAL_API_H_
#define _PORTAL_API_H_

#include "AtwTypesPublic.h"
#include "AW_CAPI.h"

// mode: 0 , disable reprojection, but still sync with vsync.
// mode: 1 , dynamic enable/disable reprojection. (60->30, 30->60)
// mode: 2 , enable reprojection, eg, keep renderfps=30
int awInitPortalSdk(int mode);
int awStartHeadTracker(PTRFUN_HeadTracker callBackHeadTracker);
int awStopHeadTracker();
int awPrepareFrame(avrQuatf pose);

#endif // _PORTAL_API_H_
