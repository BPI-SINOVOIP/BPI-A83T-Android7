package com.softwinner.VrModeSelector.utils;

import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.util.Log;

import com.softwinner.VrModeSelector.SelectorApp;

import java.util.List;

public class GlobalUtils {

    public static boolean isMainProcess(Context context) {
        ActivityManager activityManager = ((ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE));
        List<ActivityManager.RunningAppProcessInfo> processInfos = activityManager.getRunningAppProcesses();
        String mainProcessName = context.getPackageName();
        int myPid = android.os.Process.myPid();
        for (ActivityManager.RunningAppProcessInfo info : processInfos) {
            if (info.pid == myPid && mainProcessName.equals(info.processName)) {
                return true;
            }
        }
        return false;
    }

    public static int getAppVersionCode(Context context, String pkg) {
        PackageManager pm = context.getPackageManager();
        try {
            PackageInfo info = pm.getPackageInfo(pkg, 0);
            return info.versionCode;
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
            return 0;
        }
    }

    public static String getAppVersionName(Context context, String pkg) {
        PackageManager pm = context.getPackageManager();
        try {
            PackageInfo info = pm.getPackageInfo(pkg, 0);
            return info.versionName;
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
            return null;
        }
    }

    public static String getMD5(byte[] source) {
        String md5 = null;
        char hexDigits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
        try {
            java.security.MessageDigest messageDigest = java.security.MessageDigest.getInstance("MD5");
            messageDigest.update(source);
            byte bytes[] = messageDigest.digest();
            char str[] = new char[16 * 2];
            int k = 0;
            for (int i = 0; i < 16; i++) {
                byte theByte = bytes[i];
                str[k++] = hexDigits[theByte >>> 4 & 0xf];
                str[k++] = hexDigits[theByte & 0xf];
            }
            md5 = new String(str);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return md5;
    }

    public static String byteToHex(byte[] b) {
        StringBuilder builder = new StringBuilder();
        String stmp;
        for (int n = 0; n < b.length; n++) {
            stmp = (Integer.toHexString(b[n] & 0xff));
            if (stmp.length() == 1) {
                builder.append("0" + stmp);
            } else {
                builder.append(stmp);
            }
        }
        return builder.toString().toUpperCase();
    }

    public static int currentTimeMillisInt() {
        return (int)(System.currentTimeMillis() % Integer.MAX_VALUE);
    }

    public static boolean isNetworkConnected() {
        Context context = SelectorApp.get();
        ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(
            Context.CONNECTIVITY_SERVICE);
        /*NetworkInfo[] networkInfos = connectivityManager.getAllNetworkInfo();
        if (null == networkInfos) {
            return false;
        }
        for (NetworkInfo networkInfo : networkInfos) {
            if (NetworkInfo.State.CONNECTED == networkInfo.getState()) {
                return true;
            }
        }*/
        NetworkInfo networkInfo = connectivityManager.getActiveNetworkInfo();
        if (null != networkInfo && NetworkInfo.State.CONNECTED == networkInfo.getState()) {
            return true;
        } else {
            return false;
        }
    }

    public static boolean startActivitySafely(Context context, Intent intent) {
        try {
            context.startActivity(intent);
            return true;
        } catch (Exception e) {
            Log.e(DebugUtils.TAG, "start activity error: ", e);
            return false;
        }
    }

}
