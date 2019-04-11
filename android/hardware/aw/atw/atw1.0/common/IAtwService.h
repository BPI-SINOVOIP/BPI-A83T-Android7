#pragma once

#include <utils/Errors.h>  // for status_t
#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>

#include <vector>

#define ATW_SERVICE_STRING16 ("softwinner.atw")

namespace android {

class IAtwService: public IInterface
{
public:

    DECLARE_META_INTERFACE(AtwService);

    virtual status_t     createAtwConnection(pid_t pid, sp<IBinder> *connection) = 0;
    virtual status_t     destroyAtwConnection(pid_t pid) = 0;
    virtual status_t     getDisplayInfo(int &width, int &height) = 0;
};

// ----------------------------------------------------------------------------

class BnAtwService: public BnInterface<IAtwService>
{
public:
    virtual status_t    onTransact( uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags = 0);
};

}; // namespace android
