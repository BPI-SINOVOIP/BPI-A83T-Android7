LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc

LOCAL_SRC_FILES :=  src/PortalApi.cpp

LOCAL_LDLIBS := -landroid -llog -lz -lEGL

LOCAL_CPPFLAGS := -std=c++11

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= libatw_api

LOCAL_PRELINK_MODULE:= false

LOCAL_CPPFLAGS += -D_LOG_TAG_=\"PortalApi\"

include $(BUILD_SHARED_LIBRARY)
