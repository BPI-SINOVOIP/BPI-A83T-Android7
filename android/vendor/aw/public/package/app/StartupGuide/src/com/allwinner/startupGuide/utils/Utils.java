package com.allwinner.startupGuide.utils;

import android.os.LocaleList;
import android.util.Log;

import java.lang.reflect.Method;
import java.util.Locale;

public class Utils {

    public static final String TAG = "guide";
    private static Method sGetStringMethod;

    public static Locale getDefaultLocale() {
        LocaleList defaultLocaleList = LocaleList.getDefault();
        return defaultLocaleList.get(0);
    }

    public static boolean isCN(Locale locale) {
        if (locale.equals(Locale.CHINESE) || locale.equals(Locale.CHINA)) {
            return true;
        } else {
            return false;
        }
    }
    public static String get(final String key, final String def) {
        try {
            if (sGetStringMethod == null) {
                sGetStringMethod = Class.forName("android.os.SystemProperties")
                        .getMethod("get", String.class,String.class);
            }
            return (String)sGetStringMethod.invoke(null, key, def);
        } catch (Exception e) {
            Log.e(TAG, "Platform error: " + e.toString());
            return def;
        }
    }
}
