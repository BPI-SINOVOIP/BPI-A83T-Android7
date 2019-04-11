package com.allwinner.startupGuide.Pager;

import java.lang.reflect.Method;
import java.util.Locale;

import android.annotation.TargetApi;
import android.content.Context;
import android.content.res.Configuration;
import android.os.LocaleList;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageButton;

import com.allwinner.startupGuide.R;
import com.allwinner.startupGuide.utils.Utils;

public class LanguagePager extends BasePager implements OnClickListener {

    private static final String TAG = Utils.TAG;

    private Button mBtnCN;
    private Button mBtnEN;
    private ImageButton mBtnNext;

    public LanguagePager(Context context, PagerHost host) {
        super(context, host);
    }

    @Override
    protected View initView() {
        View rootView = View.inflate(mAppCtx, R.layout.language_pager, null);

        mBtnCN = (Button) rootView.findViewById(R.id.btn_cn);
        mBtnEN = (Button) rootView.findViewById(R.id.btn_en);
        mBtnNext = (ImageButton) rootView.findViewById(R.id.btn_next);

        mBtnCN.setOnClickListener(this);
        mBtnEN.setOnClickListener(this);
        mBtnNext.setOnClickListener(this);

        Locale defaultLocale = Utils.getDefaultLocale();
        loadUIRes(defaultLocale);

        if (Utils.isCN(defaultLocale)) {
            mBtnCN.setEnabled(false);
            mBtnEN.setEnabled(true);
        } else {
            mBtnCN.setEnabled(true);
            mBtnEN.setEnabled(false);
        }

        Log.d(TAG, "w=" + mBtnCN.getWidth() + ", h=" + mBtnCN.getHeight());

        return rootView;
    }

    @Override
    protected void destroyView() {
        // do noting
    }

    @Override
    public String getTitle() {
        return mAppCtx.getString(R.string.LanguageSetting);
    }

    @Override
    public void onConfigChange(Configuration config) {
        if (!isInit()) {
            return;
        }
        loadUIRes(config.locale);
    }

    @Override
    public void onClick(View view) {
        if (view.equals(mBtnCN)) {
            switchLanguage(true);
        } else if (view.equals(mBtnEN)) {
            switchLanguage(false);
        } else if (view.equals(mBtnNext)) {
            mHost.goToNextPage();
        }
    }

    private void loadUIRes(Locale locale) {
        mBtnCN.setText(mAppCtx.getString(R.string.Chinese));
        mBtnEN.setText(mAppCtx.getString(R.string.English));
    }

    @TargetApi(25)
    private void switchLanguage(boolean isCN) {
        if (isCN) {
            mBtnCN.setEnabled(false);
            mBtnEN.setEnabled(true);
        } else {
            mBtnCN.setEnabled(true);
            mBtnEN.setEnabled(false);
        }

        try {
            // need system app permission
            LocaleList objIActMag;
            Class clzActMagNative = Class.forName("com.android.internal.app.LocalePicker");
            Method mtdActMagNative$getLocales = clzActMagNative.getDeclaredMethod("getLocales");
            objIActMag = (LocaleList)mtdActMagNative$getLocales.invoke(clzActMagNative);
            Locale[] newList = new Locale[1];
            if (isCN) {
                //newList[0]=Locale.CHINESE;
                newList[0]=Locale.CHINA;
            } else {
                newList[0]=Locale.US;
            }
            LocaleList ll = new LocaleList(newList);
            Class[] clzParams = { LocaleList.class };
            Method mtdIActMag$updateLocales = clzActMagNative.getDeclaredMethod(
                "updateLocales", clzParams);
            mtdIActMag$updateLocales.invoke(objIActMag, ll);
        } catch (Exception e) {
            Log.e(TAG, "error: ", e);
        }
    }

}
