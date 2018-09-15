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
project(image-webp C)

set(LIBWEBP_SOURCES
    image_webp.c
    libwebp/src/dec/alpha_dec.c
    libwebp/src/dec/buffer_dec.c
    libwebp/src/dec/frame_dec.c
    libwebp/src/dec/idec_dec.c
    libwebp/src/dec/io_dec.c
    libwebp/src/dec/quant_dec.c
    libwebp/src/dec/tree_dec.c
    libwebp/src/dec/vp8_dec.c
    libwebp/src/dec/vp8l_dec.c
    libwebp/src/dec/webp_dec.c
    libwebp/src/demux/anim_decode.c
    libwebp/src/demux/demux.c
    libwebp/src/dsp/alpha_processing.c
    libwebp/src/dsp/alpha_processing_mips_dsp_r2.c
    libwebp/src/dsp/alpha_processing_neon.c
    libwebp/src/dsp/alpha_processing_sse2.c
    libwebp/src/dsp/alpha_processing_sse41.c
    libwebp/src/dsp/cpu.c
    libwebp/src/dsp/dec.c
    libwebp/src/dsp/dec_clip_tables.c
    libwebp/src/dsp/dec_mips32.c
    libwebp/src/dsp/dec_mips_dsp_r2.c
    libwebp/src/dsp/dec_msa.c
    libwebp/src/dsp/dec_neon.c
    libwebp/src/dsp/dec_sse2.c
    libwebp/src/dsp/dec_sse41.c
    libwebp/src/dsp/filters.c
    libwebp/src/dsp/filters_mips_dsp_r2.c
    libwebp/src/dsp/filters_msa.c
    libwebp/src/dsp/filters_neon.c
    libwebp/src/dsp/filters_sse2.c
    libwebp/src/dsp/lossless.c
    libwebp/src/dsp/lossless_mips_dsp_r2.c
    libwebp/src/dsp/lossless_msa.c
    libwebp/src/dsp/lossless_neon.c
    libwebp/src/dsp/lossless_sse2.c
    libwebp/src/dsp/rescaler.c
    libwebp/src/dsp/rescaler_mips32.c
    libwebp/src/dsp/rescaler_mips_dsp_r2.c
    libwebp/src/dsp/rescaler_msa.c
    libwebp/src/dsp/rescaler_neon.c
    libwebp/src/dsp/rescaler_sse2.c
    libwebp/src/dsp/upsampling.c
    libwebp/src/dsp/upsampling_mips_dsp_r2.c
    libwebp/src/dsp/upsampling_msa.c
    libwebp/src/dsp/upsampling_neon.c
    libwebp/src/dsp/upsampling_sse2.c
    libwebp/src/dsp/upsampling_sse41.c
    libwebp/src/dsp/yuv.c
    libwebp/src/dsp/yuv_mips32.c
    libwebp/src/dsp/yuv_mips_dsp_r2.c
    libwebp/src/dsp/yuv_neon.c
    libwebp/src/dsp/yuv_sse2.c
    libwebp/src/dsp/yuv_sse41.c
    libwebp/src/utils/bit_reader_utils.c
    libwebp/src/utils/color_cache_utils.c
    libwebp/src/utils/filters_utils.c
    libwebp/src/utils/huffman_utils.c
    libwebp/src/utils/quant_levels_dec_utils.c
    libwebp/src/utils/random_utils.c
    libwebp/src/utils/rescaler_utils.c
    libwebp/src/utils/thread_utils.c
    libwebp/src/utils/utils.c
)
set(LIBWEBP_INCLUDES
    ${IMAGE_EXPORT_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libwebp
)
set(LIBWEBP_DEFINITIONS
)
set(LIBWEBP_LIBRARIES
)

add_definitions(-DWEBP_SWAP_16BIT_CSP=1)
add_definitions(-DWEBP_HAVE_NEON_RTCD)

if(IMAGE_SINGLE_SHARED_LIB)
    # Compile a static library for image-singlelib
    set(LIBWEBP_LIBRARY_NAME image-webp-static)
    set(LIBWEBP_LIBRARY_TYPE STATIC)
    set(LIBWEBP_INCLUDES_VISIBILITY PUBLIC) # Share include dir with image project
else()
    # Compile a shared library to be imported by image-core
    set(LIBWEBP_LIBRARY_NAME image-webp)
    set(LIBWEBP_LIBRARY_TYPE SHARED)
    set(LIBWEBP_INCLUDES_VISIBILITY PRIVATE)

    # Link against image shared lib to get helper methods
    set(LIBWEBP_LIBRARIES
        ${LIBWEBP_LIBRARIES}
        image
        log
    )
endif()

add_library(${LIBWEBP_LIBRARY_NAME} ${LIBWEBP_LIBRARY_TYPE} ${LIBWEBP_SOURCES})
target_include_directories(${LIBWEBP_LIBRARY_NAME} ${LIBWEBP_INCLUDES_VISIBILITY}
${LIBWEBP_INCLUDES})
target_compile_definitions(${LIBWEBP_LIBRARY_NAME} PRIVATE ${LIBWEBP_DEFINITIONS})
target_link_libraries(${LIBWEBP_LIBRARY_NAME} PRIVATE ${LIBWEBP_LIBRARIES})
