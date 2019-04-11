
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include "IAtwClient.h"

#include "AtwLog.h"

namespace android
{

typedef enum
{
    CREATE_SURFACE = 0,
    DESTROY_SURFACE,
    SET_HEADTRACKING_THREAD,
    SET_FRAME_POSE,
    GET_FRAME_PRESENT_TIME,
    SET_WARP_DATA,
    GET_SERVICE_TIMING_DATA
}AtwClientCommand_t;

class BpAtwClient : public BpInterface<IAtwClient>
{
public:
    BpAtwClient(const sp<IBinder>& impl)
        : BpInterface<IAtwClient>(impl) {
    }

    virtual ~BpAtwClient();

    virtual status_t createSurface(uint32_t w, uint32_t h, PixelFormat format, uint32_t flags, sp<IGraphicBufferProducer>* gbp)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAtwClient::getInterfaceDescriptor());

        data.writeUint32(w);
        data.writeUint32(h);
        data.writeInt32(static_cast<int32_t>(format));
        data.writeUint32(flags);
        remote()->transact(CREATE_SURFACE, data, &reply);

        *gbp = interface_cast<IGraphicBufferProducer>(reply.readStrongBinder());
        return reply.readInt32();
    }
    virtual status_t destroySurface()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAtwClient::getInterfaceDescriptor());
        return remote()->transact(DESTROY_SURFACE, data, &reply);
    }

    virtual status_t setHeadTrackerThread(pid_t tid)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAtwClient::getInterfaceDescriptor());
        data.write(&tid, sizeof(pid_t));
        return remote()->transact(SET_HEADTRACKING_THREAD, data, &reply);
    }

    virtual status_t setFramePose(const uint64_t frameNumber, const avrQuatf &pose, const uint64_t expectedDisplayTimeInNano)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAtwClient::getInterfaceDescriptor());
        data.writeUint64(frameNumber);
        data.writeUint64(expectedDisplayTimeInNano);
        data.write(&pose, sizeof(avrQuatf));
        return remote()->transact(SET_FRAME_POSE, data, &reply);
    }

    virtual status_t getFramePresentTime(const uint64_t frameNumber, uint64_t *actualPresentTimeInNano)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAtwClient::getInterfaceDescriptor());
        data.writeUint64(frameNumber);
        status_t result = remote()->transact(GET_FRAME_PRESENT_TIME, data, &reply);
        if(result == NO_ERROR)
        {
            uint64_t presentTimeInNano = 0;
            reply.readUint64(&presentTimeInNano);
            *actualPresentTimeInNano = presentTimeInNano;
        }
        return result;
    }

    virtual status_t setWarpData(const WarpData_t &warp)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAtwClient::getInterfaceDescriptor());
        data.write(&warp, sizeof(WarpData_t));
        return remote()->transact(SET_WARP_DATA, data, &reply);
    }

    virtual status_t getServiceTimingState(ServiceTimingData_t *timing)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAtwClient::getInterfaceDescriptor());
        status_t result = remote()->transact(GET_SERVICE_TIMING_DATA, data, &reply);
        if(result == NO_ERROR)
        {
            ServiceTimingData_t readed;
            reply.read(&readed, sizeof(ServiceTimingData_t));
            *timing = readed;
        }
        return result;
    }
};

// Out-of-line virtual method definition to trigger vtable emission in this
// translation unit (see clang warning -Wweak-vtables)
BpAtwClient::~BpAtwClient() {}

IMPLEMENT_META_INTERFACE(AtwClient, "allwinner.atw.IAtwClient");

status_t BnAtwClient::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
     switch(code) {
        case CREATE_SURFACE: {
            CHECK_INTERFACE(IAtwClient, data, reply);

            uint32_t w = data.readUint32();
            uint32_t h = data.readUint32();
            PixelFormat format = static_cast<PixelFormat>(data.readInt32());
            uint32_t flags = data.readUint32();

            sp<IGraphicBufferProducer> gbp;
            status_t result = createSurface(w, h, format, flags, &gbp);
            reply->writeStrongBinder(IInterface::asBinder(gbp));
            reply->writeInt32(result);

            return NO_ERROR;
        }
        case DESTROY_SURFACE:
        {
            CHECK_INTERFACE(IAtwClient, data, reply);
            return destroySurface();
        }
        case SET_HEADTRACKING_THREAD:
        {
            CHECK_INTERFACE(IAtwClient, data, reply);
            pid_t pid = -1;
            data.read(&pid, sizeof(pid_t));
            return setHeadTrackerThread(pid);
        }
        case SET_FRAME_POSE:
        {
            CHECK_INTERFACE(IAtwClient, data, reply);
            uint64_t frameNumber;
            uint64_t expectedDisplayTimeInNano;
            avrQuatf pose;
            data.readUint64(&frameNumber);
            data.readUint64(&expectedDisplayTimeInNano);
            data.read(&pose, sizeof(avrQuatf));
            return setFramePose(frameNumber, pose, expectedDisplayTimeInNano);
        }
        case GET_FRAME_PRESENT_TIME:
        {
            CHECK_INTERFACE(IAtwClient, data, reply);
            uint64_t frameNumber;
            data.readUint64(&frameNumber);

            uint64_t actualPresentTimeInNano;
            status_t result = getFramePresentTime(frameNumber, &actualPresentTimeInNano);
            if(result == NO_ERROR)
            {
                reply->writeUint64(actualPresentTimeInNano);
            }
            return result;
        }
        case SET_WARP_DATA:
        {
            CHECK_INTERFACE(IAtwClient, data, reply);
            WarpData_t warp;
            data.read(&warp, sizeof(WarpData_t));
            return setWarpData(warp);
        }
        case GET_SERVICE_TIMING_DATA:
        {
            CHECK_INTERFACE(IAtwClient, data, reply);
            ServiceTimingData_t timing;
            status_t result = getServiceTimingState(&timing);
            if(result == NO_ERROR)
            {
                reply->write(&timing, sizeof(ServiceTimingData_t));
            }
            return result;
        }
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

}; // namespace android
