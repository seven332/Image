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

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.test.InstrumentationTestCase;

import junit.framework.Assert;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Locale;

public class ImageTest extends InstrumentationTestCase {

    public void testCreate() {
        Bitmap bitmap = Bitmap.createBitmap(100, 100, Bitmap.Config.ARGB_8888);
        Image image = Image.create(bitmap);
        Assert.assertNotNull(image);
        image.recycle();
        bitmap.recycle();
    }

    private void getByteCountTest(AssetManager assetManager, String name, int size) throws IOException {
        InputStream is = assetManager.open(name);
        Image image = Image.decode(is, false);
        Assert.assertNotNull(image);
        Assert.assertEquals(size, image.getByteCount());
        image.recycle();
        is.close();
    }

    public void testGetByteCount() throws IOException {
        AssetManager assetManager = getInstrumentation().getContext().getResources().getAssets();
        getByteCountTest(assetManager, "lena.jpg", 512 * 512 * 4);
        getByteCountTest(assetManager, "lena.png", 512 * 512 * 4);
        getByteCountTest(assetManager, "lena.gif", 512 * 512 * 4 + 512 * 512 * 4 + 512 * 512);
    }

    private void saveToBitmap(String name) throws IOException {
        AssetManager assetManager = getInstrumentation().getContext().getResources().getAssets();
        Image image = Image.decode(assetManager.open(name), false);
        Assert.assertNotNull(image);

        Bitmap bitmap = Bitmap.createBitmap(image.getWidth(), image.getHeight(), Bitmap.Config.ARGB_8888);
        File dir = getInstrumentation().getContext().getCacheDir();
        for (int i = 0; i < image.getFrameCount(); i++) {
            File file = new File(dir, name + String.format(Locale.ENGLISH, "-%03d", i) + ".png");
            image.render(0, 0, bitmap, 0, 0, image.getWidth(), image.getHeight(), false, 0);
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, new FileOutputStream(file));
            image.advance();
        }

        image.recycle();
    }

    public void testBPG() throws IOException {
        saveToBitmap("lena.jpg");
        saveToBitmap("lena.png");
        saveToBitmap("lena.gif");
    }

    public void testAnimation() throws IOException, InterruptedException {
        AssetManager assetManager = getInstrumentation().getContext().getResources().getAssets();
        final Image image = Image.decode(assetManager.open("animation.gif"), true);
        Assert.assertNotNull(image);

        Thread thread = new Thread() {
            @Override
            public void run() {
                image.complete();
                image.recycle();
            }
        };
        thread.start();
        thread.join();
    }

    public void testSupportedImageFormats() {
        int[] formats = Image.getSupportedImageFormats();
        Assert.assertEquals(3, formats.length);
        Assert.assertEquals(Image.FORMAT_JPEG, formats[0]);
        Assert.assertEquals(Image.FORMAT_PNG, formats[1]);
        Assert.assertEquals(Image.FORMAT_GIF, formats[2]);
    }

    public void testDecoderDescription() {
        Assert.assertEquals("libjpeg-turbo 1.5.0", Image.getDecoderDescription(Image.FORMAT_JPEG));
        Assert.assertEquals("libpng 1.6.23+apng", Image.getDecoderDescription(Image.FORMAT_PNG));
        Assert.assertEquals("giflib 5.1.4", Image.getDecoderDescription(Image.FORMAT_GIF));
    }
}
