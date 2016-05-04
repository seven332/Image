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

import android.graphics.Bitmap;

import java.io.InputStream;
import java.util.concurrent.atomic.AtomicInteger;

public class Image {

    public static final int FORMAT_UNKNOWN = -1;
    public static final int FORMAT_PLAIN = 0;
    public static final int FORMAT_JPEG = 1;
    public static final int FORMAT_PNG = 2;
    public static final int FORMAT_GIF = 3;

    private static final AtomicInteger sImageCount = new AtomicInteger();

    private long mNativePtr;
    private final int mFormat;
    private final int mWidth;
    private final int mHeight;

    private Image(long nativePtr, int format, int width, int height) {
        mNativePtr = nativePtr;
        mFormat = format;
        mWidth = width;
        mHeight = height;

        sImageCount.getAndIncrement();
    }

    public int getFormat() {
        return mFormat;
    }

    public int getWidth() {
        return mWidth;
    }

    public int getHeight() {
        return mHeight;
    }

    private void checkRecycled() {
        if (mNativePtr == 0) {
            throw new IllegalStateException("The image is recycled.");
        }
    }

    public boolean complete() {
        checkRecycled();
        return nativeComplete(mNativePtr, mFormat);
    }

    public boolean isCompleted() {
        checkRecycled();
        return nativeIsCompleted(mNativePtr, mFormat);
    }

    public void render(int srcX, int srcY, Bitmap dst, int dstX, int dstY,
            int width, int height, boolean fillBlank, int defaultColor) {
        checkRecycled();
        nativeRender(mNativePtr, mFormat, srcX, srcY, dst, dstX, dstY,
                width, height, fillBlank, defaultColor);
    }

    public void texImage(boolean init, int offsetX, int offsetY, int width, int height) {
        checkRecycled();
        nativeTexImage(mNativePtr, mFormat, init, offsetX, offsetY, width, height);
    }

    public void advance() {
        checkRecycled();
        nativeAdvance(mNativePtr, mFormat);
    }

    public int getDelay() {
        checkRecycled();
        return nativeGetDelay(mNativePtr, mFormat);
    }

    public int getFrameCount() {
        checkRecycled();
        return nativeFrameCount(mNativePtr, mFormat);
    }

    public boolean isOpaque() {
        checkRecycled();
        return nativeIsOpaque(mNativePtr, mFormat);
    }

    public void recycle() {
        if (mNativePtr != 0) {
            nativeRecycle(mNativePtr, mFormat);
            mNativePtr = 0;

            sImageCount.getAndDecrement();
        }
    }

    public boolean isRecycled() {
        return mNativePtr == 0;
    }

    public static Image decode(InputStream is, boolean partially) {
        return nativeDecode(is, partially);
    }

    public static Image create(Bitmap bitmap) {
        return nativeCreate(bitmap);
    }

    public static int getImageCount() {
        return sImageCount.get();
    }

    static {
        System.loadLibrary("image");
    }

    private static native Image nativeDecode(InputStream is, boolean partially);

    private static native Image nativeCreate(Bitmap bitmap);

    private static native boolean nativeComplete(long nativePtr, int format);

    private static native boolean nativeIsCompleted(long nativePtr, int format);

    private static native void nativeRender(long nativePtr, int format,
            int srcX, int srcY, Bitmap dst, int dstX, int dstY,
            int width, int height, boolean fillBlank, int defaultColor);

    private static native void nativeTexImage(long nativePtr, int format,
            boolean init, int offsetX, int offsetY, int width, int height);

    private static native void nativeAdvance(long nativePtr, int format);

    private static native int nativeGetDelay(long nativePtr, int format);

    private static native int nativeFrameCount(long nativePtr, int format);

    private static native boolean nativeIsOpaque(long nativePtr, int format);

    private static native void nativeRecycle(long nativePtr, int format);
}
