/*
 * Copyright (C) 2012 The Android Open Source Project
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

#define LOG_TAG "display"
#define LOGE ALOGE

#include <cutils/log.h>

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <hardware/display.h>
#include <hardware/hardware.h>
#include <hardware/sunxi_display2.h>
#include <cutils/list.h>
#include <stdlib.h>
#include <sys/cdefs.h>
#include <cutils/properties.h>
#include "display_hw.h"

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static int fd;
static int fd3d;
static int fdEnhance;

/*TrdMode*/
static int display_device_set_3d_mode(int disp, int mode)
{
    int ret = 0;
    int i = 0;
    char trdmode[PROPERTY_VALUE_MAX];

    snprintf(trdmode, sizeof(trdmode), "%d", mode);
    lseek(fd3d, 0, SEEK_SET);
    ret = write(fd3d, trdmode, 1);
    if (ret < 0) {
        ALOGE("###write 3d mode failed! fd3d=%d", fd3d);
    }

    if (property_set("sys.display.trdmode", trdmode) == 0) {
        ALOGD("sys.display.trdmode=%s", trdmode);
    } else {
        ALOGD("No trdmode property node.");
        return -1;
    }

    return 0;
}

static int display_device_get_3d_mode(int dispId)
{
    int state = 0;
    char trdmode_property[PROPERTY_VALUE_MAX];

    if (!(dispId < SUPPORT_DE_NUM)) {
        /* Too many devices */
        return -1;
    }

    if (property_get("sys.display.trdmode", trdmode_property, NULL) >= 0) {
        state = atoi(trdmode_property);
        ALOGD("trdmode property state=%d.", state);
        return state;
    } else {
        ALOGE("No trdmode property node.");
        return 0;
    }
}

static int display_device_get_smt_backlight(int dispId)
{
    int mode = 0;
    char property[PROPERTY_VALUE_MAX];

    if (property_get("persist.display.smbl", property, NULL) >= 0) {
        mode = atoi(property);
        ALOGE("smbl property mode=%d.", mode);
        return mode;
    } else {
        ALOGE("No smbl property node.");
        return 0;
    }
}

static int display_device_get_enhance_mode(int dispId)
{
    char property[PROPERTY_VALUE_MAX];

    if (property_get("persist.display.enhance", property, NULL) >= 0) {
        ALOGD("enhance property mode=%s.", property);
    if (strncmp(property, DISPLAY_ENHANCE_OFF, 1) == 0)
        return DISPLAY_FUNC_OFF_MASK;
    if (strncmp(property, DISPLAY_ENHANCE_ON, 1) == 0)
        return DISPLAY_FUNC_ON_MASK;
    if (strncmp(property, DISPLAY_ENHANCE_DEMO_ON, 1) == 0)
        return DISPLAY_FUNC_DEMO_ON_MASK;
        return 0;
    } else {
        ALOGE("No enhance property node.");
        return 0;
    }
}

static int display_device_set_smt_backlight(int disp, int mode)
{
    unsigned long arg[4] = {0};
    struct disp_rect win;
    char smbl_mode = 0;

    if(disp == 0 /*default primary device*/
        /* && DisplayType != DISP_OUTPUT_TYPE_HDMI*/) {
        arg[0] = 0; //fix
        win.x = 0;
        win.y = 0;
        win.width = ioctl(fd, DISP_GET_SCN_WIDTH, arg);
        win.height = ioctl(fd, DISP_GET_SCN_HEIGHT, arg);
        arg[1] = (unsigned long)&win;

        if (mode) {
            if (mode == DISPLAY_FUNC_DEMO_ON_MASK) {
                /* demo mode.*/
                if (win.width > win.height) {
                    win.width /= 2;
                } else {
                    win.height /= 2;
                }
                ioctl(fd, DISP_SMBL_SET_WINDOW, arg);
                ioctl(fd, DISP_SMBL_ENABLE, arg);
            } else {
                /* enable smbl.*/
                ioctl(fd, DISP_SMBL_SET_WINDOW, arg);
                ioctl(fd, DISP_SMBL_ENABLE, arg);
            }
        } else {
            ioctl(fd, DISP_SMBL_DISABLE, arg);
        }
        ALOGE("WIN w=%d, h=%d", win.width, win.height);

        smbl_mode = mode + '0';
        if (property_set("persist.display.smbl", &smbl_mode) == 0) {
            ALOGD("persist.display.smbl=%d.", mode);
        } else {
            ALOGD("No smbl property node.");
            return -1;
            }
        }
    return 0;
}

