#include "headtracking.h"
#include <cutils/properties.h>

using namespace android;

uint64_t GetTicksNanos()
{
    struct timespec tp;
    const int       status = clock_gettime(CLOCK_MONOTONIC, &tp);
    if(status != 0)
    {
        //DBG("clock_gettime return %d \n", status);
    }

    const uint64_t result = (uint64_t)tp.tv_sec * (uint64_t)(1000 * 1000 * 1000) + uint64_t(tp.tv_nsec);
    return result;
}

Headtracking* Headtracking::mHeadtracking = nullptr;

Headtracking::Headtracking() {
    char vWidth[8];
    property_get("ro.boot.lcd_x", vWidth, "1080");
    int lcdWidth = atoi(vWidth);
    char vHeight[8];
    property_get("ro.boot.lcd_y", vHeight, "1920");
    int lcdHeight = atoi(vHeight);
    //make screen to be lanscape;
    mScreenWidth = lcdWidth > lcdHeight ? lcdWidth : lcdHeight;
    mScreenHeight = lcdWidth > lcdHeight ? lcdHeight : lcdWidth;
}
Headtracking::~Headtracking() {
    if (NULL != mHeadtracking) {
        delete(mHeadtracking);
    }
}

Headtracking* Headtracking::getInstance() {
    if (NULL == mHeadtracking) {
        mHeadtracking = new Headtracking();
    }
    return mHeadtracking;
}

void Headtracking::recenterYaw() {
    static sp<IHeadTrackingService> headTrackingService;
    bool inited = false;
    if (!inited) {
        headTrackingService = getHeadTrackingService();
        inited = true;
    }

    headTrackingService->recenterYaw();
}

void Headtracking::recenterOrientation() {
    static sp<IHeadTrackingService> headTrackingService;
    bool inited = false;
    if (!inited) {
        headTrackingService = getHeadTrackingService();
        inited = true;
    }

    headTrackingService->recenterOrientation();
}

#define UI_DISTANCE 3.0f

//TODO: async get inter point 60hz
void Headtracking::getInterPoint(float &pointerx, float &pointery) {
    // get orientation

    bool inited = false;
    static sp<IHeadTrackingService> headTrackingService;
    if (!inited) {
        headTrackingService = getHeadTrackingService();
        inited = true;
    }
    double now = double(GetTicksNanos()) * 0.000000001;
    std::vector<float> orien = headTrackingService->getPredictionForTime(now);

    // compute the inter point
    std::vector<float>::iterator it;
    OVR::Matrix4f orienInMatrix;
    int i = 0;
    for (it = orien.begin(); it != orien.end(); it++) {
        orienInMatrix.M[i/4][i%4] = *it;
        i++;
    }

    OVR::Vector3f gazeForward = OVR::Vector3f(0, 0, -1);
    // assume gaze position (0, 0, 0)
    OVR::Vector3f gazeForwardInWorld = orienInMatrix.Transform(gazeForward);

    float angle = gazeForwardInWorld.Angle(gazeForward);
    float projectDistance = tan(angle) * UI_DISTANCE;

    //get x & y
    float x = gazeForwardInWorld.x;
    float y = gazeForwardInWorld.y;
    float absx = fabs(x);
    float absy = fabs(y);
    float distany = sin(atan(absy/absx)) * projectDistance;
    float distanx = cos(atan(absy/absx)) * projectDistance;
    if (x < 0)
        distanx *= -1;
    if (y > 0)
        distany *= -1;

    // compute the point
    double displayx = mScreenWidth / 2.0f + distanx * 1000;
    double displayy = mScreenHeight / 2.0f + distany * 1000;

    //ALOGE("VrDesk %f, %f, screen %d %d", float(displayx), float(displayy), mScreenWidth, mScreenHeight);
    pointerx = float(displayx);
    pointery = float(displayy);
}

sp<IHeadTrackingService> Headtracking::getHeadTrackingService() {
    sp<IServiceManager> sm = defaultServiceManager();
    ALOGD("test htserver");
    sp<IBinder> binder = sm->getService(String16("softwinner.HeadTrackingService"));
    sp<IHeadTrackingService> headTrackingService = interface_cast<IHeadTrackingService>(binder);
    return headTrackingService;
}

