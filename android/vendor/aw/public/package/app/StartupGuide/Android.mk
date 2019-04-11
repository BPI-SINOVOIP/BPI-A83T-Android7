LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_JAVA_LIBRARIES := \
    android-support-v4

#LOCAL_STATIC_JAVA_AAR_LIBRARIES := \
#	viewpagerindicator

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res

#LOCAL_ASSET_DIR := $(LOCAL_PATH)/assets

LOCAL_SDK_VERSION := current
LOCAL_OVERRIDES_PACKAGES := SetupWizard

LOCAL_AAPT_FLAGS := --auto-add-overlay
#--extra-packages com.viewpagerindicator

LOCAL_PACKAGE_NAME := StartupGuide
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := platform
LOCAL_MULTILIB := 32
LOCAL_DEX_PREOPT := nostripping
#LOCAL_CERTIFICATE := PRESIGNED
#LOCAL_BUILT_MODULE_STEM := package.apk

#include vendor/aw/public/package/app/ViewPagerIndicatorLibrary/common.mk
include $(BUILD_PACKAGE)

include $(CLEAR_VARS)

#LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := \
#	viewpagerindicator:libs/viewpagerindicator-2.4.1.aar

include $(BUILD_MULTI_PREBUILT)

