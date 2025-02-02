# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


LOCAL_PATH:= $(call my-dir)

# HAL module implemenation, not prelinked and stored in
# hw/<COPYPIX_HARDWARE_MODULE_ID>.<ro.board.platform>.so

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES += $(LOCAL_PATH) \
                    $(TARGET_KERNEL_INCLUDE) \
                    $(TARGET_HARDWARE_INCLUDE)

ifeq ($(TARGET_USES_DE2),true)
LOCAL_SRC_FILES := display2.cpp
else
ifeq ($(TARGET_USES_DE3),true)
LOCAL_SRC_FILES := display3.cpp
else
LOCAL_SRC_FILES := display.cpp
endif
endif

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SHARED_LIBRARIES := liblog \
    libcutils \

TARGET_GLOBAL_CFLAGS += -DTARGET_BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE := display.$(TARGET_BOARD_PLATFORM)
include $(BUILD_SHARED_LIBRARY)
