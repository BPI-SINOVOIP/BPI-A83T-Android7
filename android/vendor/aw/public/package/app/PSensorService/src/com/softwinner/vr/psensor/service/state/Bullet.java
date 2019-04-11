package com.softwinner.vr.psensor.service.state;

import com.softwinner.vr.psensor.service.Utils;

/**
 * StateMachine: Bullet
 *
 * @author humingming@allwinnertech.com
 */
public abstract class Bullet {

    private static final String TAG = Utils.TAG;
    private static final boolean DEBUG = Utils.DEBUG;

    public Bullet() {
    }

    public abstract String getType();

    public abstract boolean test(Bullet test);

    @Override
    public String toString() {
        return getType();
    }

}
