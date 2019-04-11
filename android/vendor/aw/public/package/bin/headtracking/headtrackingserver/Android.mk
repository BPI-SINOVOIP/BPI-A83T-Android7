LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	main_HeadTrackingServer.cpp 

LOCAL_SHARED_LIBRARIES := \
	libheadtrackingservice \
	libheadtrackingnative \
	libutils \
	libcutils \
	liblog \
	libbinder

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../libheadtrackingservice \
    system/core/init

LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_LIBRARIES := libinit

LOCAL_MODULE := htserver
LOCAL_CFLAGS += -Wno-non-virtual-dtor
LOCAL_INIT_RC := htserver.rc
include $(BUILD_EXECUTABLE)
