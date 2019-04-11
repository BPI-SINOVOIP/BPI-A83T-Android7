package com.allwinner.startupGuide.Pager;

import android.content.Context;
import android.content.res.Configuration;
import android.view.View;

public abstract class BasePager {

    protected Context mAppCtx;
    protected View mRootView;
    protected PagerHost mHost;

    private boolean mIsInit;

    public interface PagerHost {
        void goToPrevPage();
        void goToNextPage();
        void finishAndExit();
    }

    public BasePager(Context context, PagerHost host) {
        mAppCtx = context.getApplicationContext();
        mHost = host;
        mRootView = null;
        mIsInit = false;
    }

    public final View getRootView() {
        return mRootView;
    }

    public final void initPager() {
        if (!mIsInit) {
            mRootView = initView();
            mIsInit = true;
        }
    }

    public final void destroyPager() {
        destroyView();
        mRootView = null;
        mIsInit = false;
    }

    public abstract String getTitle();

    public abstract void onConfigChange(Configuration config);

    public void onPagerSelected() {}

    protected abstract View initView();

    protected abstract void destroyView();

    protected final boolean isInit() {
        return mIsInit;
    }

}
