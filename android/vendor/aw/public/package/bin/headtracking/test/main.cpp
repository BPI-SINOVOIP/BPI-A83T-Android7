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

// System headers required for setgroups, etc.
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/Parcel.h>
#include <utils/Log.h>

#include "IHeadTrackingService.h"
#include "log.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

int main(int argc, char** argv)
{
    sp<IServiceManager> sm = defaultServiceManager();
    ALOGD("test htserver");
    sp<IBinder> binder = sm->getService(String16("softwinner.HeadTrackingService"));
    sp<IHeadTrackingService> headTrackingService = interface_cast<IHeadTrackingService>(binder);
    std::vector<float> orien;

    double now;
    double lasttime;
    now = lasttime = double(GetTicksNanos()) * 0.000000001;

    while(1) {
        now  = double(GetTicksNanos()) * 0.000000001;
        if ((now - lasttime) > 1) {
            lasttime = now;
            orien = headTrackingService->getPredictionForTime(now);
            std::vector<float>::iterator it;
            ALOGD("get orientation");
            for (it = orien.begin(); it != orien.end(); it++) {
                ALOGD("\t\tmatrix[] = %f", *it);
            }
        }
    }
}
