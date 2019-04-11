LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src) $(call all-Iaidl-files-under, src)

#LOCAL_STATIC_JAVA_LIBRARIES := \
#    framework-protos \
#    SystemUI-proto-tags

#LOCAL_JAVA_LIBRARIES := telephony-common

LOCAL_PACKAGE_NAME := Screenshot
LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

include $(BUILD_PACKAGE)

