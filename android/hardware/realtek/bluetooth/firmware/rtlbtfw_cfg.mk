#
# Copyright (C) 2008 The Android Open Source Project
#

LOCAL_PATH = hardware/realtek/bluetooth/firmware

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rtl8703a_config:system/etc/firmware/rtl8703a_config \
    $(LOCAL_PATH)/rtl8703a_fw:system/etc/firmware/rtl8703a_fw \
    $(LOCAL_PATH)/rtl8703b_config:system/etc/firmware/rtl8703b_config \
    $(LOCAL_PATH)/rtl8703b_fw:system/etc/firmware/rtl8703b_fw \
    $(LOCAL_PATH)/rtl8723a_config:system/etc/firmware/rtl8723a_config \
    $(LOCAL_PATH)/rtl8723a_fw:system/etc/firmware/rtl8723a_fw \
    $(LOCAL_PATH)/rtl8723b_config:system/etc/firmware/rtl8723b_config \
    $(LOCAL_PATH)/rtl8723b_config_2Ant_S0:system/etc/firmware/rtl8723b_config_2Ant_S0 \
    $(LOCAL_PATH)/rtl8723b_fw:system/etc/firmware/rtl8723b_fw \
    $(LOCAL_PATH)/rtl8723bs_config:system/etc/firmware/rtl8723bs_config \
    $(LOCAL_PATH)/rtl8723bs_fw:system/etc/firmware/rtl8723bs_fw \
    $(LOCAL_PATH)/rtl8723bs_VQ0_config:system/etc/firmware/rtl8723bs_VQ0_config \
    $(LOCAL_PATH)/rtl8723bs_VQ0_fw:system/etc/firmware/rtl8723bs_VQ0_fw \
    $(LOCAL_PATH)/rtl8723bu_config:system/etc/firmware/rtl8723bu_config \
    $(LOCAL_PATH)/rtl8723b_VQ0_config:system/etc/firmware/rtl8723b_VQ0_config \
    $(LOCAL_PATH)/rtl8723b_VQ0_fw:system/etc/firmware/rtl8723b_VQ0_fw \
    $(LOCAL_PATH)/rtl8723cs_cg_config:system/etc/firmware/rtl8723cs_cg_config \
    $(LOCAL_PATH)/rtl8723cs_cg_fw:system/etc/firmware/rtl8723cs_cg_fw \
    $(LOCAL_PATH)/rtl8723cs_vf_config:system/etc/firmware/rtl8723cs_vf_config \
    $(LOCAL_PATH)/rtl8723cs_vf_fw:system/etc/firmware/rtl8723cs_vf_fw \
    $(LOCAL_PATH)/rtl8723cs_xx_config:system/etc/firmware/rtl8723cs_xx_config \
    $(LOCAL_PATH)/rtl8723cs_xx_fw:system/etc/firmware/rtl8723cs_xx_fw \
    $(LOCAL_PATH)/rtl8723d_config:system/etc/firmware/rtl8723d_config \
    $(LOCAL_PATH)/rtl8723d_fw:system/etc/firmware/rtl8723d_fw \
    $(LOCAL_PATH)/rtl8723ds_config:system/etc/firmware/rtl8723ds_config \
    $(LOCAL_PATH)/rtl8723ds_fw:system/etc/firmware/rtl8723ds_fw \
    $(LOCAL_PATH)/rtl8761a_config:system/etc/firmware/rtl8761a_config \
    $(LOCAL_PATH)/rtl8761a_fw:system/etc/firmware/rtl8761a_fw \
    $(LOCAL_PATH)/rtl8761at_config:system/etc/firmware/rtl8761at_config \
    $(LOCAL_PATH)/rtl8761at_fw:system/etc/firmware/rtl8761at_fw \
    $(LOCAL_PATH)/rtl8761au8192ee_fw:system/etc/firmware/rtl8761au8192ee_fw \
    $(LOCAL_PATH)/rtl8761au8812ae_fw:system/etc/firmware/rtl8761au8812ae_fw \
    $(LOCAL_PATH)/rtl8761au_fw:system/etc/firmware/rtl8761au_fw \
    $(LOCAL_PATH)/rtl8761aw8192eu_config:system/etc/firmware/rtl8761aw8192eu_config \
    $(LOCAL_PATH)/rtl8761aw8192eu_fw:system/etc/firmware/rtl8761aw8192eu_fw \
    $(LOCAL_PATH)/rtl8821a_config:system/etc/firmware/rtl8821a_config \
    $(LOCAL_PATH)/rtl8821a_fw:system/etc/firmware/rtl8821a_fw \
    $(LOCAL_PATH)/rtl8821as_config:system/etc/firmware/rtl8821as_config \
    $(LOCAL_PATH)/rtl8821as_fw:system/etc/firmware/rtl8821as_fw \
    $(LOCAL_PATH)/rtl8821c_config:system/etc/firmware/rtl8821c_config \
    $(LOCAL_PATH)/rtl8821c_fw:system/etc/firmware/rtl8821c_fw \
    $(LOCAL_PATH)/rtl8821cs_config:system/etc/firmware/rtl8821cs_config \
    $(LOCAL_PATH)/rtl8821cs_fw:system/etc/firmware/rtl8821cs_fw \
    $(LOCAL_PATH)/rtl8822b_config:system/etc/firmware/rtl8822b_config \
    $(LOCAL_PATH)/rtl8822b_fw:system/etc/firmware/rtl8822b_fw \
    $(LOCAL_PATH)/rtl8822bs_config:system/etc/firmware/rtl8822bs_config \
    $(LOCAL_PATH)/rtl8822bs_fw:system/etc/firmware/rtl8822bs_fw \
    $(TOP_DIR)device/softwinner/$(basename $(TARGET_DEVICE))/configs/bluetooth/rtkbt.conf:system/etc/bluetooth/rtkbt.conf
