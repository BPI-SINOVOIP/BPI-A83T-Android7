#ifndef HEADTRACKING_H
#define HEADTRACKING_H

#include <sys/types.h>
#include <unistd.h>
#include <grp.h>

//#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/Parcel.h>
#include <utils/Log.h>

#include "IHeadTrackingService.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "Math.h"

namespace android {

class Headtracking {
public:
    ~Headtracking();
    static Headtracking* getInstance();
    void getInterPoint(float &x, float &y);
    void recenterYaw();
    void recenterOrientation();
private:
    Headtracking();
    sp<IHeadTrackingService> getHeadTrackingService();
    static Headtracking* mHeadtracking;
    int mScreenWidth;
    int mScreenHeight;
};

}

#endif
