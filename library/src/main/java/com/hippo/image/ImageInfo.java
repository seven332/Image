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
 * Created by Hippo on 9/12/2016.
 */

import android.support.annotation.Keep;

@Keep
public final class ImageInfo {

    public int width;
    public int height;
    public int format;
    public boolean opaque;
    /**
     * -1 for unknown
     */
    public int frameCount;

    // For native code
    private void set(int width, int height, int format, boolean opaque, int frameCount) {
        this.width = width;
        this.height = height;
        this.format = format;
        this.opaque = opaque;
        this.frameCount = frameCount;
    }
}
