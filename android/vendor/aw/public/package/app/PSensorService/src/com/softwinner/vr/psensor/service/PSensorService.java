package com.softwinner.vr.psensor.service;

import android.app.Service;
import android.content.Context;
import android.content.ContentResolver;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.provider.Settings;
import android.os.Binder;
import android.os.IBinder;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.os.UserHandle;
import android.text.TextUtils;
import android.util.Log;

import com.softwinner.vr.psensor.service.state.State;
import com.softwinner.vr.psensor.service.state.Trigger;
import com.softwinner.vr.psensor.service.state.Bullet;
import com.softwinner.vr.psensor.service.state.StateMachine;

import java.lang.Exception;

/**
 * PSensor service handle psensor wake and sleep
 *
 * @auther humingming@allwinnertech.com
 */
public class PSensorService extends Service {

    private static final String TAG = Utils.TAG;
    private static final boolean DEBUG = Utils.DEBUG;

    // unit: ms
    private static final int DEFAULT_SCREEN_OFF_TIMEOUT = 10 * 1000;

    private static final int MSG_SCREEN_TIMEOUT = 100;

    private int mScreenOffTimeOut;

    private Handler mUIHandler;

    private ServiceReceiver mReceiver;
    private ContentResolver mResolver;
    private SettingsObserver mObserver;

    private WakeLock mWakeLock;
    private PowerManager mPowerMgr;

    public PSensorService() {
    }

    @Override
    public void onCreate() {
        if (DEBUG) {
            Log.i(TAG, "onCreate ... ");
        }
        super.onCreate();

        mResolver = getContentResolver();
        mPowerMgr = (PowerManager)getSystemService(Context.POWER_SERVICE);
        mWakeLock = mPowerMgr.newWakeLock(PowerManager.FULL_WAKE_LOCK |
                PowerManager.ACQUIRE_CAUSES_WAKEUP, "PSensorService");

        readConfig();
        initHandler();
        buildState();

        mReceiver = new ServiceReceiver(true);
        try {
            registerReceiver(mReceiver, mReceiver.getRegisterFilter());
        } catch (Exception e) {
            Log.e(TAG, "error: ", e);
        }
        mObserver = new SettingsObserver(mUIHandler);
        try {
            mResolver.registerContentObserver(Settings.System.getUriFor(
                        Settings.System.CONTENT_URI,
                        Settings.System.PSENSOR_SCREEN_OFF_TIMEOUT),
                    true, mObserver);
        } catch (Exception e) {
            Log.e(TAG, "error: ", e);
        }
    }

    @Override
    public void onDestroy() {
        if (DEBUG) {
            Log.i(TAG, "onDestroy ... ");
        }
        super.onDestroy();

        try {
            unregisterReceiver(mReceiver);
        } catch (Exception e) {
            Log.e(TAG, "error: ", e);
        }
        try {
            mResolver.unregisterContentObserver(mObserver);
        } catch (Exception e) {
            Log.e(TAG, "error: ", e);
        }

        StateMachine.get().reset();
    }

    @Override
    public IBinder onBind(Intent intent) {
        // don't support bind action
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (null != intent) {
            String action = intent.getAction();
            if (DEBUG) {
                Log.i(TAG, "onStartCommand: " + action);
            }
        }
        // we want service still running after start command
        return START_STICKY;
    }

    private void initHandler() {
        mUIHandler = new Handler(Looper.getMainLooper(), new Handler.Callback() {
            @Override
            public boolean handleMessage(Message msg) {
                switch (msg.what) {
                    case MSG_SCREEN_TIMEOUT:
                        sleep();
                        return true;
                }
                return false;
            }
        });
    }

