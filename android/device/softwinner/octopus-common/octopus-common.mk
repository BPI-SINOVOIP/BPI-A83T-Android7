# inherit common.mk
$(call inherit-product, device/softwinner/common/common.mk)

DEVICE_PACKAGE_OVERLAYS := \
    device/softwinner/octopus-common/overlay \
    $(DEVICE_PACKAGE_OVERLAYS)
PRODUCT_PACKAGES += \
    hwcomposer.octopus \
    lights.octopus \
    camera.octopus \
    display.octopus \
    keystore.exdroid

PRODUCT_PACKAGES += \
    libion

# power
PRODUCT_PACKAGES += \
    power.octopus

# audio
PRODUCT_PACKAGES += \
    audio.primary.octopus

USE_XML_AUDIO_POLICY_CONF := 1

PRODUCT_COPY_FILES += \
    device/softwinner/octopus-common/configs/audio_policy_configuration.xml:system/etc/audio_policy_configuration.xml \
    device/softwinner/octopus-common/configs/audio_policy_volumes_drc.xml:system/etc/audio_policy_volumes_drc.xml \
    device/softwinner/octopus-common/configs/audio_platform_info.xml:system/etc/audio_platform_info.xml \
    device/softwinner/octopus-common/configs/audio_mixer_paths.xml:system/etc/audio_mixer_paths.xml \
    device/softwinner/octopus-common/configs/cfg-videoplayer.xml:system/etc/cfg-videoplayer.xml \

PRODUCT_COPY_FILES += \
    device/softwinner/octopus-common/configs/media_codecs_performance.xml:system/etc/media_codecs_performance.xml \
    device/softwinner/octopus-common/configs/media_codecs.xml:system/etc/media_codecs.xml \
    device/softwinner/octopus-common/configs/media_codecs_google_video.xml:system/etc/media_codecs_google_video.xml \
    device/softwinner/octopus-common/init.recovery.sun8iw6p1.rc:root/init.recovery.sun8iw6p1.rc \
    device/softwinner/octopus-common/init.sun8iw6p1.rc:root/init.sun8iw6p1.rc \
    device/softwinner/octopus-common/init.sun8iw6p1.usb.rc:root/init.sun8iw6p1.usb.rc \
    device/softwinner/octopus-common/ueventd.sun8iw6p1.rc:root/ueventd.sun8iw6p1.rc

# egl
PRODUCT_COPY_FILES += \
    hardware/aw/egl/octopus/aw_version:system/vendor/lib/egl/aw_version \
    hardware/aw/egl/octopus/pvrsrvctl:system/vendor/bin/pvrsrvctl \
    hardware/aw/egl/octopus/libusc.so:system/vendor/lib/libusc.so \
    hardware/aw/egl/octopus/libglslcompiler.so:system/vendor/lib/libglslcompiler.so \
    hardware/aw/egl/octopus/libIMGegl.so:system/vendor/lib/libIMGegl.so \
    hardware/aw/egl/octopus/libpvr2d.so:system/vendor/lib/libpvr2d.so \
    hardware/aw/egl/octopus/libpvrANDROID_WSEGL.so:system/vendor/lib/libpvrANDROID_WSEGL.so \
    hardware/aw/egl/octopus/libPVRScopeServices.so:system/vendor/lib/libPVRScopeServices.so \
    hardware/aw/egl/octopus/libsrv_init.so:system/vendor/lib/libsrv_init.so \
    hardware/aw/egl/octopus/libsrv_um.so:system/vendor/lib/libsrv_um.so \
    hardware/aw/egl/octopus/libEGL_POWERVR_SGX544_115.so:system/vendor/lib/egl/libEGL_POWERVR_SGX544_115.so \
    hardware/aw/egl/octopus/libGLESv1_CM_POWERVR_SGX544_115.so:system/vendor/lib/egl/libGLESv1_CM_POWERVR_SGX544_115.so \
    hardware/aw/egl/octopus/libGLESv2_POWERVR_SGX544_115.so:system/vendor/lib/egl/libGLESv2_POWERVR_SGX544_115.so \
    hardware/aw/egl/octopus/gralloc.sunxi.so:system/vendor/lib/hw/gralloc.sun8iw6p1.so \
    hardware/aw/egl/octopus/memtrack.sunxi.so:system/vendor/lib/hw/memtrack.sun8iw6p1.so \
    hardware/aw/egl/octopus/powervr.ini:system/etc/powervr.ini

