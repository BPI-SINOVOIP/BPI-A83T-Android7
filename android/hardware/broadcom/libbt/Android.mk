LOCAL_PATH := $(call my-dir)

ifneq ($(BOARD_HAVE_BLUETOOTH_BCM),)

include $(CLEAR_VARS)

ifneq ($(BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR),)
  bdroid_C_INCLUDES := $(BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR)
  bdroid_CFLAGS += -DHAS_BDROID_BUILDCFG
else
  bdroid_C_INCLUDES :=
  bdroid_CFLAGS += -DHAS_NO_BDROID_BUILDCFG
endif

BDROID_DIR := $(TOP_DIR)system/bt
ifeq ($(BOARD_HAVE_BLUETOOTH_NAME), gb9663)
LOCAL_CFLAGS += -DUSE_GB9663_BT_MODULE
endif

ifeq ($(BOARD_HAVE_BLUETOOTH_NAME), ap6212a)
LOCAL_CFLAGS += -DUSE_AP6212A_BT_MODULE
endif

ifeq ($(BOARD_HAVE_BLUETOOTH_NAME), ap6255)
LOCAL_CFLAGS += -DUSE_AP6255_BT_MODULE
endif

ifeq ($(BOARD_HAVE_BLUETOOTH_NAME), ap6330)
LOCAL_CFLAGS += -DUSE_AP6330_BT_MODULE
endif

ifeq ($(BOARD_HAVE_BLUETOOTH_NAME), ap6210)
LOCAL_CFLAGS += -DUSE_AP6210_BT_MODULE
endif

ifeq ($(BOARD_HAVE_BLUETOOTH_NAME), ap6212)
LOCAL_CFLAGS += -DUSE_AP6212_BT_MODULE
endif

ifeq ($(BOARD_HAVE_BLUETOOTH_NAME), ap6234)
LOCAL_CFLAGS += -DUSE_AP6234_BT_MODULE
endif

ifeq ($(BOARD_HAVE_BLUETOOTH_NAME), ap6476)
LOCAL_CFLAGS += -DUSE_AP6476_BT_MODULE
endif

ifeq ($(BOARD_HAVE_BLUETOOTH_NAME), ap6335)
LOCAL_CFLAGS += -DUSE_AP6335_BT_MODULE
endif

LOCAL_CFLAGS += -DHAVE_BLUETOOTH_BCM

ifeq ($(strip $(USE_BLUETOOTH_BCM4343)),true)
LOCAL_CFLAGS += -DUSE_BLUETOOTH_BCM4343
endif

LOCAL_SRC_FILES := \
        src/bt_vendor_brcm.c \
        src/bt_vendor_brcm_a2dp.c \
        src/hardware.c \
        src/userial_vendor.c \
        src/upio.c \
        src/conf.c

LOCAL_C_INCLUDES += \
        $(LOCAL_PATH)/include \
        $(BDROID_DIR)/hci/include \
        $(BDROID_DIR)/include \
        $(BDROID_DIR)/stack/include \
        $(BDROID_DIR)/gki/ulinux

LOCAL_C_INCLUDES += $(bdroid_C_INCLUDES)
LOCAL_CFLAGS += $(bdroid_CFLAGS)

LOCAL_SHARED_LIBRARIES := \
        libcutils \
        liblog

LOCAL_MODULE := libbt-vendor
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := broadcom
LOCAL_PROPRIETARY_MODULE := true

include $(LOCAL_PATH)/vnd_buildcfg.mk

include $(BUILD_SHARED_LIBRARY)

ifeq ($(TARGET_PRODUCT), full_maguro)
    include $(LOCAL_PATH)/conf/samsung/maguro/Android.mk
endif
ifeq ($(TARGET_PRODUCT), full_crespo)
    include $(LOCAL_PATH)/conf/samsung/crespo/Android.mk
endif
ifeq ($(TARGET_PRODUCT), full_crespo4g)
    include $(LOCAL_PATH)/conf/samsung/crespo4g/Android.mk
endif
ifeq ($(TARGET_PRODUCT), full_wingray)
    include $(LOCAL_PATH)/conf/moto/wingray/Android.mk
endif
ifeq ($(TARGET_PRODUCT), gce_x86_phone)
    include $(LOCAL_PATH)/conf/google/gce_x86/Android.mk
endif

endif # BOARD_HAVE_BLUETOOTH_BCM
