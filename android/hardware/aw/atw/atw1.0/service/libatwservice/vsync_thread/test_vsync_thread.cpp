#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#include "hwtw_vsyncthread.h"

using namespace android;

// TODO:
// 1. 是否有必要让应用 app 获得 vsync 信息 ?
// 2. 非常有必要让service 获得 display timing info && vsync 信息

class MyVsyncListener : public VsyncListener
{
public:
    MyVsyncListener(int id) : VsyncListener(id){}
    virtual ~MyVsyncListener() {}

    virtual void onVsync(const HwVsync_t &vsync)
    {
        printf("on vsync = {%d, %" PRIu64 "}\n", vsync.id, vsync.timestamp);
    }
};

int main(void)
{
    printf("test vsync uevent recv thread \n");

    sp<MyVsyncListener> callback = new MyVsyncListener(0);

    sp<VsyncThread> thread = new VsyncThread();

    printf("run atw vsync thread without call back \n");
    thread->run("atw-vsync", PRIORITY_URGENT_DISPLAY);

    printf("sleep 10 seconds . should no vsync msg \n");
    usleep(10*1000*1000);

    printf("registerVsyncListener . should get vsync msg \n");
    thread->registerVsyncListener(callback);
    usleep(1000*1000*10);

    printf("unregisterVsyncListener .\n");
    thread->unregisterVsyncListener(callback);

    printf("sleep 10 seconds. should no vsync msg \n");
    usleep(1000*1000*10);

    status_t status = thread->requestExitAndWait();
    if(status != NO_ERROR)
    {
        printf("request and wait vsync thread exit return status=%x \n", status);
    }
    else
    {
        printf("request and wait vsync thread success \n");
    }

    return 0;
}