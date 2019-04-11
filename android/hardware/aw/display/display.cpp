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
#include <hardware/drv_display.h>
#include <cutils/list.h>
#include <stdlib.h>
#include <sys/cdefs.h>
#include <cutils/properties.h>


static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static int fd;
static int fd3d;

static int display_device_set_3d_mode(int disp, int mode)
{
    int ret = 0;
    int i = 0;
    char data = (char)mode;

    data += '0';
    lseek(fd3d, 0, SEEK_SET);
    ret = write(fd3d, &data, 1);

    ALOGD("###write 3d mode, ret=%d, data = %d", ret, data);
    if (ret < 0) {
        ALOGE("###write 3d mode failed!");
    }
    return 0;
}

static int display_device_set_smt_backlight(int disp, int mode)
{
    unsigned long arg[4] = {0};
    char smbl_mode = 0;
    disp_window win;
    win.x = 0;
    win.y = 0;
    win.width = ioctl(fd,DISP_CMD_GET_SCN_WIDTH,arg);
    win.height = ioctl(fd,DISP_CMD_GET_SCN_HEIGHT,arg);

    if(disp == 0 /*default primary device*/
        /* && DisplayType != DISP_OUTPUT_TYPE_HDMI*/) {
        win.x = 0;
        win.y = 0;
        win.width = ioctl(fd, DISP_CMD_GET_SCN_WIDTH, arg);
        win.height = ioctl(fd, DISP_CMD_GET_SCN_HEIGHT, arg);
        /* demo mode.*/
        if (mode == 3) {
            win.height /= 2;
            arg[1] = (unsigned long)&win;
            ioctl(fd, DISP_CMD_DRC_SET_WINDOW, arg);
            arg[1] = 0;
            ioctl(fd, DISP_CMD_DRC_ENABLE, arg);
        } else if (mode == 1) {
            arg[1] = (unsigned long)&win;
            ioctl(fd, DISP_CMD_DRC_SET_WINDOW, arg);
            arg[1] = 0;
            ioctl(fd, DISP_CMD_DRC_ENABLE, arg);
        } else {
            ioctl(fd, DISP_CMD_DRC_DISABLE, arg);
        }
        ALOGV("WIN w=%d, h=%d", win.width, win.height);
    }

    smbl_mode = mode + '0';

    if (property_set("persist.display.smbl", &smbl_mode) == 0) {
        return 0;
    } else {
        ALOGD("No smbl property node.");
        return -1;
    }

    return 0;
}

static int display_device_set_enhance_mode(int disp, int mode)
{
    int ret = 0;
    unsigned long arg[4]={0};
    enum tag_DISP_CMD ioctl_arg;
    static int last_mode = 0;
    disp_window window;
    char enhance_mode = 0;

    if (disp != 0) {
        /* is not primary display */
        return -1;
    }

    window.x = 0;
    window.y = 0;
    window.width = ioctl(fd, DISP_CMD_GET_SCN_WIDTH, arg);
    window.height = ioctl(fd, DISP_CMD_GET_SCN_HEIGHT, arg);

    if (mode == 0) {
        ioctl(fd, DISP_CMD_ENHANCE_DISABLE, arg);
    } else if (mode == 3) {
        window.width /= 2;
        arg[1] = (unsigned long)&window;
        ioctl(fd, DISP_CMD_SET_ENHANCE_WINDOW, arg);
        arg[1] = 0;
        ioctl(fd, DISP_CMD_ENHANCE_ENABLE, arg);
    } else if (mode == 1) {
        arg[1] = (unsigned long)&window;
        ioctl(fd, DISP_CMD_SET_ENHANCE_WINDOW, arg);
        arg[1] = 0;
        ioctl(fd, DISP_CMD_ENHANCE_ENABLE, arg);
    }

    enhance_mode = mode + '0';

    if (property_set("persist.display.enhance", &enhance_mode) == 0) {
        return 0;
    } else {
        ALOGD("No enhance property node.");
        return -1;
    }
    return 0;
}

static int display_device_get_smt_backlight(int dispId)
{
    int mode = 0;
    char property[PROPERTY_VALUE_MAX];

    if (property_get("persist.display.smbl", property, NULL) >= 0) {
        mode = atoi(property);
        ALOGD("enhance property mode=%d.", mode);
        return mode;
    } else {
        ALOGE("No smbl property node.");
        return -1;
    }
}

static int display_device_get_enhance_mode(int dispId)
{
    int mode = 0;
    char property[PROPERTY_VALUE_MAX];

    if (property_get("persist.display.enhance", property, NULL) >= 0) {
        mode = atoi(property);
        ALOGD("enhance property mode=%d.", mode);
        return mode;
    } else {
        ALOGE("No enhance property node.");
        return -1;
    }
}

static int display_open(struct display_device_t *dev)
{
    fd = open("dev/disp", O_RDWR);
    int enhance_mode = 0;
    int smbl_mode = 0;
    if (fd >=0 /* && fd3d >= 0 */) {
        ALOGD("display open.");
    } else {
        ALOGE("display open failed! fd = %d, fd3d=%d",
                fd, fd3d);
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

    ALOGV("DISPLAY CTRL: disp=%d, CMD =%d, para0 = %d, para1 = %d, fd=%d, fd3d=%d",
        dispId, cmd, para0, para1, fd, fd3d);

    switch (cmd) {
    case DISPLAY_CMD_SET_3DMODE:
        ret = display_device_set_3d_mode(dispId, para0);
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

    ALOGD("display device open.");
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
