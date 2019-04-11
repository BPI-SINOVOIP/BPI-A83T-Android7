LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := test_hwtw
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../common
LOCAL_SRC_FILES := test_hwtimewarp.cpp
LOCAL_SHARED_LIBRARIES := libEGL libGLESv2 libdl libhardware
include $(BUILD_EXECUTABLE)