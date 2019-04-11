package com.softwinner.vr.psensor.service.state;

import android.content.Context;
import android.util.Log;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import com.softwinner.vr.psensor.service.Utils;

import java.lang.Math;
import java.lang.System;
import java.util.List;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Random;

/**
 * StateMachine: state
 *
 * @author humingming@allwinnertech.com
 */
public class StateMachine {

    private static final String TAG = Utils.TAG;
    private static final boolean DEBUG = Utils.DEBUG;

    // we put all bullet in a message queue, so one bullet handle once
    private static final int MSG_RUN = 100;

    // singleton
    private volatile static StateMachine sInstance;

    private State mCurrentState;
    private List<State> mStateList;

    private Handler mUIHandler;

    public static StateMachine get() {
        // double check lock
        if (null == sInstance) {
            synchronized(StateMachine.class) {
                if (null == sInstance) {
                    sInstance = new StateMachine();
                }
            }
        }
        return sInstance;
    }

    private StateMachine() {
        mStateList = new ArrayList<State>();
        initHandler();
        reset();
    }

    private void initHandler() {
        mUIHandler = new Handler(Looper.getMainLooper(), new Handler.Callback() {
            @Override
            public boolean handleMessage(Message msg) {
                switch (msg.what) {
                    case MSG_RUN:
                        handleRun((Bullet)msg.obj);
                        return true;
                    default:
                        return false;
                }
            }
        });
    }

    public void reset() {
        mUIHandler.removeMessages(MSG_RUN);
        mCurrentState = null;
        mStateList.clear();
    }

    public void start(State initState) {
        mCurrentState = initState;
        if (DEBUG) {
            Log.d(TAG, "SM: start with init state: " + mCurrentState);
            Log.d(TAG, "");
        }
    }

    public void run(Bullet bullet) {
       Message msg = mUIHandler.obtainMessage(MSG_RUN, 0, 0, bullet);
       mUIHandler.sendMessage(msg);
    }

    private void handleRun(Bullet bullet) {
        if (null == mCurrentState) {
            Log.e(TAG, "SM: current state is null, can not run !");
            return;
        }

        List<Trigger> triggerList = mCurrentState.getTriggerList();
        if (null == triggerList) {
            Log.e(TAG, "SM: current state don't have any trigger, can not run !");
            return;
        }

        boolean ret = false;
        for (Trigger trigger : triggerList) {
            if (!mCurrentState.isSame(trigger.getSrc())) {
                Log.w(TAG, "SM current state don't match trigger src state, ignore it !");
                continue;
            }
            ret = trigger.isFired(bullet);
            if (DEBUG) {
                Log.d(TAG, "SM: trigger: " + trigger);
                Log.d(TAG, "SM: test bullet: " + bullet + ", ret: " + ret);
            }
            if (ret) {
                mCurrentState = trigger.getDst();
                Log.d(TAG, "SM: current state change to: " + mCurrentState.getName());
                trigger.onTrigger(bullet);
                break;
            }
        }
        if (DEBUG) {
            Log.d(TAG, "");
        }
    }

    public State getCurrentState() {
        return mCurrentState;
    }

}
