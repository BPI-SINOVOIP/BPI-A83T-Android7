#pragma once
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <map>
#include <mutex>
#include <utils/Thread.h>
#include <cutils/log.h>
#include <cutils/uevent.h>

namespace android
{

typedef struct
{
    int32_t  id;
    uint64_t timestamp;
}HwVsync_t;

class VsyncListener :  virtual public RefBase
{
public:
    VsyncListener(uint32_t id) : mThisID(id){}
    virtual ~VsyncListener(){}

    virtual void onVsync(const HwVsync_t &vsync) = 0;

    inline uint32_t getId()
    {
        return mThisID;
    }
private:
    uint32_t mThisID = -1;// 用于 register 和 unregister
};

class VsyncThread : public Thread
{
public:
    VsyncThread();
    virtual ~VsyncThread();
    virtual status_t readyToRun();

    bool registerVsyncListener(sp<VsyncListener> listener);
    bool unregisterVsyncListener(sp<VsyncListener> listener);
protected:
    bool isValidMessage(int msgLen);
    void parse_vsync_uevent(const char* msg);
private:
    virtual bool threadLoop();
private:
    int mUEventFd = -1;

    std::mutex mListenersMutex;
    std::map<uint32_t, sp<VsyncListener>> mListeners;
};

}; // namespace android