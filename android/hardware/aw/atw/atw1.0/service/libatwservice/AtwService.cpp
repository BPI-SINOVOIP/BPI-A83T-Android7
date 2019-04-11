#include <string.h>

#include <binder/IServiceManager.h>
#include <utils/Errors.h>  // for status_t
#include <utils/String8.h>

#include <sync/sync.h>
#include <linux/sw_sync.h>

#include "AtwLog.h"
#include "AtwTypes.h"
#include "AtwService.h"
#include "AtwClient.h"
#include "AtwCommitThread.h"
#include "AtwHal.h"

namespace android {

    void AtwService::instantiate()
    {
        defaultServiceManager()->addService(String16(ATW_SERVICE_STRING16), new AtwService());
    }

    AtwService::AtwService()
    {
        _LOGV("atw service create. version=%8.6f", ATW_VERSION_NUM);

        sp<AtwHal> device = new AtwHal();
        if(NO_ERROR != device->initCheck()) // 内部会创建一个 vsync thread 来监听 kernel vsync uevent 事件。
        {
            _FATAL("could not create atw hal device");
        }
        if(NO_ERROR != device->getDisplayInfo(mDispInfo))
        {
            _FATAL("could not get displayinfo from atw hal device");
        }

        sp<AtwThread> thread = new AtwThread(device, this, mDispInfo);
        if(NO_ERROR != thread->initCheck()) // use neon core
        {
            _FATAL("init check atw thread failed");
        }

        mDevice = device;
        mThread = thread;
    }

    AtwService::~AtwService()
    {
        _LOGV("AtwService Destroyed. pid=%d", getpid());
    }

    sp<AtwClient>  AtwService::getClient(const pid_t pid)
    {
        std::lock_guard<std::mutex> guard(mLock);
        sp<AtwClient> client = 0;
        auto itr = mClients.find(pid);
        if(itr != mClients.end())
        {
            client = itr->second;
        }
        _LOGV("get client(0x%p) for pid=%d", client.get(), pid);
        return client;
    }

    status_t  AtwService::createAtwConnection(pid_t pid, sp<IBinder> *conn)
    {
        _LOGV("AtwService. version=%4.2f", ATW_VERSION_NUM);

        std::lock_guard<std::mutex> guard(mLock);

        int index = mClientIndex++;

        _LOGV("createAtwConnection: %d %d", pid, index);

        sp<AtwClient> client = 0;
        auto itr = mClients.find(pid);
        if(itr != mClients.end())
        {
            // use the already created one.
            _LOGV("use existed client for %d", pid);
            client = itr->second;
        }
        else
        {
            // create a client for this process.
            client = new AtwClient(pid, index);
            _LOGV("new client for %d %d. client=%p", pid, index, client.get());
            if(client->initCheck() != NO_ERROR)
            {
                _LOGE("init check client failed");
                return -1;
            }
            mClients.insert(std::pair<pid_t, sp<AtwClient>>(pid, client)); // push it into our local side.
        }

        mThread->getMessageQueue().PostPrintf("attach %d", pid);
        *conn = IInterface::asBinder(client);
        return NO_ERROR;
    }

    status_t  AtwService::destroyAtwConnection(pid_t pid)
    {
        _LOGV("AtwService. version=%4.2f", ATW_VERSION_NUM);

        std::lock_guard<std::mutex> guard(mLock);

        _LOGV("destroyAtwConnection: %d", pid);

        sp<AtwClient> client = 0;
        auto itr = mClients.find(pid);
        if(itr == mClients.end())
        {
            _LOGV("WARNING:  try to destroy already destroyed client(%d)", pid);
            return NO_ERROR;
        }
        else
        {
            client = itr->second;
            _LOGV("destroy client {%d %d, %p} in service", client->getPid(), client->getIndex(), client.get());
            mClients.erase(itr);// clear in our local side
        }

        mThread->getMessageQueue().PostPrintf("dettach %d", pid);
        return NO_ERROR;
    }

    status_t AtwService::getDisplayInfo(int &width, int &height)
    {
        width = mDispInfo.rendertarget_w;
        height = mDispInfo.rendertarget_h;
        return NO_ERROR;
    }
} // namespace android
