
# Copyright (C) 2008 The Android Open Source Project
# Copyright (C) 2012 Broadcom Corporation
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
#

LOCAL_PATH := $(call my-dir)


########################################
include $(CLEAR_VARS)
LOCAL_MODULE := VoiceService
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)

LOCAL_SRC_FILES := VoiceService.apk

LOCAL_PREBUILT_JNI_LIBS := \
    lib/armeabi/libbdEASRAndroid.v1.9.14..so \
    lib/armeabi/libBDSpeechDecoder_V1.so \
    lib/armeabi/libbdtts.so \
    lib/armeabi/libbd_easr_s1_merge_normal_20151216.dat.so \
    lib/armeabi/libbd_etts.so \
    lib/armeabi/libgnustl_shared.so \
    lib/armeabi/libBDVoiceRecognitionClient_MFE_V1_s2.so

LOCAL_MULTILIB :=32

include $(BUILD_PREBUILT)

