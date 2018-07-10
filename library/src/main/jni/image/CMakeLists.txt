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
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/javah
    ${CMAKE_CURRENT_SOURCE_DIR}/stream
)
set(IMAGE_EXPORT_INCLUDES ${IMAGE_INCLUDES} PARENT_SCOPE)

if(IMAGE_SINGLE_SHARED_LIB)
    set(IMAGE_DEFINITIONS
        -DIMAGE_SINGLE_SHARED_LIB
    )
endif()

set(IMAGE_LIBRARIES
    log
    jnigraphics
    GLESv2
)

if(IMAGE_SINGLE_SHARED_LIB)
    set(IMAGE_LIBRARIES
        ${IMAGE_LIBRARIES}
        image-jpeg-static
        image-png-static
        image-gif-static
    )
else()
    set(IMAGE_LIBRARIES
        ${IMAGE_LIBRARIES}
        dl
    )
endif()

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