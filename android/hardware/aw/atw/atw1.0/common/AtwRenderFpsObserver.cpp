#include "AtwRenderFpsObserver.h"
#include "AtwLog.h"

namespace allwinner
{
    AtwRenderFpsObserver::AtwRenderFpsObserver(){}
    AtwRenderFpsObserver::~AtwRenderFpsObserver(){}

    void AtwRenderFpsObserver::ResetReprojectionCondition(long long vsyncPeriod, int32_t observedSeconds, int32_t maxTerribleSeconds, int32_t maxDroppedFramesPerSecond)
    {
        VSYNC_PERIOD = vsyncPeriod;
        MaxObservedPeriodInSeconds = observedSeconds;
        Threshold = maxTerribleSeconds;
        MaxDropFramesInSecond = maxDroppedFramesPerSecond;
    }

    bool AtwRenderFpsObserver::CollectTimestamp(long long now, long long sleptTimeInNano)
    {
        // 计算当前帧的drawSeconds，如果超过 VSYNC_PERIOD, 丢帧统计加1
        const long long drawSeconds = (mLastTimestampInNano == 0) ? VSYNC_PERIOD : (now-mLastTimestampInNano-sleptTimeInNano);
        mLastTimestampInNano = now;
        mDroppedFrameInLastSecond = (drawSeconds > VSYNC_PERIOD) ? (mDroppedFrameInLastSecond+1) : mDroppedFrameInLastSecond;

        // 如果是app第一秒的第一帧，则初始化这一秒的开始时间戳为 now-VSYNC_PERIOD
        mLastSecondBenginInNano = mLastSecondBenginInNano == 0 ? (now-VSYNC_PERIOD) : mLastSecondBenginInNano;
        if((now-mLastSecondBenginInNano) <= 1e9)
        { // 统计时间不到1秒
            return mReprojectionEnabled;
        }

        // 根据过去这一秒的丢帧情况，来判断它是否是 terrible second.
        mTerribleSecondsInTotal = (mDroppedFrameInLastSecond >= MaxDropFramesInSecond) ? mTerribleSecondsInTotal+1 : mTerribleSecondsInTotal;
        mLastSecondBenginInNano = now;
        mDroppedFrameInLastSecond = 0;
        mObservedSecondsInTotal++;

        // 根据过去几秒的丢帧情况
        bool enableReprojection = mReprojectionEnabled;
        if(mTerribleSecondsInTotal >= Threshold)
        {// 一旦统计到超过 Threshold 丢帧严重，立即切换为插帧模式
            _LOGV("Found %d terrible seconds in total(%d). suggest to enable reprojection", mTerribleSecondsInTotal, mObservedSecondsInTotal);
            enableReprojection = true;
            mObservedSecondsInTotal = 0;
            mTerribleSecondsInTotal = 0;
        }
        else
        {// 否则必须等到统计足够多的秒数， 即达到 MaxObservedPeriodInSeconds
            if(mObservedSecondsInTotal >= MaxObservedPeriodInSeconds)
            {
                if(mTerribleSecondsInTotal == 0)
                {// 只有0个丢帧秒才会关闭插帧模式。
                    enableReprojection = false;
                    _LOGV("Found %d nice seconds . suggest to disable reprojection", mObservedSecondsInTotal);
                }
                // else keep enableReprojection as mReprojectionEnabled.
                mObservedSecondsInTotal = 0;
                mTerribleSecondsInTotal = 0;
            }
            // else.
        }

        mReprojectionEnabled = enableReprojection;
        return mReprojectionEnabled;
    }
};