static int display_device_set_enhance_mode(int disp, int mode)
{
    int ret = 0;
    char* enhance_mode;

    if (disp != 0) {
        /* is not primary display */
        return -1;
    }

    switch(mode){
    case DISPLAY_FUNC_OFF_MASK:
        enhance_mode = DISPLAY_ENHANCE_OFF;
        break;
        case DISPLAY_FUNC_ON_MASK:
        enhance_mode = DISPLAY_ENHANCE_ON;
        break;
        case DISPLAY_FUNC_DEMO_ON_MASK:
        enhance_mode = DISPLAY_ENHANCE_DEMO_ON;
        break;
    }

    lseek(fdEnhance, 0, SEEK_SET);
    ret = write(fdEnhance, enhance_mode, 1);
    if (ret < 0) {
        ALOGE("###write enhance mode failed!, fdEnhance=0x%d", fdEnhance);
        return ret;
    }

    if (property_set("persist.display.enhance", enhance_mode) == 0) {
        ALOGD("persist.display.enhance=%s",enhance_mode);
    } else {
        ALOGD("No enhance property node.");
        return -1;
    }

    return 0;
}

static int display_open(struct display_device_t *dev)
{
    fd = open("dev/disp", O_RDWR);
    fd3d = open("/sys/class/disp/disp/attr/operate_3d_mode", O_RDWR);
    fdEnhance = open("/sys/class/disp/disp/attr/enhance_mode", O_RDWR);
    int enhance_mode = 0;
    int smbl_mode = 0;

    if (fd >=0 && fd3d >= 0 && fdEnhance >=0) {
        ALOGE("display open.");
    } else {
        ALOGE("display open failed! fd = %d, fd3d=%d, fdEnhance=%d",
                fd, fd3d, fdEnhance);
        return -1;
    }

    enhance_mode = display_device_get_enhance_mode(0);
    smbl_mode = display_device_get_smt_backlight(0);
    display_device_set_enhance_mode(0, enhance_mode);
    display_device_set_smt_backlight(0, smbl_mode);
    return 0;
}

static int display_ctrl(struct display_device_t *dev,
                 int dispId, int cmd, int para0, int para1)
{
    int ret = 0;

    ALOGE("DISPLAY CTRL: disp=%d, CMD =%d, para0 = %d, para1 = %d, fd=%d, fd3d=%d, fdEnhance=%d",
        dispId, cmd, para0, para1, fd, fd3d, fdEnhance);

    switch (cmd) {
    case DISPLAY_CMD_SET_3DMODE:
        ret = display_device_set_3d_mode(dispId, para0);
        break;
    case DISPLAY_CMD_GET_3DMODE:
        ret = display_device_get_3d_mode(dispId);
        break;
    case DISPLAY_CMD_SET_BACKLIGHT:
        ret = display_device_set_smt_backlight(dispId, para0);
        break;
    case DISPLAY_CMD_GET_BACKLIGHT:
        ret = display_device_get_smt_backlight(dispId);
        return ret;
    case DISPLAY_CMD_SET_ENHANCE:
        ret = display_device_set_enhance_mode(dispId, para0);
        break;
    case DISPLAY_CMD_GET_ENHANCE:
        ret = display_device_get_enhance_mode(dispId);
        return ret;
    }
    return 0;
}

static int display_close(struct display_device_t *dev)
{
    if(fd >= 0) {
        close(fd);
    }
    if (dev)
        free(dev);
    return 0;
}

/** Open a new instance of a lights device using name */
static int display_device_open(const struct hw_module_t *module, char const *name,
               struct hw_device_t **device)
{
    if (strcmp(DISPLAY_HARDWARE_MODULE_ID, name)) {
        return -EINVAL;
    }

    pthread_mutex_init(&g_lock, NULL);

    struct display_device_t *dev = (struct display_device_t *)malloc(sizeof(struct display_device_t));
    memset(dev, 0, sizeof(*dev));
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t *)module;
    dev->common.close = (int(*)(struct hw_device_t *))display_close;
    dev->display_open = display_open;
    dev->display_ctrl = display_ctrl;
    *device = (struct hw_device_t *)dev;

    return 0;
}

static struct hw_module_methods_t display_methods =
{
    .open =  display_device_open,
};

/*
 * The backlight Module
 */
struct hw_module_t HAL_MODULE_INFO_SYM =
{
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = DISPLAY_HARDWARE_MODULE_ID,
    .name = "SoftWinner display manager Module",
    .author = "SOFTWINNER",
    .methods = &display_methods,
};
