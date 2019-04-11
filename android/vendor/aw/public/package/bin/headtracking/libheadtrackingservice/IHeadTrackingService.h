/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_IHEADTRACKINGSERVICE_H
#define ANDROID_IHEADTRACKINGSERVICE_H

#include <utils/Errors.h>  // for status_t
#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>



namespace android {


class IHeadTrackingService: public IInterface
{
public:
    DECLARE_META_INTERFACE(HeadTrackingService);

    virtual std::vector<float> getPredictionForTime(const double absoluteTimeSeconds) = 0;
    virtual void recenterYaw() = 0;
    virtual void recenterOrientation() = 0;
    virtual void setStatus(bool enable) = 0;
};

// ----------------------------------------------------------------------------

class BnHeadTrackingService: public BnInterface<IHeadTrackingService>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace android

#endif // ANDROID_IVRSERVICE_H

