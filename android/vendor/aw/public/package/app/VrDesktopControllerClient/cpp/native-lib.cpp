#include <jni.h>
#include "IHeadTrackingService.h"
#include <string.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/Parcel.h>
#include <private/gui/ComposerService.h>
#include <gui/ISurfaceComposer.h>

using namespace android;
sp<IHeadTrackingService> mHeadTrackingService;
sp<ISurfaceComposer> mSfComposer(ComposerService::getComposerService());

uint64_t getTicksNanos(){
    struct timespec tp;
    const int status = clock_gettime(CLOCK_MONOTONIC, &tp);
    const uint64_t result = (uint64_t)tp.tv_sec * (uint64_t)(1000 * 1000 * 1000) + uint64_t(tp.tv_nsec);
    return result;
}

extern "C"
jfloatArray Java_com_allwinnertech_vr_vrdesktop_MyService_getHeadTrackingData(JNIEnv* env, jobject thiz){
    if(mHeadTrackingService.get() == NULL){
        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder = sm->getService(String16("softwinner.HeadTrackingService"));
        mHeadTrackingService = interface_cast<IHeadTrackingService>(binder);
    }
    if(mHeadTrackingService.get() != NULL){
        double now = double(getTicksNanos()) * 0.000000001;
        std::vector<float> orient = mHeadTrackingService->getPredictionForTime(now);
        float* data = orient.data();
        if(data == NULL){
            ALOGE("getHeadTrackingData ret null");
            return NULL;
        }
        jfloatArray array = (*env).NewFloatArray(16);
        (*env).SetFloatArrayRegion(array, 0, 16, data);
        return array;
    }
    ALOGE("wait for headtracking service to be ready...");
    return NULL;
}

extern "C"
void Java_com_allwinnertech_vr_vrdesktop_MyService_recenterHeadOrientation(JNIEnv * env,jobject thiz){
    if(mHeadTrackingService.get() == NULL){
        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder = sm->getService(String16("softwinner.HeadTrackingService"));
        mHeadTrackingService = interface_cast<IHeadTrackingService>(binder);
    }
    if(mHeadTrackingService.get() != NULL){
        mHeadTrackingService->recenterOrientation();
    }
}

extern "C"
void Java_com_allwinnertech_vr_vrdesktop_MyService_sendDataToSf(JNIEnv * env,jobject thiz,
        jint status, jint needRecenter, jfloat x, jfloat y, jfloat z, jfloat w, jfloat l){
    if(mSfComposer.get() == NULL){
        mSfComposer = ComposerService::getComposerService();
    }
    if(mSfComposer.get() != NULL){
        ALOGV("LENGTH IN JNI = %f", l);
        mSfComposer->sendControllerData(status, needRecenter, x, y, z, w, l);
    }
}