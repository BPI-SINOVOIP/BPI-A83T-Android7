/*
 * Copyright (C) 2011 The Android Open Source Project
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
//#define LOG_NDEBUG 0
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <utils/Trace.h>

#include <cutils/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <cutils/uevent.h>
#include <system/graphics.h>
#include <cutils/properties.h>

#include "hwc_event_thread.h"
#include "../hwc.h"
#ifndef MAX_DEVICE_NUM
#define MAX_DEVICE_NUM 2
#endif

struct device_info {
    hwc2_display_t display;

    int32_t active;
    int32_t vsync;
    int32_t vsync_enable;
    int32_t connected;

    /*
     * pending_hotplug:
     *  - store the pending hotplug event
     *    which will update to SurfaceFlinger after
     *    hotplug callback installed
     *
     *    we only record the plugin event here.
     */
    int32_t pending_hotplug;
    hotplug_type_t type;
    char *switch_name;

    uint64_t frame_number;
    unsigned presentCnt;
    unsigned fpsFrameCnt;
    double preTime;

    int64_t timestamp[2];
    float frame_rate;
};

struct callback_info {
    hwc2_callback_data_t data;
    hwc2_function_pointer_t pointer;
};

typedef void (*HWC2_PFN_GLOBAL_VSYNC)(hwc2_display_t display, uint64_t framenumber, int64_t timestamp);
typedef void (*HWC2_PFN_DEVICE_MANAGER)(hwc2_display_t display, bool hotplug);

typedef enum callback_type {
    CALLBACK_VSYNC = 0,
    CALLBACK_HOTPLUG,
    CALLBACK_REFRESH,
    CALLBACK_GLOBAL_VSYNC,
    CALLBACK_DEVICE_SWITCH,
} callback_type_t;

struct event_thread_context {
    event_thread_t interface;

    /*
     * callback[0] for vsync        callback
     * callback[1] for hotplug      callback
     * callbakc[2] for global vsync callback
     * callback[3] for device switch callback
     */
    struct callback_info callback[5];

    // display device index by vsync id
    struct device_info device[MAX_DEVICE_NUM];

    char thread_name[32];
    int priority;
    pthread_t thread_id;
    pthread_mutex_t mutex;
    int stop;
};

#define __array_size(__array) \
    (sizeof(__array) / sizeof(__array[0]))

static void
call_hotplug(struct event_thread_context *context, struct device_info *dev, int32_t connect)
{
    int needupdate = 0;
    struct device_info *device = NULL;
    Display *dp = NULL;
    device = &context->device[1];
    dp = (Display*) device->display;

    /* switch device. */

    ALOGV("%s : connect=%d, vsync=%d", __FUNCTION__, connect, dev->vsync);
    pthread_mutex_lock(&context->mutex);
    dev->connected = connect;
    if (!context->callback[CALLBACK_HOTPLUG].pointer) {
        ALOGE("hotplug callback not install yet.");
        dev->pending_hotplug = connect;
    } else {
        needupdate = 1;
    }

    if (needupdate) {
        struct callback_info *cb = &context->callback[CALLBACK_HOTPLUG];
        struct callback_info *cb2 = &context->callback[CALLBACK_REFRESH];
        struct callback_info *cb1 = &context->callback[CALLBACK_DEVICE_SWITCH];

        HWC2_PFN_DEVICE_MANAGER deviceManager;
        if (dev->type == HOTPLUG_TYPE_SWITCH_EVENT) {
            deviceManager = (HWC2_PFN_DEVICE_MANAGER)cb1->pointer;
            if (deviceManager) {
                Display *dp;
                dp = (Display*)dev->display;
                ALOGV("%s : dev->display=%p, id=%d, num = %d",
                    __FUNCTION__, dp, dp->displayId, dp->configNumber);
                deviceManager(dev->display, connect);
                if (connect == 0) {
                    //sw_sync_timeline_inc(dp->syncTimeLine, 1);
                }
            }
        }

        HWC2_PFN_HOTPLUG hotplug = (HWC2_PFN_HOTPLUG)cb->pointer;
        ALOGV("%s : dev->display=%p, dev->display->id=%d, connect=%d",
            __FUNCTION__, dev->display, ((Display*)dev->display)->displayId,connect);
        hotplug(cb->data, dev->display, connect ? HWC2_CONNECTION_CONNECTED : HWC2_CONNECTION_DISCONNECTED);

        HWC2_PFN_REFRESH refresh = (HWC2_PFN_REFRESH)cb2->pointer;
        //if (refresh != NULL)
            //refresh(cb2->data, dev->display);
        /* surfaceFlinger hook. */
    }

    pthread_mutex_unlock(&context->mutex);
}

