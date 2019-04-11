#include <memory.h>
#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <binder/IMemory.h>

#include "IAtwService.h"
#include "AtwLog.h"

#include <utils/Errors.h>  // for status_t

namespace android {

    typedef enum AtwServiceCode {
        ATWSERVICE_BINDER_CREATE_CONNECTION = IBinder::FIRST_CALL_TRANSACTION,
        ATWSERVICE_BINDER_DESTROY_CONNECTION,
        ATWSERVICE_BINDER_GET_DISPLAY_INFO
    }AtwServiceCode_t;

// client 端能够拿到的 IAtwService
class BpAtwService: public BpInterface<IAtwService>
{
public:
    BpAtwService(const sp<IBinder>& impl)
        : BpInterface<IAtwService>(impl)
    {
    }

    virtual status_t     createAtwConnection(pid_t pid, sp<IBinder> *connection)
    {
        ENTER_FUNC();
        Parcel data, reply;
        data.writeInterfaceToken(IAtwService::getInterfaceDescriptor());
        data.write(&pid, sizeof(pid_t));
        int result = remote()->transact(ATWSERVICE_BINDER_CREATE_CONNECTION, data, &reply);
        if(result != NO_ERROR)
        {
            _LOGV("create atw connection failed");
            return -1;
        }

        sp<IBinder> b = reply.readStrongBinder();
        if(b == 0)
        {
            _LOGV("atw service return null connection to us");
            return -1;
        }

        *connection = b;
        return NO_ERROR;
    }
    virtual status_t     destroyAtwConnection(pid_t pid)
    {
        ENTER_FUNC();

        Parcel data, reply;
        data.writeInterfaceToken(IAtwService::getInterfaceDescriptor());
        data.write(&pid, sizeof(pid_t));
        int result = remote()->transact(ATWSERVICE_BINDER_DESTROY_CONNECTION, data, &reply);
        if(result != NO_ERROR)
        {
            _LOGV("destroy atw connection from service failed");
        }
        return result;
    }

    virtual status_t     getDisplayInfo(int &width, int &height)
    {
        ENTER_FUNC();

        Parcel data, reply;
        data.writeInterfaceToken(IAtwService::getInterfaceDescriptor());
        int result = remote()->transact(ATWSERVICE_BINDER_GET_DISPLAY_INFO, data, &reply);
        if(result == NO_ERROR)
        {
            reply.read(&width, sizeof(int));
            reply.read(&height, sizeof(int));
        }
        return result;
    }
};

IMPLEMENT_META_INTERFACE(AtwService, "com.softwinner.IAtwService");
// ----------------------------------------------------------------------

status_t BnAtwService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code)
    {
        case ATWSERVICE_BINDER_CREATE_CONNECTION:
        {
            CHECK_INTERFACE(IAtwService, data, reply);
            sp<IBinder> conn = 0;
            pid_t pid = 0;
            data.read(&pid, sizeof(pid_t));
            status_t result = createAtwConnection(pid, &conn);
            if(result != NO_ERROR)
            {
                _LOGV("create atw connection failed in server. err=%d", result);
                return -1;
            }
            if(conn == 0)
            {
                _LOGV("service return null conn to us");
                return -1;
            }
            reply->writeStrongBinder(conn);
            return NO_ERROR;
        }
        case ATWSERVICE_BINDER_DESTROY_CONNECTION:
        {
            CHECK_INTERFACE(IAtwService, data, reply);
            pid_t pid = 0;
            data.read(&pid, sizeof(pid_t));
            return destroyAtwConnection(pid);
        }
        case ATWSERVICE_BINDER_GET_DISPLAY_INFO:
        {
            CHECK_INTERFACE(IAtwService, data, reply);
            int width;
            int height;
            status_t result = getDisplayInfo(width, height);
            if(result == NO_ERROR)
            {
                reply->write(&width, sizeof(int));
                reply->write(&height, sizeof(int));
            }
            return result;
        }
        default:
        {
            _LOGV("BnAtwService[pid-%d] receive code(%d)", getpid(), code);
            return BBinder::onTransact(code, data, reply, flags);
        }
    }
    return NO_ERROR;
}

// ----------------------------------------------------------------------------

}; // namespace android
