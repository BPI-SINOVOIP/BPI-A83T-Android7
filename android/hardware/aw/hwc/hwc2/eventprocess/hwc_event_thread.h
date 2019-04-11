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

#ifndef HWC_EVENT_THREAD_H
#define HWC_EVENT_THREAD_H

#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <cutils/log.h>
#include <hardware/hwcomposer2.h>
/*
 * Display device hotplug attribute,
 *  - HOTPLUG_TYPE_NONE :
 *    Not support hotplug detect, asume it is always online.
 *  - HOTPLUG_TYPE_SWITCH_EVENT :
 *    Support hotplug detect through switch event.
 */
typedef enum {
    HOTPLUG_TYPE_NONE = 0,
    HOTPLUG_TYPE_SWITCH_EVENT
} hotplug_type_t;

typedef struct event_thread event_thread_t;
struct event_thread {

    void (*start)(event_thread_t *thread, const char* name, int priority);
    void (*stop)(event_thread_t *thread);

    hwc2_error_t (*register_event_callback)(event_thread_t *thread,
                    int32_t descriptor, hwc2_callback_data_t callback_data,
                    hwc2_function_pointer_t pointer);
    hwc2_error_t (*register_global_vsync_callback)(event_thread_t *thread, hwc2_function_pointer_t pointer);
    hwc2_error_t (*set_hotplug_attribute)(event_thread_t *thread,
                    hwc2_display_t display, hotplug_type_t type, const void* patten);
    hwc2_error_t (*set_vsync_enabled)(event_thread_t *thread, hwc2_display_t display, hwc2_vsync_t enable);
    hwc2_error_t (*register_hotplug_callback)(event_thread_t *thread, hwc2_function_pointer_t pointer);
};

struct event_thread* event_thread_create(void);
void event_thread_destroy(struct event_thread* thread);

#endif
