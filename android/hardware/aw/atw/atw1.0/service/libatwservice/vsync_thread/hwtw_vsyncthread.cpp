#include "hwtw_vsyncthread.h"
#include "AtwLog.h"

namespace android
{

const int UEVENT_MSG_LEN = 2048;

VsyncThread::VsyncThread() : Thread(false /*do not need call java*/){}

VsyncThread::~VsyncThread()
{
    if(mUEventFd > 0)
    {
        close(mUEventFd);
        mUEventFd = -1;
    }
}

status_t VsyncThread::readyToRun()
{
    int fd = uevent_open_socket(64*1024, true);
    if (fd < 0) {
        _FATAL("open uevent socket failed. errno=%s", strerror(errno));
        return -1;
    }
    mUEventFd = fd;
    return NO_ERROR;
}

void VsyncThread::parse_vsync_uevent(const char *msg)
{
    while (*msg) {
        if (!strncmp(msg, "VSYNC", strlen("VSYNC"))) {
            msg += strlen("VSYNC");
            int32_t id = *(msg) - '0';
            uint64_t timestamp = strtoull(msg + 2, NULL, 0);
            HwVsync_t vsync = {id, timestamp};

            mListenersMutex.lock();
            std::map<uint32_t, sp<VsyncListener>>::iterator it = mListeners.begin();
            while(it != mListeners.end())
            {
                sp<VsyncListener> &listener = it->second;
                listener->onVsync(vsync);
                ++it;
            }
            mListenersMutex.unlock();
        }
        while (*msg++);
    }
}

bool VsyncThread::isValidMessage(int msgLen)
{
    if(msgLen == 0 || msgLen >= UEVENT_MSG_LEN)
    {
        return false;
    }
    return true;
}

bool VsyncThread::threadLoop()
{
    char msg[UEVENT_MSG_LEN + 2];

    int recvlen = uevent_kernel_multicast_recv(mUEventFd, msg, UEVENT_MSG_LEN);
    if(isValidMessage(recvlen))
    {
        msg[recvlen]   = 0;
        msg[recvlen+1] = 0;
        parse_vsync_uevent(msg);
    }
    return true;
}

bool VsyncThread::registerVsyncListener(sp<VsyncListener> listener)
{
    auto thisListenerID = listener->getId();
    if(mListeners.find(thisListenerID) != mListeners.end())
    {
        return false;
    }

    mListenersMutex.lock();
    mListeners.insert( std::pair<uint32_t,sp<VsyncListener>>(thisListenerID,listener) );
    mListenersMutex.unlock();
    return true;
}

bool VsyncThread::unregisterVsyncListener(sp<VsyncListener> listener)
{
    auto thisListenerID = listener->getId();
    std::map<uint32_t,sp<VsyncListener>>::iterator it = mListeners.find(thisListenerID);
    if(it != mListeners.end())
    {
        mListenersMutex.lock();
        mListeners.erase(it);
        mListenersMutex.unlock();
        return true;
    }
    return false;
}

}; // namespace android