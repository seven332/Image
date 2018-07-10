# Image

Image 是把图像数据放到 native heap 的 Android 图像库，可以降低 OutOfMemoryError 的出现概率。支持 JPEG，PNG（包括 apng）和 GIF（包括动态图）。

Image is an Android image library with data stored in native heap. JPEG, PNG(include apng) and GIF(include animated) supported.


# Usage

在最外面的 `build.gradle` 里加上 jitpack，别加到 buildscript 里了。

Add jitpack repository in top `build.gradle`, DO **NOT** ADD IT TO buildscript.

    allprojects {
        repositories {
            ...
            maven { url "https://jitpack.io" }
        }
    }

## Single library
    
TODO: 中文翻译。 在项目 `build.gradle` 里添加 Image 依赖。

Image is available as a single library with all codecs included. Add Image as dependency in project `build.gradle`.

    dependencies {
        ...
        compile 'com.github.seven332.Image:singlelib:0.2.8'
    }

## Multi library

TODO: 中文翻译。

Image is also available as multiple libraries. Include the core, then add on whichever codecs you need.

Note: If you need every image codec, use the single library release above.

    dependencies {
        ...
        compile 'com.github.seven332.Image:core:0.2.8'
        compile 'com.github.seven332.Image:jpeg:0.2.8' // Optional
        compile 'com.github.seven332.Image:png:0.2.8'  // Optional
        compile 'com.github.seven332.Image:gif:0.2.8'  // Optional
    }

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
