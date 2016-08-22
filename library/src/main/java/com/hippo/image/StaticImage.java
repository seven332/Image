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
 * Created by Hippo on 8/4/2016.
 */

import android.support.annotation.NonNull;

final class StaticImage implements ImageData {

    private long mNativePtr;
    private int mWidth;
    private int mHeight;
    private int mFormat;
    private boolean mOpaque;
    private int mByteCount;

    private int mReference;

    private StaticImage(long nativePtr, int width, int height,
            int format, boolean opaque, int byteCount) {
        mNativePtr = nativePtr;
        mWidth = width;
        mHeight = height;
        mFormat = format;
        mOpaque = opaque;
        mByteCount = byteCount;
    }

    @Override
    public void recycle() {
        // Check reference
        if (mReference > 0) {
            throw new IllegalStateException("Can't release referenced ImageData");
        }
        // Release native data
        if (mNativePtr != 0) {
            nativeRecycle(mNativePtr);
            mNativePtr = 0;
        }
    }

    @Override
    public boolean isRecycled() {
        return mNativePtr == 0;
    }

    // Throw IllegalStateException if recycled
    private void checkRecycled(String errorMessage) {
        if (mNativePtr == 0) {
            throw new IllegalStateException(errorMessage);
        }
    }

    // Return native ptr
    long getNativePtr() {
        checkRecycled("Can't use recycled ImageData");
        return mNativePtr;
    }

    @NonNull
    @Override
    public ImageRenderer createImageRenderer() {
        checkRecycled("Can't create ImageRenderer on recycled ImageData");
        ++mReference;
        return new StaticDelegateImage(this);
    }

    @Override
    public boolean isReferenced() {
        return mReference != 0;
    }

    @Override
    public void addReference() {
        ++mReference;
    }

    @Override
    public void removeReference() {
        --mReference;
        // Check reference valid
        if (mReference < 0) {
            throw new IllegalStateException();
        }
    }

    @Override
    public void complete() {
        // Empty, nothing to do for StaticImage.
    }

    @Override
    public boolean isCompleted() {
        // StaticImage is always completed.
        return true;
    }

    @Override
    public int getWidth() {
        return mWidth;
    }

    @Override
    public int getHeight() {
        return mHeight;
    }

    @Override
    public int getFormat() {
        return mFormat;
    }

    @Override
    public boolean isOpaque() {
        return mOpaque;
    }

    @Override
    public int getFrameCount() {
        // Frame count of StaticImage is always 1.
        return 1;
    }

    @Override
    public int getDelay(int frame) {
        return Integer.MAX_VALUE;
    }

    @Override
    public int getByteCount() {
        return mByteCount;
    }

    private static native void nativeRecycle(long nativePtr);
}
