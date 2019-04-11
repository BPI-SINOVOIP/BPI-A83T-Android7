/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.softwinner;

import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

import java.io.IOException;
import java.lang.Integer;
import java.lang.String;

/**
 * Class that provides access to some of the gpio management functions.
 *
 */
public class AWDisplay {
    public static final String TAG = "awdisplay";
    public static final int DISP0 = 0;
    public static final int DISP1 = 1;
    public static final int DISPLAY_CMD_SET_3DMODE = 0x01;    //取值范围 0,1,2,3,4,5,6
    public static final int DISPLAY_CMD_GET_3DMODE = 0x02;
    public static final int DISPLAY_CMD_SET_BACKLIGHT = 0x03; //取值范围 0,1,2
    public static final int DISPLAY_CMD_GET_BACKLIGHT = 0x04;
    public static final int DISPLAY_CMD_SET_ENHANCE = 0x05;   //取值范围0,1,2,3
    public static final int DISPLAY_CMD_GET_ENHANCE = 0x06;
    public static final int DISPLAY_CMD_SET_READING_MODE = 0x08;
    public static final int DISPLAY_CMD_GET_READING_STATE = 0x09;
    public static final int DISPLAY_CMD_GET_READING_STRENGTH = 0x0a;
    public static final int DISPLAY_CMD_SET_COLOR_TEMPERATURE = 0x0b;
    public static final int DISPLAY_CMD_GET_COLOR_TEMPERATURE = 0x0c;

    /*Status Masks for Enhance/smbl/reading mode*/
    public static final int DISPLAY_FUNC_OFF_MASK = 0x00;
    public static final int DISPLAY_FUNC_ON_MASK = 0x01;
    public static final int DISPLAY_FUNC_DEMO_ON_MASK = 0x03;

    public static boolean sReady = false;
    public static int sSmartBacklight; // 智能背光
    public static int sColorEnhance; // 丽色系统
    public static int sReadingModeState;
    public static int sReadingModeStrength;
    public static int sColorTemperature;

    private static IDisplayService mBinder;

    static {
        init();
    }

    // can't instantiate this class
    private AWDisplay() {
    }

    private static void init() {
        mBinder = IDisplayService.Stub.asInterface(ServiceManager
                .getService("aw_display"));
        if (mBinder != null) {
            sReady = true;
            sSmartBacklight = dispCtrl(DISP0, DISPLAY_CMD_GET_BACKLIGHT, 0, 0);
            sColorEnhance = dispCtrl(DISP0, DISPLAY_CMD_GET_ENHANCE, 0, 0);
            sReadingModeState = dispCtrl(DISP0, DISPLAY_CMD_GET_READING_STATE, 0, 0);
            sReadingModeStrength = dispCtrl(DISP0, DISPLAY_CMD_GET_READING_STRENGTH, 0, 0);
            sColorTemperature = dispCtrl(DISP0, DISPLAY_CMD_GET_COLOR_TEMPERATURE, 0, 0);
        } else {
            Log.w(TAG, "Cannot connect to aw_display");
        }
    }

    public static boolean ready() {
        return sReady;
    }

    public static int dispCtrl(int disp, int cmd, int val0, int val1) {
        int ret = -1;
        if (mBinder != null) {
            try {
                ret = mBinder.displayCtrl(disp, cmd, val0, val1);
            } catch (RemoteException e) {
                Log.w(TAG, "Cannot connect to aw_display", e);
            }
        } else {
            Log.w(TAG, "Cannot dispCtrl");
        }
        return ret;
    }

    public static boolean getSmartBacklight() {
        return (sSmartBacklight & DISPLAY_FUNC_ON_MASK) == DISPLAY_FUNC_ON_MASK;
    }

    public static void setSmartBacklight(boolean on) {
        if (on) sSmartBacklight |= DISPLAY_FUNC_ON_MASK;
        else sSmartBacklight &= DISPLAY_FUNC_OFF_MASK;
        dispCtrl(DISP0, DISPLAY_CMD_SET_BACKLIGHT, sSmartBacklight, 0);
    }

    public static boolean getSmartBacklightDemo() {
        return (sSmartBacklight & DISPLAY_FUNC_DEMO_ON_MASK) == DISPLAY_FUNC_DEMO_ON_MASK;
    }

    public static void setSmartBacklightDemo(boolean on) {
        if (on) sSmartBacklight |= DISPLAY_FUNC_DEMO_ON_MASK;
        else sSmartBacklight &= DISPLAY_FUNC_ON_MASK;
        dispCtrl(DISP0, DISPLAY_CMD_SET_BACKLIGHT, sSmartBacklight, 0);
    }

    public static boolean getColorEnhance() {
        return (sColorEnhance & DISPLAY_FUNC_ON_MASK) == DISPLAY_FUNC_ON_MASK;
    }

    public static void setColorEnhance(boolean on) {
        if (on)  sColorEnhance |= DISPLAY_FUNC_ON_MASK;
        else sColorEnhance &= DISPLAY_FUNC_OFF_MASK;
        dispCtrl(DISP0, DISPLAY_CMD_SET_ENHANCE, sColorEnhance, 0);
    }

    public static boolean getColorEnhanceDemo() {
        return (sColorEnhance & DISPLAY_FUNC_DEMO_ON_MASK) == DISPLAY_FUNC_DEMO_ON_MASK;
    }

    public static void setColorEnhanceDemo(boolean on) {
        if (on) sColorEnhance |= DISPLAY_FUNC_DEMO_ON_MASK;
        else sColorEnhance &= DISPLAY_FUNC_ON_MASK;
        dispCtrl(DISP0, DISPLAY_CMD_SET_ENHANCE, sColorEnhance, 0);
    }

    public static int getColorTemperature() {
        return dispCtrl(DISP0, DISPLAY_CMD_GET_COLOR_TEMPERATURE, 0, 0);
    }

    public static void setColorTemperature(int value) {
        dispCtrl(DISP0, DISPLAY_CMD_SET_COLOR_TEMPERATURE, value, 0);
    }

    public static boolean getReadingModeIsEnabled() {
        sReadingModeState = dispCtrl(DISP0, DISPLAY_CMD_GET_READING_STATE, 0, 0);
        return (sReadingModeState & DISPLAY_FUNC_ON_MASK) == DISPLAY_FUNC_ON_MASK;
    }

    public static int getReadingModeStrength() {
        return dispCtrl(DISP0, DISPLAY_CMD_GET_READING_STRENGTH, 0, 0);
    }

    public static void setReadingMode(boolean on, int value) {
        if (on) dispCtrl(DISP0, DISPLAY_CMD_SET_READING_MODE, DISPLAY_FUNC_ON_MASK, value);
        else dispCtrl(DISP0, DISPLAY_CMD_SET_READING_MODE, DISPLAY_FUNC_OFF_MASK, value);
    }

    public static int get3DMode() {
        return dispCtrl(DISP1, DISPLAY_CMD_GET_3DMODE, 0, 0);
    }

    public static boolean set3DMode(int mode) {
        return dispCtrl(DISP1, DISPLAY_CMD_SET_3DMODE, mode, 0) == 0;
    }
}

