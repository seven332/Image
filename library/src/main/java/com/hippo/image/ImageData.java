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

public interface ImageData {

    /**
     * Recycle the ImageData, it can not be used anymore,
     * the resources associated with it will be released.
     * Call {@link #isReferenced()} to make sure the ImageData
     * is not referenced, or throw IllegalStateException.
     * <p>
     * In default, {@code recycle()} will be called when the
     * ImageData changed from referenced to non-referenced.
     * If you want to use this feature, make sure each ImageData
     * added a reference after created, and all referenced removed
     * when it is useless.
     *
     * @see #isReferenced()
     * @see #setAutomatic(boolean)
     */
    void recycle();

    /**
     * Return true if the ImageData is recycled.
     */
    boolean isRecycled();

    /**
     * Whether ImageData will recycle itself when it's not referenced.
     * <p>
     * Recycling action will only be triggered when the ImageData changed
     * from referenced to non-referenced.
     */
    void setAutomatic(boolean automatic);

    /**
     * Return true if ImageData will recycle itself when it's not referenced.
     * Return true in default.
     *
     * @see #setAutomatic(boolean)
     */
    boolean isAutomatic();

    /**
     * Create a ImageRenderer to render the ImageData.
     * It will add reference to ImageData.
     *
     * @see #addReference()
     */
    @NonNull
    ImageRenderer createImageRenderer();

    /**
     * Check whether the ImageData is referenced.
     * It is safe to release if it is not referenced.
     *
     * @see #addReference()
     * @see #removeReference()
     */
    boolean isReferenced();

    /**
     * Add reference to the ImageData.
     * ImageData can only be recycled when
     * it is not referenced.
     *
     * @see #isReferenced()
     * @see #removeReference()
     */
    void addReference();

    /**
     * Remove reference from the ImageData.
     * Throw IllegalStateException if the Image is not
     * referenced before {@code removeReference()} called.
     *
     * @see #isReferenced()
     * @see #addReference()
     */
    void removeReference();

    /**
     * Complete the image decoding. It will be ignored if the
     * image is totally decoded.
     */
    void complete();

    /**
     * Check whether the image is totally decoded.
     */
    boolean isCompleted();

    /**
     * Return the width of the image.
     */
    int getWidth();

    /**
     * Return the height of the image.
     */
    int getHeight();

    /**
     * Return the format of the image.
     *
     * @see Image#FORMAT_UNKNOWN
     * @see Image#FORMAT_BMP
     * @see Image#FORMAT_JPEG
     * @see Image#FORMAT_PNG
     * @see Image#FORMAT_GIF
     */
    int getFormat();

    /**
     * Return true is the image is opaque.
     */
    boolean isOpaque();

    /**
     * Return the frame count of the image.
     * Return 0 if it is not a animated image.
     * <p>
     * It should not be called when not completed
     * or throw IllegalStateException
     */
    int getFrameCount();

    /**
     * Return the delay in millisecond.
     * Return {@link Integer#MAX_VALUE} if it is not a animated image.
     * <p>
     * It should not be called when not completed
     * or throw IllegalStateException
     */
    int getDelay(int frame);

    /**
     * Get the memory size to store the ImageData.
     * <p>
     * It should not be called when not completed
     * or throw IllegalStateException
     */
    int getByteCount();

    /**
     * If BrowserCompat is enabled, delay than equal or smaller than
     * 10 ms will be 100 ms. For static image, do noting.
     */
    void setBrowserCompat(boolean enable);

    /**
     * Return whether BrowserCompat enabled. For static image,
     * always return true.
     */
    boolean isBrowserCompat();
}
