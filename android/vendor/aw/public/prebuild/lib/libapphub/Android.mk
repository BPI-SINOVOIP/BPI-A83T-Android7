LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := apphub:apphub.jar
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
