/*
 * Copyright (C) 2010 The Android Open Source Project
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

#define LOG_TAG "FakeInputDispatcher"
#define ATRACE_TAG ATRACE_TAG_INPUT

#include "FakeInputDispatcher.h"

#include <utils/Trace.h>
#include <cutils/log.h>
#include <powermanager/PowerManager.h>
#include <ui/Region.h>

#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

#include <sys/types.h>
#include <unistd.h>
#include <grp.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/Parcel.h>
#include <utils/Log.h>

#include "IHeadTrackingService.h"
#include "headtracking.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <android/keycodes.h>

#define DEBUG_INBOUND_EVENT_DETAILS 0
#define DEBUG_CHANGE_COORD 1

namespace android {
FakeInputDispatcher::FakeInputDispatcher () {

}

FakeInputDispatcher* FakeInputDispatcher::mFakeInputDispatcher = nullptr;

FakeInputDispatcher::~FakeInputDispatcher () {
    if (NULL != mFakeInputDispatcher) {
        delete(mFakeInputDispatcher);
    }
}
/*
FakeInputDispatcher* FakeInputDispatcher::getInstance() {
    if (NULL == mFakeInputDispatcher) {
        mFakeInputDispatcher = new FakeInputDispatcher();
    }
    return mFakeInputDispatcher;
}*/

uint64_t GetTicksNanos()
{
    struct timespec tp;
    const int       status = clock_gettime(CLOCK_MONOTONIC, &tp);
    if(status != 0)
    {
        //DBG("clock_gettime return %d \n", status);
    }

    const uint64_t result = (uint64_t)tp.tv_sec * (uint64_t)(1000 * 1000 * 1000) + uint64_t(tp.tv_nsec);
    return result;
}

/*
nsecs_t eventTime;
int32_t deviceId;
uint32_t source;
uint32_t policyFlags;
int32_t action;
int32_t flags;
int32_t keyCode;
int32_t scanCode;
int32_t metaState;
nsecs_t downTime;
*/

#define LONG_PRESS_TIMEOUT 0.5 * 1000 * 1000 * 1000

static bool lastButtonIsBackDown = false;
static bool longBackPressed = false;
static bool pressedFinish = false;

void * checkLongBackPress(void* arg) {
    pthread_setname_np( pthread_self(), "checkLongBackPress" );
    pthread_detach(pthread_self());

    longBackPressed = false;
    pressedFinish = false;

    //if (arg == NULL)
    //    return NULL;
    double start = GetTicksNanos();
    while (1) {
        if (!lastButtonIsBackDown) {
            pressedFinish = true;
            return NULL;
        }
        if ((GetTicksNanos() - start) < LONG_PRESS_TIMEOUT) {
            usleep(10 * 1000);
        } else {
            Headtracking::getInstance()->recenterOrientation();
            ALOGD("Headtracking.getInstance().recenterOrientation();");
            longBackPressed = true;
            pressedFinish = true;
            return NULL;
        }
    }
}

bool FakeInputDispatcher::notifyKey(NotifyKeyArgs* args, InputListenerInterface* listener) {
    int32_t keyCode = args->keyCode;
    bool down = args->action == AKEY_EVENT_ACTION_DOWN ? true : false;
    switch(keyCode){
        case AKEYCODE_BACK:
            if (down) {
                lastButtonIsBackDown= true;
                pthread_t threadID = 0;
                int ret = pthread_create(&threadID, NULL, checkLongBackPress, (void *)NULL);
                while (pressedFinish) {
                    if (longBackPressed) {
                        return true;
                    }
                    usleep(1000);
                }
            } else {
                lastButtonIsBackDown = false;
                if (longBackPressed) {
                    ALOGD("longBackPressed ignore ACTION_UP");
                    // ignore ACTION_UP
                    return true;
                }
            }
            break;
        case AKEYCODE_DPAD_CENTER:
        case AKEYCODE_ENTER:
            lastButtonIsBackDown = false;
            //simulate a touch event, and send it to listener.
            nsecs_t now = systemTime();
            PointerProperties pointerProperties;
            pointerProperties.clear();
            pointerProperties.id = 0;
            pointerProperties.toolType = AMOTION_EVENT_TOOL_TYPE_FINGER;
            PointerCoords pointerCoords;
            pointerCoords.clear();

            //caculate the touch point,and set it to the pointerCoords
            float x = 0;
            float y = 0;
            Headtracking::getInstance()->getInterPoint(x, y);
            pointerCoords.setAxisValue(AMOTION_EVENT_AXIS_X, x);
            pointerCoords.setAxisValue(AMOTION_EVENT_AXIS_Y, y);
            if (down) {

                NotifyMotionArgs argsDown(now, 1, AINPUT_SOURCE_TOUCHSCREEN, 0,
                    AMOTION_EVENT_ACTION_DOWN, 0, 0, 0, 0,
                    AMOTION_EVENT_EDGE_FLAG_NONE,
                    ADISPLAY_ID_DEFAULT, 1, &pointerProperties, &pointerCoords,
                    1.0f, 1.0f, now);
                listener->notifyMotion(&argsDown);
            } else {
                NotifyMotionArgs argsUp(now, 1, AINPUT_SOURCE_TOUCHSCREEN, 0,
                    AMOTION_EVENT_ACTION_UP, 0, 0, 0, 0,
                    AMOTION_EVENT_EDGE_FLAG_NONE,
                    ADISPLAY_ID_DEFAULT, 1, &pointerProperties, &pointerCoords,
                    1.0f, 1.0f, now);
                listener->notifyMotion(&argsUp);
            }
            //don't send the dpad_center key to the input dispatcher.
            return true;
    }
    return false;
}

