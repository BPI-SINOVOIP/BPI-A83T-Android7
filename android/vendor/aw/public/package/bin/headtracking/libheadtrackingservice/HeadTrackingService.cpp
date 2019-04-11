/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

// Proxy for secure file implementations

#undef NDEBUG
#define LOG_TAG "HeadTrackingService"
#include <utils/Log.h>

//#include <sys/types.h>
//#include <sys/stat.h>
//#include <dirent.h>
//#include <unistd.h>

#include <string.h>

#include <binder/IServiceManager.h>
//#include <cutils/atomic.h>
//#include <cutils/properties.h> // for property_get
//
//#include <utils/misc.h>
//
//#include <android_runtime/ActivityManager.h>
//
//#include <binder/IPCThreadState.h>

#include <utils/Errors.h>  // for status_t
//#include <utils/String8.h>

#include "HeadTrackingService.h"

#include "LocalArray.h"
#include "ScopedFd.h"
#include "ScopedLocalRef.h"
#include "ScopedPrimitiveArray.h"
#include "ScopedUtfChars.h"

#include <stdio.h>
#include <malloc.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>

#include <androidfw/AssetManager.h>
#include <binder/IPCThreadState.h>
#include <utils/Atomic.h>
#include <utils/Errors.h>
#include <utils/Log.h>

#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>

#include "../sensorfusion/OVR_SensorFusion.h"

#define    DEBUG true
namespace android {

void HeadTrackingService::init() {
    if (NULL == mSensorHal) {
        mSensorHal = new AwSensorThreadImpl;
        mSensorFusion = mSensorHal->StartThread();
    }
}

void HeadTrackingService::instantiate() {
    defaultServiceManager()->addService(
            String16("softwinner.HeadTrackingService"), new HeadTrackingService());
}

HeadTrackingService::HeadTrackingService(): mSensorHal(NULL), mSensorFusion(NULL)
{
    ALOGD("HeadTrackingService created");
    init();
}

HeadTrackingService::~HeadTrackingService()
{
    ALOGD("HeadTrackingService destroyed");
}


std::vector<float> HeadTrackingService::getPredictionForTime(const double absoluteTimeSeconds) {
    OVR::SensorState sensorState = mSensorFusion->GetPredictionForTime(absoluteTimeSeconds);
    OVR::Matrix4f sensorStateInMatrix = OVR::Matrix4f(sensorState.Predicted.Transform.Orientation);
    std::vector<float> result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.push_back(sensorStateInMatrix.M[i][j]);
        }
    }
    return result;
}

void HeadTrackingService::recenterYaw() {
    mSensorFusion->RecenterYaw();
}

void HeadTrackingService::recenterOrientation() {
    mSensorFusion->RecenterOrientation();
}

void HeadTrackingService::setStatus(bool enable) {
    //mSensorHal.
}

};
