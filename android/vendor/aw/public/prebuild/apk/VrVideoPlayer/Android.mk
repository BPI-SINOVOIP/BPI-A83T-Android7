###############################################################################
# Vr app android system build Android.mk template
# copy it to android system and rename to Android.mk
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# TODO: set your vr app module name here
# must be same as VrNative/xx, like: VrLauncher, VrVideoPlayer, VrAppCenter ...
LOCAL_MODULE := VrVideoPlayer

LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_MULTILIB := 32
LOCAL_DEX_PREOPT := nostripping
#LOCAL_MODULE_PATH := $(TARGET_OUT)/preinstall
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_OVERRIDES_PACKAGES := VideoPlayer
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
include $(BUILD_PREBUILT)


# check and build vr app
# Don't need to modify it
#-------------------------------------------
VR_APP_PATH := $(ANDROID_BUILD_TOP)/../ovr/VrNative/$(LOCAL_MODULE)

ifeq ($(wildcard $(VR_APP_PATH)), $(VR_APP_PATH))

APP_OUT_PATH := $(ANDROID_BUILD_TOP)/$(LOCAL_PATH)
# give buid type
ifeq ($(TARGET_BUILD_VARIANT), user)
	APP_BUILD_TYPE := release
else
	APP_BUILD_TYPE := debug
endif

$(info build vr app: $(LOCAL_MODULE) $(APP_BUILD_TYPE)  please wait ... )

# TODO: how let the build.sh output to console ...
EXE := $(shell $(VR_APP_PATH)/build.sh $(APP_BUILD_TYPE) $(APP_OUT_PATH) $(VR_APP_PATH))
#$(info $(EXE))

ERROR := $(shell cat $(VR_APP_PATH)/out/error_ret)
# if build vr app error, we give a error command let make error and stop ...
ifdef ERROR
$(info $(shell cat $(VR_APP_PATH)/out/error))
info error
else
$(info build vr app: $(LOCAL_MODULE) done )
endif

endif