static void
call_hotplug1(struct event_thread_context *context, struct device_info *dev, int32_t connect)
{
    int needupdate = 0;

    /* switch device. */

    pthread_mutex_lock(&context->mutex);

    if (!context->callback[CALLBACK_HOTPLUG].pointer) {
        pthread_mutex_unlock(&context->mutex);
        return;
    }

    struct callback_info *cb = &context->callback[CALLBACK_HOTPLUG];
    struct callback_info *cb1 = &context->callback[CALLBACK_DEVICE_SWITCH];
    HWC2_PFN_DEVICE_MANAGER deviceManager;

    if (dev->type == HOTPLUG_TYPE_SWITCH_EVENT) {
        deviceManager = (HWC2_PFN_DEVICE_MANAGER)cb1->pointer;
        if (deviceManager) {
            Display *dp;
            dp = (Display*)dev->display;

            ALOGD("%s : hdmi: display=%p, id=%d, num = %d",
                __FUNCTION__, dp, dp->displayId, dp->configNumber);
            deviceManager(dev->display, connect);
        }

    }

    ALOGD("%s :displayID=%d, connect=%d",
                __FUNCTION__, ((Display*)dev->display)->displayId, connect);

    HWC2_PFN_HOTPLUG hotplug = (HWC2_PFN_HOTPLUG)cb->pointer;

    hotplug(cb->data, dev->display,
            connect ? HWC2_CONNECTION_CONNECTED : HWC2_CONNECTION_DISCONNECTED);

    pthread_mutex_unlock(&context->mutex);

}


static void showFps(struct event_thread_context *context, int32_t vsync)
{
    struct device_info *device;
    double curTime = 0;
    timeval tv = {0};
    char property[PROPERTY_VALUE_MAX];
    int fpsFlag = 0;
    Display* dp = NULL;
    device = &context->device[vsync];
    dp = (Display*)device->display;

    gettimeofday(&tv, NULL);
    curTime = tv.tv_sec + tv.tv_usec / 1.0e6;
    if (curTime - device->preTime < 1) {
        /* can not detect fps */
        return;
    }

    if (property_get("debug.hwc.showfps", property, NULL) >= 0) {
        fpsFlag = atoi(property);
    } else {
        ALOGD("No hwc debug attribute node.");
        return;
    }

    ALOGV(">>>fps DBG:: frameCnt = %d, fpsFrameCnt=%d, curTime = %llu, preTime=%llu\n",
            dp->frameCount, device->fpsFrameCnt, curTime, device->preTime);
    if(fpsFlag == 1) {
        ALOGD("###### %s hwc fps print ######", fpsFlag ? "Enable" : "Disable");
        ALOGD(">>>Disp%d fps::  %d\n", vsync,
                            (int)((dp->frameCount- device->fpsFrameCnt) * 1.0f
                                  / (curTime- device->preTime)));
    }
    #if 0
    //force gpu comp
    if ((dp->frameCount- device->fpsFrameCnt) < 2) {
        if (!dp->forceClient) {
            cb = &context->callback[CALLBACK_REFRESH];
            refresh_function = (HWC2_PFN_REFRESH)cb->pointer;
            refresh_function(cb->data, device->display);
            dp->forceClient = true;
        }
    } else {
        dp->forceClient = false;
    }
    #endif
    device->preTime= curTime;
    device->fpsFrameCnt = dp->frameCount;
}

