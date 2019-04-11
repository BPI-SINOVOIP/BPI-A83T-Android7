package com.allwinner.startupGuide.Pager;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.TextView;

import android.content.ComponentName;
import android.provider.Settings;

import com.allwinner.startupGuide.R;
import com.allwinner.startupGuide.View.ButtonScrollView;
import com.allwinner.startupGuide.utils.Utils;
import android.content.SharedPreferences;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.util.Locale;

public class ProtocolPager extends BasePager implements OnClickListener {

    private static final String TAG = Utils.TAG;

    private static final String LAUNCHER_PKG = "com.softwinner.VrLaunch";

    private TextView mTvProtocol;
    private TextView mTvLabelProtocol;
    private TextView mTvLabelAgree;
    private Button mBtnStartUse;
    private ButtonScrollView mScrollView;
    SharedPreferences mPreferences;


    public ProtocolPager(Context context, PagerHost host) {
        super(context, host);
    }

    @Override
    protected View initView() {
        View view = View.inflate(mAppCtx, R.layout.protocol_pager, null);
        mTvProtocol = (TextView) view.findViewById(R.id.text);
        mTvLabelProtocol = (TextView) view.findViewById(R.id.tv_label_protocol);
        mTvLabelAgree = (TextView) view.findViewById(R.id.tv_label_agree);
        mBtnStartUse = (Button) view.findViewById(R.id.startlauncher);
        mScrollView = (ButtonScrollView) view.findViewById(R.id.scrollView);
        mPreferences = mAppCtx.getSharedPreferences("count",0);

        ImageButton btnUp = (ImageButton) view.findViewById(R.id.upbtn);
        ImageButton btnDown = (ImageButton) view.findViewById(R.id.downbtn);
        mScrollView.setScrollListener(btnDown, btnUp);
        mBtnStartUse.setOnClickListener(this);

        Locale defaultLocale = Utils.getDefaultLocale();
        loadUIRes(defaultLocale);

        return view;
    }

    @Override
    protected void destroyView() {
    }

    @Override
    public void onConfigChange(Configuration config) {
        if (!isInit()) {
            return;
        }
        loadUIRes(config.locale);
    }

    private void loadUIRes(Locale locale) {
        int resId = R.raw.protocol;
        if (Utils.isCN(locale)) {
            resId = R.raw.protocol_cn;
        }
        InputStream is = mAppCtx.getResources().openRawResource(resId);
        mTvProtocol.setText(loadProtocol(is));
        if (null != is) {
            try {
                is.close();
            } catch (Exception e) {
                Log.e(TAG, "error", e);
            }
        }

        mBtnStartUse.setText(R.string.StartUsing);
        mTvLabelProtocol.setText(R.string.UserProtocol);
        mTvLabelAgree.setText(R.string.bottomtext);
    }

    @Override
    public String getTitle() {
        return mAppCtx.getString(R.string.UserProtocol);
    }

    @Override
    public void onClick(View view) {
        if (view.equals(mBtnStartUse)) {
            onBtnStartUse();
        }
    }

    private void onBtnStartUse() {
        // set device_provisioned as device ready to use
        Settings.Global.putInt(mAppCtx.getContentResolver(), Settings.Global.DEVICE_PROVISIONED, 1);
        Settings.Secure.putInt(mAppCtx.getContentResolver(), "user_setup_complete", 1);

        int count=0;
        SharedPreferences.Editor editor = mPreferences.edit();
        editor.putInt("count", ++count);
        editor.commit();
        // remove this activity from the package manager.
        PackageManager pm = mAppCtx.getPackageManager();
        ComponentName self = new ComponentName(mAppCtx, com.allwinner.startupGuide.MainActivity.class);
        pm.setComponentEnabledSetting(self,
            PackageManager.COMPONENT_ENABLED_STATE_DISABLED,
            PackageManager.DONT_KILL_APP);
       // mAppCtx.sendBroadcast(new Intent("com.google.android.setupwizard.SETUP_WIZARD_FINISHED"));

        // launch real launcher
        PackageManager packageManager = mAppCtx.getPackageManager();
        // TODO: may be we should resolve by system
        Intent intent = packageManager.getLaunchIntentForPackage(LAUNCHER_PKG);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK |
            Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED |
            Intent.FLAG_ACTIVITY_CLEAR_TOP) ;
        mAppCtx.startActivity(intent);

        mHost.finishAndExit();
    }

    private String loadProtocol(InputStream inputStream) {
        InputStreamReader inputStreamReader = null;
        try {
            inputStreamReader = new InputStreamReader(inputStream, "utf-8");
        } catch (UnsupportedEncodingException e1) {
            Log.e(TAG, "error: ", e1);
            return null;
        }

        BufferedReader reader = new BufferedReader(inputStreamReader);
        StringBuffer sb = new StringBuffer("");
        String line;
        try {
            while ((line = reader.readLine()) != null) {
                sb.append(line);
                sb.append("\n");
            }
        } catch (IOException e) {
            Log.e(TAG, "error: ", e);
        }
        return sb.toString();
    }

}
