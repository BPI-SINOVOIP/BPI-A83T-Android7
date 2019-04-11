LOCAL_PATH:= $(call my-dir)

#include $(CLEAR_VARS)
#LOCAL_MODULE := sensorfusion
#LOCAL_SRC_FILES := sensorfusion/libAWSensorFusion.so
#TARGET_PRELINK_MODULES := false
#include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:=               \
    IHeadTrackingService.cpp 

LOCAL_SHARED_LIBRARIES :=     		\
	libcutils             			\
	libutils              			\
	libbinder             			\
	libui

LOCAL_C_INCLUDES := \
	libcore/include \

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libheadtrackingservice

LOCAL_PRELINK_MODULE := false
LOCAL_CFLAGS := -Wno-non-virtual-dtor

include $(BUILD_SHARED_LIBRARY)

#######################################################################################

#include $(CLEAR_VARS)
#LOCAL_MODULE := sensorfusion
#LOCAL_SRC_FILES := sensorfusion/libAWSensorFusion.so
#TARGET_PRELINK_MODULES := false
#include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:=               \
    HeadTrackingService.cpp

LOCAL_SHARED_LIBRARIES :=     		\
	libcutils             			\
	libutils              			\
	libbinder             			\
	libui                           \
	libandroid						\
	libheadtrackingservice

LOCAL_WHOLE_STATIC_LIBRARIES := libsensorfusion

LOCAL_C_INCLUDES := \
	libcore/include \
	../sensorfusion

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libheadtrackingnative

LOCAL_PRELINK_MODULE := false
LOCAL_CFLAGS := -Wno-non-virtual-dtor

include $(BUILD_SHARED_LIBRARY)

