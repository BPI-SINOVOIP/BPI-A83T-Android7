#include <jni.h>

#include <log/log.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/Parcel.h>
#include <private/gui/ComposerService.h>
#include <gui/ISurfaceComposer.h>

using namespace android;

sp<ISurfaceComposer> mSfComposer(ComposerService::getComposerService());

uint64_t getTicksNanos(){
    struct timespec tp;
    const int status = clock_gettime(CLOCK_MONOTONIC, &tp);
    const uint64_t result = (uint64_t)tp.tv_sec * (uint64_t)(1000 * 1000 * 1000) + uint64_t(tp.tv_nsec);
    return result;
}

extern "C"
void Java_com_softwinner_VrModeSelector_utils_JniUtils_sendDataToSf(JNIEnv * env, jobject thiz,
        jint status, jint needRecenter, jfloat x, jfloat y, jfloat z, jfloat w) {
    ALOGD("jni sendDataToSf ... ");
    if (NULL == mSfComposer.get()) {
        mSfComposer = ComposerService::getComposerService();
    }
    if (NULL != mSfComposer.get()) {
        mSfComposer->sendControllerData(status, needRecenter, x, y, z, w);
    }
}
