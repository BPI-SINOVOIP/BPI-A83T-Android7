#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include "IHeadTracker.h"
#include "AtwLog.h"

namespace android
{

typedef enum
{
    HEADTRACKER_BINDER_GET_ORIENTATION = IBinder::FIRST_CALL_TRANSACTION
}HeadTrackerCode_t;

class BpHeadTracker  : public BpInterface<IHeadTracker>
{
public:
    BpHeadTracker(const sp<IBinder>& impl)
        : BpInterface<IHeadTracker>(impl) {
    }
    virtual ~BpHeadTracker();

    virtual status_t getOrientation(uint64_t *timestamps, avrQuatf *poses)
    {
        status_t result = NO_ERROR;
        Parcel data, reply;
        data.writeInterfaceToken(IHeadTracker::getInterfaceDescriptor());
        data.writeUint64(timestamps[0]);
        data.writeUint64(timestamps[1]);
        result = remote()->transact(HEADTRACKER_BINDER_GET_ORIENTATION, data, &reply);
        if(result == NO_ERROR)
        {
            reply.read(&poses[0], sizeof(avrQuatf));
            reply.read(&poses[1], sizeof(avrQuatf));
        }
        return result;
    }
};

BpHeadTracker::~BpHeadTracker() {}

IMPLEMENT_META_INTERFACE(HeadTracker, "aw.atw.IHeadTracker");

status_t BnHeadTracker::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code){
        case HEADTRACKER_BINDER_GET_ORIENTATION:
        {
            CHECK_INTERFACE(IHeadTracker, data, reply);
            status_t result = NO_ERROR;

            uint64_t timestamps[2];
            data.readUint64(&timestamps[0]);
            data.readUint64(&timestamps[1]);

            avrQuatf poses[2];
            result = getOrientation(&timestamps[0], &poses[0]);
            if(result == NO_ERROR)
            {
                reply->write(&poses[0], sizeof(avrQuatf));
                reply->write(&poses[1], sizeof(avrQuatf));
            }
            return result;
        }
        default:
        {
            return BBinder::onTransact(code, data, reply, flags);
        }
    }
}

}; // namespace android