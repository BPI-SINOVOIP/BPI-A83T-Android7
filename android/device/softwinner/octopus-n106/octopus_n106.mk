$(call inherit-product, build/target/product/full_base.mk)
$(call inherit-product, device/softwinner/octopus-common/octopus-common.mk)
$(call inherit-product-if-exists, device/softwinner/octopus-n106/modules/modules.mk)

DEVICE_PACKAGE_OVERLAYS := device/softwinner/octopus-n106/overlay \
                           $(DEVICE_PACKAGE_OVERLAYS)

PRODUCT_PACKAGES += \
    ESFileExplorer \
    VideoPlayer \
    Bluetooth \
    Launcher3
#   PartnerChromeCustomizationsProvider

PRODUCT_PACKAGES += \
    auto_detect

# dm-verity relative
$(call inherit-product, build/target/product/verity.mk)
# PRODUCT_SUPPORTS_BOOT_SIGNER must be false,otherwise error will be find when boota check boot partition
PRODUCT_SUPPORTS_BOOT_SIGNER := false
#PRODUCT_SUPPORTS_VERITY_FEC := false
PRODUCT_SYSTEM_VERITY_PARTITION := /dev/block/by-name/system

# copy init.xx.rc file for realtek wifi/bt module.
# BPI-M3 WIFI
PRODUCT_COPY_FILES += \
    device/softwinner/common/init.wireless.broadcom.rc:root/init.wireless.broadcom.rc


PRODUCT_COPY_FILES += \
    device/softwinner/octopus-n106/kernel:kernel \
    device/softwinner/octopus-n106/fstab.sun8iw6p1:root/fstab.sun8iw6p1 \
    device/softwinner/octopus-n106/init.device.rc:root/init.device.rc \
    device/softwinner/octopus-n106/modules/modules/nand.ko:root/nand.ko \
    device/softwinner/octopus-n106/modules/modules/disp.ko:root/disp.ko \
    device/softwinner/octopus-n106/modules/modules/auto_detect.ko:recovery/root/auto_detect.ko \
    device/softwinner/octopus-n106/modules/modules/gslX680new.ko:recovery/root/gslX680new.ko \


#BPI-M3 Porting 
   PRODUCT_COPY_FILES += \
	device/softwinner/octopus-n106/configs/bluetooth/bt_vendor.conf:system/etc/bluetooth/bt_vendor.conf

PRODUCT_COPY_FILES += \
    device/softwinner/octopus-n106/configs/camera.cfg:system/etc/camera.cfg \
    device/softwinner/octopus-n106/configs/cfg-Gallery2.xml:system/etc/cfg-Gallery2.xml \
    device/softwinner/octopus-n106/configs/gsensor.cfg:system/usr/gsensor.cfg \
    device/softwinner/octopus-n106/configs/media_profiles.xml:system/etc/media_profiles.xml \
    device/softwinner/octopus-n106/configs/sunxi-keyboard.kl:system/usr/keylayout/sunxi-keyboard.kl \
    device/softwinner/octopus-n106/configs/sunxi-ir.kl:system/usr/keylayout/sunxi-ir.kl \
    device/softwinner/octopus-n106/configs/tp.idc:system/usr/idc/tp.idc \
    device/softwinner/octopus-n106/configs/wifi_efuse_8723bs-vq0.map:system/etc/wifi/wifi_efuse_8723bs-vq0.map

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.xml \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:system/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/etc/permissions/android.hardware.bluetooth_le.xml \
    frameworks/native/data/etc/android.hardware.ethernet.xml:system/etc/permissions/android.hardware.ethernet.xml \
    frameworks/native/data/etc/android.software.verified_boot.xml:system/etc/permissions/android.software.verified_boot.xml


PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml   \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.camera.autofocus.xml:system/etc/permissions/android.hardware.camera.autofocus.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.software.managed_users.xml:system/etc/permissions/android.software.managed_users.xml

PRODUCT_COPY_FILES += \
    device/softwinner/octopus-n106/media/bootanimation.zip:system/media/bootanimation.zip \
    #device/softwinner/octopus-n106/media/audio_conf.txt:system/media/audio_conf.txt \
    #device/softwinner/octopus-n106/media/boot.wav:system/media/boot.wav

PRODUCT_PROPERTY_OVERRIDES += \
   ro.frp.pst=/dev/block/by-name/frp

# usb
 #Justin 20190419 Porting Start
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.usb.config=mtp,adb\
    ro.adb.secure=0
#Justin 20190419 Porting End

PRODUCT_PROPERTY_OVERRIDES += \
    ro.product.first_api_level=25

PRODUCT_PROPERTY_OVERRIDES += \
    pm.dexopt.boot=verify-at-runtime \
    pm.dexopt.install=speed \
    dalvik.vm.heapsize=512m \
    dalvik.vm.heapstartsize=8m \
    dalvik.vm.heapgrowthlimit=192m \
    dalvik.vm.heaptargetutilization=0.75 \
    dalvik.vm.heapminfree=2m \
    dalvik.vm.heapmaxfree=8m \
    ro.zygote.disable_gl_preload=true

PRODUCT_PROPERTY_OVERRIDES += \
    ro.property.tabletUI=false \
    ro.sf.lcd_density=213

#scense control
PRODUCT_PROPERTY_OVERRIDES += \
    sys.p_home=true \
    sys.p_normal=true \
    sys.p_music=true


# function
PRODUCT_PROPERTY_OVERRIDES += \
    ro.spk_dul.used=true \
    ro.dmic.used=false

# Justin Porting 20190419 Start 
PRODUCT_PROPERTY_OVERRIDES += \
   	persist.sys.timezone=Asia/Taipei \
	persist.sys.language=EN \
	persist.sys.country=US
# Justin Porting 20190419 End

PRODUCT_AAPT_CONFIG := xlarge large
PRODUCT_AAPT_PREF_CONFIG := tvdpi
# A list of dpis to select prebuilt apk, in precedence order.
PRODUCT_AAPT_PREBUILT_DPI := hdpi tvdpi mdpi
PRODUCT_CHARACTERISTICS := tablet

# stoarge
PRODUCT_PROPERTY_OVERRIDES += \
    persist.fw.force_adoptable=true

# for ota
PRODUCT_PROPERTY_OVERRIDES += \
    ro.build.version.ota=7.0.0 \
    ro.sys.ota.license=1a2fafe7456caca93ab41e3eea0de13076ff0af5ae83605a084ede7ea3e720365d6bc62bfd984fbd

$(call inherit-product-if-exists, vendor/google/products/gms-mandatory.mk)

# Radio Packages and Configuration Flie
$(call inherit-product, vendor/aw/public/prebuild/lib/librild/radio_common.mk)

PRODUCT_BRAND := Allwinner
PRODUCT_NAME := octopus_n106
PRODUCT_DEVICE := octopus-n106
# PRODUCT_BOARD must equals the board name in kernel
PRODUCT_BOARD := n106
PRODUCT_MODEL := Octopus A83 N106
PRODUCT_MANUFACTURER := Allwinner
