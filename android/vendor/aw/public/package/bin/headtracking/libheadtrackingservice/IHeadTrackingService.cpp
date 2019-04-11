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
#undef NDEBUG
#define LOG_TAG "IHeadTrackingService"
#include "utils/Log.h"

#include <memory.h>
#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include "IHeadTrackingService.h"


#include <utils/Errors.h>  // for status_t
#define DEBUG false

namespace android {

enum {
    GET_PREDICTION_FOR_TIME,
    RECENTER_YAW,
    RECENTER_ORIENTATION,
    SET_STATUS,
};

class BpHeadTrackingService: public BpInterface<IHeadTrackingService>
{
public:
    BpHeadTrackingService(const sp<IBinder>& impl)
        : BpInterface<IHeadTrackingService>(impl)
    {
    }


    std::vector<float> getPredictionForTime(const double absoluteTimeSeconds)
    {
       if(DEBUG)
            ALOGV("IHeadTrackingService::getPredictionForTime()  absoluteTimeSeconds = %f",absoluteTimeSeconds);
        Parcel data, reply;
        data.writeInterfaceToken(IHeadTrackingService::getInterfaceDescriptor());
        data.writeDouble(absoluteTimeSeconds);
        remote()->transact(GET_PREDICTION_FOR_TIME, data, &reply);
        std::vector<float> result;
        reply.readFloatVector(&result);
        return result;
    }

    void recenterYaw()
    {
       if(DEBUG)
            ALOGV("IHeadTrackingService::recenterYaw()");
        Parcel data, reply;
        data.writeInterfaceToken(IHeadTrackingService::getInterfaceDescriptor());
        remote()->transact(RECENTER_YAW, data, &reply);
        return;
    }

    void recenterOrientation()
    {
       if(DEBUG)
            ALOGV("IHeadTrackingService::recenterOrientation()");
        Parcel data, reply;
        data.writeInterfaceToken(IHeadTrackingService::getInterfaceDescriptor());
        remote()->transact(RECENTER_ORIENTATION, data, &reply);
        return;
    }

    void setStatus(bool enable)
    {
       if(DEBUG)
            ALOGV("IHeadTrackingService::setStatus()  enable = %d",enable);
        Parcel data, reply;
        data.writeInterfaceToken(IHeadTrackingService::getInterfaceDescriptor());
        remote()->transact(SET_STATUS, data, &reply);
        return;
    }
};

IMPLEMENT_META_INTERFACE(HeadTrackingService, "com.android.softwinner.IHeadTrackingService");

// ----------------------------------------------------------------------

status_t BnHeadTrackingService::onTransact(  //-->server:get parcel from  client
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case GET_PREDICTION_FOR_TIME: {
            CHECK_INTERFACE(IHeadTrackingService, data, reply);
            const double absoluteTimeSeconds = data.readDouble();
            std::vector<float> result = getPredictionForTime(absoluteTimeSeconds);
            reply->writeFloatVector(result);
            return NO_ERROR;
        } break;
        case RECENTER_YAW: {
            CHECK_INTERFACE(IHeadTrackingService, data, reply);
            recenterYaw();
            return NO_ERROR;
        } break;
        case RECENTER_ORIENTATION: {
            CHECK_INTERFACE(IHeadTrackingService, data, reply);
            recenterOrientation();
            return NO_ERROR;
        } break;
        case SET_STATUS: {
            CHECK_INTERFACE(IHeadTrackingService, data, reply);
            const int enable = data.readInt32();
            setStatus(enable);
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }

}

};
