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
