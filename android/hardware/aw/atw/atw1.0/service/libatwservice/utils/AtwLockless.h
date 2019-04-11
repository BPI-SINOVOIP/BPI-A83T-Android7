#pragma once
#ifndef NO_STL_ATOMIC_SUPPORT
#include <atomic>
#endif

template<class T>
class LocklessUpdater
{
public:
    LocklessUpdater() {}
    ~LocklessUpdater() {}

    void SetState(T state)
    {
#ifndef NO_STL_ATOMIC_SUPPORT
        while(mLock.test_and_set(std::memory_order_acquire))// acquire lock
            ;// spin
        mData = state;
        mLock.clear(std::memory_order_release);             // release lock
#else
        mData = state;
#endif
    }
    T GetState()
    {
#ifndef NO_STL_ATOMIC_SUPPORT
        T ret;
        while(mLock.test_and_set(std::memory_order_acquire))// acquire lock
            ;// spin
        ret = mData;
        mLock.clear(std::memory_order_release);             // release lock
        return ret;
#else
        return mData;
#endif
    }
private:
#ifndef NO_STL_ATOMIC_SUPPORT
    std::atomic_flag mLock = ATOMIC_FLAG_INIT;//使用 atomic_flag 来实现 usr space 的 spin lock
                                            // A spinlock mutex can be implemented in userspace using an atomic_flag
                                            // http://en.cppreference.com/w/cpp/atomic/atomic_flag
#endif
    T mData;
};
