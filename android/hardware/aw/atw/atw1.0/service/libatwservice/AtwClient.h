#pragma once

#include <mutex>

#include "IAtwClient.h"
#include "AtwTypes.h"

#include <vector>

namespace android
{

class AtwThread;
class AtwLayer;
class IHeadTracker;

typedef struct
{
    bool      isValid;
    avrQuatf  orientation;
}FramePose_t;

// 切换不同的应用的时候，我们只需要释放前一个 AtwClient, 然后新创建一个 AtwClient 即可。
class AtwClient : public BnAtwClient
{
public:
    AtwClient(pid_t pid, int idx = 0);
    virtual ~AtwClient();

    status_t initCheck();

    // IAtwClient interfaces
    virtual status_t createSurface(uint32_t w, uint32_t h, PixelFormat format, uint32_t flags, sp<IGraphicBufferProducer>* gbp);
    virtual status_t destroySurface();

    virtual status_t setHeadTrackerThread(pid_t pid);
    virtual status_t setFramePose(const uint64_t frameNumber, const avrQuatf &pose, const uint64_t expectedDisplayTimeInNano = 0);
    virtual status_t getFramePresentTime(const uint64_t frameNumber, uint64_t *actualPresentTimeInNano);
    virtual uint64_t getNextVsyncFromNow(uint64_t now, long long vsyncPeriod);

    virtual status_t setWarpData(const WarpData_t &warp);
    virtual status_t getServiceTimingState(ServiceTimingData_t *timing);

    sp<AtwLayer> getLayer();
    status_t getOrientation(uint64_t *timestamps, avrQuatf *poses);

    bool getAvailableFrameBuffer(const uint64_t presentTimeInNano, const bool discardAcquireFence, BufferItem *buffer, FramePose_t *fp);
    void UpdateVsyncState(long long vsyncBase, long long swPeriod, long long hwPeriod);

    inline pid_t getPid()
    {
        return mPid;
    }

    inline int getIndex()
    {
        return mIndex;
    }
private:
    pid_t mPid;

    typedef struct
    {
        uint64_t frameNumber;
        uint64_t expectedDisplayTimeInNano;
        avrQuatf pose;

        int count;// displayed times.
        uint64_t actualPresentTimeInNano;
    }FrameStatus_t;
    std::vector<FrameStatus_t> mFramePoses;

    BufferItem mPreviousFrame;
    uint64_t mPreviousFramePresentTimeInNano = 0;

    std::mutex mMutex;
    sp<AtwLayer> mLayer;

    int mIndex = 0;

    long long mSwPeriod = -1;
    long long mHwPeriod = -1;
    long long mVsyncBase = -1;

    WarpData_t mCachedPoses[16];
    long long mTimestamps[16] = {0};
    int mCurrentPose = 0;
    bool getNearestFromCaches(const long long tolerance, const long long timestamp, int &candidate);
};

}; // namespace android

