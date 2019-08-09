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

package com.hippo.image;

/*
 * Created by Hippo on 9/12/2016.
 */

import android.graphics.Bitmap;
import android.graphics.Rect;
import android.support.annotation.Keep;
import android.support.annotation.Nullable;
import android.util.Log;

import java.io.InputStream;

public final class BitmapRegionDecoder {

    private static final String LOG_TAG = BitmapRegionDecoder.class.getSimpleName();

    private long mNativePtr;
    private final int mWidth;
    private final int mHeight;
    private final int mFormat;
    private final boolean mOpaque;

    private final Object mNativeLock = new Object();

    @Keep
    private BitmapRegionDecoder(long nativePtr, int width, int height,
            int format, boolean opaque) {
        mNativePtr = nativePtr;
        mWidth = width;
        mHeight = height;
        mFormat = format;
        mOpaque = opaque;
    }

    /**
     * Return width of the original image.
     */
    public int getWidth() {
        return mWidth;
    }

    /**
     * Return height of the original image.
     */
    public int getHeight() {
        return mHeight;
    }

    /**
     * Return format of the original image.
     */
    public int getFormat() {
        return mFormat;
    }

    /**
     * Return {@code true} if the original image is opaque.
     */
    public boolean isOpaque() {
        return mOpaque;
    }

    /**
     * config is {@code BitmapDecoder.CONFIG_AUTO}.
     * ratio is {@code 1}.
     *
     * @see #decodeRegion(Rect, int, int)
     */
    @Nullable
    public Bitmap decodeRegion(Rect rect) {
        return decodeRegion(rect, BitmapDecoder.CONFIG_AUTO, 1);
    }

    /**
     * ratio is {@code 1}.
     *
     * @see #decodeRegion(Rect, int, int)
     */
    @Nullable
    public Bitmap decodeRegion(Rect rect, @BitmapDecoder.Config int config) {
        return decodeRegion(rect, config, 1);
    }

    /**
     * Decodes a rectangle region in the image specified by rect.
     *
     * @param rect The rectangle that specified the region to be decode.
     *             Null for decode full image.
     * @param config One of {@link BitmapDecoder#CONFIG_AUTO}, {@link BitmapDecoder#CONFIG_RGB_565}
     *               and {@link BitmapDecoder#CONFIG_RGBA_8888}
     * @param ratio If set to a value > 1, requests the decoder to subsample the original.
     *               image, returning a smaller image to save memory. Power of 2 is not necessary.
     * @return The decoded bitmap, or null if the image data could not be
     *         decoded.
     */
    @Nullable
    public Bitmap decodeRegion(Rect rect, @BitmapDecoder.Config int config, int ratio) {
        synchronized (mNativeLock) {
            if (mNativePtr == 0) {
                Log.e(LOG_TAG, "This region decoder is recycled.");
                return null;
            }

            // Resolve CONFIG_AUTO
            if (config == BitmapDecoder.CONFIG_AUTO) {
                config = mOpaque ? BitmapDecoder.CONFIG_RGB_565 : BitmapDecoder.CONFIG_RGBA_8888;
            }

            if (rect == null || (rect.left == 0 && rect.top == 0 && rect.right == mWidth && rect.bottom == mHeight)) {
                // Requested full image, decode without regions
                return nativeDecodeRegion(mNativePtr, 0, 0, 0, 0, config, ratio);
            } else {
                if (rect.right <= 0 || rect.bottom <= 0 || rect.left >= mWidth || rect.top >= mHeight || rect.isEmpty()) {
                    Log.e(LOG_TAG, "The decode rect is invalid.");
                    return null;
                } else {
                    return nativeDecodeRegion(mNativePtr, rect.left, rect.top, rect.width(), rect.height(), config, ratio);
                }
            }
        }
    }

    /**
     * Frees up the memory associated with this region decoder.
     * It will return null if decodeRegion().
     */
    public void recycle() {
        synchronized (mNativeLock) {
            if (mNativePtr != 0) {
                nativeRecycle(mNativePtr);
                mNativePtr = 0;
            }
        }
    }

    /**
     * Returns true if this region decoder has been recycled.
     */
    public final boolean isRecycled() {
        return mNativePtr == 0;
    }

    /**
     * Create a BitmapRegionDecoder from the {@code InputStream}.
     * BitmapRegionDecoder will make a copy of the data in the
     * InputStream, and the InputStream will be closed.
     */
    @Nullable
    public static BitmapRegionDecoder newInstance(InputStream is) {
        if (is == null) {
            return null;
        }
        return nativeNewInstance(is);
    }

    static {
        System.loadLibrary("image");
    }

    private static native BitmapRegionDecoder nativeNewInstance(InputStream is);

    private static native Bitmap nativeDecodeRegion(long nativePtr, int x, int y, int width, int height, int config, int ratio);

    private static native void nativeRecycle(long nativePtr);
}
