/*
 * Copyright 2016 Hippo Seven
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.hippo.image.example;

/*
 * Created by Hippo on 8/19/2016.
 */

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.ColorFilter;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.drawable.Animatable;
import android.graphics.drawable.AnimationDrawable;
import android.graphics.drawable.Drawable;
import android.os.SystemClock;
import android.support.annotation.NonNull;

public class ImageDrawable extends Drawable implements Animatable, Runnable {

    private ImageBitmap mImageBitmap;
    private Paint mPaint;

    /** Whether the drawable has an animation callback posted. */
    private boolean mRunning;

    /** Whether the drawable should animate when visible. */
    private boolean mAnimating;

    public ImageDrawable(@NonNull ImageBitmap imageBitmap) {
        mImageBitmap = imageBitmap;
        mPaint = new Paint(Paint.FILTER_BITMAP_FLAG | Paint.DITHER_FLAG);
    }

    public ImageBitmap getImageBitmap() {
        return mImageBitmap;
    }

    @Override
    public boolean setVisible(boolean visible, boolean restart) {
        final boolean changed = super.setVisible(visible, restart);
        if (mImageBitmap.isAnimated()) {
            if (visible) {
                if (restart || changed) {
                    setFrame(!(restart || !mRunning), true, mAnimating);
                }
            } else {
                unscheduleSelf(this);
            }
        }
        return changed;
    }

    /**
     * Starts the animation, looping if necessary. This method has no effect
     * if the animation is running.
     * <p>
     * <strong>Note:</strong> Do not call this in the
     * {@link android.app.Activity#onCreate} method of your activity, because
     * the {@link AnimationDrawable} is not yet fully attached to the window.
     * If you want to play the animation immediately without requiring
     * interaction, then you might want to call it from the
     * {@link android.app.Activity#onWindowFocusChanged} method in your
     * activity, which will get called when Android brings your window into
     * focus.
     *
     * @see #isRunning()
     * @see #stop()
     */
    @Override
    public void start() {
        mAnimating = true;

        if (mImageBitmap.isAnimated() && !isRunning()) {
            // Start from 0th frame.
            setFrame(false, false, true);
        }
    }

    /**
     * Stops the animation. This method has no effect if the animation is not
     * running.
     *
     * @see #isRunning()
     * @see #start()
     */
    @Override
    public void stop() {
        mAnimating = false;

        if (mImageBitmap.isAnimated() && isRunning()) {
            unscheduleSelf(this);
        }
    }

    /**
     * Indicates whether the animation is currently running or not.
     *
     * @return true if the animation is running, false otherwise
     */
    @Override
    public boolean isRunning() {
        return mRunning;
    }

    /**
     * This method exists for implementation purpose only and should not be
     * called directly. Invoke {@link #start()} instead.
     *
     * @see #start()
     */
    @Override
    public void run() {
        setFrame(true, false, true);
    }

    private void setFrame(boolean resetOrNext, boolean unschedule, boolean animate) {
        mAnimating = animate;
        if (resetOrNext) {
            mImageBitmap.advance();
        } else  {
            mImageBitmap.reset();
        }
        invalidateSelf();
        if (unschedule || animate) {
            unscheduleSelf(this);
        }
        if (animate) {
            mRunning = true;
            scheduleSelf(this, SystemClock.uptimeMillis() + mImageBitmap.getCurrentDelay());
        }
    }

    @Override
    public void unscheduleSelf(@NonNull Runnable what) {
        mRunning = false;
        super.unscheduleSelf(what);
    }

    @Override
    public void draw(@NonNull Canvas canvas) {
        final Bitmap bitmap = mImageBitmap.getBitmap();
        if (bitmap != null) {
            canvas.drawBitmap(bitmap, null, getBounds(), mPaint);
        }
    }

    @Override
    public void setAlpha(int alpha) {
        mPaint.setAlpha(alpha);
    }

    @Override
    public void setColorFilter(ColorFilter colorFilter) {
        mPaint.setColorFilter(colorFilter);
    }

    @Override
    public int getOpacity() {
        return mImageBitmap.isOpaque() ? PixelFormat.OPAQUE : PixelFormat.TRANSLUCENT;
    }

    @Override
    public int getIntrinsicWidth() {
        return mImageBitmap.getWidth();
    }

    @Override
    public int getIntrinsicHeight() {
        return mImageBitmap.getHeight();
    }
}
