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
project(libwebp)

set(LIBWEBP_SOURCES
    libwebp/src/dec/alpha.c 
    libwebp/src/dec/buffer.c 
    libwebp/src/dec/frame.c 
    libwebp/src/dec/idec.c 
    libwebp/src/dec/io.c 
    libwebp/src/dec/layer.c 
    libwebp/src/dec/quant.c 
    libwebp/src/dec/tree.c 
    libwebp/src/dec/vp8.c 
    libwebp/src/dec/vp8l.c 
    libwebp/src/dec/webp.c 
    libwebp/src/dsp/cpu.c 
    libwebp/src/dsp/dec.c 
    libwebp/src/dsp/dec_sse2.c 
    libwebp/src/dsp/lossless.c 
    libwebp/src/dsp/upsampling.c 
    libwebp/src/dsp/upsampling_sse2.c 
    libwebp/src/dsp/yuv.c 
    libwebp/src/utils/bit_reader.c 
    libwebp/src/utils/bit_writer.c 
    libwebp/src/utils/color_cache.c 
    libwebp/src/utils/filters.c 
    libwebp/src/utils/huffman.c 
    libwebp/src/utils/huffman_encode.c 
    libwebp/src/utils/quant_levels.c 
    libwebp/src/utils/quant_levels_dec.c 
    libwebp/src/utils/rescaler.c 
    libwebp/src/utils/thread.c 
    libwebp/src/utils/utils.c 
)
set(LIBWEBP_INCLUDES
    ${LIBWEBP_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/libwebp/src/
    ${ANDROID_NDK}/sources/android/cpufeatures)

set(LIBWEBP_DEFINITIONS
        ANDROID
        HAVE_MALLOC_H
        HAVE_PTHREAD
        WEBP_USE_THREAD)

if (${ANDROID_ABI} STREQUAL "armeabi-v7a")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=neon")
    set(LIBWEBP_DEFINITIONS
            ${LIBWEBP_DEFINITIONS}
            -DHAVE_CPU_FEATURES_H
            )
    # Setting LOCAL_ARM_NEON will enable -mfpu=neon which may cause illegal
    # instructions to be generated for armv7a code. Instead target the neon code
    # specifically.
    set(LIBWEBP_SOURCES
        ${LIBWEBP_SOURCES}
        libwebp/src/dsp/dec_neon.c
        libwebp/src/dsp/upsampling_neon.c)
endif ()


add_library(libwebp STATIC ${LIBWEBP_SOURCES})
target_include_directories(libwebp PUBLIC ${LIBWEBP_INCLUDES})
target_compile_definitions(libwebp PRIVATE ${LIBWEBP_DEFINITIONS})


