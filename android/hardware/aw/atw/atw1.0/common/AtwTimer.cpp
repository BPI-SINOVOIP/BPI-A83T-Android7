#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "AtwTimer.h"

AtwTimer::AtwTimer(const char* token)
{
#if 0
    mStart = std::chrono::high_resolution_clock::now();
#else
    mStart = NanoTime();
#endif

    if(nullptr == token)
    {
        mToken = std::string("Default Timer");
    }
    else
    {
        mToken = std::string(token);
    }
}

AtwTimer::~AtwTimer()
{
#if 0
    mEnd = std::chrono::high_resolution_clock::now();
    uint64_t duration = uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(mEnd - mStart).count());
#else
    mEnd = NanoTime();
    uint64_t duration = mEnd - mStart;
#endif
    _LOG("duration = %" PRIu64 " ------ %s", duration, mToken.c_str());
}

long long       NanoTime()
{
    // This should be the same as java's system.nanoTime(), which is what the
    // choreographer vsync timestamp is based on.
    struct timespec tp;
    const int status = clock_gettime(CLOCK_MONOTONIC, &tp);
    if ( status != 0 )
    {
        _LOG( "clock_gettime status=%i", status );
    }
    const long long result = (long long)tp.tv_sec * (long long)(1000 * 1000 * 1000) + (long long)(tp.tv_nsec);
    return result;
}

double TimeInSeconds()
{
        return NanoTime() * 1e-9;
}

float SleepUntilTimePoint( const double targetSeconds, const bool busyWait )
{
    const float currentSeconds = TimeInSeconds();
    float sleepSeconds = targetSeconds - currentSeconds;
    if ( sleepSeconds > 0 )
    {
        if ( busyWait )
        {
            while( targetSeconds - TimeInSeconds() > 0 )
            {
            }
        }
        else
        {
            // I'm assuming we will never sleep more than one full second.
            timespec    t, rem;
            t.tv_sec = 0;
            t.tv_nsec = sleepSeconds * 1e9;
            nanosleep( &t, &rem );

#if DEBUG_OVER_SLEEP_PROBLEM
            const double overSleep = TimeInSeconds() - targetSeconds;
            if ( overSleep > 0.002 )// 2 ms
            {
                _LOG( "Overslept %f seconds", overSleep );
            }
#endif
        }
    }
    else
    {
#if DEBUG_OVER_SLEEP_PROBLEM
        const double overSleep = currentSeconds - targetSeconds;
        _LOG( "targetSeconds(%f) < currentSeconds(%f). overSlept=%f", targetSeconds, currentSeconds, overSleep);
        sleepSeconds = 0.0f;
#endif
    }
    return sleepSeconds;
}