static void
call_vsync(struct event_thread_context *context, int32_t vsync, int64_t timestamp)
{
    ATRACE_CALL();
    struct device_info *device = 0;
    HWC2_PFN_VSYNC vsync_function;
    Display* dp = NULL;
    device = &context->device[vsync];
    dp = (Display*) device->display;

#if 1
    pthread_mutex_lock(&dp->mutex);
    if (dp->frameCount - (int)device->presentCnt > 0) {
        //sw_sync_timeline_inc(dp->syncTimeLine, dp->frameCount - (int)device->presentCnt);
    }
    showFps(context, vsync);
    device->presentCnt = dp->frameCount;
    pthread_mutex_unlock(&dp->mutex);

    pthread_mutex_lock(&context->mutex);
    struct callback_info *cb = NULL;
    if (!device->active) {
        ALOGD("no display device binding to VSYNC%d, ts:%lld.", vsync, timestamp);
        goto __out;
    }
    if (!device->vsync_enable)
        goto __out;

    if (!context->callback[CALLBACK_VSYNC].pointer) {
        ALOGE("vsync callback not install yet.");
        goto __out;
    }
    pthread_mutex_unlock(&context->mutex);
#endif
    cb = &context->callback[CALLBACK_VSYNC];
    vsync_function = (HWC2_PFN_VSYNC)cb->pointer;
    vsync_function(cb->data, device->display, timestamp);
    return;

__out:
    pthread_mutex_unlock(&context->mutex);
    return;
}

static void
call_global_vsync(struct event_thread_context *context, int32_t vsync, int64_t timestamp)
{
    struct device_info *device = 0;
    struct callback_info *cb = 0;
    HWC2_PFN_GLOBAL_VSYNC global_vsync;

    pthread_mutex_lock(&context->mutex);
    device = &context->device[vsync];
    if (!context->callback[CALLBACK_GLOBAL_VSYNC].pointer) {
        ALOGE("global vsync callback not install yet.");
        goto __out;
    }
    pthread_mutex_unlock(&context->mutex);

    cb = &context->callback[CALLBACK_GLOBAL_VSYNC];
    global_vsync = (HWC2_PFN_GLOBAL_VSYNC)cb->pointer;
    global_vsync(device->display, device->frame_number++, timestamp);
    return;

__out:
    pthread_mutex_unlock(&context->mutex);
    return;
}

static int ready;

