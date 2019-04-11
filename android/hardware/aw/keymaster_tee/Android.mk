# Copyright (C) 2012 The Android Open Source Project
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


LOCAL_PATH := $(call my-dir)

#########################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := keymaster_na.cpp logger.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
    $(TOP)/hardware/aw/optee_client-master/public

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := keystore.$(TARGET_BOARD_PLATFORM)
LOCAL_MULTILIB := both
LOCAL_MODULE_PATH_32 := $(TARGET_OUT)/lib/hw
LOCAL_MODULE_PATH_64 := $(TARGET_OUT)/lib64/hw
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := \
    liblog   \
  libteec

include $(BUILD_SHARED_LIBRARY)



