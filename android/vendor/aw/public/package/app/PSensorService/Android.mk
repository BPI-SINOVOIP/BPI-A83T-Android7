LOCAL_PATH := $(call my-dir)

######################## build apk ##############################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res

# for access hide api, use system internal sdk
#LOCAL_SDK_VERSION := current

LOCAL_AAPT_FLAGS := --auto-add-overlay

LOCAL_MODULE_TAGS := optional
#LOCAL_MULTILIB := 32
#LOCAL_DEX_PREOPT := nostripping
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_PACKAGE_NAME := PSensorService
LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)

include $(CLEAR_VARS)