static hwc2_error_t
register_event_callback(event_thread_t *thread, int32_t descriptor,
    hwc2_callback_data_t callback_data, hwc2_function_pointer_t pointer)
{
    hwc2_error_t ret = HWC2_ERROR_NONE;
    struct callback_info *cb;
    struct event_thread_context *context = (struct event_thread_context *)(thread);

    pthread_mutex_lock(&context->mutex);
    switch (descriptor) {
    case HWC2_CALLBACK_HOTPLUG:
        ALOGV("HWC2_CALLBACK_HOTPLUG");
        cb = &context->callback[CALLBACK_HOTPLUG];
        cb->data = callback_data;
        cb->pointer = pointer;
        ready++;
        break;
    case HWC2_CALLBACK_VSYNC:
        ALOGV("HWC2_CALLBACK_VSYNC");
        cb = &context->callback[CALLBACK_VSYNC];
        cb->data = callback_data;
        cb->pointer = pointer;
        ready++;

        break;
    case HWC2_CALLBACK_REFRESH:
        ALOGV("HWC2_CALLBACK_REFRESH");
        cb = &context->callback[CALLBACK_REFRESH];
        cb->data = callback_data;
        cb->pointer = pointer;
        ready++;
        break;
    defalut:
        ret = HWC2_ERROR_UNSUPPORTED;
        ALOGE("unsupport callbakc descriptor %d", descriptor);
        break;
    }

    /*
     * Check the pending hotplug event
     */

    int needupdate = 0;
    struct device_info* pending_device[MAX_DEVICE_NUM] = {0};
    //memset(pending_device, 0, sizeof(struct device_info *) * MAX_DEVICE_NUM);

    if (descriptor == HWC2_CALLBACK_HOTPLUG) {
        int i;
        for (i = 0; i < __array_size(context->device); i++) {
            if (context->device[i].pending_hotplug != 0
                && context->device[i].type == HOTPLUG_TYPE_NONE) {
                pending_device[i] = &context->device[i];
                pending_device[i]->pending_hotplug = 0;
                needupdate++;
            }
        }
    }

    pthread_mutex_unlock(&context->mutex);

    if (needupdate) {
        for (int i = 0; i < __array_size(pending_device); i++)
            if (pending_device[i]
                && context->device[i].type == HOTPLUG_TYPE_NONE) {
                call_hotplug(context, pending_device[i], 1);
        }
    }
/*
    if (descriptor == HWC2_CALLBACK_HOTPLUG) {
        //for (int i = 0; i < 2; i++) {
            call_hotplug(context, &context->device[0], 1);

            sleep(1);
            //call_hotplug(context, &context->device[1], 0);
            //call_hotplug(context, &context->device[1], 1);
            //sleep(1);
            //call_hotplug(context, &context->device[1], 0);
            //sleep(1);
            //call_hotplug(context, &context->device[1], 1);
       // }
    }
*/
     return ret;
}

static hwc2_error_t
register_hotplug_callback(event_thread_t *thread, hwc2_function_pointer_t pointer)
{
    struct callback_info *cb;
    struct event_thread_context *context = (struct event_thread_context *)(thread);
    pthread_mutex_lock(&context->mutex);
    cb = &context->callback[CALLBACK_DEVICE_SWITCH];
    cb->data = 0;
    cb->pointer = pointer;
    pthread_mutex_unlock(&context->mutex);
    return HWC2_ERROR_NONE;
}

static hwc2_error_t
register_global_vsync_callback(event_thread_t *thread, hwc2_function_pointer_t pointer)
{
    struct callback_info *cb;
    struct event_thread_context *context = (struct event_thread_context *)(thread);
    pthread_mutex_lock(&context->mutex);
    cb = &context->callback[CALLBACK_GLOBAL_VSYNC];
    cb->data = 0;
    cb->pointer = pointer;
    pthread_mutex_unlock(&context->mutex);

    return HWC2_ERROR_NONE;
}

static struct device_info *
__hwc2_display_to_device(event_thread_t *thread, hwc2_display_t display)
{
    // FIXME: Get vsync id from hwc2_display_t
    Display* dp = (Display*) display;
    static int _fake_vsync_id = 0;
    _fake_vsync_id = dp->displayId;
    struct event_thread_context *context = (struct event_thread_context *)(thread);
    struct device_info *device = 0;

    ALOGV("%s entry", __func__);
    pthread_mutex_lock(&context->mutex);
    for (int i = 0; i < __array_size(context->device); i++) {
        if (context->device[i].display == display && context->device[i].active) {
            device = &context->device[i];
            break;
        }
    }

    // not match, create it!
    if (!device && _fake_vsync_id < MAX_DEVICE_NUM) {
        device = &context->device[_fake_vsync_id];
        memset(device, 0, sizeof(*device));
        device->display = display;
        device->active  = 1;
        device->vsync   = _fake_vsync_id;
        ALOGE("%s: create device.%d, vsync = %d", __func__, (int)display, device->vsync);
    }
    pthread_mutex_unlock(&context->mutex);
    return device;
}

