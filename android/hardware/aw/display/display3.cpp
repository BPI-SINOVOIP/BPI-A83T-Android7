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
static int fdEnhance_bright;
static int fdEnhance_contrast;
static int fdEnhance_denoise;
static int fdEnhance_detail;
static int fdEnhance_edge;
static int fdEnhance_saturation;
static int fdDisp;
static int fdColorTemperature;

/*DE Color Temperature Ctrl*/
static int display_de_set_color_temperature(int disp, int ct_value)
{
    int ret = 0;
    char color_temp_dispid[PROPERTY_VALUE_MAX];
    char color_temp_val[PROPERTY_VALUE_MAX];

    if (disp < SUPPORT_DE_NUM) {
        lseek(fdDisp, 0, SEEK_SET);
        snprintf(color_temp_dispid, sizeof(color_temp_val), "%d", disp);
        ret = write(fdDisp, color_temp_dispid, sizeof(color_temp_dispid));
        if (ret < 0) {
            ALOGE("###write color_temp_dispid failed!, fdDisp=0x%d", fdDisp);
            return ret;
        }
    }else{
        ALOGE("###dispId = %d is not supported !", disp);
        return -1;
    }

    lseek(fdColorTemperature, 0, SEEK_SET);
    snprintf(color_temp_val, sizeof(color_temp_val), "%d", ct_value);
    ret = write(fdColorTemperature, color_temp_val, sizeof(color_temp_val));
    if (ret < 0) {
        ALOGE("###write color_temp_val failed!, fdColorTemperature=0x%d", fdColorTemperature);
        return ret;
    }
    return 0;
}

/*3D mode */
static int display_device_set_3d_mode(int dispId, int mode)
{
    int ret = 0;
    int i = 0;
    char data = (char)mode;
    char mode_prop[PROPERTY_VALUE_MAX];

    if (!(dispId < SUPPORT_DE_NUM)) {
        /* Too many devices */
        return -1;
    }

    lseek(fd3d, 0, SEEK_SET);
    snprintf(mode_prop, sizeof(mode_prop), "%d", mode);
    ret = write(fd3d, mode_prop, 1);
    if (ret < 0) {
        ALOGE("###write 3d mode failed! fd3d=%d", fd3d);
    }
    return 0;
}

/*Smart Backlight */
static int display_device_get_smt_backlight(int dispId)
{
    int mode = 0;
    char property[PROPERTY_VALUE_MAX];

    if (!(dispId < SUPPORT_DE_NUM)) {
        /* Too many devices */
        return -1;
    }

    if (property_get("persist.display.smbl", property, NULL) >= 0) {
        mode = atoi(property);
        ALOGE("smbl property mode=%d.", mode);
        return mode;
    } else {
        ALOGE("No smbl property node.");
        return 0;
    }
}

static int display_device_set_smt_backlight(int dispId, int mode)
{
    unsigned long arg[4] = {0};
    struct disp_rect win;
    char smbl_mode[PROPERTY_VALUE_MAX];

    if(dispId == 0 /*default primary device*/
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
        ALOGV("WIN w=%d, h=%d", win.width, win.height);

        snprintf(smbl_mode, sizeof(smbl_mode), "%d", mode);
        if (property_set("persist.display.smbl", smbl_mode) == 0) {
            ALOGD("persist.display.smbl=%d.", mode);
        } else {
            ALOGD("No smbl property node.");
            return -1;
            }
        }
    return 0;
}

