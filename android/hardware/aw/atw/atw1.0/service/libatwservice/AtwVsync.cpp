#include "AtwVsync.h"
#include "AtwLog.h"

#include <errno.h>
#include <math.h>
#include <unistd.h>            // for usleep
#include <time.h>

//------------------------------------------------------------------------------------------------------------
void VsyncState::GetLastestVsyncState(double &vsyncNow, uint64_t &latestHwVsyncBaseInNano, uint64_t &latestHwVsyncPeriodsInNano)
{
    const HardwareVsync hwVsync = mHwVsync.GetState();

    if(hwVsync.vsyncPeriodsNano == 0)
    {
        //_LOGE("current vsync period is 0");
        vsyncNow = -1.0f;
        return;
    }

    const int64_t t = NanoTime();
    const double vsync = (double) hwVsync.vsyncCount + (double) (t - hwVsync.vsyncBaseNano) * 1.0f / hwVsync.vsyncPeriodsNano; // 目前为止，新的的 vsync 数目

    vsyncNow = vsync;
    latestHwVsyncBaseInNano = hwVsync.vsyncBaseNano;
    latestHwVsyncPeriodsInNano = hwVsync.vsyncPeriodsNano;
}

void VsyncState::UpdateVsync(const HardwareVsync& vsync)
{
    mHwVsync.SetState(vsync);
}

double VsyncState::FramePointTimeInSeconds( const double framePoint )
{
    HardwareVsync vsyncState = mHwVsync.GetState();
    const double seconds = 1e-9 * (1.0f) * (vsyncState.vsyncBaseNano + (framePoint - vsyncState.vsyncCount) * vsyncState.vsyncPeriodsNano);
    return seconds;
}