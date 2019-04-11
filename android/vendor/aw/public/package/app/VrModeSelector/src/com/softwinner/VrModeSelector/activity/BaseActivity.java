package com.softwinner.VrModeSelector.activity;

import android.app.Activity;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;

import com.softwinner.VrModeSelector.utils.DebugUtils;

public class BaseActivity extends Activity {

    private final static String TAG = DebugUtils.TAG;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    private void configWindow() {
        Window window = getWindow();
        ViewGroup decorView = (ViewGroup) window.getDecorView();
        decorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN
            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
            | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
            | View.SYSTEM_UI_FLAG_LOW_PROFILE);
        DisplayMetrics metrics = getResources().getDisplayMetrics();
        Log.i(TAG, "dpi: " + metrics.densityDpi
            + ", density: " + metrics.density
            + ", " + metrics.widthPixels
            + "x" + metrics.heightPixels
            + ", xdpi: " + metrics.xdpi
            + ", ydpi: " + metrics.ydpi);
    }

    @Override
    protected void onResume() {
        super.onResume();
        configWindow();
    }

}
