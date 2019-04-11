package com.softwinner.VrModeSelector.utils;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.util.Log;

public class DebugUtils {

    public final static String TAG = "selector.test";
    public final static String UI_TAG = "selector.ui";

    private static boolean sIsDebug;

    public static void init(Context context) {
        sIsDebug = false;
        try {
            ApplicationInfo info = context.getApplicationInfo();
            sIsDebug = 0 != (info.flags & ApplicationInfo.FLAG_DEBUGGABLE);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static boolean isDebug() {
        return sIsDebug;
    }

    public static void Log(String tag, String msg) {
        if (isDebug()) {
            Log.d(tag, msg);
        }
    }

    public static void Log(String tag, String msg, Throwable e) {
        if (isDebug()) {
            Log.d(tag, msg, e);
        }
    }

    public static void LogI(String tag, String msg) {
        Log.i(tag, msg);
    }

    public static void LogI(String tag, String msg, Throwable e) {
        Log.i(tag, msg, e);
    }

    public static void assertFalse() {
        if (isDebug()) {
            throw new RuntimeException("Cannot run to here!");
        }
    }

    public static void assertFalse(int message) {
        if (isDebug()) {
            throw new RuntimeException("Cannot run to here! Fail message: " + message);
        }
    }

    public static void assertFalse(String message) {
        if (isDebug()) {
            throw new RuntimeException("Cannot run to here! Fail message: " + message);
        }
    }

}
