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
import android.support.annotation.NonNull;

public class ImageWrapper {

    private final Image mImage;
    private int mReferences;

    public ImageWrapper(@NonNull Image image) {
        mImage = image;
    }

    /**
     * Obtain the image
     *
     * @return false for the image is recycled and obtain failed
     */
    public synchronized boolean obtain() {
        if (mImage.isRecycled()) {
            return false;
        } else {
            ++mReferences;
            return true;
        }
    }

    /**
     * Release the image
     */
    public synchronized void release() {
        --mReferences;
        if (mReferences <= 0 && !mImage.isRecycled()) {
            mImage.recycle();
        }
    }

    public int getFormat() {
        return mImage.getFormat();
    }

    public int getWidth() {
        return mImage.getWidth();
    }

    public int getHeight() {
        return mImage.getHeight();
    }

    public boolean complete() {
        return mImage.complete();
    }

    public boolean isCompleted() {
        return mImage.isCompleted();
    }

    public void render(int srcX, int srcY, Bitmap dst, int dstX, int dstY,
            int width, int height, boolean fillBlank, int defaultColor) {
        mImage.render(srcX, srcY, dst, dstX, dstY,
                width, height, fillBlank, defaultColor);
    }

    public void texImage(boolean init, int tileType, int offsetX, int offsetY) {
        mImage.texImage(init, tileType, offsetX, offsetY);
    }

    public void advance() {
        mImage.advance();
    }

    public int getDelay() {
        return mImage.getDelay();
    }

    public int getFrameCount() {
        return mImage.getFrameCount();
    }

    public boolean isOpaque() {
        return mImage.isOpaque();
    }

    public boolean isRecycled() {
        return mImage.isRecycled();
    }
}
