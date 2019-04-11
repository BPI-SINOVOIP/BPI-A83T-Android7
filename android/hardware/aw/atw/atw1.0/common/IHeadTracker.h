#pragma once

#include <stdint.h>
#include <sys/types.h>

#include <utils/Errors.h>
#include <utils/RefBase.h>

#include <binder/IInterface.h>

#include "AtwTypes.h"

namespace android{

class IHeadTracker : public IInterface
{
public:
    DECLARE_META_INTERFACE(HeadTracker);

    virtual status_t getOrientation(uint64_t *timestamps, avrQuatf *poses) = 0;
};

class BnHeadTracker : public BnInterface<IHeadTracker> {
public:
    virtual status_t onTransact(uint32_t code, const Parcel& data,
            Parcel* reply, uint32_t flags = 0);
};

}; //namespace android