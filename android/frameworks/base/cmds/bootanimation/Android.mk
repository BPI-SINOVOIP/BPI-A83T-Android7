LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    bootanimation_main.cpp \
    AudioPlayer.cpp \
    BootAnimation.cpp

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES

LOCAL_CFLAGS += -Wall -Werror -Wunused -Wunreachable-code

LOCAL_C_INCLUDES += \
    external/tinyalsa/include \
    frameworks/wilhelm/include

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libandroidfw \
    libutils \
    libbinder \
    libui \
    libskia \
    libEGL \
    libGLESv1_CM \
    libgui \
    libOpenSLES \
    libtinyalsa

ifneq ($(TARGET_BOOTANIMATION_MULTITHREAD_DECODE),false)
    LOCAL_CFLAGS += -DMULTITHREAD_DECODE
endif
ifeq ($(TARGET_BOOTANIMATION_SPLIT_SCREEN), true)
    LOCAL_CPPFLAGS += -DSPLIT_SCREEN
endif

ifneq ($(filter neptune,$(TARGET_BOARD_PLATFORM)),)
    LOCAL_CFLAGS += -DINPUT_EVENT_PATH=\"/sys/module/sun50iw3_sndcodec/parameters/switch_state\"
else
    LOCAL_CFLAGS += -DINPUT_EVENT_PATH=\"/sys/module/sunxi_sndcodec/parameters/switch_state\"
endif

LOCAL_MODULE:= bootanimation

LOCAL_INIT_RC := bootanim.rc

ifdef TARGET_32_BIT_SURFACEFLINGER
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_EXECUTABLE)
