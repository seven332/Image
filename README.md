# Image

Image 是把图像数据放到 native heap 的 Android 图像库，可以降低 OutOfMemoryError 的出现概率。支持 JPEG，PNG，GIF（包括动态图） 和 BPG（包括动态图）。

Image is an Android image library with data stored in native heap. JPEG, PNG, GIF(animated include) and BPG(animated include) supported.


# Usage

在最外面的 `build.gradle` 里加上 jitpack，别加到 buildscript 里了。

Add jitpack repository in top `build.gradle`, DO **NOT** ADD IT TO buildscript.

    allprojects {
        repositories {
            ...
            maven { url "https://jitpack.io" }
        }
    }

在项目 `build.gradle` 里添加 Image 依赖。

Add Image as dependency in project `build.gradle`.

    dependencies {
        ...
        compile 'com.github.seven332:image:0.1.6'
    }

在代码中使用：

Use Image in your code:

    // 读取文件
    // Read file
    InputStream is = new FileInputStream(file);
    Image image1 = Image.decode(is, false);

    // 从 Bitmap 创建，目前支持 ARGB_8888
    // Create from Bitmap, only support ARGB_8888 for now
    Bitmap bitmap = Bitmap.createBitmap(100, 100, Bitmap.Config.ARGB_8888);
    Image image2 = Image.create(bitmap);

    ...

    // 释放
    // Recycle
    image1.recycle();
    image2.recycle();


# License

    Copyright (C) 2015-2016 Hippo Seven

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
