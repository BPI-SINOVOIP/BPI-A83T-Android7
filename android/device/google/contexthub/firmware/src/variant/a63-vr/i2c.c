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

#include <plat/inc/i2c.h>
#include <util.h>

static const struct ArI2cBoardCfg mArI2cBoardCfgs[] = {
	[0] = {
        .gpioScl = I2C1_GPIO_SCL_PL8,
        .gpioSda = I2C1_GPIO_SDA_PL9,

        .gpioPull = GPIO_PULL_NONE,
        .sleepDev = Stm32sleepDevI2c1,
    },
    [1] = {
        .gpioScl = I2C2_GPIO_SCL_PL10,
        .gpioSda = I2C2_GPIO_SDA_PL11,

        .gpioPull = GPIO_PULL_NONE,

        .sleepDev = Stm32sleepDevI2c1,
    },
};

const struct ArI2cBoardCfg *boardArI2cCfg(uint8_t busId)
{
    if (busId >= ARRAY_SIZE(mArI2cBoardCfgs))
        return NULL;

    return &mArI2cBoardCfgs[busId];
}
