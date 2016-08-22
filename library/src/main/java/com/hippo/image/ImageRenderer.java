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

import android.graphics.Bitmap;
import android.support.annotation.NonNull;

public interface ImageRenderer {

    /**
     * Recycle the ImageRenderer, it can not be used anymore,
     * the resources associated with it will be released.
     * It will remove reference from its ImageData.
     *
     * @see ImageData#removeReference()
     */
    void recycle();

    /**
     * Return true if the ImageRenderer is recycled.
     */
    boolean isRecycled();

    /**
     * Return the ImageData associated with this ImageRenderer.
     * It will not return null, but the ImageData might be recycled.
     */
    @NonNull
    ImageData getImageData();

    /**
     * Return delay of current delay.
     * Return 0 if current frame is invalid.
     */
    int getCurrentDelay();

    /**
     * Render image to Bitmap.
     *
     * @param width src width
     * @param height src height
     * @param ratio dst width = src width / ratio
     */
    void render(Bitmap bitmap, int dstX, int dstY, int srcX, int srcY,
            int width, int height, int ratio, boolean fillBlank, int fillColor);

    /**
     * Call glTexImage2D or glTexSubImage2D.
     *
     * @param width src width
     * @param height src height
     * @param ratio dst width = src width / ratio
     */
    void glTex(boolean init, int texW, int texH, int dstX, int dstY,
            int srcX, int srcY, int width, int height, int ratio);

    /**
     * Advance current frame.
     * <p>
     * It should not be called when not completed
     * or throw IllegalStateException.
     */
    void advance();

    /**
     * Set current frame to the first one.
     * <p>
     * It should not be called when not completed
     * or throw IllegalStateException.
     */
    void reset();
}
