#include <stdlib.h>
#include <stdio.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <hardware/hardware.h>
#include "../hwtimewarp.h"

//int main(int argc, char **argv)
int main(void)
{
    hw_module_t const* module;
    hwtimewarp_device_1_t *hwtimewarp;

    if (hw_get_module(HWTIMEWARP_HARDWARE_MODULE_ID, &module) != 0) {
        printf("cannot open hw timewarp module\n");
        return -1;
    }

    if (hwtimewarp_open_1(module, &hwtimewarp) != 0) {
        printf("cannot open hwtimewarp device\n");
        return -1;
    }


    hwtimewarp_close_1(hwtimewarp);
    return 0;
}
