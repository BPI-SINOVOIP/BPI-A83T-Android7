LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_32_BIT_ONLY := true
LOCAL_CFLAGS := -O2
LOCAL_CFLAGS += -D_ARM_ASSEM_

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../common \
                    $(LOCAL_PATH)/../libatwservice/

LOCAL_SRC_FILES:= main_atwservice.cpp

LOCAL_SHARED_LIBRARIES := \
    libatwservice_1.0 \
    libbinder \
    libutils \
    libcutils

LOCAL_CPPFLAGS := -std=c++11

LOCAL_INIT_RC := atwservice.rc

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= service_atw

include $(BUILD_EXECUTABLE)
