package com.softwinner.VrModeSelector.utils;

import android.os.IBinder;
import android.os.Parcel;
import android.util.Log;

/**
 * Java interface.
 *
 * @author humingming@allwinnertech.com
 */
public class JavaUtils {

    private static final String TAG = DebugUtils.TAG;

    public static final int CMD_GET = 7;
    public static final int CMD_SET = 8;

    public static final int MODE_VR = 1;
    public static final int MODE_NORMAL = 0;

    // TODO: sync with ISurfaceCompose.h
    private static final int SEND_DATE_TO_SF = IBinder.FIRST_CALL_TRANSACTION + 22;
    private static final int DISPLAY_DATA = IBinder.FIRST_CALL_TRANSACTION + 23;


    public static void sendDataToSf(int status, int needRecenter, float x, float y, float z, float w) {
        try {
            IBinder surfaceFlinger = getSurfaceFlinger();
            Log.i(TAG, "surfaceFlinger object: " + surfaceFlinger);
            if (null == surfaceFlinger) {
                Log.i(TAG, "get surfaceFlinger null !");
                return;
            }

            // call SurfaceFlinger IPC
            Parcel data = Parcel.obtain();
            data.writeInterfaceToken("android.ui.ISurfaceComposer");
            data.writeInt(status);
            data.writeInt(needRecenter);
            data.writeFloat(x);
            data.writeFloat(y);
            data.writeFloat(z);
            data.writeFloat(w);
            surfaceFlinger.transact(SEND_DATE_TO_SF, data, null, 0);
            data.recycle();

        } catch (Exception e) {
            Log.e(TAG, "sendDataToSf error: ", e);
        }
    }

    public static int displayCtrl(int disp, int cmd, int val0, int val1) {
        try {
            /*IBinder displayService = getDisplayService();
            Log.i(TAG, "displayService object: " + displayService);
            if (null == displayService) {
                Log.i(TAG, "get displayService null !");
                return -1;
            }

            int code = IBinder.FIRST_CALL_TRANSACTION;
            // call displayService IPC
            Parcel data = Parcel.obtain();
            Parcel reply = Parcel.obtain();
            data.writeInterfaceToken("com.softwinner.IDisplayService");
            data.writeInt(disp);
            data.writeInt(cmd);
            data.writeInt(val0);
            data.writeInt(val1);
            displayService.transact(code, data, reply, 0);
            data.recycle();
            return reply.readInt();*/

            IBinder surfaceFlinger = getSurfaceFlinger();
            Log.i(TAG, "surfaceFlinger object: " + surfaceFlinger);
            if (null == surfaceFlinger) {
                Log.i(TAG, "get surfaceFlinger null !");
                return -1;
            }

            // call SurfaceFlinger IPC
            Parcel data = Parcel.obtain();
            Parcel reply = Parcel.obtain();
            data.writeInterfaceToken("android.ui.ISurfaceComposer");
            data.writeInt(disp);
            data.writeInt(cmd);
            data.writeInt(val0);
            data.writeInt(val1);
            surfaceFlinger.transact(DISPLAY_DATA, data, reply, 0);
            data.recycle();
            return reply.readInt();

        } catch (Exception e) {
            Log.e(TAG, "surfaceFlinger error: ", e);
            return -1;
        }
    }

    private static IBinder getSurfaceFlinger() {
        try {
            Class serviceManager = Class.forName("android.os.ServiceManager");
            return (IBinder) ReflectUtils.invokeStaticMethod(serviceManager, "getService",
                    new Class[] {String.class},
                    new Object[] {"SurfaceFlinger"});
        } catch (Exception e) {
            Log.e(TAG, "get surface flinger error: " + e);
            return null;
        }
    }

    private static IBinder getDisplayService() {
        try {
            Class serviceManager = Class.forName("android.os.ServiceManager");
            return (IBinder) ReflectUtils.invokeStaticMethod(serviceManager, "getService",
                new Class[] {String.class},
                new Object[] {"aw_display"});
        } catch (Exception e) {
            Log.e(TAG, "get display service error: " + e);
            return null;
        }
    }

}
