LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libVrModeSelectorJni

LOCAL_SRC_FILES := \
	native_lib.cpp

LOCAL_C_INCLUDES := \
	system/core/include \
	frameworks/native/include

LOCAL_LDLIBS += -llog -lutils -lbinder -lgui
#LOCAL_CFLAGS := 

include $(BUILD_SHARED_LIBRARY)