/*Enhance Mode*/
static int display_device_get_enhance_mode(int dispId)
{
    char property[PROPERTY_VALUE_MAX];

    if (!(dispId < SUPPORT_DE_NUM)) {
        /* Too many devices */
        return -1;
    }

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

static int display_device_set_enhance_mode(int dispId, int mode)
{
    int ret = 0;
    char *enhance_mode;

    if (dispId != 0) {
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

    if ( mode > 0 ) {
        lseek(fdEnhance_bright, 0, SEEK_SET);
        ret = write(fdEnhance_bright, "5", 1);
        if (ret < 0) {
            ALOGE("###write enhance bright failed!, fdEnhance_bright=0x%d", fdEnhance_bright);
           return ret;
        }

        lseek(fdEnhance_contrast, 0, SEEK_SET);
        ret = write(fdEnhance_contrast, "5", 1);
        if (ret < 0) {
            ALOGE("###write enhance contrast failed!, fdEnhance_contrast=0x%d", fdEnhance_contrast);
            return ret;
        }

        lseek(fdEnhance_denoise, 0, SEEK_SET);
        ret = write(fdEnhance_denoise, "5", 1);
        if (ret < 0) {
            ALOGE("###write enhance denoise failed!, fdEnhance_denoise=0x%d", fdEnhance_denoise);
            return ret;
        }

        lseek(fdEnhance_detail, 0, SEEK_SET);
        ret = write(fdEnhance_detail, "5", 1);
        if (ret < 0) {
            ALOGE("###write enhance detail failed!, fdEnhance_detail=0x%d", fdEnhance_detail);
            return ret;
        }

        lseek(fdEnhance_edge, 0, SEEK_SET);
        ret = write(fdEnhance_edge, "5", 1);
        if (ret < 0) {
            ALOGE("###write enhance edge failed!, fdEnhance_edge=0x%d", fdEnhance_edge);
            return ret;
        }

        lseek(fdEnhance_saturation, 0, SEEK_SET);
        ret = write(fdEnhance_saturation, "5", 1);
        if (ret < 0) {
            ALOGE("###write enhance saturation failed!, fdEnhance_saturation=0x%d", fdEnhance_saturation);
            return ret;
        }
    }

    if (property_set("persist.display.enhance", enhance_mode) == 0) {
        ALOGD("persist.display.enhance=%s",enhance_mode);
    } else {
        ALOGD("No enhance property node.");
        return -1;
    }

    return 0;
}

/*Reading Mode*/
static int display_device_get_reading_mode_state(int dispId)
{
    int state = 0;
    char rm_property[PROPERTY_VALUE_MAX];

    if (!(dispId < SUPPORT_DE_NUM)) {
        /* Too many devices */
        return -1;
    }

    if (property_get("persist.display.rm_on", rm_property, NULL) >= 0) {
        state = atoi(rm_property);
        ALOGD("reading mode on property state=%d.", state);
        return (state > 0 ) ? true : false;
    } else {
        ALOGE("No reading mode on property node.");
        return 0;
    }
}

static int display_device_get_reading_mode_strength(int dispId)
{
    int strength = 0;
    char rm_property[PROPERTY_VALUE_MAX];

    if (!(dispId < SUPPORT_DE_NUM)) {
        /* Too many devices */
        return -1;
    }

    if (property_get("persist.display.rm_strength", rm_property, NULL) >= 0) {
        strength = atoi(rm_property);
        ALOGD("reading mdoe strength property strength=%d.", strength);
        return strength;
    } else {
        ALOGE("No reading mdoe strength property node.");
        return 0;
    }
}

static int display_device_set_reading_mode(int dispId, int on, int strength)
{
    int ret = 0;
    char reading_mode_on[PROPERTY_VALUE_MAX];
    char reading_mdoe_strength[PROPERTY_VALUE_MAX];

    if (on) {
        ret = display_de_set_color_temperature(dispId, (strength >= READING_MODE_STRENGTH_MIN) ?  \
                    strength : READING_MODE_STRENGTH_MIN);
        if (ret < 0) {
            ALOGE("### display_device_set_reading_mode on failed!");
            return ret;
        }
    }else{
        ret = display_de_set_color_temperature(dispId, 0);
        if (ret < 0) {
            ALOGE("### display_device_set_reading_mode off failed!");
            return ret;
        }
    }

    snprintf(reading_mode_on, sizeof(reading_mode_on), "%d", on);
    if (property_set("persist.display.rm_on", reading_mode_on) == 0) {
        ALOGD("persist.display.rm_on=%s", reading_mode_on);
    } else {
        ALOGD("No reading mode on property node.");
        return -1;
    }

    snprintf(reading_mdoe_strength, sizeof(reading_mdoe_strength), "%d", strength);
    if (property_set("persist.display.rm_strength", reading_mdoe_strength) == 0) {
        ALOGD("persist.display.rm_strength=%s", reading_mdoe_strength);
    } else {
        ALOGD("No reading mdoe strength property node.");
        return -1;
    }
    return 0;
}

/*Color Temperature*/
static int display_device_get_color_temperature(int dispId)
{
    int ct_val = 0;
    char color_temp_val[PROPERTY_VALUE_MAX];

    if (!(dispId < SUPPORT_DE_NUM)) {
        /* Too many devices */
        return -1;
    }

    if (property_get("persist.display.color_temp_val", color_temp_val, NULL) >= 0) {
        ct_val = atoi(color_temp_val) + RM_STRENGTH_OFFSET;
        ALOGD("color_temp_val property ct_val=%d.", ct_val);
        return ct_val;
    } else {
        ALOGE("No color_temp_val property node.");
        return 0;
    }
}

static int display_device_set_color_temperature(int dispId, int ct_val)
{
    int ret = 0;
    char color_temp_val[PROPERTY_VALUE_MAX];
    /*Fix me*/
    int real_ct_val = ct_val - RM_STRENGTH_OFFSET ;

    ret = display_de_set_color_temperature(dispId, real_ct_val);
    if (ret < 0) {
        ALOGE("### display_device_set_color_temperature failed!");
        return ret;
    }

    snprintf(color_temp_val, sizeof(color_temp_val), "%d", real_ct_val);
    if (property_set("persist.display.color_temp_val", color_temp_val) == 0) {
        ALOGD("persist.display.color_temp_val=%s", color_temp_val);
    } else {
        ALOGE("No color_temp_val property node.");
        return -1;
    }
    return 0;
}

static int display_open(struct display_device_t *dev)
{
    fd = open("dev/disp", O_RDWR);
    fd3d = open("/sys/class/disp/disp/attr/operate_3d_mode", O_RDWR);
    fdEnhance = open("/sys/class/disp/disp/attr/enhance_mode", O_RDWR);
    fdEnhance_bright = open("/sys/class/disp/disp/attr/enhance_bright", O_RDWR);
    fdEnhance_contrast = open("/sys/class/disp/disp/attr/enhance_contrast", O_RDWR);
    fdEnhance_denoise = open("/sys/class/disp/disp/attr/enhance_denoise", O_RDWR);
    fdEnhance_detail = open("/sys/class/disp/disp/attr/enhance_detail", O_RDWR);
    fdEnhance_edge = open("/sys/class/disp/disp/attr/enhance_edge", O_RDWR);
    fdEnhance_saturation = open("/sys/class/disp/disp/attr/enhance_saturation", O_RDWR);
    fdDisp = open("/sys/class/disp/disp/attr/disp", O_RDWR);
    fdColorTemperature = open("/sys/class/disp/disp/attr/color_temperature", O_RDWR);
    int enhance_mode = 0;
    int smbl_mode = 0;
    int rm_state = 0;
    int rm_strength = 0;
    int ct_val = 0;

    if (fd >=0) {
        ALOGD("display open.");
    } else {
        ALOGE("display open failed! fd=%d", fd);
        return -1;
    }

    if (fd3d >= 0) {
        ALOGV("fd3d = %d", fd3d);
    } else {
        ALOGW("display open fd3d failed, fd3d=%d", fd3d);
    }

    if (fdEnhance >=0 && fdEnhance_bright >= 0 && fdEnhance_contrast >= 0 \
            && fdEnhance_denoise >= 0 && fdEnhance_detail >= 0 \
            && fdEnhance_edge >= 0 && fdEnhance_saturation >=0) {
        /*Remember our enhance mode*/
        enhance_mode = display_device_get_enhance_mode(0);
        display_device_set_enhance_mode(0, enhance_mode);
        ALOGV("Enhance resource is fine !");
    } else {
        ALOGW("display open fdEnhance failed, fdEnhance=%d", fdEnhance);
    }

    /*Remember our smbl*/
    smbl_mode = display_device_get_smt_backlight(0);
    display_device_set_smt_backlight(0, smbl_mode);

    if (fdDisp >= 0 && fdColorTemperature >= 0) {
        /*Remember our reading mode and color temperature setting*/
        rm_state = display_device_get_reading_mode_state(0);
        if (rm_state != 0) {
            rm_strength = display_device_get_reading_mode_strength(0);
            display_device_set_reading_mode(0, 1, rm_strength);
        } else {
            ct_val = display_device_get_color_temperature(0);
            display_device_set_color_temperature(0, ct_val);
        }
        ALOGV("fdDisp = %d, fdColorTemperature = %d", fdDisp, fdColorTemperature);
    } else {
        ALOGW("display open fdDisp/fdColorTemperature failed, fdDisp=%d, fdColorTemperature=%d",
                fdDisp, fdColorTemperature);
        return -1;
    }

    return 0;
}

static int display_ctrl(struct display_device_t *dev,
                 int dispId, int cmd, int para0, int para1)
{
    int ret = 0;

    ALOGD("DISPLAY CTRL: disp=%d, CMD =%d, para0 = %d, para1 = %d, fd=%d, fd3d=%d, fdEnhance=%d, fdColorTemperature=%d",
        dispId, cmd, para0, para1, fd, fd3d, fdEnhance, fdColorTemperature);

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
        case DISPLAY_CMD_SET_READING_MODE:
            ret = display_device_set_reading_mode(dispId, para0, para1);
            break;
        case DISPLAY_CMD_GET_READING_STATE:
            ret = display_device_get_reading_mode_state(dispId);
            return ret;
        case DISPLAY_CMD_GET_READING_STRENGTH:
            ret = display_device_get_reading_mode_strength(dispId);
            return ret;
        case DISPLAY_CMD_SET_COLOR_TEMPERATURE:
            ret = display_device_set_color_temperature(dispId, para0);
            break;
        case DISPLAY_CMD_GET_COLOR_TEMPERATURE:
            ret = display_device_get_color_temperature(dispId);
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
