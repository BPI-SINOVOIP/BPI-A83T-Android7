# 3G Data Card Packages
PRODUCT_PACKAGES += \
	chat \
	pppd \
	rild

# 3G Data Card Configuration Flie
PRODUCT_COPY_FILES += \
	vendor/aw/public/prebuild/lib/librild/ip-down:system/etc/ppp/ip-down \
	vendor/aw/public/prebuild/lib/librild/ip-up:system/etc/ppp/ip-up \
	vendor/aw/public/prebuild/lib/librild/3g_dongle.cfg:system/etc/3g_dongle.cfg \
	vendor/aw/public/prebuild/lib/librild/usb_modeswitch:system/bin/usb_modeswitch \
	vendor/aw/public/prebuild/lib/librild/call-pppd:system/xbin/call-pppd \
	vendor/aw/public/prebuild/lib/librild/usb_modeswitch.sh:system/xbin/usb_modeswitch.sh \
	vendor/aw/public/prebuild/lib/librild/apns-conf_sdk.xml:system/etc/apns-conf.xml \
	vendor/aw/public/prebuild/lib/librild/lib/libsoftwinner-ril-7.0.so:system/lib/libsoftwinner-ril-7.0.so\
	vendor/aw/public/prebuild/lib/librild/lib64/libsoftwinner-ril-7.0.so:system/lib64/libsoftwinner-ril-7.0.so

# 3G Data Card usb modeswitch File
PRODUCT_COPY_FILES += \
	$(call find-copy-subdir-files,*,vendor/aw/public/prebuild/lib/librild/usb_modeswitch.d,system/etc/usb_modeswitch.d)

# Radio parameter
PRODUCT_PROPERTY_OVERRIDES += \
	rild.libargs=-d/dev/ttyUSB2 \
	rild.libpath=libsoftwinner-ril-7.0.so \
	ro.sw.embeded.telephony=false
