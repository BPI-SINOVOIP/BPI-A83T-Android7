#pragma once
#include <stdint.h>
#include <chrono> // for std::chrono
#include <string>

#include "AtwLog.h"


class AtwTimer
{
public:
    AtwTimer(const char* token);
    AtwTimer() = delete;
    AtwTimer& operator=(const AtwTimer&) = delete;
    AtwTimer(const AtwTimer&) = delete;
    ~AtwTimer();
private:
#if 0
    std::chrono::time_point<std::chrono::high_resolution_clock> mStart;
    std::chrono::time_point<std::chrono::high_resolution_clock> mEnd;
#else
    uint64_t mStart;
    uint64_t mEnd;
#endif
    std::string mToken;
};

#define FUNC_ENTER() _LOG("Entering %s[line=%d]", __func__, __LINE__)
#define FUNC_LEAVE() _LOG("Leaving %s[line=%d]", __func__, __LINE__)

long long       NanoTime();
double          TimeInSeconds();

// Does a nanosleep() that will wake shortly after the given targetSeconds.
// Returns the seconds that were requested to sleep, which will be <=
// the time actually slept, which may be negative if already past the
// framePoint.
float                   SleepUntilTimePoint( const double targetSeconds, const bool busyWait );