# inherit tools.mk
$(call inherit-product, vendor/aw/public/prebuild/bin/tools/tools.mk)
$(call inherit-product, vendor/aw/public/package/display/display.mk)

DEVICE_PACKAGE_OVERLAYS := \
    device/softwinner/common/overlay

PRODUCT_COPY_FILES += \
    device/softwinner/common/init.common.rc:root/init.common.rc \
    device/softwinner/common/init.debug.rc:root/init.debug.rc

ifeq ($(TARGET_BUILD_VARIANT),eng)
PRODUCT_PROPERTY_OVERRIDES += \
       dalvik.vm.image-dex2oat-filter="" \
       dalvik.vm.dex2oat-filter=""
endif

# scense_control
PRODUCT_PROPERTY_OVERRIDES += \
    sys.p_bootcomplete= true \
    sys.p_debug= false \
    sys.p_benchmark= true

PRODUCT_PACKAGES += \
    Screenshot \
    Screenrecord \
    htserver \
    httest

PRODUCT_PACKAGES += \
    WallpaperPicker \
    SaturnServer \
    Update \
    VideoPlayer

# preinstall apk
PRODUCT_PACKAGES += \
    DragonFire \
    DragonAging \
    DragonComposite \
    ESFileExplorer \

# usb
PRODUCT_PACKAGES += \
    com.android.future.usb.accessory

# wifi
PRODUCT_PACKAGES += \
    libwpa_client \
    hostapd \
    dhcpcd.conf \
    wpa_supplicant \
    wpa_supplicant.conf

# nand trim
PRODUCT_PACKAGES += \
    nand_trim

# param opt
PRODUCT_PACKAGES += \
    paramopt

PRODUCT_COPY_FILES += \
    device/softwinner/common/config/config_mem.ini:root/config_mem.ini \
    device/softwinner/common/config/vrdesktop_bg.jpg:system/media/vrdesktop_bg.jpg \
    device/softwinner/common/config/gaze_cursor.png:system/media/gaze_cursor.png
# charger
PRODUCT_PACKAGES += \
    charger_res_images \
    charger

# audio
PRODUCT_PACKAGES += \
    audio.a2dp.default \
    audio.usb.default \
    audio.r_submix.default

USE_XML_AUDIO_POLICY_CONF := 1

PRODUCT_COPY_FILES += \
    frameworks/av/services/audiopolicy/config/a2dp_audio_policy_configuration.xml:system/etc/a2dp_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/usb_audio_policy_configuration.xml:system/etc/usb_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:system/etc/r_submix_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/default_volume_tables.xml:system/etc/default_volume_tables.xml

# video
PRODUCT_COPY_FILES += \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:system/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_telephony.xml:system/etc/media_codecs_google_telephony.xml

include frameworks/av/media/libcedarc/libcdclist.mk
include frameworks/av/media/libcedarx/libcdxlist.mk

# xml
PRODUCT_COPY_FILES += \
    device/softwinner/common/config/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.software.midi.xml:system/etc/permissions/android.software.midi.xml \
    frameworks/native/data/etc/android.hardware.ethernet.xml:system/etc/permissions/android.hardware.ethernet.xml

PRODUCT_PACKAGES += \
    sensors.exdroid

PRODUCT_PACKAGES += \
    SoundRecorder

# add for hwc debug
PRODUCT_PROPERTY_OVERRIDES += \
    debug.hwc.showfps=0
