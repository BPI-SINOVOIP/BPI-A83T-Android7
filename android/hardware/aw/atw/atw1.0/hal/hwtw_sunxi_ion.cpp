#include "hwtw_sunxi_ion.h"
#include "AtwLog.h"

#include <hardware/hardware.h>
#include <linux/ion.h>
#include <ion/ion.h>
#include <sys/ioctl.h>

#define ION_IOC_SUNXI_PHYS_ADDR 7

typedef struct {
    ion_user_handle_t handle;
    unsigned int phys_addr;
    unsigned int size;
}sunxi_phys_data;

unsigned int ionGetAddr(int ionFd, int sharefd)
{
    int ret = -1;
    struct ion_custom_data custom_data;
    sunxi_phys_data phys_data;
    ion_handle_data freedata;
    struct ion_fd_data data;

    data.fd = sharefd;
    ret = ioctl(ionFd, ION_IOC_IMPORT, &data);
    if (ret < 0) {
        _LOGE("%s: ION_IOC_IMPORT failed(ret=%d)", __func__, ret);
        return 0;
    }
    custom_data.cmd = ION_IOC_SUNXI_PHYS_ADDR;
    phys_data.handle = data.handle;
    custom_data.arg = (unsigned long)&phys_data;
    ret = ioctl(ionFd, ION_IOC_CUSTOM, &custom_data);
    if(ret < 0) {
        _LOGE("%s: ION_IOC_CUSTOM failed(ret=%d)", __func__, ret);
        return 0;
    }
    freedata.handle = data.handle;
    ret = ioctl(ionFd, ION_IOC_FREE, &freedata);
    if(ret < 0) {
        _LOGE("%s: ION_IOC_FREE failed(ret=%d)", __func__, ret);
        return 0;
    }
    return phys_data.phys_addr;
}



