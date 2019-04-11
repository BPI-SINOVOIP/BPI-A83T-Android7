#pragma once

#include <utils/Log.h>
#include <utils/Errors.h>

#include <hardware/hardware.h>

#include <map>
#include <mutex>

#include "AtwTypes.h"
#include "IAtwService.h"

namespace android {

// AtwService(BinderThread), AtwThread(CommitThread), AtwHal(VsyncThread)
// solution1: pause, modify, restart.
// solution2: message queue.

class AtwHal;
class AtwThread;
class AtwClient;
class AtwService : public BnAtwService
{
public:
    static  void     instantiate();

    virtual status_t     createAtwConnection(pid_t pid, sp<IBinder> *connection);
    virtual status_t     destroyAtwConnection(pid_t pid);

    virtual status_t     getDisplayInfo(int &width, int &height);

protected:
    sp<AtwClient>       getClient(const pid_t pid);

private:
                                        AtwService();
    virtual                             ~AtwService();

private:
    sp<AtwHal>                      mDevice;
    sp<AtwThread>                   mThread;

    DisplayInfo_t                   mDispInfo;

    int                             mClientIndex = 0; // used for debug only.

    std::mutex                      mLock;
    std::map<pid_t, sp<AtwClient>>  mClients;
    pid_t                           mClientPid; // store the lasted one.

    friend class AtwThread;
};

// ----------------------------------------------------------------------------

}; // namespace android
