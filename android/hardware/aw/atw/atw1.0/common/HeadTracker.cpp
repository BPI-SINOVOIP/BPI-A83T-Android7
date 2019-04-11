#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "HeadTracker.h"
#include "AtwLog.h"

namespace android
{

HeadTracker::HeadTracker(){}

HeadTracker::HeadTracker(PTRFUN_HeadTracker tracker)
{
    mpFuncHeadTrackerImpl = tracker;
}

HeadTracker::~HeadTracker(){}

status_t HeadTracker::getOrientation(uint64_t *timestamps, avrQuatf *poses)
{
    //_LOGV("call HeadTracker::getOrientation, PFUNC=%p, timestamp=%" PRIu64 ".", mpFuncHeadTrackerImpl, timestamp);
    if(mpFuncHeadTrackerImpl == nullptr)
    {
        return -1;
    }

    int ret = 0;
    ret = mpFuncHeadTrackerImpl(timestamps[0], poses[0]);
    ret += mpFuncHeadTrackerImpl(timestamps[1], poses[1]);
    return ret;
}

}; // namespace android
