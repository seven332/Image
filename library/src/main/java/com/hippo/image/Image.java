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

import java.io.BufferedInputStream;
import java.io.InputStream;

public final class Image {

    /**
     * Unknown image format
     */
    public static final int FORMAT_UNKNOWN = -1;

    /**
     * Plain image format, for {@code Image} from {@link #create(Bitmap)}
     */
    public static final int FORMAT_PLAIN = 0;

    /**
     * Bmp image format
     */
    public static final int FORMAT_BMP = 1;

    /**
     * JPEG image format
     */
    public static final int FORMAT_JPEG = 2;

    /**
     * PNG image format
     */
    public static final int FORMAT_PNG = 3;

    /**
     * GIF image format
     */
    public static final int FORMAT_GIF = 4;

    private static long mBuffer = 0;
    private static int mBufferSize;

    static int mImageDataCount;
    static int mImageRendererCount;

    public static ImageData decode(@NonNull InputStream is) {
        return decode(is, false);
    }

    public static ImageData decode(@NonNull InputStream is, boolean partially) {
        if (!(is instanceof BufferedInputStream)) {
            is = new BufferedInputStream(is);
        }
        return nativeDecode(is, partially);
    }

    public static ImageData create(@NonNull Bitmap bitmap) {
        return nativeCreate(bitmap);
    }

    /**
     * Actual buffer byte count = size * 4
     */
    public static void createBuffer(int size) {
        if (mBuffer == 0) {
            mBuffer = nativeCreateBuffer(size);
            mBufferSize = size;
        }
    }

    public static void destroyBuffer() {
        if (mBuffer != 0) {
            nativeDestroyBuffer(mBuffer);
            mBuffer = 0;
        }
    }

    static long getBufferPtr(int size) {
        if (mBuffer == 0) {
            throw new IllegalStateException("Please call createBuffer first");
        }
        if (size > mBufferSize) {
            throw new IllegalStateException("Requested buffer size can't be more than allocated buffer size");
        }
        return mBuffer;
    }

    /**
     * Return all supported image formats, exclude {@link #FORMAT_PLAIN} and {@link #FORMAT_UNKNOWN}
     */
    public static int[] getSupportedImageFormats() {
        return nativeGetSupportedImageFormats();
    }

    /**
     * Return decoder description of the image format,
     * {@code null} for invalid image format.
     */
    public static String getDecoderDescription(int format) {
        return nativeGetDecoderDescription(format);
    }

    /**
     * Return the number of all ImageData.
     */
    public static int getImageDataCount() {
        return mImageDataCount;
    }

    /**
     * Return the number of all ImageRenderer.
     */
    public static int getmageRendererCount() {
        return mImageRendererCount;
    }

    static {
        System.loadLibrary("image");
    }

    private static native ImageData nativeDecode(InputStream is, boolean partially);

    private static native ImageData nativeCreate(Bitmap bitmap);

    private static native long nativeCreateBuffer(int size);

    private static native void nativeDestroyBuffer(long buffer);

    private static native int[] nativeGetSupportedImageFormats();

    private static native String nativeGetDecoderDescription(int format);
}
