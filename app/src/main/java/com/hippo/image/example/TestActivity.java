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

package com.hippo.image.example;

/*
 * Created by Hippo on 9/16/2016.
 */

import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.Rect;
import android.os.Bundle;
import android.os.Debug;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import com.hippo.image.BitmapDecoder;
import com.hippo.image.BitmapRegionDecoder;
import com.hippo.image.Image;
import com.hippo.image.ImageData;
import com.hippo.image.ImageInfo;
import com.hippo.image.ImageRenderer;
import com.hippo.yorozuya.FileUtils;
import com.hippo.yorozuya.MathUtils;

import junit.framework.Assert;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;

public class TestActivity extends AppCompatActivity implements Runnable {

    private static final String LOG_TAG = TestActivity.class.getSimpleName();

    private static class ImageRes {

        public int id;
        public int width;
        public int height;
        public int format;
        public boolean opaque;
        public int frameCount;

        public ImageRes(int id, int width, int height, int format, boolean opaque, int frameCount) {
            this.id = id;
            this.width = width;
            this.height = height;
            this.format = format;
            this.opaque = opaque;
            this.frameCount = frameCount;
        }
    }

    private static final ImageRes[] IMAGES = {
            new ImageRes(R.raw.apng_rgb,  580,  584,  Image.FORMAT_PNG,  true,  1),
            new ImageRes(R.raw.apng_rgba, 100,  100,  Image.FORMAT_PNG,  false, 20),
            new ImageRes(R.raw.gif,       540,  540,  Image.FORMAT_GIF,  false, -1),
            new ImageRes(R.raw.jpeg_cmyk, 256,  256,  Image.FORMAT_JPEG, true,  1),
            new ImageRes(R.raw.jpeg_ycck, 1713, 2479, Image.FORMAT_JPEG, true,  1),
            new ImageRes(R.raw.png_rgba,  800,  600,  Image.FORMAT_PNG,  false,  1),
            new ImageRes(R.raw.png_test,  300,  300,  Image.FORMAT_PNG,  false,  1),
    };

    private static final int[] CONFIGS = {
            BitmapDecoder.CONFIG_AUTO,
            BitmapDecoder.CONFIG_RGB_565,
            BitmapDecoder.CONFIG_ARGB_8888,
    };

    private static final int[] RATIOS = {
            1, 2, 3, 4, 5, 6, 7, 8, 9,
    };


    private Resources mResources;
    private File mDir;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mResources = getResources();
        mDir = getExternalFilesDir("test");
        FileUtils.deleteContent(mDir);

