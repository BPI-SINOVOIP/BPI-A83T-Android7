package com.allwinner.startupGuide;

import android.content.res.Resources;
import android.os.Bundle;
import android.app.Activity;
import android.content.res.Configuration;
import android.support.v4.view.ViewPager;
import android.support.v4.view.ViewPager.OnPageChangeListener;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.TextView;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.Intent;
import android.content.ComponentName;
import android.provider.Settings;

import com.allwinner.startupGuide.Pager.AdjustPager;
import com.allwinner.startupGuide.Pager.BasePager;
import com.allwinner.startupGuide.Pager.LanguagePager;
import com.allwinner.startupGuide.Pager.ProtocolPager;
import com.allwinner.startupGuide.View.SlidingTabLayout;
import com.allwinner.startupGuide.utils.Utils;
import java.lang.reflect.Method;
import static com.allwinner.startupGuide.utils.Utils.get;
import java.util.ArrayList;

public class MainActivity extends Activity implements BasePager.PagerHost, OnPageChangeListener, SlidingTabLayout.TabColorizer {

    private static final String TAG = Utils.TAG;
    private static final String LAUNCHER_PKG = "com.softwinner.VrLaunch";

    private View mRootView;
    private ViewPager mViewPager;
   // private TabPageIndicator mIndicator;
    private SlidingTabLayout mTabLayout = null;
    private MenuDetailAdapter mPageAdapter;
    private ArrayList<BasePager> mPagerList;
    private int mTabSelectedColor = 0;
    private int mTabUnSelectedColor = 0;
    SharedPreferences mPreferences;
    private static final String KEY_FORCELANDSCAPE = "ro.sys.vr.forcelandscape";
   // private static Method getStringMethod = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate ... ");