    private void buildState() {
        State sScreenOffFar = new State(StateConst.SCREEN_OFF_FAR);
        State sScreenOffNear = new State(StateConst.SCREEN_OFF_NEAR);
        State sScreenOnFar = new State(StateConst.SCREEN_ON_FAR);
        State sScreenOnNear = new State(StateConst.SCREEN_ON_NEAR);

        Bullet bullet;

        // S1: screen_off_far --> screen_off_near: move near
        bullet = EventBullet.getBullet(EventBullet.EVENT_PSENSOR_NEAR);
        Trigger S1 = new Trigger(sScreenOffFar, sScreenOffNear, bullet) {
            @Override
            public void onTrigger(Bullet bullet) {
                wakeup();
                // we don't need hold wakelock because our vr platfrom
                // is set screen off time out to never !!
            }
        };

        // S2: screen_off_far --> screen_on_far: power on
        bullet = EventBullet.getBullet(EventBullet.EVENT_POWER_ON);
        Trigger S2 = new Trigger(sScreenOffFar, sScreenOnFar, bullet) {
            @Override
            public void onTrigger(Bullet bullet) {
                mUIHandler.removeMessages(MSG_SCREEN_TIMEOUT);
            }
        };

        // S3: screen_off_far --> screen_off_far: ac in
        bullet = EventBullet.getBullet(EventBullet.EVENT_AC_PLUG_IN);
        Trigger S3 = new Trigger(sScreenOffFar, sScreenOffFar, bullet) {
            @Override
            public void onTrigger(Bullet bullet) {
                setupSleepAction();
            }
        };

        // S4: screen_off_far --> screen_off_far: ac out
        bullet = EventBullet.getBullet(EventBullet.EVENT_AC_PLUG_OUT);
        Trigger S4 = new Trigger(sScreenOffFar, sScreenOffFar, bullet) {
            @Override
            public void onTrigger(Bullet bullet) {
                setupSleepAction();
            }
        };

        // S5: screen_off_near --> screen_off_far: move far
        bullet = EventBullet.getBullet(EventBullet.EVENT_PSENSOR_FAR);
        Trigger S5 = new Trigger(sScreenOffNear, sScreenOffFar, bullet) {
            @Override
            public void onTrigger(Bullet bullet) {
                // do noting
            }
        };

        // S6: screen_off_near --> screen_on_near: screen on
        bullet = EventBullet.getBullet(EventBullet.EVENT_SCREEN_ON);
        Trigger S6 = new Trigger(sScreenOffNear, sScreenOnNear, bullet) {
            @Override
            public void onTrigger(Bullet bullet) {
                mUIHandler.removeMessages(MSG_SCREEN_TIMEOUT);
            }
        };

        // S7: screen_off_near --> screen_off_near: ac in
        bullet = EventBullet.getBullet(EventBullet.EVENT_AC_PLUG_IN);
        Trigger S7 = new Trigger(sScreenOffNear, sScreenOffNear, bullet) {
            @Override
            public void onTrigger(Bullet bullet) {
                setupSleepAction();
            }
        };

        // S8: screen_off_near --> screen_off_near: ac out
        bullet = EventBullet.getBullet(EventBullet.EVENT_AC_PLUG_OUT);
        Trigger S8 = new Trigger(sScreenOffNear, sScreenOffNear, bullet) {
            @Override
            public void onTrigger(Bullet bullet) {
                setupSleepAction();
            }
        };

        // S9: screen_on_near --> screen_off_near: screen off
        bullet = EventBullet.getBullet(EventBullet.EVENT_SCREEN_OFF);
        Trigger S9 = new Trigger(sScreenOnNear, sScreenOffNear, bullet) {
            @Override
            public void onTrigger(Bullet bullet) {
                // do noting
            }
        };

        // S10: screen_on_near --> screen_on_far: move far
        bullet = EventBullet.getBullet(EventBullet.EVENT_PSENSOR_FAR);
        Trigger S10 = new Trigger(sScreenOnNear, sScreenOnFar, bullet) {
            @Override
            public void onTrigger(Bullet bullet) {
                setupSleepAction();
            }
        };

        // S11: screen_on_far --> screen_off_far: screen off
        bullet = EventBullet.getBullet(EventBullet.EVENT_SCREEN_OFF);
        Trigger S11 = new Trigger(sScreenOnFar, sScreenOffFar, bullet) {
            @Override
            public void onTrigger(Bullet bullet) {
                // do noting
            }
        };

        // S12: screen_on_far --> screen_on_near: move near
        bullet = EventBullet.getBullet(EventBullet.EVENT_PSENSOR_NEAR);
        Trigger S12 = new Trigger(sScreenOnFar, sScreenOnNear, bullet) {
            @Override
            public void onTrigger(Bullet bullet) {
                mUIHandler.removeMessages(MSG_SCREEN_TIMEOUT);
            }
        };

        // S13: screen_off_near --> screen_on_near: power on
        bullet = EventBullet.getBullet(EventBullet.EVENT_POWER_ON);
        Trigger S13 = new Trigger(sScreenOffNear, sScreenOnNear, bullet) {
            @Override
            public void onTrigger(Bullet bullet) {
                mUIHandler.removeMessages(MSG_SCREEN_TIMEOUT);
            }
        };

        sScreenOffFar.addTrigger(S1);
        sScreenOffFar.addTrigger(S2);
        sScreenOffFar.addTrigger(S3);
        sScreenOffFar.addTrigger(S4);

        sScreenOffNear.addTrigger(S5);
        sScreenOffNear.addTrigger(S6);
        sScreenOffNear.addTrigger(S7);
        sScreenOffNear.addTrigger(S8);
        sScreenOffNear.addTrigger(S13);

        sScreenOnNear.addTrigger(S9);
        sScreenOnNear.addTrigger(S10);

        sScreenOnFar.addTrigger(S11);
        sScreenOnFar.addTrigger(S12);

        // TODO: give a state
        // start with init state
        StateMachine.get().start(sScreenOnNear);
    }

