package com.allwinner.startupGuide.utils;

import android.util.Log;

import java.io.BufferedReader;
import java.io.InputStreamReader;

/**
 * Sensor calibrate bin utils
 *
 * @author humingming@allwinnertech.com
 */
public class SensorCalibrateUtils {

    private static final String TAG = Utils.TAG;

    public static final String SENSOR_ACCEL = "accel";
    public static final String SENSOR_GYRO = "gyro";
    public static final String SENSOR_MAG = "mag";

    // need system uid user group permission
    public static void nanoappCalibrate(String sensor) {
        int count = 0;
        Process proc;
        BufferedReader in;
        try {
            // running shell to chmod files
            String command = "/system/bin/nanoapp_cmd calibrate " + sensor;
            proc = Runtime.getRuntime().exec(command);

            // wait for shell execute
            //proc.waitFor();

            // get result of execute command
            Log.i(TAG, "shell ret: ");
            in = new BufferedReader(new InputStreamReader(proc.getInputStream()));
            String line;
            while ((line = in.readLine()) != null && count <= 2) {
                Log.i(TAG, line);
                count += 1;
            }

            // release resources
            proc.destroy();
            in.close();
        } catch (Exception e) {
            Log.e(TAG, "error: ", e);
        }
    }

}
