package com.softwinner.vr.psensor.service.state;

import android.content.Context;
import android.text.TextUtils;
import android.util.Log;

import com.softwinner.vr.psensor.service.Utils;

import java.lang.Math;
import java.lang.System;
import java.lang.String;
import java.util.List;
import java.util.ArrayList;

/**
 * StateMachine: state
 *
 * @author humingming@allwinnertech.com
 */
public class State {

    private static final String TAG = Utils.TAG;
    private static final boolean DEBUG = Utils.DEBUG;

    private String mName;
    private List<Trigger> mTriggerList;

    public State(String name) {
        mName = name;
        mTriggerList = new ArrayList<Trigger>();
    }

    // we don't check duplicate trigger
    public void addTrigger(Trigger trigger) {
        mTriggerList.add(trigger);
    }

    public String getName() {
        return mName;
    }

    public List<Trigger> getTriggerList() {
        return mTriggerList;
    }

    public void clear() {
        mTriggerList.clear();
    }

    public boolean isSame(State other) {
        return TextUtils.equals(mName, other.getName());
    }

    @Override
    public String toString() {
        if (DEBUG) {
            StringBuffer buffer = new StringBuffer();
            buffer.append("state: " + mName + "\n");
            buffer.append("trigger list: \n");
            for (Trigger t : mTriggerList) {
                buffer.append(t.toString() + "\n");
            }
            return buffer.toString();
        } else {
            return mName;
        }
    }

}
