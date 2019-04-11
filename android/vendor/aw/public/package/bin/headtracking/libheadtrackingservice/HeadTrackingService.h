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

#ifndef ANDROID_HEADTRACKINGSERVICE_H
#define ANDROID_HEADTRACKINGSERVICE_H

#include <utils/Log.h>
#include <utils/Errors.h>

#include "../sensorfusion/AwVRSensorHal.h"
#include "../sensorfusion/OVR_SensorFusion.h"

#include "IHeadTrackingService.h"

namespace android {


class HeadTrackingService : public BnHeadTrackingService
{
public:
    static  void     instantiate();
    void init();

    virtual std::vector<float> getPredictionForTime(const double absoluteTimeSeconds);
    virtual void recenterYaw();
    virtual void recenterOrientation();
    virtual void setStatus(bool enable);

                                 HeadTrackingService();
    virtual                        ~HeadTrackingService();

private:
    AwSensorThreadImpl *mSensorHal;
    OVR::SensorFusion *mSensorFusion;
};

// ----------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_SECUREFILESERVICE_H

