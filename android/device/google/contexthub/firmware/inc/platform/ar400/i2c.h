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

#ifndef __PLAT_I2C_H
#define __PLAT_I2C_H

#include <gpio.h>
#include <platform.h>
#include <plat/inc/cmsis.h>
#include <plat/inc/gpio.h>
#include <plat/inc/plat.h>

struct ArI2cDmaCfg {
    uint8_t channel;
    uint8_t stream;
};

struct ArI2cGpioCfg {
    uint8_t gpioNum;
    enum ArGpioAltFunc func;
};

struct ArI2cBoardCfg {
    struct ArI2cGpioCfg gpioScl;
    struct ArI2cGpioCfg gpioSda;

    enum ArGpioSpeed gpioSpeed;
    enum GpioPullMode gpioPull;

    enum PlatSleepDevID sleepDev;
};

#define I2C_DMA_BUS         0
#define I2C1_GPIO_SCL_PL0   { .gpioNum = GPIO_PL(0), .func = GPIO_AF_I2C1 }
#define I2C1_GPIO_SDA_PL1   { .gpioNum = GPIO_PL(1), .func = GPIO_AF_I2C1 }


#define I2C1_GPIO_SCL_PL8   { .gpioNum = GPIO_PL(8), .func = GPIO_AF_I2C1 }
#define I2C1_GPIO_SDA_PL9   { .gpioNum = GPIO_PL(9), .func = GPIO_AF_I2C1 }


#define I2C2_GPIO_SCL_PL10  { .gpioNum = GPIO_PL(10), .func = GPIO_AF_I2C2 }
#define I2C2_GPIO_SDA_PL11   { .gpioNum = GPIO_PL(11), .func = GPIO_AF_I2C2 }

extern const struct ArI2cBoardCfg *boardArI2cCfg(uint8_t busId);

#endif /* __PLAT_I2C_H */
