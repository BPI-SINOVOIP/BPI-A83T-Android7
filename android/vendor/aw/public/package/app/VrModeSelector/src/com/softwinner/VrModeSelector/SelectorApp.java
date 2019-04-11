package com.softwinner.VrModeSelector;

import android.app.Application;
import android.content.Intent;

import com.softwinner.VrModeSelector.utils.DebugUtils;
import com.softwinner.VrModeSelector.utils.GlobalUtils;

public class SelectorApp extends Application {

    private final static String TAG = DebugUtils.TAG;

    private static SelectorApp sApp;

    @Override
    public void onCreate() {
        super.onCreate();

        sApp = this;

        DebugUtils.init(this);
        if (GlobalUtils.isMainProcess(this)) {
            initConfig();
        }
    }

    private void initConfig() {
    }

    public static SelectorApp get() {
        return sApp;
    }

}
