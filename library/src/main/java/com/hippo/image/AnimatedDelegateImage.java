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
 * Created by Hippo on 8/6/2016.
 */

import android.graphics.Bitmap;

final class AnimatedDelegateImage implements ImageRenderer {

    private AnimatedImage mAnimatedImage;
    private long mNativePtr;

    AnimatedDelegateImage(AnimatedImage animatedImage) {
        mAnimatedImage = animatedImage;
        mNativePtr = nativeNew(animatedImage.getWidth(), animatedImage.getHeight());
        if (mNativePtr == 0) {
            throw new IllegalStateException("Can't new AnimatedDelegateImage data");
        }
        // To render first frame
        reset();
    }

    @Override
    public void recycle() {
        if (mNativePtr != 0) {
            mAnimatedImage.removeReference();
            nativeRecycle(mNativePtr);
            mNativePtr = 0;
        }
    }

    @Override
    public ImageData getImageData() {
        return mAnimatedImage;
    }

    // Throw IllegalStateException if recycled
    private void checkRecycled(String errorMessage) {
        if (mNativePtr == 0) {
            throw new IllegalStateException(errorMessage);
        }
    }

    @Override
    public int getCurrentDelay() {
        checkRecycled("Can't call getCurrentDelay on recycled ImageRender");
        return nativeGetCurrentDelay(mNativePtr, mAnimatedImage.getNativePtr());
    }

    @Override
    public void render(Bitmap bitmap, int dstX, int dstY, int srcX, int srcY,
            int width, int height, int ratio, boolean fillBlank, int fillColor) {
        checkRecycled("Can't call render on recycled ImageRender");
        nativeRender(mNativePtr, bitmap, dstX, dstY,
                srcX, srcY, width, height, ratio, fillBlank, fillColor);
    }

    @Override
    public void glTex(boolean init, int texW, int texH,
            int dstX, int dstY, int srcX, int srcY,
            int width, int height, int ratio) {
        checkRecycled("Can't glTex render on recycled ImageRender");
        nativeGlTex(mNativePtr, Image.getBufferPtr(texW * texH),
                init, texW, texH, dstX, dstY, srcX, srcY, width, height, ratio);
    }

    @Override
    public void advance() {
        checkRecycled("Can't call advance on recycled ImageRender");
        nativeAdvance(mNativePtr, mAnimatedImage.getNativePtr());
    }

    @Override
    public void reset() {
        checkRecycled("Can't call reset on recycled ImageRender");
        nativeReset(mNativePtr, mAnimatedImage.getNativePtr());
    }

    private static native long nativeNew(int width, int height);

    private static native void nativeRecycle(long nativePtr);

    private static native int nativeGetCurrentDelay(long render, long data);

    private static native void nativeRender(long nativePtr,
            Bitmap bitmap, int dstX, int dstY, int srcX, int srcY,
            int width, int height, int ratio, boolean fillBlank, int fillColor);

    private static native void nativeGlTex(long nativePtr, long bufferPtr,
            boolean init, int texW, int texH, int dstX, int dstY,
            int srcX, int srcY, int width, int height, int ratio);

    private static native void nativeAdvance(long render, long data);

    private static native void nativeReset(long render, long data);
}
