package com.allwinner.startupGuide.View;

import com.allwinner.startupGuide.R;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ScrollView;
import android.view.View.OnClickListener;

public class ButtonScrollView extends ScrollView implements OnClickListener{
    private ImageButton downBtn;
    private ImageButton upBtn;
    private int screenHeight;
    private boolean isScrolledToTop = true;
    private boolean isScrolledToBottom = false;
    public ButtonScrollView(Context context, AttributeSet attrs)
    {
        super(context, attrs);
    }

    public void setScrollListener(ImageButton mDownbtn,ImageButton mUpbtn) {
        this.downBtn = mDownbtn;
        this.downBtn.setOnClickListener(this);
        this.upBtn = mUpbtn;
        this.upBtn.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.downbtn) {
            this.arrowScroll(View.FOCUS_DOWN);

        }
        if (v.getId() == R.id.upbtn) {
            this.arrowScroll(View.FOCUS_UP);
        }
    }

    @SuppressLint("NewApi") @Override
    protected void onOverScrolled(int scrollX, int scrollY, boolean clampedX, boolean clampedY) {
        super.onOverScrolled(scrollX, scrollY, clampedX, clampedY);
        if (scrollY == 0) {
            isScrolledToTop = clampedY;
            isScrolledToBottom = false;
        } else {
            isScrolledToTop = false;
            isScrolledToBottom = clampedY;
        }
        notifyScrollChangedListeners();
    }

    @Override
    protected void onScrollChanged(int l, int t, int oldl, int oldt) {
        super.onScrollChanged(l, t, oldl, oldt);
        Log.e("getScrollY()", ":"+getScrollY());
        if (getScrollY() == 0) {
            isScrolledToTop = true;
            isScrolledToBottom = false;
        } else if (getScrollY() + getHeight() - getPaddingTop()-getPaddingBottom() == getChildAt(0).getHeight()) {
            isScrolledToBottom = true;
            isScrolledToTop = false;
        } else {
            isScrolledToTop = false;
            isScrolledToBottom = false;
        }
        notifyScrollChangedListeners();
    }

    private void notifyScrollChangedListeners() {
        if (isScrolledToTop) {
            Log.e("isScrolledToTop", ":+isScrolledToTop");
            this.upBtn.setEnabled(false);
        } else if (isScrolledToBottom) {
            Log.e("isScrolledToBottom", ":+isScrolledToBottom");
            this.downBtn.setEnabled(false);
        } else{
            this.upBtn.setEnabled(true);
            this.downBtn.setEnabled(true);
        }
    }

    public boolean isScrolledToTop() {
        return isScrolledToTop;
    }

    public boolean isScrolledToBottom() {
        return isScrolledToBottom;
    }
}
