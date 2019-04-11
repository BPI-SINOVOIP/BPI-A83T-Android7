#ifndef _UI_FAKE_INPUT_DISPATCHER_H
#define _UI_FAKE_INPUT_DISPATCHER_H

#include <input/Input.h>
#include <input/InputTransport.h>
#include <utils/KeyedVector.h>
#include <utils/Vector.h>
#include <utils/threads.h>
#include <utils/Timers.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/Looper.h>
#include <utils/BitSet.h>
#include <cutils/atomic.h>

#include <stddef.h>
#include <unistd.h>
#include <limits.h>

#include "InputWindow.h"
#include "InputApplication.h"
#include "InputListener.h"
#include "InputDispatcher.h"
#include "headtracking.h"

namespace android {

extern uint64_t GetTicksNanos();

class FakeInputDispatcher {

public:
    ~FakeInputDispatcher();

    static FakeInputDispatcher* getInstance() {
        if (NULL == mFakeInputDispatcher) {
            mFakeInputDispatcher = new FakeInputDispatcher();
        }
        return mFakeInputDispatcher;
    }
    void notifyMotion(NotifyMotionArgs* args);
    bool notifyKey(NotifyKeyArgs* args, InputListenerInterface* listener);

public:
    FakeInputDispatcher();
    static FakeInputDispatcher* mFakeInputDispatcher;

private:
    float mPreOrigX[MAX_POINTERS];
    float mPreOrigY[MAX_POINTERS];
    float mPreDestX;
    float mPreDestY;
};
}
#endif // _UI_FAKE_INPUT_DISPATCHER_H
