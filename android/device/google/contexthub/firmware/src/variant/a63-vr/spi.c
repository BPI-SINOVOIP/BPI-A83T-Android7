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

#include <plat/inc/spi.h>
#include <util.h>

static const struct ArSpiBoardCfg mArSpiBoardCfgs[] = {
    [0] = {
        .gpioMiso = GPIO_PL(15),
        .gpioMosi = GPIO_PL(14),
        .gpioSclk = GPIO_PL(13),
        .gpioNss =  GPIO_PL(12),

        .gpioFunc = GPIO_AF_SPI0,
        .gpioSpeed = GPIO_SPEED_MEDIUM,

        .irqNss = R_SPI_IRQn,

        .sleepDev = -1,
    },
};

const struct ArSpiBoardCfg *boardArSpiCfg(uint8_t busId)
{
    if (busId >= ARRAY_SIZE(mArSpiBoardCfgs))
        return NULL;

    return &mArSpiBoardCfgs[busId];
}
