#pragma once
#include <stdint.h>

namespace allwinner{

class AtwRenderFpsObserver
{
public:
    AtwRenderFpsObserver();
    ~AtwRenderFpsObserver();

    void ResetReprojectionCondition(long long vsyncPeriod, int32_t observedSeconds, int32_t maxTerribleSeconds, int32_t maxDroppedFramesPerSecond);
    bool CollectTimestamp(long long now, long long sleptTimeInNano);

private:
    long long mLastTimestampInNano = 0;
    long long mLastSecondBenginInNano = 0;
    int32_t mDroppedFrameInLastSecond = 0;
    int32_t mTerribleSecondsInTotal = 0; // 统计周期内，有多少秒丢帧严重
    int32_t mObservedSecondsInTotal = 0; // 当前统计周期的统计秒数。

    long long VSYNC_PERIOD = 1e9 / 60;
    int32_t MaxObservedPeriodInSeconds = 5;
    int32_t Threshold = 2;
    int32_t MaxDropFramesInSecond = 4;
    bool mReprojectionEnabled = false;
};

}; // namespace allwinner