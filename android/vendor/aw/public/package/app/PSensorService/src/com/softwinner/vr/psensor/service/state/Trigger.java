package com.softwinner.vr.psensor.service.state;

import android.content.Context;
import android.text.TextUtils;
import android.util.Log;
import android.hardware.input.InputManager;
import android.view.InputEvent;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.util.SparseArray;

import com.softwinner.vr.psensor.service.Utils;

import java.lang.Math;
import java.lang.System;
import java.util.List;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Random;

/**
 * StateMachine: Trigger
 *
 * @author humingming@allwinnertech.com
 */
public class Trigger {

    private static final String TAG = Utils.TAG;
    private static final boolean DEBUG = Utils.DEBUG;

    private State mSrc;
    private State mDst;
    private Bullet mBullet;

    public Trigger(State src, State dst, Bullet bullet) {
        mSrc = src;
        mDst = dst;
        mBullet = bullet;
    }

    public boolean isFired(Bullet test) {
        return mBullet.test(test);
    }

    public State getSrc() {
        return mSrc;
    }

    public State getDst() {
        return mDst;
    }

    @Override
    public String toString() {
        StringBuffer buffer = new StringBuffer();
        buffer.append("state: " + mSrc.getName() + " to " + mDst.getName());
        if (DEBUG) {
            buffer.append(" bullet: " + mBullet.toString());
        }
        return buffer.toString();
    }

    // TODO: override it
    public void onTrigger(Bullet bullet) {
    }

}