static hwc2_error_t
set_vsync_enabled(event_thread_t *thread, hwc2_display_t display, hwc2_vsync_t enable)
{
    struct event_thread_context *context = (struct event_thread_context *)(thread);
    struct device_info *device;

#if 0
    pthread_mutex_lock(&context->mutex);
    for (int i = 0; i < __array_size(context->device); i++) {
        if (context->device[i].display == display) {
            context->device[i].vsync_enable = (enable == HWC2_VSYNC_ENABLE) ? 1 : 0;
            break;
        }
    }
    pthread_mutex_unlock(&context->mutex);
#else
    device = __hwc2_display_to_device(thread, display);
    if (!device){
        return HWC2_ERROR_UNSUPPORTED;
    }
    device->vsync_enable = 1;
    //test//device->vsync_enable = (enable == HWC2_VSYNC_ENABLE) ? 1 : 0;
#endif
    return HWC2_ERROR_NONE;
}

static int32_t
__read_switch_state(char *name)
{
    int fd, value;
    char data[2];
    char path[256];
    sprintf(path, "/sys/class/switch/%s/state", name);

    value = 0;
    fd = open(path, O_RDONLY, 0);
    if (fd >= 0) {
        read(fd, data, 1);
        close(fd);

        if (data[0] == '1')
            value = 1;
    }
    return value;
}

static void
__write_switch_state(char *name, int state)
{
    int fd, value;
    char data[2];
    char path[256];
    sprintf(path, "/sys/class/switch/%s/state", name);

    value = 0;
    data[0] = state + '0';
    fd = open(path, O_RDWR);
    if (fd >= 0) {
        int ret = 0;
        ret = write(fd, data, 1);
        ALOGD("write ret = %d", ret);
        close(fd);
    }
}

void testHotPlug()
{
    static int first = 0;
    if (!first) {
        ALOGD("%s", __FUNCTION__);
        first = 1;
        __write_switch_state("hdmi", 0);
        sleep(1);
        //__write_switch_state("hdmi", 1);
    }
}

static hwc2_error_t
set_hotplug_attribute(event_thread_t *thread,
    hwc2_display_t display, hotplug_type_t type, const void* patten)
{
    int plugin = 0;
    struct device_info *device = 0;
    struct event_thread_context *context = (struct event_thread_context *)(thread);

    device = __hwc2_display_to_device(thread, display);
    pthread_mutex_lock(&context->mutex);
    if (device) {
        device->type = type;
        if (type == HOTPLUG_TYPE_SWITCH_EVENT) {
            char *name = (char *)patten;
            if (device->switch_name)
                free(device->switch_name);
            device->switch_name = (char *)malloc(strlen(name) + 1);
            strcpy(device->switch_name, name);
            plugin = __read_switch_state(name);
            ALOGV("hdmi type, plug=%d\n", plugin);
        } else if (type == HOTPLUG_TYPE_NONE) {
            plugin = 1;
        }
    }
    pthread_mutex_unlock(&context->mutex);

    if (plugin && device) {
        call_hotplug(context, device, 1);
    }
    return HWC2_ERROR_NONE;
}


static void
vsync_uevent_parse(struct event_thread_context *context, const char *msg)
{
    while (*msg) {

        if (!strncmp(msg, "VSYNC", strlen("VSYNC"))) {
            msg += strlen("VSYNC");
            int32_t vsync_id = *(msg) - '0';
            int64_t timestamp = strtoull(msg + 2, NULL, 0);
            call_vsync(context, vsync_id, timestamp);
            call_global_vsync(context, vsync_id, timestamp);
        }
        while (*msg++);
    }
}

