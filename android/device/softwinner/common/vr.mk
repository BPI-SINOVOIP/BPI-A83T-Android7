# vr product config
PRODUCT_PACKAGES += \
    VrSettings\
    VrLauncher \
    VrVideoPlayer \
    libvrdeskcontroll \
    VrDeskController \
    VrModeSelector \
    StartupGuide \
    PSensorService \
    ControllerService

PRODUCT_PROPERTY_OVERRIDES += \
    ro.sys.vr.build=1 \
    persist.vr.enable=1