        mPreferences = getSharedPreferences("count",MODE_PRIVATE);
        int count =mPreferences.getInt("count", 0);
        if (count == 1) {
        Log.d(TAG, "count == 1");
        Settings.Global.putInt(getContentResolver(), Settings.Global.DEVICE_PROVISIONED, 1);
        Settings.Secure.putInt(getContentResolver(), "user_setup_complete", 1);
        PackageManager pm = getPackageManager();
        ComponentName self = new ComponentName(this, com.allwinner.startupGuide.MainActivity.class);
        pm.setComponentEnabledSetting(self,
            PackageManager.COMPONENT_ENABLED_STATE_DISABLED,
            PackageManager.DONT_KILL_APP);
        PackageManager packageManager = getPackageManager();
        // TODO: may be we should resolve by system
        Intent intent = packageManager.getLaunchIntentForPackage(LAUNCHER_PKG);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK |
            Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED |
            Intent.FLAG_ACTIVITY_CLEAR_TOP) ;
        startActivity(intent);
        finish();
        }
        Log.d(TAG, "count != 1");
        String forcelandscape=get(KEY_FORCELANDSCAPE,"0");
        Log.d(TAG,forcelandscape);
        DisplayMetrics dm = new DisplayMetrics();

        getWindowManager().getDefaultDisplay().getMetrics(dm);

        Log.d(TAG, "dpi=" + dm.densityDpi + ", den=" + dm.density);

        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.activity_main);

        initData();
        initView();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        Log.d(TAG, "onConfigurationChanged .... ");
        super.onConfigurationChanged(newConfig);


        // let main ui draw, otherwise our disableDirectBootBlock maybe let boot complete flag not set !!
        /*if (Configuration.ORIENTATION_LANDSCAPE != newConfig.orientation) {
            Log.d(TAG, "current orientation is not land, hide all ui");
            mRootView.setVisibility(View.GONE);
            return;
        } else {
            Log.d(TAG, "current orientation is land, show all ui");
            mRootView.setVisibility(View.VISIBLE);
        }*/
        updateTabText();
        for (BasePager pager : mPagerList) {
            pager.onConfigChange(newConfig);
            mPageAdapter.notifyDataSetChanged();
        }
      //  mIndicator.notifyDataSetChanged();
    }

    private void initData() {
        mPagerList = new ArrayList<BasePager>();
    }

    private void initView() {
      //  mIndicator = (TabPageIndicator)findViewById(R.id.indicator);
        mTabLayout = (SlidingTabLayout) findViewById(R.id.tab_layout);
        mViewPager = (ViewPager) findViewById(R.id.vp_menu_detail);
        mRootView = findViewById(R.id.root_view);

        // TODO: now we have 3 pages ...
        mViewPager.setOffscreenPageLimit(3);
        Resources res = getResources();
        mTabSelectedColor = res.getColor(R.color.tabselectcolor);
        mTabUnSelectedColor = res.getColor(R.color.tabcolor);
        // in display order
        mPagerList.add(new LanguagePager(this, this));
        mPagerList.add(new AdjustPager(this, this));
        mPagerList.add(new ProtocolPager(this, this));

        mPageAdapter = new MenuDetailAdapter(mPagerList);
        mViewPager.setAdapter(mPageAdapter);
//        mIndicator.setViewPager(mViewPager);
//        mIndicator.setOnPageChangeListener(this);
        mTabLayout.setDistributeEvenly(true);
        mTabLayout.setCustomTabView(R.layout.theme_tab_view, R.id.tab_title);
        mTabLayout.setCustomTabColorizer(this);
        mTabLayout.setBottomBorderColor(mTabUnSelectedColor);
        mTabLayout.setViewPager(mViewPager);
        mTabLayout.setOnPageChangeListener(this);
        // disable click event
        //mTabLayout.setEnabled(false);
        //mTabLayout.getTabStrip().setEnabled(false);
        ViewGroup viewGroup = (ViewGroup) mTabLayout.getTabStrip();
        for (int i = 0; i < viewGroup.getChildCount(); i++) {
            View childView = viewGroup.getChildAt(i);
            childView.setEnabled(false);
        }
        updateTabTitle(0);

        // let main ui draw, otherwise our disableDirectBootBlock maybe let boot complete flag not set !!
        /*if (Configuration.ORIENTATION_LANDSCAPE == getResources().getConfiguration().orientation) {
            Log.d(TAG, "current orientation is land, show all ui");
            mRootView.setVisibility(View.VISIBLE);
        } else {
            Log.d(TAG, "current orientation is not land, hide all ui");
            mRootView.setVisibility(View.GONE);
        }*/
    }
    private void updateTabTitle(int position) {
        if (null == mTabLayout) {
            return;
        }
        try {
            TextView titleView = null;
            ViewGroup tabStrip = (ViewGroup) mTabLayout.getTabStrip();
            for (int i = 0; i < tabStrip.getChildCount(); i++) {
                titleView = (TextView) tabStrip.getChildAt(i);
              //  titleView.setText(mPagerList.get(i).getTitle());
                if (position == i) {
                     Log.d("position", position+" ");
                    titleView.setTextColor(mTabSelectedColor);
                }  else {
                    titleView.setTextColor(mTabUnSelectedColor);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    private void updateTabText() {
        if (null == mTabLayout) {
            return;
        }
        try {
            TextView titleView = null;
            ViewGroup tabStrip = (ViewGroup) mTabLayout.getTabStrip();
            for (int i = 0; i < tabStrip.getChildCount(); i++) {
                titleView = (TextView) tabStrip.getChildAt(i);
                titleView.setText(mPagerList.get(i).getTitle());

            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy ... ");
        super.onDestroy();

        mPagerList.clear();
        mPageAdapter.notifyDataSetChanged();
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        Log.d(TAG, "dispatchKeyEvent: keyCode: " + event.getKeyCode() + ", action: " + event.getAction());
        int keyCode = event.getKeyCode();
        // disable dpad input to swap
        // TODO: now vr desktop controller don't support left, right swap
        if (KeyEvent.KEYCODE_DPAD_LEFT == keyCode ||
            KeyEvent.KEYCODE_DPAD_RIGHT == keyCode ||
            // also intercept back key
            KeyEvent.KEYCODE_BACK == keyCode) {
            return true;
        }
        return super.dispatchKeyEvent(event);
    }
    @Override
    public void goToPrevPage() {
        int current = mViewPager.getCurrentItem();
        if (current > 0) {
            mViewPager.setCurrentItem(current - 1, false);
        }
    }

    @Override
    public void goToNextPage() {
        int current = mViewPager.getCurrentItem();
        if (current < mPageAdapter.getCount() - 1) {
            mViewPager.setCurrentItem(current + 1, false);
        }
    }

    @Override
    public void finishAndExit() {
        finish();
    }

    @Override
    public void onPageScrollStateChanged(int state) {
    }

    @Override
    public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {
    }

    @Override
    public void onPageSelected(int position) {
        Log.d(TAG, "onPageSelected: " + position);
        updateTabTitle(position);
        BasePager pager = mPagerList.get(position);
        pager.initPager();
        pager.onPagerSelected();
    }

    @Override
    public int getIndicatorColor(int position) {
        return mTabSelectedColor;
    }
}
