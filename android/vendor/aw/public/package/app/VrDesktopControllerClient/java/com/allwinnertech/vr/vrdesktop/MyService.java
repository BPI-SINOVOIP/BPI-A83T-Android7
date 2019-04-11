package com.allwinnertech.vr.vrdesktop;

import android.app.Service;
import android.os.IBinder;
import android.content.Intent;
import android.hardware.input.InputManager;
import android.util.Log;
import android.view.InputEvent;
import android.view.KeyEvent;
import android.os.SystemProperties;
import android.content.Context;
import android.os.SystemClock;
import android.view.WindowManager;
import android.opengl.Matrix;
import android.view.MotionEvent;
import android.view.InputDevice;
import android.os.Handler;
import java.lang.Math;

import com.allwinnertech.vr.controller.ControllerManager;
import com.allwinnertech.vr.controller.ControllerStates;

public class MyService extends Service{
    private static final String TAG = "VrDeskController";
    private ControllerManager mManager = null;
    private static final String VRDESK_ENABLE = "service.vrdesktop.enable";
    private static final String VR_ENABLE = "persist.vr.enable";
    private Context mContext = null;
    private int mScreenWidth = 0;
    private int mScreenHeight = 0;

    final float START_POINTX = 0.35f;
    final float START_POINTY = -0.3f;
    final float START_POINTZ = -0.3f;
    final float SCREEN_DISTANCE = -3.0f;

    @Override
    public IBinder onBind(Intent arg0){
        return null;
    }

