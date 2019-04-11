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
#include <utils/Log.h>

#include "HeadTrackingService.h"
#include <private/android_filesystem_config.h>
#include "log.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define LOG_TAG "htserver"

using namespace android;

int main(int argc, char** argv)
{
    INFO("main_headtrackingserver is running%s", "");
    printf("main_headtrackingserver is running%s", "");
    ALOGD("main_headtrackingserver is running.");
    sp<ProcessState> proc(ProcessState::self()); //  create  process's  processstate example
    sp<IServiceManager> sm = defaultServiceManager();  //get SM's BpServiceManager
    ALOGI("headtracking, ServiceManager: %p", sm.get());
    HeadTrackingService::instantiate();   //add support  service
    ProcessState::self()->startThreadPool();  // start pool thread
    IPCThreadState::self()->joinThreadPool();

}
