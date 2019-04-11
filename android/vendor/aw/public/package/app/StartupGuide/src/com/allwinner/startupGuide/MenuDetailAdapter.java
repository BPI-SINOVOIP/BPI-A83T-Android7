package com.allwinner.startupGuide;

import java.util.ArrayList;

import android.support.v4.view.PagerAdapter;
import android.view.View;
import android.view.ViewGroup;

import com.allwinner.startupGuide.Pager.BasePager;
import com.allwinner.startupGuide.utils.Utils;

public class MenuDetailAdapter extends PagerAdapter {

    private static final String TAG = Utils.TAG;

    private ArrayList<BasePager> mPagerList;

    public MenuDetailAdapter(ArrayList<BasePager> pagerList) {
        mPagerList = pagerList;
    }

    @Override
    public CharSequence getPageTitle(int position) {
        return mPagerList.get(position).getTitle();
    }

    @Override
    public int getCount() {
        return mPagerList.size();
    }

    @Override
    public boolean isViewFromObject(View view, Object object) {
        BasePager pager = (BasePager) object;
        return view.equals(pager.getRootView());
    }

    @Override
    public Object instantiateItem(ViewGroup container, int position) {
        BasePager pager = mPagerList.get(position);
        pager.initPager();
        container.addView(pager.getRootView());
        return pager;
    }

    @Override
    public void destroyItem(ViewGroup container, int position, Object object) {
        BasePager pager = (BasePager) object;
        container.removeView(pager.getRootView());
        pager.destroyPager();
    }

    @Override
    public int getItemPosition(Object object) {
        BasePager item = (BasePager) object;
        for (int i = 0; i < mPagerList.size(); i++) {
            if (item.equals(mPagerList.get(i))) {
                return i;
            }
        }
        return POSITION_NONE;
    }

}
