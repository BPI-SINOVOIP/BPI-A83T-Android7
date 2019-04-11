/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.softwinner.screenrecord;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.os.UserHandle;
import android.service.quicksettings.Tile;
import android.service.quicksettings.TileService;
import android.util.Log;

public class ScreenrecordTileService extends TileService {
    private static final String TAG = "ScreenrecordTileService";
    private static final int DELAY = 500;
    private Handler mHandler;

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            if (GlobalScreenrecord.SCREENRECORD_STATE_CHANGED_ACTION.equals(action)) {
                boolean isRecording = intent.getBooleanExtra(GlobalScreenrecord.EXTRA_SCREENRECORD_RECORDING, false);
                updateState(isRecording);
            }
        }
    };

    public ScreenrecordTileService() {
        mHandler = new Handler();
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.i(TAG, "onBind");
        return super.onBind(intent);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "onDestroy");
    }

    @Override
    public void onTileAdded() {
        Log.i(TAG, "onTileAdded");
    }

    @Override
    public void onTileRemoved() {
        Log.i(TAG, "onTileRemoved");
    }

    @Override
    public void onStartListening() {
        Log.i(TAG, "onStartListening");
        IntentFilter filter = new IntentFilter();
        filter.addAction(GlobalScreenrecord.SCREENRECORD_STATE_CHANGED_ACTION);
        this.registerReceiverAsUser(mBroadcastReceiver, UserHandle.ALL, filter, null, null);
        updateState(false);
    }

    @Override
    public void onStopListening() {
        Log.i(TAG, "onStopListening");
    }

    @Override
    public void onClick() {
        Log.i(TAG, "onClick");
        autoRecord();
    }

    private void updateState(boolean isRecording) {
        getQsTile().setState(isRecording ? Tile.STATE_ACTIVE : Tile.STATE_INACTIVE);
        getQsTile().updateTile();
    }

    public void autoRecord() {
        mHandler.postDelayed(mScreenrecordRunnable, DELAY);
    }

    private final Runnable mScreenrecordRunnable = new Runnable() {
        @Override
        public void run() {
            takeScreenrecord();
        }
    };

    final Object mScreenrecordLock = new Object();
    // Assume this is called from the Handler thread.
    private void takeScreenrecord() {
        synchronized (mScreenrecordLock) {
            ComponentName cn = new ComponentName("com.softwinner.screenrecord",
                    "com.softwinner.screenrecord.TakeScreenrecordService");
            Intent intent = new Intent();
            intent.setComponent(cn);
            ServiceConnection conn = new ServiceConnection() {
                @Override
                public void onServiceConnected(ComponentName name, IBinder service) {
                    synchronized (mScreenrecordLock) {
                        Messenger messenger = new Messenger(service);
                        Message msg = Message.obtain(null, 1);
                        try {
                            messenger.send(msg);
                        } catch (RemoteException e) {
                        }
                    }
                }
                @Override
                public void onServiceDisconnected(ComponentName name) {}
            };
            ScreenrecordTileService.this.bindServiceAsUser(intent, conn, Context.BIND_AUTO_CREATE, UserHandle.CURRENT);
        }
    }
}
