package com.softwinner.vr.psensor.service;

import com.softwinner.vr.psensor.service.state.Bullet;

/**
 * StateMachine: EventBullet
 *
 * @author humingming@allwinnertech.com
 */
public class EventBullet extends Bullet {

    private static final String TAG = Utils.TAG;
    private static final boolean DEBUG = Utils.DEBUG;

    public static final String TYPE = "Event";

    public static final int EVENT_PSENSOR_NEAR  = 1;
    public static final int EVENT_PSENSOR_FAR   = 2;
    public static final int EVENT_AC_PLUG_IN    = 3;
    public static final int EVENT_AC_PLUG_OUT   = 4;
    public static final int EVENT_POWER_ON      = 5;
    public static final int EVENT_POWER_OFF     = 6;
    public static final int EVENT_SCREEN_ON     = 7;
    public static final int EVENT_SCREEN_OFF    = 8;

    private int mEvent;

    public EventBullet(int event) {
        super();
        mEvent = event;
    }

    @Override
    public String getType() {
        return TYPE;
    }

    @Override
    public boolean test(Bullet test) {
        if (!(test instanceof EventBullet)) {
            return false;
        }
        EventBullet eTest = (EventBullet)test;
        return mEvent == eTest.getEvent();
    }

    public int getEvent() {
        return mEvent;
    }

    public static EventBullet getBullet(int event) {
        return new EventBullet(event);
    }

    private static String getEventString(int event) {
        switch (event) {
            case EVENT_PSENSOR_NEAR:
               return "near";
            case EVENT_PSENSOR_FAR:
               return "far";
            case EVENT_AC_PLUG_IN:
               return "ac_plug_in";
            case EVENT_AC_PLUG_OUT:
               return "ac_plug_out";
            case EVENT_POWER_ON:
               return "power_on";
            case EVENT_POWER_OFF:
               return "power_off";
            case EVENT_SCREEN_ON:
               return "screen_on";
            case EVENT_SCREEN_OFF:
               return "screen_off";
        }
        return "unknown";
    }

    @Override
    public String toString() {
        return "event: " + getEventString(mEvent);
    }

}