    /*private void lockWake() {
        if (DEBUG) {
            Log.d(TAG, "wakelock acquire");
        }
        // remove setupSleepAction timer
        mUIHandler.removeMessages(MSG_SCREEN_TIMEOUT);
        if (!mWakeLock.isHeld()) {
            try {
                mWakeLock.acquire();
            } catch (Exception e) {
                Log.e(TAG, "error: ", e);
            }
        }
    }

    private void unlockWake() {
        if (DEBUG) {
            Log.d(TAG, "wakelock release");
        }
        if (mWakeLock.isHeld()) {
            try {
                mWakeLock.release();
            } catch (Exception e) {
                Log.e(TAG, "error: ", e);
            }
        }
    }*/

    private void wakeup() {
        if (DEBUG) {
            Log.d(TAG, "wakeup system");
        }
        mReceiver.markScreenOn();
        mPowerMgr.wakeUp(SystemClock.uptimeMillis());
    }

    private void sleep() {
        if (DEBUG) {
            Log.d(TAG, "go to sleep");
        }
        mReceiver.markScreenOff();
        //unlockWake();
        mPowerMgr.goToSleep(SystemClock.uptimeMillis());
    }

    private void setupSleepAction() {
        if (DEBUG) {
            Log.d(TAG, "setup a sleep timer ... ");
        }
        mUIHandler.removeMessages(MSG_SCREEN_TIMEOUT);
        mUIHandler.sendEmptyMessageDelayed(MSG_SCREEN_TIMEOUT, mScreenOffTimeOut);
    }

    private void readConfig() {
        // get time out config from settings
        int newVal;
        try {
            newVal = Settings.System.getInt(mResolver,
                    Settings.System.PSENSOR_SCREEN_OFF_TIMEOUT,
                    DEFAULT_SCREEN_OFF_TIMEOUT);
            if (newVal != mScreenOffTimeOut && null != mUIHandler && mUIHandler.hasMessages(MSG_SCREEN_TIMEOUT)) {
                // reset it
                Log.i(TAG, "screen off timeout changed and setup, and reset it !");
                mUIHandler.removeMessages(MSG_SCREEN_TIMEOUT);
                mUIHandler.sendEmptyMessageDelayed(MSG_SCREEN_TIMEOUT, newVal);
            }
            mScreenOffTimeOut = newVal;
        } catch (Exception e) {
            Log.e(TAG, "error: ", e);
        }
        Log.i(TAG, "psensor screen off timeout: " + mScreenOffTimeOut);
    }

    private class SettingsObserver extends ContentObserver {
        public SettingsObserver(Handler handler) {
            super(handler);
        }

        @Override
        public void onChange(boolean selfChange) {
            readConfig();
        }
    }

}
