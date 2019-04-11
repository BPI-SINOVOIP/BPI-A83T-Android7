LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
    IDisplayService.cpp \
    DisplayService.cpp
LOCAL_MODULE := libdisplayservice
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libbinder \
    libandroid_runtime \
    libhardware
LOCAL_C_INCLUDES += \
    libcore/include
LOCAL_PRELINK_MODULE:= false
include $(BUILD_SHARED_LIBRARY)

############# main #################
include $(CLEAR_VARS)
LOCAL_SRC_FILES := main_displayservice.cpp
LOCAL_MODULE := displayservice
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := \
    libdisplayservice \
    libutils \
    liblog \
    libbinder
LOCAL_INIT_RC := displayservice.rc
include $(BUILD_EXECUTABLE)

