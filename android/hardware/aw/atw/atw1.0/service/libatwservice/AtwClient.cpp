#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <utils/Trace.h>

#include <stdint.h>
#include <sys/types.h>

#include <binder/IPCThreadState.h>
#include <utils/Trace.h>

#include "AtwClient.h"
#include "AtwLayer.h"
#include "IHeadTracker.h"
#include "AtwLog.h"
#include "AtwTimer.h"

namespace android
{

const static int max_array_nums = 64;

AtwClient::AtwClient(pid_t pid, int idx) : mPid(pid), mIndex(idx)
{
    mFramePoses.reserve(max_array_nums);
    mFramePoses.resize(max_array_nums);
}

AtwClient::~AtwClient()
{
    _LOGV("#### destroy client {pid,idx}={%d,%d}", mPid, mIndex);
    mLayer = 0;
}

status_t AtwClient::initCheck()
{
    return NO_ERROR;
}

sp<AtwLayer> AtwClient::getLayer()
{
    std::lock_guard<std::mutex> guard(mMutex);
    return mLayer;
}

bool AtwClient::getNearestFromCaches(const long long tolerance, const long long timestamp, int &candidate)
{
    bool keptInTolerance = false;
    int found = -1;
    long long minDst = 0;
    int tail = mCurrentPose % 16;
    for(int i=0; i<16; i++)
    {
        int idx = (tail-i+16)%16;
        if(mTimestamps[idx] == 0)
        {
            break;
        }

        long long dst = llabs(mTimestamps[idx] - timestamp);
        if(dst > minDst && minDst != 0)
        {
            if(tolerance >= minDst)
            {
                keptInTolerance = true;
            }
            break;
        }
        // else
        found = idx;
        minDst = dst;
    }
    candidate = found;
    return keptInTolerance;
}

// 基于角度变化速率的插值
// 优点: 原地插值， 不要求 timestamp 必须在 range 之内
// 缺点: 计算复杂。
static avrQuatf interpolationInPlace(long long timestamp, const WarpData_t &range)
{
    allwinner::Quatf q0 = range.poses[0];
    allwinner::Quatf q1 = range.poses[1];
    //1. 计算 q0,q1 的各轴角度
    float x[2];
    float y[2];
    float z[2];
    q0.GetEulerAngles<allwinner::Axis_X, allwinner::Axis_Y, allwinner::Axis_Z>(&x[0],&y[0],&z[0]);
    q1.GetEulerAngles<allwinner::Axis_X, allwinner::Axis_Y, allwinner::Axis_Z>(&x[1],&y[1],&z[1]);

    //2. 计算 poses 的平均角度变化速率
    float duration = (range.timestamps[1] - range.timestamps[0]) * 1.0f * 1e-6; // 转为毫秒 ms
    float vx = (x[1]-x[0]) / duration;
    float vy = (y[1]-y[0]) / duration;
    float vz = (z[1]-z[0]) / duration;

    //3. 中点的角度。
    long long mid = (range.timestamps[0] + range.timestamps[1]) / 2;
    float mx = (x[0]+x[1]) / 2;
    float my = (y[0]+y[1]) / 2;
    float mz = (z[0]+z[1]) / 2;

    //4. 用中点和平均角速度变化速率插值得到 timestamp 的各轴角度
    float dt = (mid-timestamp) * 1.0f * 1e-6;
    float dx = mx + dt * vx;
    float dy = my + dt * vy;
    float dz = mz + dt * vz;

    //5. 将dx,dy,dz转换为四元数返回。
    allwinner::Quatf xx = allwinner::Quatf(allwinner::Axis_X, dx);
    allwinner::Quatf yy = allwinner::Quatf(allwinner::Axis_Y, dy);
    allwinner::Quatf zz = allwinner::Quatf(allwinner::Axis_Z, dz);
    return xx*yy*zz;
}

// 简单的四元数线性插值
// 优点: 计算简单
// 缺点:
// 要求 assert(timestamp >= range.timestamps[0] && timestamp <= range.timestamps[1])
// 并且理论上可能出现插值的输入朝向是很旧的朝向, 导致插值结果不符合实际。
static avrQuatf interpolationNotInPlace(long long timestamp, const WarpData_t &range)
{
    if(!(timestamp >= range.timestamps[0] && timestamp <= range.timestamps[1]))
    {
        return avrQuatf();
    }
    allwinner::Quatf q0 = range.poses[0];
    allwinner::Quatf q1 = range.poses[1];
    float t = (timestamp - range.timestamps[0]) *1.0f / (range.timestamps[1] - range.timestamps[0]);
    return q0 * (1-t) + q0 * t; // if timestamp == range.timestamps[0], we have t == 0, then just return range.poses[0]
}

status_t AtwClient::getOrientation(uint64_t *timestamps, avrQuatf *poses)
{
    ATRACE_CALL();

    // 1. 寻找最近匹配的 WarpData_t ，并判断是否在允许的误差范围内
    // 2. 线性插值: setPoses 会不停地输入最新的朝向，然后更新内部client状态，用线性插值算法来处理超过误差范围的朝向。
    std::lock_guard<std::mutex> guard(mMutex);
    const long long tolerance = 1e6*2.5f; // 2.5ms
    const long long mid = (timestamps[0] + timestamps[1]) / 2;
    int candidate = -1;
    bool keptInTolerance = getNearestFromCaches(tolerance, mid, candidate);// 从 mTimestamps[16] 中找到最接近 mid 的那一项的索引，作为候选。
    if(candidate == -1)
    {
        return -1;// 应用端还没传入任意一组朝向数据
    }
    else
    {
        if(keptInTolerance == false)
        {
            poses[0] = interpolationInPlace(timestamps[0], mCachedPoses[candidate]);
            poses[1] = interpolationInPlace(timestamps[1], mCachedPoses[candidate]);
        }
        else
        {
            WarpData_t &nearestSample = mCachedPoses[candidate];
            poses[0] = nearestSample.poses[0];
            poses[1] = nearestSample.poses[1];
        }
    }
    return NO_ERROR;
}

status_t AtwClient::setHeadTrackerThread(pid_t pid)
{
    _LOGV("set client headtracker thread=%d", pid);
    return NO_ERROR;
}

status_t AtwClient::createSurface(uint32_t w, uint32_t h, PixelFormat format, uint32_t flags, sp<IGraphicBufferProducer>* gbp)
{
    std::lock_guard<std::mutex> guard(mMutex);
    _LOGV("create atw surface w=%d, h=%d, format=%d, flags=%d. cur_layer=%p", w, h, format, flags, mLayer.get());
    sp<AtwLayer> layer = new AtwLayer(mPid);
    layer->setBuffers(w,h,format);
    *gbp = layer->getProducer();
    mLayer = layer;
    return NO_ERROR;
}

status_t AtwClient::destroySurface()
{
    return NO_ERROR;
}

status_t AtwClient::setFramePose(const uint64_t frameNumber, const avrQuatf &pose, const uint64_t expectedDisplayTimeInNano)
{
    //_LOGV("set frameNumber=%" PRIu64 " in client caches.", frameNumber);

    std::lock_guard<std::mutex> guard(mMutex);
    int idx = frameNumber % max_array_nums;
    FrameStatus_t fp = {frameNumber, expectedDisplayTimeInNano, pose, 0, 0};
    mFramePoses[idx] = fp;
    return NO_ERROR;
}

uint64_t AtwClient::getNextVsyncFromNow(uint64_t now, long long vsyncPeriod){
    if(mPreviousFramePresentTimeInNano == 0){
        return 0;
    }
    uint64_t nextVsyncFromNow = mPreviousFramePresentTimeInNano;
    while(true){
        if(nextVsyncFromNow >= now){
            return nextVsyncFromNow;
        }else{
            nextVsyncFromNow += vsyncPeriod;
        }
    }
}

status_t AtwClient::getFramePresentTime(const uint64_t frameNumber, uint64_t *actualPresentTimeInNano)
{
    ATRACE_CALL();
    (void) frameNumber;
    std::lock_guard<std::mutex> guard(mMutex);

    if(mPreviousFramePresentTimeInNano == 0)
    {
        *actualPresentTimeInNano = 0;
        return NO_ERROR;
    }

    // 做帧率控制。
    const float acquireBufferOffsetRatio = 0.25f;
    long long curHwPeriod = mHwPeriod; // 当前 service commit fps
    long long curSwPeriod = mSwPeriod; // 允许的 app 最大 render fps

    // 三种情况:
    // 1. curHwPeriod == curSwPeriod 或者 llabs(curHwPeriod-curSwPeriod) <= (1e6*2)
    //      要求应用渲染速率等于显示刷新率
    // 2. curHwPeriod < curSwPeriod 并且 llabs(curHwPeriod-curSwPeriod) > (1e6*2)
    //      要求应用渲染速率低于显示刷新率
    // 3. curHwPeriod > curSwPeriod 并且 llabs(curHwPeriod-curSwPeriod) > (1e6*2)
    //      要求应用渲染速率高于显示刷新率
    if(curSwPeriod == curHwPeriod || llabs(curSwPeriod - curHwPeriod) < 1e6)
    {
        // 显示帧率 == 渲染帧率
        uint64_t now = NanoTime();
        uint64_t nextVsyncTimeInNano = getNextVsyncFromNow(now, curHwPeriod);
        uint64_t nextAcquireBufferTimeInNano = nextVsyncTimeInNano - curHwPeriod*acquireBufferOffsetRatio;
        if(now < nextAcquireBufferTimeInNano)
        {
            *actualPresentTimeInNano = nextAcquireBufferTimeInNano;
        }
        else
        {
            *actualPresentTimeInNano = nextAcquireBufferTimeInNano + curHwPeriod;
        }
    }
    else // curSwPeriod != curHwPeriod
    {
        // 显示帧率 != 渲染帧率
        if(curHwPeriod < curSwPeriod)
        {
            // 插帧
            curSwPeriod = 2*curHwPeriod; // 目前只处理 30->60 之类的双倍插帧处理
            *actualPresentTimeInNano = getNextVsyncFromNow(NanoTime(), curSwPeriod) - curHwPeriod*acquireBufferOffsetRatio; // 让 app 按照 2*hwPeriod 来同步。
        }
        else
        {
            // 调试需求，例如希望应用以 70hz 进行渲染，但是实际仍然以 60hz 进行显示。
            // 问题: 这里无法实现应用 70hz 渲染， 因为释放的速率是 60hz.
            *actualPresentTimeInNano = getNextVsyncFromNow(NanoTime(), curSwPeriod); // 让 app render 按照 swPeriod 来同步。
        }
    }
    return NO_ERROR;
}

bool AtwClient::getAvailableFrameBuffer(const uint64_t presentTimeInNano, const bool discardAcquireFence, BufferItem *buffer, FramePose_t *fp)
{
    ATRACE_CALL();

    std::lock_guard<std::mutex> guard(mMutex);
    if(mLayer == 0)
    {
        return false;
    }

    BufferItem bufferFound;
    avrQuatf matchedOrientation;
    bool isValidOrientation = false;
    if(true == mLayer->getLatestAvailableBuffer(&bufferFound, discardAcquireFence))
    {
    // we have got a new frame.
        auto frameNumber = bufferFound.mFrameNumber;
        auto idx = frameNumber % max_array_nums;
        isValidOrientation = (mFramePoses[idx].frameNumber == frameNumber);
        matchedOrientation = isValidOrientation ? mFramePoses[idx].pose : matchedOrientation;

        // return current frame and update previous frame.
        *buffer = bufferFound;
        mPreviousFrame = bufferFound;
    }
    else
    {
    // try to use previous one
        if( mPreviousFrame.mSlot != BufferItem::INVALID_BUFFER_SLOT )
        {
        // previous frame is valid
            auto frameNumber = mPreviousFrame.mFrameNumber;
            auto idx = frameNumber % max_array_nums;
            isValidOrientation = (mFramePoses[idx].frameNumber == frameNumber);
            matchedOrientation = isValidOrientation ? mFramePoses[idx].pose : matchedOrientation;

            // return previous frame
            *buffer = mPreviousFrame;
        }
        else
        {
        // there is neither new frame nor previous frame.
            return false;
        }
    }

    *fp = {isValidOrientation, matchedOrientation};
    mPreviousFramePresentTimeInNano = presentTimeInNano;
    return true;
}

status_t AtwClient::setWarpData(const WarpData_t &warp)
{
    ATRACE_CALL();
    std::lock_guard<std::mutex> guard(mMutex);
    int idx = (mCurrentPose+1)%16;
    mCachedPoses[idx] = warp;
    mTimestamps[idx] = (warp.timestamps[0] + warp.timestamps[1]) / 2; // 用于快速比较是否是匹配的朝向。
    mCurrentPose++;
    return NO_ERROR;
}

status_t AtwClient::getServiceTimingState(ServiceTimingData_t *timing)
{
    ATRACE_CALL();
    std::lock_guard<std::mutex> guard(mMutex);

    ServiceTimingData_t data = {mVsyncBase, mHwPeriod}; // 传递 mHwPeriod 给 app procces， 该 timing 用于控制 app 端的头部跟踪数据发送的 timing.
    *timing = data;
    return NO_ERROR;
}

void AtwClient::UpdateVsyncState(long long vsyncBase, long long swPeriod, long long hwPeriod)
{
    ATRACE_CALL();
    std::lock_guard<std::mutex> guard(mMutex);

    mVsyncBase = vsyncBase;
    mSwPeriod = swPeriod;
    mHwPeriod = hwPeriod;
}
};
