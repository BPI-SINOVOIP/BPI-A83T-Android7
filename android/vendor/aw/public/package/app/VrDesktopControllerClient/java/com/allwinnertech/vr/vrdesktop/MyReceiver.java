package com.allwinnertech.vr.vrdesktop;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

/*
* operate the controller in vrdesktop
*/

public class MyReceiver extends BroadcastReceiver{
    private static final String TAG = "VrDeskController";
    @Override
    public void onReceive(Context context, Intent intent){
        Log.d(TAG, "on boot_complete receive");
        Intent service = new Intent(context, MyService.class);
        context.startService(service);
    }
}