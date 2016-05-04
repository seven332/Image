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

import java.io.IOException;
import java.io.InputStream;

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
        Image image = Image.decode(is, true);
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
}
