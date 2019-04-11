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

#ifndef ANDROID_LIGHTS_INTERFACE_H
#define ANDROID_LIGHTS_INTERFACE_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <hardware/hardware.h>

__BEGIN_DECLS

/**
 * The id of this module
 */
#define DISPLAY_HARDWARE_MODULE_ID "display"

enum display_cmd {
    DISPLAY_CMD_SET_3DMODE = 0x01,
    DISPLAY_CMD_GET_3DMODE = 0x02,
    DISPLAY_CMD_SET_BACKLIGHT = 0x03,
    DISPLAY_CMD_GET_BACKLIGHT = 0x04,
    DISPLAY_CMD_SET_ENHANCE = 0x05,
    DISPLAY_CMD_GET_ENHANCE = 0x06,
    DISPLAY_CMD_SET_OUTPUT = 0x07,
    DISPLAY_CMD_SET_READING_MODE = 0x08,
    DISPLAY_CMD_GET_READING_STATE = 0x09,
    DISPLAY_CMD_GET_READING_STRENGTH = 0x0a,
    DISPLAY_CMD_SET_COLOR_TEMPERATURE = 0x0b,
    DISPLAY_CMD_GET_COLOR_TEMPERATURE = 0x0c,
};

struct display_device_t {
    struct hw_device_t common;

    /**
     * Set the provided lights to the provided values.
     *
     * Returns: 0 on succes, error code on failure.
     */
    int (*display_open) (struct display_device_t* dev);
    int (*display_ctrl)(struct display_device_t* dev,
    int dispId, int para0, int para1, int para2);
};


__END_DECLS

#endif  // ANDROID_LIGHTS_INTERFACE_H

