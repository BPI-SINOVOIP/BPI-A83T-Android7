package com.softwinner.vr.psensor.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.util.Log;

import com.softwinner.vr.psensor.service.state.StateMachine;

import java.lang.Runnable;
import java.lang.Exception;

public class ServiceReceiver extends BroadcastReceiver {

    private static final String TAG = Utils.TAG;
    private static final boolean DEBUG = Utils.DEBUG;

    // TODO: match our system sendboradcast
    private static final String ACTION_PSENSOR_NEAR = "com.softwinner.vr.action.psensor_near";
    private static final String ACTION_PSENSOR_FAR = "com.softwinner.vr.action.psensor_far";
    private static final String ACTION_POWER_ON = "com.softwinner.vr.action.power_on";

    // we give a key for debug keep device wakeup
    private static final String DISABLE_KEY = "persist.vr.psensor.disable";

    private int mDisabled = 0;
    private int mChargedStatus = -1;

    private volatile int mScreenOnFlag = 0;
    private volatile int mScreenOffFlag = 0;

    public ServiceReceiver() {
        try {
            mDisabled = Integer.parseInt(SystemProperties.get(DISABLE_KEY, "0"));
        } catch (Exception e) {
            mDisabled = 0;
            Log.e(TAG, "error: ", e);
        }
    }

    public ServiceReceiver(boolean selfNew) {
        this();
    }

    public void markScreenOn() {
        mScreenOnFlag = 1;
    }

    public void markScreenOff() {
        mScreenOffFlag = 1;
    }

    public IntentFilter getRegisterFilter() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_BATTERY_CHANGED);
        filter.addAction(Intent.ACTION_SCREEN_OFF);
        filter.addAction(Intent.ACTION_SCREEN_ON);
        filter.addAction(ACTION_PSENSOR_NEAR);
        filter.addAction(ACTION_PSENSOR_FAR);
        filter.addAction(ACTION_POWER_ON);
        return filter;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if (1 == mDisabled) {
            Log.i(TAG, "psensor service disabled by setting, just for debug !");
            return;
        }

        String action = intent.getAction();
        Log.i(TAG, "get receive: " + action);

        if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
            Intent startService = new Intent(context, PSensorService.class);
            context.startService(startService);
        } else if (Intent.ACTION_BATTERY_CHANGED.equals(action)) {
            int status = intent.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
            int isCharging = 0;
            if (status == BatteryManager.BATTERY_STATUS_CHARGING ||
                    status == BatteryManager.BATTERY_STATUS_FULL) {
                isCharging = 1;
            }
            Log.i(TAG, "changed: isCharging: " + isCharging + ", mChargedStatus: " + mChargedStatus);
            if (isCharging != mChargedStatus) {
                mChargedStatus = isCharging;
                onAcPlugEvent(1 == mChargedStatus);
            }
        } else if (Intent.ACTION_SCREEN_OFF.equals(action)) {
            onScreenEvent(false);
        } else if (Intent.ACTION_SCREEN_ON.equals(action)) {
            Log.i(TAG, "SCREEN_ON: mScreenOnFlag: " + mScreenOnFlag);
            if (0 != mScreenOnFlag) {
                mScreenOnFlag = 0;
                onScreenEvent(true);
            }
        } else if (ACTION_PSENSOR_NEAR.equals(action)) {
            onPSensorEvent(true);
        } else if (ACTION_PSENSOR_FAR.equals(action)) {
            onPSensorEvent(false);
        } else if (ACTION_POWER_ON.equals(action)) {
            onPowerEvent(true);
        }
    }

    private void onAcPlugEvent(boolean isPlugIn) {
        Log.i(TAG, "ac plug event: " + isPlugIn);
        int event = EventBullet.EVENT_AC_PLUG_OUT;
        if (isPlugIn) {
            event = EventBullet.EVENT_AC_PLUG_IN;
        }
        StateMachine.get().run(EventBullet.getBullet(event));
    }

    private void onScreenEvent(boolean isScreenOn) {
        Log.i(TAG, "screen event: " + isScreenOn);
        int event = EventBullet.EVENT_SCREEN_OFF;
        if (isScreenOn) {
            event = EventBullet.EVENT_SCREEN_ON;
        }
        StateMachine.get().run(EventBullet.getBullet(event));
    }

    private void onPowerEvent(boolean isPowerOn) {
        Log.i(TAG, "power event: " + isPowerOn);
        int event = EventBullet.EVENT_POWER_OFF;
        if (isPowerOn) {
            event = EventBullet.EVENT_POWER_ON;
        }
        StateMachine.get().run(EventBullet.getBullet(event));
    }

    private void onPSensorEvent(boolean isNear) {
        Log.i(TAG, "psensor event: " + isNear);
        int event = EventBullet.EVENT_PSENSOR_FAR;
        if (isNear) {
            event = EventBullet.EVENT_PSENSOR_NEAR;
        }
        StateMachine.get().run(EventBullet.getBullet(event));
    }

}
