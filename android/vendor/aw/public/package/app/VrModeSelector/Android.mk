LOCAL_PATH := $(call my-dir)


# build apk 
# ----------------------------
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

#LOCAL_STATIC_JAVA_LIBRARIES := \
    android-support-v4 \
	ViewPagerIndicatorLibrary

#LOCAL_JNI_SHARED_LIBRARIES := libVrModeSelectorJni
#LOCAL_JNI_SHARED_LIBRARIES := libdisplay_jni

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res
#LOCAL_ASSET_DIR := $(LOCAL_PATH)/assets

LOCAL_SDK_VERSION := current
LOCAL_AAPT_FLAGS := --auto-add-overlay

LOCAL_PACKAGE_NAME := VrModeSelector
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)


# build jni 
# ----------------------------
#include $(call all-makefiles-under, $(LOCAL_PATH))
