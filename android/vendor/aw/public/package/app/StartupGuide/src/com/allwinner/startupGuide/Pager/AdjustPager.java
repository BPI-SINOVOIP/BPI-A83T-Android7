package com.allwinner.startupGuide.Pager;

import android.app.Dialog;
import android.content.Context;
import android.content.res.Configuration;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.MediaPlayer;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.VideoView;
import android.os.Looper;
import android.os.Handler;
import android.widget.TextView;

import com.allwinner.startupGuide.R;
import com.allwinner.startupGuide.utils.SensorCalibrateUtils;
import com.allwinner.startupGuide.utils.Utils;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

public class AdjustPager extends BasePager implements OnClickListener,
        MediaPlayer.OnCompletionListener, MediaPlayer.OnPreparedListener, MediaPlayer.OnErrorListener,
        SensorEventListener {

    private static final String TAG = Utils.TAG;

    // 1min
    private static final long ADJUST_TIME_OUT = 1 * 20 * 1000;

    // unit: us (1s = 1000ms = 1000 * 1000us)
    private static final int SENSOR_SAMPLE_RATE = 5 * 1000; // 200Hz
    private static final int SENSOR_SAMPLE_COUNT = 800;

    // TODO: may be we should add timeout ...
    private static final int STATE_PLAYING_TUTORIAL = 0;
    private static final int STATE_READY_TO_ADJUST = 1;
    private static final int STATE_DOING_ADJUST = 2;
    private static final int STATE_ADJUST_SUCCESS = 3;
    private static final int STATE_ADJUST_FAILED = 4;
    private static final int STATE_ADJUST_DONE_SUCCESS = 5;
    private static final int STATE_ADJUST_DONE_FAILED = 6;

    private View mVideoContainer;
    private VideoView mVideoView;

    private Button mBtnReplay;
    private Button mBtnStartAdjust;

    private View mBtnNext;
    private View mSuccessView;
    private View mFailedView;

    private Dialog mLEDDlg;
    private View mLEDColorView;

    private boolean mIsSetTutorialRes;
    private boolean mIsTutorialVideoReady;

    private int mCurrentState;

    private int mIndex;
    private int mFailTime;
    private List<Double> mCalculateX;
    private List<Double> mCalculateY;
    private List<Double> mCalculateZ;
    private TextView mSuccessText;

    private Sensor mSensorAccelerometer;
    private SensorManager mSensorManager;

    private Runnable mTimeOutAction;
    private Handler mUIHandler;
    private TextView mFailedText;
    private TextView mCorrectFailed;
    private LinearLayout mLlayoutFailOnetime;
    private LinearLayout mLlayoutFailSecondtime;
    private Button mSkip;
    private Button mBtnFailStartAdjust;
    public AdjustPager(Context context, PagerHost host) {
        super(context, host);
        mFailTime=0;
        mIsSetTutorialRes = false;
        mIsTutorialVideoReady = false;
    }

    @Override
    protected View initView() {
        View view = View.inflate(mAppCtx, R.layout.adjust_pager, null);
        mVideoContainer = view.findViewById(R.id.tutorial_container);
        mLlayoutFailOnetime=(LinearLayout)view.findViewById(R.id.llayout1);
        mLlayoutFailSecondtime=(LinearLayout)view.findViewById(R.id.llayout2);
        mVideoView = (VideoView) view.findViewById(R.id.video_view);
        mBtnReplay = (Button) view.findViewById(R.id.redemo);
        mBtnStartAdjust = (Button) view.findViewById(R.id.start);
        mSkip = (Button) view.findViewById(R.id.skip);
        mBtnFailStartAdjust = (Button) view.findViewById(R.id.start2);
        mSuccessText=(TextView)view.findViewById(R.id.success);

        mSuccessView = view.findViewById(R.id.stubview_success);
        mFailedView = view.findViewById(R.id.stubview_failure);
        mFailedText= (TextView)view.findViewById(R.id.text3);
        mBtnNext = view.findViewById(R.id.nextpager);
        mCorrectFailed= (TextView)view.findViewById(R.id.text2);

        Context activity = (Context)mHost;
        mLEDDlg = new Dialog(activity, R.style.Dialog_Fullscreen);
        mLEDDlg.setCancelable(false);
        mLEDDlg.setCanceledOnTouchOutside(false);
        LayoutInflater layoutInflater = LayoutInflater.from(mAppCtx);
        View dlgContentView = layoutInflater.inflate(R.layout.dialog_layout, null);
        mLEDColorView = dlgContentView.findViewById(R.id.layout_root);
        mLEDDlg.setContentView(dlgContentView);
        // notice: set title to CustomDialog to let VrDesktop show this dialog fullscreen
        // don't change this title !!
        mLEDDlg.setTitle("CustomDialog");

        mBtnReplay.setOnClickListener(this);
        mBtnStartAdjust.setOnClickListener(this);
        mBtnNext.setOnClickListener(this);
        mSkip.setOnClickListener(this);
        mBtnFailStartAdjust.setOnClickListener(this);

        mVideoView.setOnPreparedListener(this);
        mVideoView.setOnCompletionListener(this);
        mVideoView.setOnErrorListener(this);

        Locale defaultLocale = Utils.getDefaultLocale();
        loadUIRes(defaultLocale);

        mIndex = 0;
        mCalculateX = new ArrayList<Double>();
        mCalculateY = new ArrayList<Double>();
        mCalculateZ = new ArrayList<Double>();

        mSensorManager = (SensorManager) mAppCtx.getSystemService(Context.SENSOR_SERVICE);
        mSensorAccelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

        mCurrentState = STATE_PLAYING_TUTORIAL;
        initHandler();

        return view;
    }

    private void initHandler() {
        mTimeOutAction = new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "adjust time out, we doing adjust ... ");
               // onDetectDeviceNoShake();
                switchState(STATE_ADJUST_FAILED);
            }
        };
        mUIHandler = new Handler(Looper.getMainLooper());
    }

    @Override
    protected void destroyView() {
         Log.d(TAG, "adjust destroyView ... ");
        if (mIsTutorialVideoReady) {
            mVideoView.stopPlayback();
        }

        mSensorManager.unregisterListener(this);
        mUIHandler.removeCallbacks(mTimeOutAction);
    }

    @Override
    public String getTitle() {
        return mAppCtx.getString(R.string.BasicAdjust);
    }

    @Override
    public void onConfigChange(Configuration config) {
        if (!isInit()) {
            return;
        }

        // reload tutorial view
        if (mIsTutorialVideoReady) {
            mVideoView.stopPlayback();
        }
        mIsSetTutorialRes = false;
        mIsTutorialVideoReady = false;
        loadUIRes(config.locale);
    }

    private void loadUIRes(Locale locale) {
        mBtnReplay.setText(R.string.Redemonstrate);
        mBtnStartAdjust.setText(R.string.StartCorrection);
        mSuccessText.setText(R.string.AdjustSuccess);
        mSkip.setText(R.string.Skip);
        mBtnFailStartAdjust.setText(R.string.StartCorrection);
        mCorrectFailed.setText(R.string.AdjustFailure);
        mFailedText.setText(R.string.FailureAnnotations);
    }

    @Override
    public void onPagerSelected() {
        mVideoView.setVisibility(View.VISIBLE);
        switchState(STATE_PLAYING_TUTORIAL);
    }

    @Override
    public void onClick(View view) {
        if (view.equals(mBtnNext)) {
            mHost.goToNextPage();
        } else if (view.equals(mBtnReplay)) {
            Log.d(TAG, "mBtnReplayo ");
            switchState(STATE_PLAYING_TUTORIAL);
        } else if (view.equals(mBtnStartAdjust)) {
            switchState(STATE_DOING_ADJUST);
        }else if(view.equals(mBtnFailStartAdjust)){
            switchState(STATE_DOING_ADJUST);
        }else if(view.equals(mSkip)){
            mHost.goToNextPage();
        }
    }

    @Override
    public void onCompletion(MediaPlayer mp) {
        switchState(STATE_READY_TO_ADJUST);
    }

    @Override
    public void onPrepared(MediaPlayer mp) {
        Log.e(TAG, "onPrepared");
        mIsTutorialVideoReady = true;
        mVideoView.start();
    }

    @Override
    public boolean onError(MediaPlayer mp, int what, int extra) {
        Log.e(TAG, "play video error: what: " + what + ", extra: " + extra);
        switchState(STATE_READY_TO_ADJUST);
        // return false: let onCompletion called
        return false;
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        // call in main thread

        float[] values = event.values;
        double x = values[0];
        double y = values[1];
        double z = values[2];
        //Log.d(TAG, x + ", " + y + ", " + z);

        mCalculateX.add(x);
        mCalculateY.add(y);
        mCalculateZ.add(z);
        mIndex++;

        // sample count
        if (mIndex >= SENSOR_SAMPLE_COUNT) {
            // detect shake
            double varianceX = getVariance(mCalculateX);
            double varianceY = getVariance(mCalculateY);
            double varianceZ = getVariance(mCalculateZ);

            // detect position horizontal
            double averageX = getAverage(mCalculateX);
            double averageY = getAverage(mCalculateY);
            double averageZ = getAverage(mCalculateZ);

            boolean isShake = true;
            boolean isHorizontalPos = false;
            if (varianceX <= 0.015 && varianceY <= 0.015 && varianceZ <= 0.015) {
                isShake = false;
            }
            if ((Math.abs(Math.abs(averageZ) - 9.8) < 0.8)
                    // z is 9.8
                  ) {
               // || (Math.abs(averageX - 9.8) < 0.8)
                isHorizontalPos = true;
            }

            Log.d(TAG, "variance: " + varianceX + ", " + varianceY + ", " + varianceZ
                    + ", avr: " + averageX + ", " + averageY + ", " + averageZ
                    + ", isShake: " + isShake + ", isHor: " + isHorizontalPos);

            if (!isShake && isHorizontalPos) {
                onDetectDeviceNoShake();
            } else if(isShake){
                onDetectDeviceSnake();
            }

            mIndex = 0;
            mCalculateX.clear();
            mCalculateY.clear();
            mCalculateZ.clear();
        }

        //Log.d(TAG, "mIndex = " + mIndex);
    }

    private double getVariance(List<Double> list) {
        double sum = 0;
        int count = list.size();
        double average = getAverage(list);
        for (int i = 0; i < count; i++) {
            sum += (list.get(i) - average) * (list.get(i) - average);
        }
        return sum / count;
    }

    private double getAverage(List<Double> list) {
        double sum = 0;
        int count = list.size();
        for (int i = 0; i < count; i++) {
            sum += list.get(i);
        }
        return sum / count;
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
    }

    private void switchState(int state) {
        Log.d(TAG, "switchState: " + state);
        mCurrentState = state;
        switch (state) {
            case STATE_PLAYING_TUTORIAL:
                if (!mIsSetTutorialRes) {
                    mVideoView.setVideoPath(resolveTutorialResPath());
                    mIsSetTutorialRes = true;
                }
                if (mIsTutorialVideoReady) {
                    Log.d(TAG, "PLAY");
                    mVideoView.seekTo(0);
                    mVideoView.start();
                }
                mSuccessView.setVisibility(View.GONE);
                mFailedView.setVisibility(View.GONE);
                mBtnStartAdjust.setVisibility(View.GONE);
                mBtnReplay.setVisibility(View.GONE);
                mBtnNext.setVisibility(View.GONE);
                mLEDDlg.dismiss();
                mVideoContainer.setVisibility(View.VISIBLE);
                break;

            case STATE_READY_TO_ADJUST:
                mSuccessView.setVisibility(View.GONE);
                mFailedView.setVisibility(View.GONE);
                mBtnStartAdjust.setVisibility(View.VISIBLE);
                mBtnReplay.setVisibility(View.VISIBLE);
                mLEDDlg.dismiss();
                break;

            case STATE_DOING_ADJUST:
                mVideoView.setVisibility(View.GONE);
                mLEDColorView.setBackgroundColor(mAppCtx.getColor(R.color.yellow));
                mLEDDlg.show();
                mSensorManager.registerListener(this,
                        mSensorAccelerometer, SENSOR_SAMPLE_RATE);
                // avoid block in sensor adjust ...
                mUIHandler.postDelayed(mTimeOutAction, ADJUST_TIME_OUT);
                break;

            case STATE_ADJUST_SUCCESS:
                mUIHandler.removeCallbacks(mTimeOutAction);
                mLEDColorView.setBackgroundColor(mAppCtx.getColor(R.color.aqua));
                break;

            case STATE_ADJUST_FAILED:
                mUIHandler.removeCallbacks(mTimeOutAction);
                mLEDColorView.setBackgroundColor(mAppCtx.getColor(R.color.red));
                break;

            case STATE_ADJUST_DONE_SUCCESS:
                mLEDDlg.dismiss();
                mVideoContainer.setVisibility(View.GONE);
                mSuccessView.setVisibility(View.VISIBLE);
                mFailedView.setVisibility(View.GONE);
                mBtnNext.setVisibility(View.VISIBLE);
                mBtnStartAdjust.setVisibility(View.GONE);
                mBtnReplay.setVisibility(View.GONE);
                mSensorManager.unregisterListener(this);
                break;

            case STATE_ADJUST_DONE_FAILED:
                mLEDDlg.dismiss();
                mVideoContainer.setVisibility(View.GONE);
                mSuccessView.setVisibility(View.GONE);
                mFailedView.setVisibility(View.VISIBLE);
                mBtnNext.setVisibility(View.GONE);
                if(mFailTime==0){
                    mLlayoutFailOnetime.setVisibility(View.VISIBLE);
                    mLlayoutFailSecondtime.setVisibility(View.GONE);
                    mBtnReplay.setOnClickListener(new OnClickListener() {
                        @Override
                        public void onClick(View view) {
                            Log.d(TAG, "mBtnReplayonClick: ");
                            mFailedView.setVisibility(View.GONE);
                            mVideoView.setVisibility(View.VISIBLE);
                            switchState(STATE_PLAYING_TUTORIAL);
                        }
                    });
                    // mBtnReplay.setVisibility(View.VISIBLE)；
                }
                else{
                    mLlayoutFailOnetime.setVisibility(View.GONE);
                    mLlayoutFailSecondtime.setVisibility(View.VISIBLE);
                    mFailedText.setText(R.string.FailureSecondAnnotations);
                }
                mFailTime++;
                mSensorManager.unregisterListener(this);
                break;

            default:
                break;
        }
    }

    private void onDetectDeviceSnake() {
        Log.d(TAG, "shake, state: " + mCurrentState);
        if (STATE_ADJUST_SUCCESS == mCurrentState) {
            switchState(STATE_ADJUST_DONE_SUCCESS);
        } else if (STATE_ADJUST_FAILED == mCurrentState) {
            switchState(STATE_ADJUST_DONE_FAILED);
        }
    }

    private void onDetectDeviceNoShake() {
        Log.d(TAG, "no shake, state: " + mCurrentState);
        if (STATE_DOING_ADJUST == mCurrentState) {
            // do sensor calibrate
            doSensorCalibrate();
            // the sensor hub cmd don't return, so we think sensor calibrate always success
            switchState(STATE_ADJUST_SUCCESS);
        }
    }

    private void doSensorCalibrate() {
        boolean isNeed = mAppCtx.getResources().getBoolean(R.bool.need_sensor_calibrate);
        if (isNeed) {
            Log.d(TAG, "do a real sensor calibrate ... ");
            SensorCalibrateUtils.nanoappCalibrate(SensorCalibrateUtils.SENSOR_GYRO);
            SensorCalibrateUtils.nanoappCalibrate(SensorCalibrateUtils.SENSOR_ACCEL);
            SensorCalibrateUtils.nanoappCalibrate(SensorCalibrateUtils.SENSOR_MAG);
        } else {
            Log.d(TAG, "just a fake sensor calibrate ... ");
        }
    }

    private String resolveTutorialResPath() {
        Locale locale = Utils.getDefaultLocale();
        int resId = R.raw.calibrate_tutorial;
        if (Utils.isCN(locale)) {
            resId = R.raw.calibrate_tutorial_cn;
        }
        return "android.resource://" + mAppCtx.getPackageName() + "/" + resId;
    }

}