static void
switch_uevent_parse(struct event_thread_context *context, const char *msg)
{
    char switch_name[32];
    while (*msg) {
        if (!strncmp(msg, "SWITCH_NAME=", strlen("SWITCH_NAME="))) {
            msg += strlen("SWITCH_NAME=");

            // parse switch name
            int length = 0;
            while (*(msg + length) != 0 && *(msg + length) != '\n')
                length++;
            strncpy(switch_name, msg, length);
            switch_name[length] = 0;

            // parse switch state
            msg += length;
            while (*(msg) == 0 || *(msg) == '\n')
                msg++;
            if (!strncmp(msg, "SWITCH_STATE=", strlen("SWITCH_STATE="))) {

                int32_t state;
                struct device_info *device;
                int match = 0;
                msg += strlen("SWITCH_STATE=");
                state = *(msg) - '0';

                pthread_mutex_lock(&context->mutex);
                for (int i = 0; i < __array_size(context->device); i++) {
                    device = &context->device[i];
                    if (device->type == HOTPLUG_TYPE_SWITCH_EVENT
                            && !strcmp(device->switch_name, switch_name))
                        match = 1;
                }

                pthread_mutex_unlock(&context->mutex);
                if (match) {
                    ALOGD("Recive HDMI SWITCH state=%d.\n", state);
                    call_hotplug(context, device, state);
                }

            }
        }
        while (*msg++);
    }
}
extern int g_test_sync;

#define UEVENT_MSG_LEN  2048
static void *event_thread_loop(void *user)
{
    int ueventfd;
    int recvlen = 0;
    char msg[UEVENT_MSG_LEN + 2];
    struct event_thread_context *context = (struct event_thread_context *)user;

    ueventfd = uevent_open_socket(64*1024, true);
    if (ueventfd < 0) {
        ALOGE("uevent_open_socket error");
        return NULL;
    }

    setpriority(PRIO_PROCESS, 0, context->priority);
    while (!context->stop) {
        if (g_test_sync == 1) {
            for (int i = 0; i < __array_size(context->device); i++) {
                if (context->device[i].pending_hotplug != 0
                    && context->device[i].type == HOTPLUG_TYPE_SWITCH_EVENT) {
                    ALOGV("hotplug device%d", i);
                    call_hotplug(context, &context->device[1], 1);
                }
            }
            g_test_sync = 2;
        }

        recvlen = uevent_kernel_multicast_recv(ueventfd, msg, UEVENT_MSG_LEN);
        if (recvlen <= 0 || recvlen >= UEVENT_MSG_LEN)
            continue;

        msg[recvlen]   = 0;
        msg[recvlen+1] = 0;

        vsync_uevent_parse(context, msg);
        switch_uevent_parse(context, msg);
    }
    close(ueventfd);
    return NULL;
}

static void
start(event_thread_t *thread, const char* name, int priority)
{
    struct event_thread_context *context = (struct event_thread_context *)thread;

    context->priority = priority ? priority : HAL_PRIORITY_URGENT_DISPLAY;
    strncpy(context->thread_name, name, 31);
    pthread_create(&context->thread_id, NULL, event_thread_loop, context);
}

static void
stop(event_thread_t *thread)
{
    struct event_thread_context *context = (struct event_thread_context *)thread;
    context->stop = 1;
    pthread_join(context->thread_id, NULL);
}

struct event_thread*
event_thread_create(void)
{
    struct event_thread_context *context;

    context = (struct event_thread_context*)malloc(sizeof(*context));
    if (!context) {
        ALOGE("malloc for event thread\n");
        return 0;
    }
    memset(context, 0, sizeof(*context));
    pthread_mutex_init(&context->mutex, 0);

    context->interface.start = start;
    context->interface.stop  = stop;

    context->interface.register_event_callback = register_event_callback;
    context->interface.register_global_vsync_callback = register_global_vsync_callback;
    context->interface.set_hotplug_attribute = set_hotplug_attribute;
    context->interface.set_vsync_enabled = set_vsync_enabled;
    context->interface.register_hotplug_callback = register_hotplug_callback;

    return &context->interface;
}

void
event_thread_destroy(struct event_thread* thread)
{
    struct event_thread_context *context = (struct event_thread_context *)(thread);
    if (context)
        free(context);
    return;
}
