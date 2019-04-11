LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common

LOCAL_SRC_FILES :=  PortalClient.cpp PortalClientLocal.cpp HeadTrackerThread.cpp \
                    ../common/IAtwService.cpp ../common/IAtwClient.cpp ../common/AtwTimer.cpp ../common/AtwRenderFpsObserver.cpp

LOCAL_SHARED_LIBRARIES := libgui libbinder libutils libcutils

LOCAL_LDLIBS := -landroid -llog -lz

LOCAL_CPPFLAGS := -std=c++11

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= libatw_2.0

LOCAL_PRELINK_MODULE:= false

LOCAL_CPPFLAGS += -D_LOG_TAG_=\"AtwClient\"

include $(BUILD_SHARED_LIBRARY)
