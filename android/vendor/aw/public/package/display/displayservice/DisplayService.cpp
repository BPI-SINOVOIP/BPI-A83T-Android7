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
#define LOG_TAG "DisplayService"
#include "DisplayService.h"
#include <binder/IServiceManager.h>
#include <utils/misc.h>
#include <utils/Log.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#ifdef HAVE_ANDROID_OS      // just want PAGE_SIZE define
#include <asm/page.h>
#else
#include <sys/user.h>
#endif


#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdlib.h>


#define DEBUG false


namespace android{

void DisplayService::instantiate(){
    defaultServiceManager()->addService(
        String16("aw_display"), new DisplayService());
}

DisplayService::DisplayService(){
    mopenFlag = 0;
    displayOpen();
    ALOGD("displayService created");
}

DisplayService::~DisplayService(){
    ALOGD("displayService destroyed");
}

int DisplayService::displayOpen()
{
    int err;
    hw_module_t const *module = nullptr;
    hw_device_t* device = nullptr;
    if (mopenFlag) {
        ALOGV("display open already!");
        return -1;
    }

    if (hw_get_module(DISPLAY_HARDWARE_MODULE_ID, &module) != 0) {
        ALOGE("%s module not found, aborting", DISPLAY_HARDWARE_MODULE_ID);
        return -1;
    }
    err = module->methods->open(module, DISPLAY_HARDWARE_MODULE_ID, &device);

    if (err != 0) {
        ALOGE("Failed to open Display device, aborting");
        return -1;
    }
    mopenFlag = 1;
    mdevice = (display_device_t *)device;
    return mdevice->display_open(mdevice);
}

int DisplayService::dispClose()
{
    int ret = 0;
    ret = mdevice->common.close(&mdevice->common);
    mdevice = nullptr;
    return ret;
}

int DisplayService::displayCtrl(int disp, int cmd, int val0, int val1)
{
    int ret = 0;
    ALOGV("DisplayService::displayCtrl disp=%d, CMD =%d, val0 = %d, val1 = %d",
        disp, cmd, val0, val1);
    return mdevice->display_ctrl(mdevice, disp, cmd, val0, val1);
}
}
