#ifndef _ANDROID_SENSOR_DEVICE_H_
#define _ANDROID_SENSOR_DEVICE_H_
#include <android/sensor.h>// ndk only expose this header file for app developers.

#include "OVR_SensorFusion.h"

#include <sys/time.h>


#define ANDROID_DEBUG 0

#if ANDROID_DEBUG
#include <android/log.h>
#define LOG_TAG "AwSensorHal"
#define DBG(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#else
#define DBG(...)
#endif //ANDROID_DEBUG

#include <stdlib.h>
#define AWASSERT(a) \
    {\
        if( (a) != (true) )\
            abort();\
    }\

typedef struct
{
    bool                bStop;
    OVR::SensorFusion*    pFusion;
}SensorThreadParms_t;

class AwSensorThreadImpl
{
public:
    AwSensorThreadImpl();
    ~AwSensorThreadImpl();
    OVR::SensorFusion *StartThread();
    void StopThread();
protected:
    static void* ThreadFunc(void *arg);
private:
    SensorThreadParms_t *mCurrent;
};

#endif //_ANDROID_SENSOR_DEVICE_H_
