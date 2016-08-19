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
import android.support.annotation.Nullable;

import com.hippo.image.ImageData;
import com.hippo.image.ImageRenderer;

/**
 * This class uses {@link Bitmap} to represent {@link ImageRenderer}.
 * Call {@link #getBitmap()} to get the bitmap.
 */
public class ImageBitmap {

    private int mRatio;
    private int mWidth;
    private int mHeight;
    private int mFormat;
    private boolean mOpaque;
    private int mFrameCount;
    private int[] mDelayArray;
    private int mByteCount;

    @Nullable
    private Bitmap mBitmap;
    @Nullable
    private ImageRenderer mImageRenderer;

    /**
     * The ImageData must be completed and
     * ratio <= width && ratio <= height, or throw
     * IllegalStateException.
     */
    public ImageBitmap(ImageData imageData, int ratio) {
        // Only completed image supported
        if (!imageData.isCompleted()) {
            throw new IllegalStateException("ImageBitmap can only handle completed ImageData");
        }

        int width = imageData.getWidth();
        int height = imageData.getHeight();
        if (ratio > width || ratio > height) {
            throw new IllegalStateException("Ratio is too big");
        }

        mRatio = ratio;
        mWidth = width / ratio;
        mHeight = height / ratio;
        mFormat = imageData.getFormat();
        mOpaque = imageData.isOpaque();
        mFrameCount = imageData.getFrameCount();
        mByteCount = imageData.getByteCount();
        mDelayArray = new int[mFrameCount];
        for (int i = 0; i < mFrameCount; i++) {
            mDelayArray[i] = imageData.getDelay(i);
        }
        mBitmap = Bitmap.createBitmap(mWidth, mHeight, Bitmap.Config.ARGB_8888);

        // Render first frame
        ImageRenderer imageRenderer = imageData.createImageRenderer();
        imageRenderer.reset();
        imageRenderer.render(mBitmap, 0, 0, 0, 0, width, height, ratio, false, 0);

        if (mFrameCount == 1) {
            // Recycle image renderer if it is not animated
            imageRenderer.recycle();
        } else {
            // Store image renderer if it is animated
            mImageRenderer = imageRenderer;
        }
    }

    /**
     * Recycle the Bitmap and ImageRenderer.
     */
    public void recycle() {
        if (mBitmap != null) {
            mBitmap.recycle();
            mBitmap = null;
        }
        if (mImageRenderer != null) {
            mImageRenderer.recycle();
            mBitmap = null;
        }
    }

    /**
     * Draw first frame to bitmap.
     */
    public void reset() {
        if (mBitmap != null && mImageRenderer != null) {
            mImageRenderer.reset();
            mImageRenderer.render(mBitmap, 0, 0, 0, 0, mWidth, mHeight, mRatio, false, 0);
        }
    }

    /**
     * Draw next frame to bitmap.
     */
    public void advance() {
        if (mBitmap != null && mImageRenderer != null) {
            mImageRenderer.advance();
            mImageRenderer.render(mBitmap, 0, 0, 0, 0, mWidth, mHeight, mRatio, false, 0);
        }
    }

    /**
     * Get the Bitmap.
     */
    @Nullable
    public Bitmap getBitmap() {
        return mBitmap;
    }

    /**
     * Return the ratio.
     */
    public int getRatio() {
        return mRatio;
    }

    /**
     * Return the width of the Bitmap.
     */
    public int getWidth() {
        return mWidth;
    }

    /**
     * Return the height of the Bitmap.
     */
    public int getHeight() {
        return mHeight;
    }

    /**
     * Return the format of the ImageData.
     */
    public int getFormat() {
        return mFormat;
    }

    /**
     * Return true if the ImageData is opaque.
     */
    public boolean isOpaque() {
        return mOpaque;
    }

    /**
     * Return the frame count of the ImageData.
     */
    public int getFrameCount() {
        return mFrameCount;
    }

    /**
     * Return true if it is animated image
     */
    public boolean isAnimated() {
        return mFrameCount > 1;
    }

    /**
     * Return the delay of the frame.
     */
    public int getDelay(int frame) {
        return mDelayArray[frame];
    }

    /**
     * Return delay of current delay.
     * Return 0 if current frame is invalid.
     */
    public int getCurrentDelay() {
        if (mImageRenderer != null) {
            return mImageRenderer.getCurrentDelay();
        } else {
            return Integer.MAX_VALUE;
        }
    }

    /**
     * Return the byte count of the ImageData.
     */
    public int getByteCount() {
        return mByteCount;
    }
}
