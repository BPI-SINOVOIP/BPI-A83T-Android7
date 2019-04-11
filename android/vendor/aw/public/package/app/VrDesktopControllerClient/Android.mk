LOCAL_PATH := $(call my-dir)
#########################build library#############################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
    cpp/native-lib.cpp

LOCAL_C_INCLUDES := \
    vendor/aw/public/package/bin/headtracking/libheadtrackingservice \
    system/core/init \
    frameworks/native/include

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libutils \
    libbinder \
    libheadtrackingservice \
    libgui

LOCAL_MODULE := libvrdeskcontroll
include $(BUILD_SHARED_LIBRARY)

#########################build apk#################################################
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := 32
LOCAL_DEX_PREOPT := nostripping

LOCAL_SRC_FILES := $(call all-java-files-under, java)

LOCAL_STATIC_JAVA_LIBRARIES := \
    ControllerAPI

LOCAL_SHARED_LIBRARIES := \
    libvrdeskcontroll

LOCAL_PACKAGE_NAME := VrDeskController
LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)

include $(CLEAR_VARS)

LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := \
    ControllerAPI:libs/ControllerAPI.jar

include $(BUILD_MULTI_PREBUILT)

