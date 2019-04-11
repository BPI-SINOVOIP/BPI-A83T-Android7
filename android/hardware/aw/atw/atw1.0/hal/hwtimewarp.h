#ifndef ALLWINNER_INCLUDE_HARDWARE_HWTIMEWARP_H
#define ALLWINNER_INCLUDE_HARDWARE_HWTIMEWARP_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <hardware/hardware.h>
#include "AtwTypes.h"

__BEGIN_DECLS

#define HWTIMEWARP_HARDWARE_MODULE_ID "hwtimewarp"
#define HWTIMEWARP_HARDWARE_WARP   "warp"

typedef struct hwtimewarp_module {
    struct hw_module_t common;
} hwtimewarp_module_t;

typedef struct hwtimewarp_device_1 {
    struct hw_device_t common;

    int (*enable)(struct hwtimewarp_device_1 *dev, bool enable);

    //TODO:
    // 理论上最好是 setAtwLayer 返回一个 fenceFd， 然后在disp driver中断下半部 signal 前一帧的 fence
    // 但是现在为了简化，直接在上层就创建和处理完 fence.
    int (*setAtwLayer)(struct hwtimewarp_device_1 *dev, AtwData_t *info);
    int (*getDisplayInfo)(struct hwtimewarp_device_1 *dev, DisplayInfo_t *dispInfo);
    int (*getDisplayVsync)(struct hwtimewarp_device_1 *dev, HardwareVsync_t *hwVsync);
    int (*getDisplayTiming)(struct hwtimewarp_device_1 *dev, DisplayTconTiming_t *dispTconTiming);

    // Reserved for future use.
    void*   reserved_proc[3];
} hwtimewarp_device_1_t;

// convenience API for opening and closing a device

static inline int hwtimewarp_open_1(const struct hw_module_t* module,
        struct hwtimewarp_device_1** device) {
    return module->methods->open(module,
            HWTIMEWARP_HARDWARE_WARP, (struct hw_device_t**)device);
}

static inline int hwtimewarp_close_1(struct hwtimewarp_device_1* device) {
    return device->common.close(&device->common);
}

__END_DECLS

#endif // ALLWINNER_INCLUDE_HARDWARE_HWTIMEWARP_H 