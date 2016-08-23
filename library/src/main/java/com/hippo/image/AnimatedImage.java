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

final class AnimatedImage implements ImageData {

    private long mNativePtr;
    private int mWidth;
    private int mHeight;
    private int mFormat;
    private boolean mOpaque;

    private int mFrameCount;
    private int[] mDelayArray;
    private int mByteCount;

    private boolean mCompleted;

    private int mReference;

    private AnimatedImage(long nativePtr, int width, int height, int format, boolean opaque) {
        mNativePtr = nativePtr;
        mWidth = width;
        mHeight = height;
        mFormat = format;
        mOpaque = opaque;
        mCompleted = false;

        // For statistics
        ++Image.mNumberOfImageData;
    }

    // Called in complete native
    private void onComplete(int frameCount, int[] delayArray, int byteCount) {
        mFrameCount = frameCount;
        mDelayArray = delayArray;
        mByteCount = byteCount;
        mCompleted = true;
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

            // For statistics
            --Image.mNumberOfImageData;
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
        return new AnimatedDelegateImage(this);
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
        if (mNativePtr != 0 && !mCompleted) {
            nativeComplete(this, mNativePtr);
        }
    }

    @Override
    public boolean isCompleted() {
        return mCompleted;
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
        if (mCompleted) {
            return mFrameCount;
        } else {
            throw new IllegalStateException("Can't get frame count on Uncompleted animated image");
        }
    }

    @Override
    public int getDelay(int frame) {
        if (mCompleted) {
            return mDelayArray[frame];
        } else {
            throw new IllegalStateException("Can't get delay on Uncompleted animated image");
        }
    }

    @Override
    public int getByteCount() {
        if (mCompleted) {
            return mByteCount;
        } else {
            throw new IllegalStateException("Can't get byte count on Uncompleted animated image");
        }
    }

    private static native void nativeRecycle(long nativePtr);

    private static native void nativeComplete(AnimatedImage image, long nativePtr);
}
