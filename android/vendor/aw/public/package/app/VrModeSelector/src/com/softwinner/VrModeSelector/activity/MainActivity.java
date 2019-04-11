package com.softwinner.VrModeSelector.activity;

import android.content.Intent;
import android.graphics.Paint;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.CheckBox;
import android.widget.TextView;

import com.softwinner.VrModeSelector.R;
import com.softwinner.VrModeSelector.utils.DebugUtils;
import com.softwinner.VrModeSelector.utils.JavaUtils;

public class MainActivity extends BaseActivity implements View.OnClickListener {

    private final static String TAG = DebugUtils.TAG;

    private int mCurrentMode;

    private CheckBox mBtnVrMode;
    private CheckBox mBtnNormalMode;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initData();
        initView();
    }

    private void initData() {
    }

    private void initView() {
        mBtnVrMode = (CheckBox) findViewById(R.id.btn_vr_mode);
        mBtnNormalMode = (CheckBox) findViewById(R.id.btn_normal_mode);

        mBtnVrMode.setChecked(true);
        mBtnNormalMode.setChecked(false);

        mBtnVrMode.setOnClickListener(this);
        mBtnNormalMode.setOnClickListener(this);
    }

    private void adjustTextPos(TextView textView) {
        // this just match single line ... multi line will use leading ...
        Paint.FontMetricsInt metrics = textView.getPaint().getFontMetricsInt();
        int fontTopDis = Math.abs(metrics.top - metrics.ascent);
        //int fontBottomDis = Math.abs(metrics.bottom - metrics.descent);
        textView.setPadding(0, -fontTopDis, 0, 0);
    }

    @Override
    protected void onResume() {
        Log.i(TAG, "onResume");
        super.onResume();

        updateCurrentMode();
    }

    @Override
    protected void onNewIntent(Intent intent) {
        Log.i(TAG, "onNewIntent");
        super.onNewIntent(intent);

        updateCurrentMode();
    }

    @Override
    protected void onDestroy() {
        Log.i(TAG, "onDestroy");
        super.onDestroy();
    }

    private void updateCurrentMode() {
        mCurrentMode = JavaUtils.displayCtrl(0, JavaUtils.CMD_GET, 0, 0);
        Log.i(TAG, "current vr mode: " + mCurrentMode);

        if (JavaUtils.MODE_VR == mCurrentMode) {
            mBtnVrMode.setChecked(true);
            mBtnNormalMode.setChecked(false);
        } else if (JavaUtils.MODE_NORMAL == mCurrentMode) {
            mBtnVrMode.setChecked(false);
            mBtnNormalMode.setChecked(true);
        }
    }

    @Override
    public void onClick(View view) {
        if (view.equals(mBtnVrMode)) {
            JavaUtils.displayCtrl(0, JavaUtils.CMD_SET, JavaUtils.MODE_VR, 0);
            mBtnVrMode.setChecked(true);
            mBtnNormalMode.setChecked(false);
            finish();
        } else if (view.equals(mBtnNormalMode)) {
            JavaUtils.displayCtrl(0, JavaUtils.CMD_SET, JavaUtils.MODE_NORMAL, 0);
            mBtnVrMode.setChecked(false);
            mBtnNormalMode.setChecked(true);
            finish();
        }
    }

}
