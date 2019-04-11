LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libatwcore
LOCAL_MODULE_SUFFIX := .so
LOCAL_32_BIT_ONLY := true
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES_arm := core/lib/libatwcore.so
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_32_BIT_ONLY := true
LOCAL_CFLAGS := -O2
LOCAL_C_INCLUDES += $(LOCAL_PATH)/core/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../hal
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/utils

LOCAL_SRC_FILES :=  AtwService.cpp \
                    AtwClient.cpp \
                    AtwLayer.cpp \
                    AtwConsumer.cpp \
                    MessageQueue.cpp \
                    AtwCommitThread.cpp \
                    AtwCommitWorker.cpp \
                    AtwVsync.cpp \
                    vsync_thread/hwtw_vsyncthread.cpp \
                    vsync_thread/hwtw_committhread.cpp \
                    ../../common/AtwTimer.cpp \
                    AtwCalculator.cpp \
                    AtwHal.cpp \
                    sw_sync/sync.c \
                    bmp/bmp.cpp \
                    ../../common/AW_Math.cpp \
                    ../../common/IAtwService.cpp \
                    ../../common/IAtwClient.cpp \
                    ../../common/IHeadTracker.cpp

LOCAL_SHARED_LIBRARIES :=             \
    libui                            \
    libcutils                         \
    libutils                          \
    libbinder                         \
    libandroid_runtime                \
    libsync                            \
    libhardware                        \
    libgui

LOCAL_SHARED_LIBRARIES += libatwcore

LOCAL_LDLIBS += -llog -lz

LOCAL_CPPFLAGS := -std=c++11
LOCAL_C_INCLUDES += libcore/include

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= libatwservice_1.0

LOCAL_PRELINK_MODULE:= false

LOCAL_CPPFLAGS += -D_LOG_TAG_=\"AtwService\"

LOCAL_CPPFLAGS += -DDEBUG_SAVE_FRAMEBUFFER=0

LOCAL_CPPFLAGS += -DDEBUG_SHADER_CODE=0

LOCAL_CPPFLAGS += -DDEBUG_ATW_PERFORMANCE_FPS=1

LOCAL_CPPFLAGS += -DDEBUG_OVER_SLEEP_PROBLEM=1

LOCAL_CPPFLAGS += -DDEBUG_RUN_ON_H8=0

LOCAL_CPPFLAGS += -DDEBUG_ONLY_CORRECT_DISTORTION=0

LOCAL_CPPFLAGS += -DENABLE_ASYNC_COMMIT=0

LOCAL_CPPFLAGS += -DDEBUG_COMPUTE_PERF=1

include $(BUILD_SHARED_LIBRARY)
