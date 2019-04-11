/*
 * Copyright (C) 2016 The Android Open Source Project
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
#define LOG_TAG "VrHal"

#include <hardware/vr.h>
#include <hardware/hardware.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <cutils/log.h>



static const char* ROOMAGE = "/sys/devices/soc/cpu_budget_cooling/roomage";
static char* ROOMAGE_PERF = "1200000 4 0 0 1200000 4 0 0";
static char* ROOMAGE_NORMAL = "816000 4 0 0 816000 4 0 0";


static void vr_fwrite(const char *path, char *s)
{
    char buf[64];
    int len;
    int fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);
        return;
    }
    len = write(fd, s, strlen(s));
    if (len < 0)
    {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error writing to %s: %s\n", path, buf);
    }
    close(fd);
}


static void vr_init(struct vr_module *module) {
    // NOOP
}

static void vr_set_vr_mode(struct vr_module *module, bool enabled) {
#if defined VR9
    if(enabled) {
        ALOGD("vr mode enabled");
        vr_fwrite(ROOMAGE,ROOMAGE_PERF);
    } else {
        ALOGD("vr mode disabled");
        vr_fwrite(ROOMAGE,ROOMAGE_NORMAL);
    }
#endif
}

static struct hw_module_methods_t vr_module_methods = {
    .open = NULL,
};


vr_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag                = HARDWARE_MODULE_TAG,
        .module_api_version = VR_MODULE_API_VERSION_1_0,
        .hal_api_version    = HARDWARE_HAL_API_VERSION,
        .id                 = VR_HARDWARE_MODULE_ID,
        .name               = "Demo VR HAL",
        .author             = "The Android Open Source Project",
        .methods            = &vr_module_methods,
    },

    .init = vr_init,
    .set_vr_mode = vr_set_vr_mode,
};
