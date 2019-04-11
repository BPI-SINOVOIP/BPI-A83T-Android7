#pragma once

#include <stdint.h>
#include <sys/types.h>

#include <utils/Errors.h>
#include <utils/KeyedVector.h>

#include "AtwTypes.h"
#include "IHeadTracker.h"

namespace android
{

class HeadTracker : public BnHeadTracker
{
public:
    HeadTracker(PTRFUN_HeadTracker tracker);
    HeadTracker();
    ~HeadTracker();
    virtual status_t getOrientation(uint64_t *timestamps, avrQuatf *poses);
private:
    PTRFUN_HeadTracker mpFuncHeadTrackerImpl = nullptr;
};

}; // namespace android