void FakeInputDispatcher::notifyMotion(NotifyMotionArgs* args) {
    /*nsecs_t eventTime;
        int32_t deviceId;
        uint32_t source;
        uint32_t policyFlags;
        int32_t action;
        int32_t actionButton;
        int32_t flags;
        int32_t metaState;
        int32_t buttonState;
        int32_t edgeFlags;
        int32_t displayId;
        uint32_t pointerCount;
        PointerProperties pointerProperties[MAX_POINTERS];
        PointerCoords pointerCoords[MAX_POINTERS];
        float xPrecision;
        float yPrecision;
        nsecs_t downTime;
    */
    ALOGD("eventtime=%lld, deviceId=%d, source=%d, policyFlags=%d, \n"
        "action=%d actionButton=%d, flags=%d, metaState=%d \n"
        "buttonState=%d, edgeFlags=%d, displayId=%d, pointerCount=%d \n"
        "pointerProperties [id=%d, tooltype=%d], pointerCoords [bits=%lld]\n"
        "xPrecision=%f, yPrecision=%f, downTime=%lld",
        args->eventTime, args->deviceId, args->source, args->policyFlags,
        args->action, args->actionButton, args->flags, args->metaState,
        args->buttonState, args->edgeFlags, args->displayId, args->pointerCount,
        args->pointerProperties[0].id, args->pointerProperties[0].toolType,
        args->pointerCoords[0].bits,
        args->xPrecision, args->yPrecision, args->downTime);

    float x, y;
    x = y = 0;

    //if (args->action == AMOTION_EVENT_ACTION_DOWN || args->action == AMOTION_EVENT_ACTION_UP) {
    //if (true/*args->action != AMOTION_EVENT_ACTION_HOVER_MOVE && args->action != AMOTION_EVENT_ACTION_MOVE*/) {
    if(args->action == AMOTION_EVENT_ACTION_DOWN){
        Headtracking::getInstance()->getInterPoint(x, y);
        mPreDestX = x;
        mPreDestY = y;
        for (uint32_t i = 0; i < args->pointerCount; i++) {
            mPreOrigX[i] = args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_X);
            mPreOrigY[i] = args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_Y);
        #if DEBUG_CHANGE_COORD
            ALOGD("origx = %f, origy = %f, newx = %f, newy = %f, [i]=%d", mPreOrigX[i], mPreOrigY[i], x, y, i);
        #endif
            PointerCoords& out = args->pointerCoords[i];
            out.setAxisValue(AMOTION_EVENT_AXIS_X, x);
            out.setAxisValue(AMOTION_EVENT_AXIS_Y, y);

        }
    }else{
        for(uint32_t i = 0; i < args->pointerCount; i++){
            PointerCoords& out = args->pointerCoords[i];
            float orgX = out.getAxisValue(AMOTION_EVENT_AXIS_X);
            float orgY = out.getAxisValue(AMOTION_EVENT_AXIS_Y);
            float deltaX = orgX - mPreOrigX[i];
            float deltaY = orgY - mPreOrigY[i];
            out.setAxisValue(AMOTION_EVENT_AXIS_X, mPreDestX + deltaX);
            out.setAxisValue(AMOTION_EVENT_AXIS_Y, mPreDestY + deltaY);
        #if DEBUG_CHANGE_COORD
            ALOGE("action=%d  deltaX=%f,  deltaY=%f, origX=%f, origY=%f", args->action, deltaX, deltaY, orgX, orgY);
        #endif
        }
    }

}

#if DEBUG_INBOUND_EVENT_DETAILS
    ALOGD("notifyMotion - eventTime=%lld, deviceId=%d, source=0x%x, policyFlags=0x%x, "
            "action=0x%x, actionButton=0x%x, flags=0x%x, metaState=0x%x, buttonState=0x%x,"
            "edgeFlags=0x%x, xPrecision=%f, yPrecision=%f, downTime=%lld",
            args->eventTime, args->deviceId, args->source, args->policyFlags,
            args->action, args->actionButton, args->flags, args->metaState, args->buttonState,
            args->edgeFlags, args->xPrecision, args->yPrecision, args->downTime);
    for (uint32_t i = 0; i < args->pointerCount; i++) {
        ALOGD("  Pointer %d: id=%d, toolType=%d, "
                "x=%f, y=%f, pressure=%f, size=%f, "
                "touchMajor=%f, touchMinor=%f, toolMajor=%f, toolMinor=%f, "
                "orientation=%f",
                i, args->pointerProperties[i].id,
                args->pointerProperties[i].toolType,
                args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_X),
                args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_Y),
                args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_PRESSURE),
                args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_SIZE),
                args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_TOUCH_MAJOR),
                args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_TOUCH_MINOR),
                args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_TOOL_MAJOR),
                args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_TOOL_MINOR),
                args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_ORIENTATION));
    }
#endif

}