        new Thread(this).start();
    }

    private String configToString(int config) {
        switch (config) {
            case BitmapDecoder.CONFIG_AUTO:
                return "AUTO";
            case BitmapDecoder.CONFIG_RGB_565:
                return "RGB_565";
            case BitmapDecoder.CONFIG_ARGB_8888:
                return "RGBA_8888";
            default:
                return "Unknown_config";
        }
    }

    @Override
    public void run() {
        test();
    }

    private void test() {
        testImage();
        //testBitmapDecoderDecodeInfo();
        //testBitmapDecoderDecode();
        //testBitmapRegionDecoderDecode();
        Log.d(LOG_TAG, "Test Done!");
    }

    private void testImage() {
        for (ImageRes res : IMAGES) {
            Log.d(LOG_TAG, "testImage " + mResources.getResourceEntryName(res.id));
            for (int i = 0; i < 2; i++) {
                final ImageData imageData;
                if (i == 0) {
                    imageData = Image.decode(mResources.openRawResource(res.id), false);
                } else {
                    imageData = Image.decode(mResources.openRawResource(res.id), true);
                    imageData.complete();
                }
                final ImageRenderer imageRenderer = imageData.createImageRenderer();

                final int h1 = imageData.getWidth() / 4;
                final int h2 = imageData.getWidth() / 3;
                final int v1 = imageData.getHeight() / 4;
                final int v2 = imageData.getHeight() / 3;
                final Rect rect = new Rect();

                for (int j = 0; j < imageData.getFrameCount() + 1; j++) {
                    for (int ratio : RATIOS) {
                        for (int left = 0; left < imageData.getWidth(); left += MathUtils.random(h1, h2)) {
                            for (int right = left + MathUtils.random(h1, h2); right < imageData.getWidth(); right += MathUtils.random(h1, h2)) {
                                for (int top = 0; top < imageData.getHeight(); top += MathUtils.random(v1, v2)) {
                                    for (int bottom = top + MathUtils.random(v1, v2); bottom < imageData.getHeight(); bottom += MathUtils.random(v1, v2)) {
                                        rect.set(left, top, right, bottom);
                                        testImage1(res.id, imageRenderer, rect, ratio);
                                    }
                                }
                            }
                        }
                    }
                    imageRenderer.advance();
                }

                imageRenderer.recycle();
                imageData.recycle();
                Log.i(LOG_TAG, "" + FileUtils.readableByteCount(Debug.getNativeHeapAllocatedSize(), false));
            }
        }
    }

    private String getImageName(int id, Rect rect, int ratio) {
        final String idName = mResources.getResourceEntryName(id);
        return "image_" + idName + "_" + rectToString(rect) + "_ratio-" + ratio;
    }

    private void testImage1(int id, ImageRenderer renderer, Rect rect, int ratio) {
        final String name = getImageName(id, rect, ratio);
        Bitmap bitmap = Bitmap.createBitmap(rect.width() / ratio, rect.height() / ratio, Bitmap.Config.ARGB_8888);
        renderer.render(bitmap, 0, 0, rect.left, rect.top, rect.width(), rect.height(), ratio, true, Color.TRANSPARENT);
        try {
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, new FileOutputStream(new File(mDir, name + ".png")));
        } catch (FileNotFoundException e) {
            Log.e(LOG_TAG, "Can't save BitmapDecoder decode " + name, e);
        }
        bitmap.recycle();
    }

    private void testBitmapDecoderDecodeInfo() {
        for (ImageRes imageRes : IMAGES) {
            Log.d(LOG_TAG, "testBitmapDecoderDecodeInfo " + mResources.getResourceEntryName(imageRes.id));

            final ImageInfo info = new ImageInfo();
            BitmapDecoder.decode(mResources.openRawResource(imageRes.id), info);
            Assert.assertEquals(imageRes.width, info.width);
            Assert.assertEquals(imageRes.height, info.height);
            Assert.assertEquals(imageRes.format, info.format);
            Assert.assertEquals(imageRes.opaque, info.opaque);
            Assert.assertEquals(imageRes.frameCount, info.frameCount);
        }
    }

    private void testBitmapDecoderDecode() {
        for (ImageRes imageRes : IMAGES) {
            if (imageRes.format == Image.FORMAT_GIF) {
                // Skip gif
                continue;
            }
            Log.d(LOG_TAG, "testBitmapDecoderDecode " + mResources.getResourceEntryName(imageRes.id));
            for (int config : CONFIGS) {
                for (int ratio : RATIOS) {
                    testBitmapDecoderDecode1(imageRes.id, config, ratio);
                }
            }
        }
    }

    private String getBitmapDecoderDecodeName(int id, int config, int ratio) {
        final String idName = mResources.getResourceEntryName(id);
        final String configName = configToString(config);
        return "bitmap_decoder_" + idName + "_" + configName + "_ratio-" + ratio;
    }

    private void testBitmapDecoderDecode1(int id, int config, int ratio) {
        final String name = getBitmapDecoderDecodeName(id, config, ratio);
        final Bitmap bitmap = BitmapDecoder.decode(mResources.openRawResource(id), config, ratio);
        Assert.assertNotNull(bitmap);

        try {
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, new FileOutputStream(new File(mDir, name + ".png")));
        } catch (FileNotFoundException e) {
            Log.e(LOG_TAG, "Can't save BitmapDecoder decode " + name, e);
        }
        bitmap.recycle();
    }

    private String rectToString(Rect rect) {
        return rect.left + "_" + rect.top + "_" + rect.right + "_" + rect.bottom;
    }

    private String getBitmapRegionDecoderDecodeName(int id, Rect rect, int config, int ratio) {
        final String idName = mResources.getResourceEntryName(id);
        final String configName = configToString(config);
        return "bitmap_region_decoder_" + idName + "_" + rectToString(rect) + "_"+ configName + "_ratio-" + ratio;
    }

    private void testBitmapRegionDecoderDecode() {
        for (ImageRes imageRes : IMAGES) {
            if (imageRes.format == Image.FORMAT_GIF) {
                // Skip gif
                continue;
            }

            Log.d(LOG_TAG, "testBitmapRegionDecoderDecode " + mResources.getResourceEntryName(imageRes.id));

            final BitmapRegionDecoder decoder = BitmapRegionDecoder.newInstance(mResources.openRawResource(imageRes.id));
            Assert.assertNotNull(decoder);
            Assert.assertEquals(imageRes.width, decoder.getWidth());
            Assert.assertEquals(imageRes.height, decoder.getHeight());
            Assert.assertEquals(imageRes.format, decoder.getFormat());
            Assert.assertEquals(imageRes.opaque, decoder.isOpaque());

            final int h1 = decoder.getWidth() / 4;
            final int h2 = decoder.getWidth() / 3;
            final int v1 = decoder.getHeight() / 4;
            final int v2 = decoder.getHeight() / 3;

            final Rect rect = new Rect();
            for (int config : CONFIGS) {
                for (int ratio : RATIOS) {
                    for (int left = 0; left < decoder.getWidth(); left += MathUtils.random(h1, h2)) {
                        for (int right = left + MathUtils.random(h1, h2); right < decoder.getWidth(); right += MathUtils.random(h1, h2)) {
                            for (int top = 0; top < decoder.getHeight(); top += MathUtils.random(v1, v2)) {
                                for (int bottom = top + MathUtils.random(v1, v2); bottom < decoder.getHeight(); bottom += MathUtils.random(v1, v2)) {
                                    rect.set(left, top, right, bottom);
                                    testBitmapRegionDecoderDecode1(imageRes.id, decoder, rect, config, ratio);
                                }
                            }
                        }
                    }
                }
            }

            decoder.recycle();
            Log.i(LOG_TAG, "" + FileUtils.readableByteCount(Debug.getNativeHeapAllocatedSize(), false));
        }
    }

    private void testBitmapRegionDecoderDecode1(int id, BitmapRegionDecoder decoder, Rect rect, int config, int ratio) {
        final String name = getBitmapRegionDecoderDecodeName(id, rect, config, ratio);
        final Bitmap bitmap = decoder.decodeRegion(rect, config, ratio);
        Assert.assertNotNull(bitmap);

        try {
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, new FileOutputStream(new File(mDir, name + ".png")));
        } catch (FileNotFoundException e) {
            Log.e(LOG_TAG, "Can't save BitmapDecoder decode " + name, e);
        }
        bitmap.recycle();
    }
}
