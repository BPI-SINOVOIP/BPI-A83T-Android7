include device/softwinner/common/BoardConfigCommon.mk

TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_SMP := true
TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_VARIANT := cortex-a7

TARGET_BOARD_PLATFORM := octopus

TARGET_BOARD_CHIP := sun8iw6p1
TARGET_BOOTLOADER_BOARD_NAME := exdroid
TARGET_BOOTLOADER_NAME := exdroid

BOARD_EGL_CFG := hardware/aw/egl/octopus/egl.cfg
BOARD_KERNEL_BASE := 0x40000000
BOARD_MKBOOTIMG_ARGS := --kernel_offset 0x8000
BOARD_CHARGER_ENABLE_SUSPEND := true

TARGET_INIT_PARTITION_FROM_CMDLINE := true

NUM_FRAMEBUFFER_SURFACE_BUFFERS := 3
TARGET_RUNNING_WITHOUT_SYNC_FRAMEWORK := true

BOARD_SEPOLICY_DIRS := \
    device/softwinner/common/sepolicy \
    device/softwinner/octopus-common/sepolicy

USE_OPENGL_RENDERER := true

TARGET_USES_HWC2 := true
TARGET_USES_DE2 := true