    @Override
    public void onCreate(){
        Log.d(TAG, "Start to connect to ControllerService");

        mContext = this;
        WindowManager wm = (WindowManager)mContext.getSystemService(Context.WINDOW_SERVICE);
        //translate the screen window to be landscale.
        mScreenWidth = wm.getDefaultDisplay().getWidth();
        mScreenHeight = wm.getDefaultDisplay().getHeight();
        if(mScreenWidth < mScreenHeight){
            int tmp = mScreenHeight;
            mScreenHeight = mScreenWidth;
            mScreenWidth = tmp;
        }
        Log.d(TAG, "screen window is (" + mScreenWidth + ", " + mScreenHeight + ")");

        mManager = new ControllerManager(mContext, new ControllerManager.EventListener(){
            private float[] mTouchDown = new float[2];      //the coordinate after translate to screen
            private float[] mTouchDownSrc = new float[2];   //the coordinate of controller touch down
            private long mTouchDownTime = 0;
            @Override
            public void handleTouchEvent(float x, float y, int action){
                Log.d(TAG, "handle touch event (" + x + ", " + y + ", " + action + ")");
                //translate the touch coordinate to screen coordinate
                //
                //                        +1
                //         ++++++++++++++++++
                //         +                +                  +
                //         +                + (0,0)          +
                //    -1  ++++++++++++++++++ +1
                //         +                +                  +
                //         +                +                  +
                //         ++++++++++++++++++
                //                        -1
                //
                //tranlate to:
                //
                //       (0, 0)
                //         ++++++++++++++++++  screenWidth
                //         +                                    +
                //         +                                    +
                //         +                                    +
                //         +                                    +
                //         +                                    +
                //         ++++++++++++++++++
                //  screenHeight
                switch(action){
                    case MotionEvent.ACTION_DOWN:
                        mTouchDownSrc[0] = x;
                        mTouchDownSrc[1] = y;
                        caclTouchCoordinate(mTouchDown);
                        mTouchDownTime = SystemClock.uptimeMillis();
                        injectMotionEvent(InputDevice.SOURCE_TOUCHSCREEN, mTouchDownTime, mTouchDownTime,
                            action, mTouchDown[0], mTouchDown[1], 1.0f);
                        break;
                    case MotionEvent.ACTION_MOVE:
                    case MotionEvent.ACTION_UP:
                        float pixelPerMeterX = mScreenWidth / 2;
                        float pixelPerMeterY = mScreenHeight / 2;
                        float touchMoveX = (x - mTouchDownSrc[0])*pixelPerMeterX + mTouchDown[0];
                        float touchMoveY = (y - mTouchDownSrc[1])*pixelPerMeterY + mTouchDown[1];
                        injectMotionEvent(InputDevice.SOURCE_TOUCHSCREEN, mTouchDownTime, SystemClock.uptimeMillis(),
                            action, touchMoveX, touchMoveY, (action == MotionEvent.ACTION_UP ? 0.0f:1.0f));
                        break;
                }
            }

            private long mBackPressDown = 0;
            private Runnable mBackLongPress = new Runnable(){
                @Override
                public void run(){
                    Log.d(TAG, "back key long press th recenter controller and headtracking");
                    recenter();
                    recenterHeadOrientation();
                }
            };
            private Handler mBackLongPressHandler = new Handler();
            @Override
            public void handleButtonEvent(int keycode, int action){
                switch(keycode){
                    case KeyEvent.KEYCODE_HOME:
                    case KeyEvent.KEYCODE_MENU:
                    case KeyEvent.KEYCODE_VOLUME_UP:
                    case KeyEvent.KEYCODE_VOLUME_DOWN:
                        injectKeyEvent(keycode, action, 0);
                        break;
                    case KeyEvent.KEYCODE_DPAD_CENTER:
                        if(action == KeyEvent.ACTION_DOWN){
                            if(caclTouchCoordinate(mTouchDown)){
                                mTouchDownTime = SystemClock.uptimeMillis();
                                injectMotionEvent(InputDevice.SOURCE_TOUCHSCREEN, mTouchDownTime, mTouchDownTime,
                                    MotionEvent.ACTION_DOWN, mTouchDown[0], mTouchDown[1], 1.0f);
                            }else{
                                mTouchDownTime = -1;
                            }
                        }else if(action == KeyEvent.ACTION_UP && mTouchDownTime != -1){
                            injectMotionEvent(InputDevice.SOURCE_TOUCHSCREEN, mTouchDownTime, SystemClock.uptimeMillis(),
                                MotionEvent.ACTION_UP, mTouchDown[0], mTouchDown[1], 0.0f);
                        }
                        break;
                    case KeyEvent.KEYCODE_BACK:
                        if(action == KeyEvent.ACTION_DOWN){
                            mBackLongPressHandler.postDelayed(mBackLongPress, 1500);
                            injectKeyEvent(keycode, action, 0);
                            mBackPressDown = SystemClock.uptimeMillis();
                        }else if(action == KeyEvent.ACTION_UP){
                            mBackLongPressHandler.removeCallbacks(mBackLongPress);
                            if(SystemClock.uptimeMillis() - mBackPressDown < 1500){
                                injectKeyEvent(keycode, action, 0);
                            }
                        }
                        break;
                }
            }

            private boolean mFirstReceive = true;
            private float[] mRecenterMatrix = new float[16];
            private float[] mOrientMatrix = new float[16];
            private float[] mOrientQuat = new float[]{0.0f, 0.0f, 0.0f, 0.0f};
            private void recenter(){
                Matrix.invertM(mRecenterMatrix, 0, mOrientMatrix, 0);
                sendControllerOrientToSf(mConnectionState, 1, mOrientQuat[0], mOrientQuat[1], mOrientQuat[2], mOrientQuat[3]);
            }
            private void quatToMatrix(float x, float y, float z, float w, float[] matrix){
                float ww = w*w;
                float xx = x*x;
                float yy = y*y;
                float zz = z*z;
                matrix[0] = ww + xx - yy - zz;
                matrix[1] = 2 * (x*y + w*z);
                matrix[2] = 2 * (x*z - w*y);
                matrix[3] = 0;

                matrix[4] = 2 * (x*y - w*z);
                matrix[5] = ww - xx + yy - zz;
                matrix[6] = 2 * (y*z + w*x);
                matrix[7] = 0;

                matrix[8] = 2 * (x*z + w*y);
                matrix[9] = 2 * (y*z - w*x);
                matrix[10] = ww - xx - yy + zz;
                matrix[11] = 0;

                matrix[12] = 0;
                matrix[13] = 0;
                matrix[14] = 0;
                matrix[15] = 1;
            }

            private void sendControllerOrientToSf(int status, int needRecenter, float x, float y, float z, float w){
                //Log.v(TAG, "status(" + status + "), needRecenter(" + needRecenter + "), orient(" +
                  //    x + ", " + y + ", " + z + ", " + w + ")");
                float l = 0.0f;
                float[] rayStart = new float[]{START_POINTX, START_POINTY, START_POINTZ, 1.0f};
                float[] rayForwardSrc = new float[]{0.0f, 0.0f, SCREEN_DISTANCE, 1.0f};
                float[] rotateMatrix = new float[16];
                float[] rayForwardCur = new float[4];
                Matrix.multiplyMM(rotateMatrix, 0, mRecenterMatrix, 0, mOrientMatrix, 0);
                Matrix.multiplyMV(rayForwardCur, 0, rotateMatrix, 0, rayForwardSrc, 0);
                float[] planePassPoint = new float[]{0, 0, SCREEN_DISTANCE};

                if(rayForwardCur[2] != 0.0) {
                    float t = (planePassPoint[2] - rayStart[2]) / rayForwardCur[2];
                    float[] intersectPoint = new float[3];
                    intersectPoint[0] = rayStart[0] + rayForwardCur[0] * t;
                    intersectPoint[1] = rayStart[1] + rayForwardCur[1] * t;
                    intersectPoint[2] = rayStart[2] + rayForwardCur[2] * t;
                    // get length
                    float disx = intersectPoint[0] - START_POINTX;
                    float disy = intersectPoint[1] - START_POINTY;
                    float disz = intersectPoint[2] - START_POINTZ;
                    l = (float)Math.sqrt((disx * disx + disy * disy + disz * disz));
                    //Log.v(TAG, "intersect("+disx+","+disy+","+disz+","+l+")");
                }
                    // do something if length is 0
                    sendDataToSf(status, needRecenter, x, y, z, w, l);
            }

            @Override
            public void handleOrientation(float x, float y, float z, float w){
                mOrientQuat[0] = x;
                mOrientQuat[1] = y;
                mOrientQuat[2] = z;
                mOrientQuat[3] = w;
                quatToMatrix(x, y, z, w, mOrientMatrix);
                if(mFirstReceive){
                    mFirstReceive = false;
                    recenter();
                }
                sendControllerOrientToSf(mConnectionState, 0, mOrientQuat[0], mOrientQuat[1], mOrientQuat[2], mOrientQuat[3]);
            }

            private int mConnectionState = 0;
            @Override
            public void notifyConnectionStateChange(int state){
                Log.d(TAG, "controller connection state change to " + ControllerStates.toString(state));
                switch(state){
                    case ControllerStates.DISCONNECTED:
                        mConnectionState = 0;
                        sendControllerOrientToSf(mConnectionState, 0, mOrientQuat[0], mOrientQuat[1], mOrientQuat[2], mOrientQuat[3]);
                        break;
                    case ControllerStates.CONNECTED:
                        Log.d(TAG, "connected.");
                        mConnectionState = 1;
                        sendControllerOrientToSf(mConnectionState, 0, mOrientQuat[0], mOrientQuat[1], mOrientQuat[2], mOrientQuat[3]);
                        break;
                 }
            }

            private long mActionDownTime = 0;
            private void injectKeyEvent(int keycode, int action, int repeateCount){
                Log.d(TAG, String.format("inject key event %d, action %d", keycode, action));
                if(action == KeyEvent.ACTION_DOWN){
                    mActionDownTime = SystemClock.uptimeMillis();
                }
                KeyEvent ev = new KeyEvent(mActionDownTime, SystemClock.uptimeMillis(),
                    action, keycode, repeateCount);
                InputManager mng = (InputManager)mContext.getSystemService(Context.INPUT_SERVICE);
                mng.injectInputEvent(ev, InputManager.INJECT_INPUT_EVENT_MODE_WAIT_FOR_RESULT);
            }

            private void injectMotionEvent(int source, long downTime, long eventTime, int action, float x, float y, float pressure){
                final float DEFAULT_SIZE = 1.0f;
                final int DEFAULT_META_STATE = 0;
                final float DEFAULT_PRECISION_X = 1.0f;
                final float DEFAULT_PRECISION_Y = 1.0f;
                final int DEFAULT_EDGE_FLAGS = 0;
                MotionEvent event = MotionEvent.obtain(downTime, eventTime, action, x, y, pressure, DEFAULT_SIZE,
                        DEFAULT_META_STATE, DEFAULT_PRECISION_X, DEFAULT_PRECISION_Y,
                        0, DEFAULT_EDGE_FLAGS);
                Log.d(TAG, "inject motion event downtime=" + downTime + ", eventTime=" + eventTime + ", action=" +
                    action + ", x=" + x + ", y=" + y);
                event.setSource(source);
                InputManager mng = (InputManager)mContext.getSystemService(Context.INPUT_SERVICE);
                mng.injectInputEvent(event, InputManager.INJECT_INPUT_EVENT_MODE_WAIT_FOR_FINISH);
            }

            //caculate the touch (x,y) point
            //return true if the controller click in the vrdesktop, otherwise false
            private boolean caclTouchCoordinate(float[] texCoord){
                float[] rayStart = new float[]{START_POINTX, START_POINTY, START_POINTZ, 1.0f};
                float[] rayForwardSrc = new float[]{0.0f, 0.0f, SCREEN_DISTANCE, 1.0f};
                float[] rotateMatrix = new float[16];
                float[] rayForwardCur = new float[4];
                Matrix.multiplyMM(rotateMatrix, 0, mRecenterMatrix, 0, mOrientMatrix, 0);
                Matrix.multiplyMV(rayForwardCur, 0, rotateMatrix, 0, rayForwardSrc, 0);
                //caculate the intersection of ray and plane
                //Algorithm like this:
                //param:
                //  ray start point is (m1, m2, m3)
                //  ray forward is (v1, v2, v3)
                //  plane pass point (n1, n2, n3)
                //  plane's normal line is (vp1, vp2, vp3)
                //  intersection point is (x, y, z)
                //if
                //  (denominator = vp1 * v1 + vp2 * v2 + vp3 * v3) != 0
                //then
                //  t = ((n1-m1)*vp1 + (n2 - m2)*vp2 + (n3 - m3)*vp3) / denominator
                //  x = m1 + v1 * t
                //  y = m2 + v2 * t
                //  z = m3 + v3 * t
                //
                //optimization
                //our (n1, n2, n3)=(0, 0, -2.5), (vp1, vp2, vp3)=(0, 0, -2.5)
                //so:
                //  demonimator = vp3 * v3
                //  t = (n3 - m3) * vp3 / denominator = (n3 - m3) / v3;
                float[] planePassPoint = new float[]{0, 0, SCREEN_DISTANCE};
                if(rayForwardCur[2] == 0.0){
                    return false;
                }
                float t = (planePassPoint[2] - rayStart[2]) / rayForwardCur[2];
                float[] intersectPoint = new float[3];
                intersectPoint[0] = rayStart[0] + rayForwardCur[0] * t;
                intersectPoint[1] = rayStart[1] + rayForwardCur[1] * t;
                intersectPoint[2] = rayStart[2] + rayForwardCur[2] * t;
                Log.d(TAG, String.format("ray hit point (%f, %f, %f)",
                    intersectPoint[0], intersectPoint[1], intersectPoint[2]));

                //translate the space coordinate to screen coordinate
                //
                //                        +0.54
                //         +++++++++++++++++++++++++++++++++++++
                //         +                +                  +
                //         +                + (0,0)            +
                //-0.96 ++++++++++++++++++++++++++++++++++++++++ +0.96
                //         +                +                  +
                //         +                +                  +
                //         +++++++++++++++++++++++++++++++++++++
                //                        -0.54
                //
                //tranlate to:
                //
                //       (0, 0)
                //         ++++++++++++++++++  screenWidth
                //         +                                    +
                //         +                                    +
                //         +                                    +
                //         +                                    +
                //         +                                    +
                //         ++++++++++++++++++
                //  screenHeight
                int x = SystemProperties.getInt("ro.boot.lcd_x", 0);
                int y = SystemProperties.getInt("ro.boot.lcd_y", 0);
                float borderW = 0;
                float borderH = 0;
                if (x > y) {
                    borderW = (float)x / 1000;
                    borderH = (float)y / 1000;
                } else {
                    borderW = (float)y / 1000;
                    borderH = (float)x / 1000;
                }

                if(-borderW/2 > intersectPoint[0] || borderW/2 < intersectPoint[0] ||
                    -borderH/2 > intersectPoint[1] || borderH/2 < intersectPoint[1]){
                    return false;
                }
                float pixelPerMeterW = mScreenWidth / borderW;
                float pixelPerMeterH = mScreenHeight / borderH;
                texCoord[0] = (intersectPoint[0] - (-borderW/2)) * pixelPerMeterW;
                texCoord[1] = (borderH/2 - intersectPoint[1]) * pixelPerMeterH;
                return true;
            }
        });

        handleControllerStatus();

        Thread thread = new Thread(new Runnable(){
            @Override
            public void run(){
                while(true){
                    try{
                        Thread.sleep(1000);
                        handleControllerStatus();
                    }catch(InterruptedException e){
                        e.printStackTrace();
                    }
                }
            }
        });
        thread.start();

    }

    private boolean mControllerConnected = false;
    private void handleControllerStatus(){
        int vrdesktopEnable = SystemProperties.getInt(VRDESK_ENABLE, 0);
        int vrEnable = SystemProperties.getInt(VR_ENABLE, 0);
        if(vrdesktopEnable == 1 && vrEnable == 1 && !mControllerConnected){
            Log.d(TAG, "Start Connect to ControllerService.");
            mManager.start();
            mControllerConnected = true;
        }else if((vrdesktopEnable != 1 || vrEnable != 1) && mControllerConnected){
            Log.d(TAG, "Stop Connect to ControllerService.");
            mManager.stop();
            mControllerConnected = false;
        }
    }

    private native float[] getHeadTrackingData();
    private native void recenterHeadOrientation();
    private native void sendDataToSf(int status, int needRecenter, float x, float y, float z, float w, float l);

    static{
        System.loadLibrary("vrdeskcontroll");
    }
}
