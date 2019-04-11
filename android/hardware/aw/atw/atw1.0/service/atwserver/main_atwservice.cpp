// System headers required for setgroups, etc.
#include <sys/types.h>
#include <unistd.h>
#include <grp.h> 

#include <sys/resource.h>
#include <sched.h>
#include <cutils/sched_policy.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>

#include "AtwService.h"

using namespace android;
int main(/*int argc, char** argv*/)
{
    ALOGE("begin start service atw");
    setpriority(PRIO_PROCESS, 0, PRIORITY_URGENT_DISPLAY);
    set_sched_policy(0, SP_VR_DAEMON);

    struct sched_param param = {0};
    param.sched_priority = 2;
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        ALOGE("Couldn't set SCHED_FIFO");
    }
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();

    AtwService::instantiate();
    IPCThreadState::self()->joinThreadPool();
}