PRODUCT_PROPERTY_OVERRIDES += \
    wifi.interface=wlan0 \
    wifi.supplicant_scan_interval=15 \
    keyguard.no_require_sim=true

PRODUCT_PROPERTY_OVERRIDES += \
    ro.kernel.android.checkjni=0

# 131072=0x20000 196608=0x30000
PRODUCT_PROPERTY_OVERRIDES += \
    ro.opengles.version=131072

PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.strictmode.visual=0 \
    persist.sys.strictmode.disable=1

PRODUCT_PROPERTY_OVERRIDES += \
    ro.sys.cputype=UltraOcta-A83

PRODUCT_PROPERTY_OVERRIDES += \
    ro.product.firmware=v7.0rc5

PRODUCT_PROPERTY_OVERRIDES += \
     debug.hwc.showfps=0

PRODUCT_PROPERTY_OVERRIDES += \
    persist.display.smbl=0 \
    persist.display.enhance=0 \
    sys.display.trdmode=0

# for gts PersistentDataHostTest#testTestGetFlashLockState
PRODUCT_PROPERTY_OVERRIDES += \
    ro.boot.flash.locked=1

# Enabling type-precise GC results in larger optimized DEX files.  The
# additional storage requirements for ".odex" files can cause /system
# to overflow on some devices, so this is configured separately for
# each product.
PRODUCT_TAGS += dalvik.gc.type-precise

# if DISPLAY_BUILD_NUMBER := true then
# BUILD_DISPLAY_ID := $(BUILD_ID).$(BUILD_NUMBER)
# required by gms.
DISPLAY_BUILD_NUMBER := true
BUILD_NUMBER := $(shell date +%Y%m%d)

# widevine
BOARD_WIDEVINE_OEMCRYPTO_LEVEL := 3
#SECURE_OS_OPTEE := yes

#add widevine libraries
PRODUCT_PROPERTY_OVERRIDES += \
    drm.service.enabled=true \
    ro.sys.widevine_oemcrypto_level=3

PRODUCT_PACKAGES += \
    com.google.widevine.software.drm.xml \
    com.google.widevine.software.drm \
    libwvm \
    libWVStreamControlAPI_L${BOARD_WIDEVINE_OEMCRYPTO_LEVEL} \
    libwvdrm_L${BOARD_WIDEVINE_OEMCRYPTO_LEVEL} \
    libdrmdecrypt \
    libwvdrmengine

# secure os
PRODUCT_PACKAGES += \
    libteec \
    tee_supplicant

# keystore & gatekeeper
#PRODUCT_COPY_FILES += \
#    device/softwinner/common/optee_ta/11111111-2222-3333-4444444444444444.ta:system/bin/11111111-2222-3333-4444444444444444.ta \
#    device/softwinner/common/optee_ta/11111111-2222-3333-4444444444445555.ta:system/bin/11111111-2222-3333-4444444444445555.ta

# widevine level 1
ifeq ($(BOARD_WIDEVINE_OEMCRYPTO_LEVEL), 1)
PRODUCT_PACKAGES += liboemcrypto

PRODUCT_COPY_FILES += \
    device/softwinner/common/optee_ta/a98befed-d679-ce4a-a3c827dcd51d21ed.ta:system/bin/a98befed-d679-ce4a-a3c827dcd51d21ed.ta \
    device/softwinner/common/optee_ta/4d78d2ea-a631-70fb-aaa787c2b5773052.ta:system/bin/4d78d2ea-a631-70fb-aaa787c2b5773052.ta \
    device/softwinner/common/optee_ta/e41f7029-c73c-344a-8c5bae90c7439a47.ta:system/bin/e41f7029-c73c-344a-8c5bae90c7439a47.ta
endif
