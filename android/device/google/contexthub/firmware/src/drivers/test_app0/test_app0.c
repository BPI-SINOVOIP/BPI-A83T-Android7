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

#include <stdlib.h>
#include <string.h>
#include <float.h>

#include <eventnums.h>
#include <gpio.h>
#include <heap.h>
#include <hostIntf.h>
#include <isr.h>
#include <nanohubPacket.h>
#include <sensors.h>
#include <seos.h>
#include <timer.h>
#include <plat/inc/include.h>
#include <plat/inc/gpio.h>
#include <plat/inc/exti.h>

#define APP_VERSION 1

static uint32_t mTestApp0Tid;
static uint32_t cnt;
static struct message test_message;

static void TimerCallback(uint32_t timerId, void *data)
{
     //osLog(LOG_INFO, "test_app0: delay: %lu0s\n", (*(uint32_t *)data)++);

    //donot test dvfs
    return;
     if ((*(uint32_t *)data) > 60)
     	(*(uint32_t *)data) = 1;
     test_message.paras[0] = 24000 * (*(uint32_t *)data);
     dvfs_set_freq(&test_message);
     osLog(LOG_INFO, "%u, %u\n", ccu_get_sclk_freq(CCU_SYS_CLK_PLL1), pmu_get_voltage(AW1660_POWER_DCDC2));
}

static struct Gpio *pin;
struct ChainedIsr isr;

static bool testIsr(struct ChainedIsr *localIsr)
{
    if (!extiIsPendingGpio(pin)) {
    	osLog(LOG_INFO, "no pending\n");
        return false;
    }

    osLog(LOG_INFO, "gpio falling interrupt coming\n");

    extiClearPendingGpio(pin);
    return true;
}

static bool startTask(uint32_t taskId)
{
    mTestApp0Tid = taskId;
    cnt = 1;
    osLog(LOG_INFO, "test_app0: task starting\n");

    pin = gpioRequest(TEST_PIN);
    gpioConfigAlt(pin, GPIO_SPEED_LOW, GPIO_PULL_NONE, 0, GPIO_AF_EINT);
    extiEnableIntGpio(pin, EXTI_TRIGGER_FALLING);
    isr.func = testIsr;
    extiChainIsr(R_GPIOL_IRQn, &isr);

    osEventSubscribe(mTestApp0Tid, EVT_APP_START);

    return true;
}

static void endTask(void)
{
     osEventUnsubscribe(mTestApp0Tid, EVT_APP_START);
}

static void handleEvent(uint32_t evtType, const void* evtData)
{
    uint32_t timerId;

    if (evtType == EVT_APP_START) {
        timerId = timTimerSet(10000000000, 100, 100, TimerCallback, &cnt, false);
        osLog(LOG_INFO, "test_app0: started with tid %lu timerid %lu\n", mTestApp0Tid, timerId);
    } else {
	osLog(LOG_INFO, "test_app0: error evtType: %lu\n", evtType);
    }
}

INTERNAL_APP_INIT(APP_ID_MAKE(APP_ID_VENDOR_GOOGLE, 20), APP_VERSION, startTask, endTask, handleEvent);
