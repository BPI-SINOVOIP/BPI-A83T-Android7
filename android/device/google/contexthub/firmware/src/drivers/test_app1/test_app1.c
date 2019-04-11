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

#define APP_VERSION 1

static uint32_t mTestApp1Tid;
static uint32_t cnt;
static uint32_t mTimerId;

static void TimerCallback(uint32_t timerId, void *data)
{
     osLog(LOG_INFO, "test_app1: put: %lus\n", (*(uint32_t *)data)++);
     osEnqueueEvt(EVT_APP1_TO_APP2, data, NULL);
}

static bool startTask(uint32_t taskId)
{
    mTestApp1Tid = taskId;
    cnt = 1;
    osLog(LOG_INFO, "test_app1: task starting\n");

    osEventSubscribe(mTestApp1Tid, EVT_APP_START);
    osEventSubscribe(mTestApp1Tid, EVT_APP2_TO_APP1);

    return true;
}

static void endTask(void)
{
     osEventUnsubscribe(mTestApp1Tid, EVT_APP_START);
}

static void handleEvent(uint32_t evtType, const void* evtData)
{
    if (evtType == EVT_APP_START) {
        mTimerId = timTimerSet(1000000000, 100, 100, TimerCallback, &cnt, false);
        osLog(LOG_INFO, "test_app1: started with tid %lu timerid %lu\n", mTestApp1Tid, mTimerId);
    } else if (evtType == EVT_APP2_TO_APP1) {
    	if (mTimerId) {
            timTimerCancel(mTimerId);
	    mTimerId = 0;
    	}
	osLog(LOG_INFO, "test_app1: test success\n");
    } else {
	osLog(LOG_INFO, "test_app1: error evtType: %lu\n", evtType);
    }
}

INTERNAL_APP_INIT(APP_ID_MAKE(APP_ID_VENDOR_GOOGLE, 21), APP_VERSION, startTask, endTask, handleEvent);
