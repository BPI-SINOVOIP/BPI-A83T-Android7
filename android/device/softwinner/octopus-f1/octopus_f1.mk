$(call inherit-product, build/target/product/full_base.mk)
$(call inherit-product, device/softwinner/octopus-common/octopus-common.mk)
$(call inherit-product-if-exists, device/softwinner/octopus-f1/modules/modules.mk)

DEVICE_PACKAGE_OVERLAYS := device/softwinner/octopus-f1/overlay \
                           $(DEVICE_PACKAGE_OVERLAYS)

PRODUCT_PACKAGES += \
    ESFileExplorer \
    VideoPlayer \
    Bluetooth \
    Launcher3
#   PartnerChromeCustomizationsProvider

PRODUCT_COPY_FILES += \
    device/softwinner/octopus-f1/kernel:kernel \
    device/softwinner/octopus-f1/fstab.sun8iw6p1:root/fstab.sun8iw6p1 \
    device/softwinner/octopus-f1/init.device.rc:root/init.device.rc \
    device/softwinner/common/verity/rsa_key/verity_key:root/verity_key \
    device/softwinner/octopus-f1/modules/modules/nand.ko:root/nand.ko \
    device/softwinner/octopus-f1/modules/modules/disp.ko:root/disp.ko \
    device/softwinner/octopus-f1/modules/modules/auto_detect.ko:recovery/root/auto_detect.ko \
#    device/softwinner/octopus-f1/modules/modules/gt9xxnew_ts.ko:recovery/root/gt9xxnew_ts.ko \

PRODUCT_COPY_FILES += \
    device/softwinner/octopus-f1/configs/camera.cfg:system/etc/camera.cfg \
    device/softwinner/octopus-f1/configs/cfg-Gallery2.xml:system/etc/cfg-Gallery2.xml \
    device/softwinner/octopus-f1/configs/gsensor.cfg:system/usr/gsensor.cfg \
    device/softwinner/octopus-f1/configs/media_profiles.xml:system/etc/media_profiles.xml \
    device/softwinner/octopus-f1/configs/sunxi-keyboard.kl:system/usr/keylayout/sunxi-keyboard.kl \
    device/softwinner/octopus-f1/configs/sunxi-ir.kl:system/usr/keylayout/sunxi-ir.kl \
    device/softwinner/octopus-f1/configs/tp.idc:system/usr/idc/tp.idc

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.xml \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:system/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/etc/permissions/android.hardware.bluetooth_le.xml \
    frameworks/native/data/etc/android.hardware.ethernet.xml:system/etc/permissions/android.hardware.ethernet.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.software.verified_boot.xml:system/etc/permissions/android.software.verified_boot.xml



PRODUCT_COPY_FILES += \
    device/softwinner/octopus-f1/configs/bluetooth/bt_vendor.conf:system/etc/bluetooth/bt_vendor.conf

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml   \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.camera.autofocus.xml:system/etc/permissions/android.hardware.camera.autofocus.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.software.managed_users.xml:system/etc/permissions/android.software.managed_users.xml

PRODUCT_COPY_FILES += \
    device/softwinner/octopus-f1/media/boot.wav:system/media/boot.wav \
    device/softwinner/octopus-f1/media/audio_conf.txt:system/media/audio_conf.txt \
    device/softwinner/octopus-f1/media/bootanimation.zip:system/media/bootanimation.zip \

PRODUCT_PROPERTY_OVERRIDES += \
   ro.frp.pst=/dev/block/by-name/frp

# usb
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.usb.config=mtp,adb \
    ro.adb.secure=1

PRODUCT_PROPERTY_OVERRIDES += \
    ro.product.first_api_level=24

PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.heapsize=512m \
    dalvik.vm.heapstartsize=8m \
    dalvik.vm.heapgrowthlimit=192m \
    dalvik.vm.heaptargetutilization=0.75 \
    dalvik.vm.heapminfree=2m \
    dalvik.vm.heapmaxfree=8m \
    ro.zygote.disable_gl_preload=true

# scense_control
PRODUCT_PROPERTY_OVERRIDES += \
    sys.p_bootcomplete= true \
    sys.p_debug=false

PRODUCT_PROPERTY_OVERRIDES += \
    ro.property.tabletUI=false \
    ro.sf.lcd_density=320

# function
PRODUCT_PROPERTY_OVERRIDES += \
    ro.spk_dul.used=true \
    ro.dmic.used=true

PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.timezone=Asia/Shanghai \
    persist.sys.country=CN \
    persist.sys.language=zh


PRODUCT_AAPT_CONFIG := normal large
PRODUCT_AAPT_PREF_CONFIG := xhdpi
# A list of dpis to select prebuilt apk, in precedence order.
PRODUCT_AAPT_PREBUILT_DPI := xxhdpi xhdpi hdpi
PRODUCT_CHARACTERISTICS := tablet

# stoarge
PRODUCT_PROPERTY_OVERRIDES += \
    persist.fw.force_adoptable=true

#$(call inherit-product-if-exists, vendor/google/products/gms-mandatory.mk)

PRODUCT_BRAND := Allwinner
PRODUCT_NAME := octopus_f1
PRODUCT_DEVICE := octopus-f1
PRODUCT_BOARD := f1
PRODUCT_MODEL := Octopus A83 F1
PRODUCT_MANUFACTURER := Allwinner
