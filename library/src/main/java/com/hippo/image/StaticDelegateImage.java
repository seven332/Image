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
 * Created by Hippo on 8/5/2016.
 */

import android.graphics.Bitmap;

final class StaticDelegateImage implements ImageRenderer {

    private StaticImage mStaticImage;
    private boolean mReleased = false;

    StaticDelegateImage(StaticImage staticImage) {
        mStaticImage = staticImage;
    }

    @Override
    public void recycle() {
        if (!mReleased) {
            mReleased = true;
            mStaticImage.destroyImageRenderer();
        }
    }

    @Override
    public ImageData getImageData() {
        return mStaticImage;
    }

    @Override
    public int getCurrentDelay() {
        return Integer.MAX_VALUE;
    }

    // Throw IllegalStateException if recycled
    private void checkRecycled(String errorMessage) {
        if (mReleased) {
            throw new IllegalStateException(errorMessage);
        }
    }

    @Override
    public void render(Bitmap bitmap, int dstX, int dstY, int srcX, int srcY,
            int width, int height, int ratio, boolean fillBlank, int fillColor) {
        checkRecycled("Can't call render on recycled ImageRender");
        nativeRender(mStaticImage.getNativePtr(), bitmap, dstX, dstY,
                srcX, srcY, width, height, ratio, fillBlank, fillColor);
    }

    @Override
    public void glTex(boolean init, int texW, int texH, int dstX, int dstY,
            int srcX, int srcY, int width, int height, int ratio) {
        checkRecycled("Can't glTex render on recycled ImageRender");
        nativeGlTex(mStaticImage.getNativePtr(), Image.getBufferPtr(texW * texH),
                init, texW, texH, dstX, dstY, srcX, srcY, width, height, ratio);
    }

    @Override
    public void advance() {
        // Empty, nothing to do for StaticDelegateImage.
    }

    @Override
    public void reset() {
        // Empty, nothing to do for StaticDelegateImage.
    }

    private static native void nativeRender(long nativePtr,
            Bitmap bitmap, int dstX, int dstY, int srcX, int srcY,
            int width, int height, int ratio, boolean fillBlank, int fillColor);

    private static native void nativeGlTex(long nativePtr, long bufferPtr,
            boolean init, int texW, int texH, int dstX, int dstY,
            int srcX, int srcY, int width, int height, int ratio);
}
