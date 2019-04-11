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

#include <plat/inc/gpio.h>
#include <plat/inc/include.h>
#include <plat/inc/pwr.h>
#include <gpio.h>
#include <cpu.h>

static void gpioSetWithNum(uint32_t gpioNum, bool value);

struct Gpio* gpioRequest(uint32_t number)
{
    return (struct Gpio*)(((uintptr_t)number) + GPIO_HANDLE_OFFSET);
}

void gpioRelease(struct Gpio* __restrict gpio)
{
    (void)gpio;
}

static void gpioConfigInputWithNum(uint32_t gpioNum, int32_t gpioSpeed, enum GpioPullMode pull)
{
    pin_set_multi_sel(gpioNum >> GPIO_PORT_SHIFT, gpioNum & GPIO_PIN_MASK, 0);
    pin_set_pull(gpioNum >> GPIO_PORT_SHIFT, gpioNum & GPIO_PIN_MASK, pull);
}

void gpioConfigInput(const struct Gpio* __restrict gpioHandle, int32_t gpioSpeed, enum GpioPullMode pull)
{
    if (gpioHandle)
        gpioConfigInputWithNum((uint32_t)gpioHandle - GPIO_HANDLE_OFFSET, gpioSpeed, pull);
}

static void gpioConfigOutputWithNum(uint32_t gpioNum, int32_t gpioSpeed, enum GpioPullMode pull, enum GpioOpenDrainMode output, bool value)
{
    pin_set_multi_sel(gpioNum >> GPIO_PORT_SHIFT, gpioNum & GPIO_PIN_MASK, 1);
    pin_set_pull(gpioNum >> GPIO_PORT_SHIFT, gpioNum & GPIO_PIN_MASK, pull);
    pin_write_data(gpioNum >> GPIO_PORT_SHIFT, gpioNum & GPIO_PIN_MASK, value);
}

void gpioConfigOutput(const struct Gpio* __restrict gpioHandle, int32_t gpioSpeed, enum GpioPullMode pull, enum GpioOpenDrainMode output, bool value)
{
    if (gpioHandle)
        gpioConfigOutputWithNum((uint32_t)gpioHandle - GPIO_HANDLE_OFFSET, gpioSpeed, pull, output, value);
}

static void gpioConfigAltWithNum(uint32_t gpioNum, int32_t gpioSpeed, enum GpioPullMode pull, enum GpioOpenDrainMode output, uint32_t altFunc)
{
    pin_set_multi_sel(gpioNum >> GPIO_PORT_SHIFT, gpioNum & GPIO_PIN_MASK, altFunc);
    pin_set_pull(gpioNum >> GPIO_PORT_SHIFT, gpioNum & GPIO_PIN_MASK, pull);
}

void gpioConfigAlt(const struct Gpio* __restrict gpioHandle, int32_t gpioSpeed, enum GpioPullMode pull, enum GpioOpenDrainMode output, uint32_t altFunc)
{
    if (gpioHandle)
        gpioConfigAltWithNum((uint32_t)gpioHandle - GPIO_HANDLE_OFFSET, gpioSpeed, pull, output, altFunc);
}

static void gpioConfigAnalogWithNum(uint32_t gpioNum)
{
    pin_set_multi_sel(gpioNum >> GPIO_PORT_SHIFT, gpioNum & GPIO_PIN_MASK, 7);
}

void gpioConfigAnalog(const struct Gpio* __restrict gpioHandle)
{
    if (gpioHandle)
        gpioConfigAnalogWithNum((uint32_t)gpioHandle - GPIO_HANDLE_OFFSET);
}

static void gpioSetWithNum(uint32_t gpioNum, bool value)
{
    pin_write_data(gpioNum >> GPIO_PORT_SHIFT, gpioNum & GPIO_PIN_MASK, value);
}

void gpioSet(const struct Gpio* __restrict gpioHandle, bool value)
{
    if (gpioHandle)
        gpioSetWithNum((uint32_t)gpioHandle - GPIO_HANDLE_OFFSET, value);
}

static bool gpioGetWithNum(uint32_t gpioNum)
{
    return !!(pin_read_data(gpioNum >> GPIO_PORT_SHIFT, gpioNum & GPIO_PIN_MASK));
}

bool gpioGet(const struct Gpio* __restrict gpioHandle)
{
    return gpioHandle ? gpioGetWithNum((uint32_t)gpioHandle - GPIO_HANDLE_OFFSET) : 0;
}


#ifdef DEBUG_UART_PIN

//this function makes more assumptions than i'd care to list, sorry...
void gpioBitbangedUartOut(uint32_t chr)
{
    static const uint32_t bsrrVals[] = {(1 << (DEBUG_UART_PIN & GPIO_PIN_MASK)) << 16, (1 << (DEBUG_UART_PIN & GPIO_PIN_MASK))};
    struct StmGpio *block = (struct StmGpio*)mGpioBases[DEBUG_UART_PIN >> GPIO_PORT_SHIFT];
    uint32_t bits[10], *bitsP = bits, base = (uint32_t)&block->BSRR;
    static bool setup = 0;
    uint64_t state;
    uint32_t i;

    if (!setup) {
        struct Gpio *gpio = gpioRequest(DEBUG_UART_PIN);

        if (!gpio)
            return;

        setup = true;
        gpioConfigOutput(gpio, GPIO_SPEED_HIGH, GPIO_PULL_NONE, GPIO_OUT_PUSH_PULL, true);
    }

    bits[0] = bsrrVals[0];
    for (i = 0; i < 8; i++, chr >>= 1)
        bits[i + 1] = bsrrVals[chr & 1];
    bits[9] = bsrrVals[1];

    #define SENDBIT "ldr %0, [%1], #4   \n\t"   \
   "str %0, [%2]   \n\t"   \
   "nop    \n\t"   \
   "nop    \n\t"   \
   "nop    \n\t"   \
   "nop    \n\t"   \
   "nop    \n\t"   \
   "nop    \n\t"

   state = cpuIntsOff();
   asm volatile(
       SENDBIT
       SENDBIT
       SENDBIT
       SENDBIT
       SENDBIT
       SENDBIT
       SENDBIT
       SENDBIT
       SENDBIT
       SENDBIT
       :"=r"(i), "=r"(bitsP), "=r"(base)
       :"0"(i), "1"(bitsP), "2"(base)
       :"memory","cc"
    );
    cpuIntsRestore(state);
}


#endif




