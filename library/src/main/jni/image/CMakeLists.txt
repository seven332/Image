# Copyright 2018 Hippo Seven
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.4.1)
project(image C)

set(IMAGE_SOURCES
    image.c
    image_plain.c
    image_bmp.c
    image_jpeg.c
    image_png.c
    image_gif.c
    image_utils.c
    image_convert.c
    static_image.c
    delegate_image.c
    bitmap_container.c
    java_wrapper.c
    stream/stream.c
    stream/buffer_stream.c
    stream/java_stream.c
)
set(IMAGE_INCLUDES
    .
    javah
    stream
    ../libjpeg-turbo
    ../libjpeg-turbo/libjpeg-turbo
    ../libpng
    ../libpng/libpng
    ../giflib/giflib/lib
)
set(IMAGE_DEFINITIONS
    -DIMAGE_SINGLE_SHARED_LIB
)
set(IMAGE_LIBRARIES
    libjpeg-turbo
    ${LIBJPEG_TURBO_EXPORT_LIBRARIES}
    libpng
    ${LIBPNG_EXPORT_LIBRARIES}
    giflib
    ${GIFLIB_EXPORT_LIBRARIES}
    log
    jnigraphics
    GLESv2
    dl
)

if (${ANDROID_ABI} STREQUAL "armeabi-v7a")

  enable_language(ASM)

  set(IMAGE_SOURCES
      ${IMAGE_SOURCES}
      image_convert_arm.c
      image_convert_neon.S
  )
  set(IMAGE_INCLUDES
      ${IMAGE_INCLUDES}
      ${ANDROID_NDK}/sources/android/cpufeatures
  )
  set(IMAGE_DEFINITIONS
      ${IMAGE_DEFINITIONS}
      -DIMAGE_CONVERT_ARM
  )
  set(IMAGE_LIBRARIES
      ${IMAGE_LIBRARIES}
      cpufeatures
  )

endif ()

add_library(image SHARED ${IMAGE_SOURCES})
target_include_directories(image PRIVATE ${IMAGE_INCLUDES})
target_compile_definitions(image PRIVATE ${IMAGE_DEFINITIONS})
target_link_libraries(image PRIVATE ${IMAGE_LIBRARIES})
