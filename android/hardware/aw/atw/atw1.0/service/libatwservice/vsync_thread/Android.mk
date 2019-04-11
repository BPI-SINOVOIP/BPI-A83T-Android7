LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := vsync_test
LOCAL_SRC_FILES := test_vsync_thread.cpp hwtw_vsyncthread.cpp
LOCAL_SHARED_LIBRARIES := libutils libcutils
include $(BUILD_EXECUTABLE)
