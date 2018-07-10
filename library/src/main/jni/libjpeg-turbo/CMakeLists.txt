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
project(libjpeg-turbo C)

set(LIBJPEG_TURBO_SOURCES
    jaricom.c
    jcomapi.c
    jdapimin.c
    jdapistd.c
    jdarith.c
    jdatasrc.c
    jdatasrc-tj.c
    jdcoefct.c
    jdcolor.c
    jddctmgr.c
    jdhuff.c
    jdinput.c
    jdmainct.c
    jdmarker.c
    jdmaster.c
    jdmerge.c
    jdphuff.c
    jdpostct.c
    jdsample.c
    jdtrans.c
    jerror.c
    jfdctflt.c
    jfdctfst.c
    jfdctint.c
    jidctflt.c
    jidctfst.c
    jidctint.c
    jidctred.c
    jmemmgr.c
    jmemnobs.c
    jquant1.c
    jquant2.c
    jutils.c
)
set(LIBJPEG_TURBO_INCLUDES
)
set(LIBJPEG_TURBO_DEFINITIONS
)
set(LIBJPEG_TURBO_LIBRARIES
)

if (${ANDROID_ABI} STREQUAL "armeabi-v7a")

  enable_language(ASM)

  set(LIBJPEG_TURBO_SOURCES
      ${LIBJPEG_TURBO_SOURCES}
      simd/jsimd_arm.c
      simd/jsimd_arm_neon.S
  )

elseif (${ANDROID_ABI} STREQUAL "arm64-v8a")

  enable_language(ASM)

  set(LIBJPEG_TURBO_SOURCES
      ${LIBJPEG_TURBO_SOURCES}
      simd/jsimd_arm64.c
      simd/jsimd_arm64_neon.S
  )

elseif (${ANDROID_ABI} STREQUAL "x86")

  enable_language(ASM_NASM)

  set(LIBJPEG_TURBO_SOURCES
      ${LIBJPEG_TURBO_SOURCES}
      simd/jsimd_i386.c
      simd/jsimdcpu.asm
      simd/jfdctflt-3dn.asm
      simd/jidctflt-3dn.asm
      simd/jquant-3dn.asm
      simd/jccolor-mmx.asm
      simd/jcgray-mmx.asm
      simd/jcsample-mmx.asm
      simd/jdcolor-mmx.asm
      simd/jdmerge-mmx.asm
      simd/jdsample-mmx.asm
      simd/jfdctfst-mmx.asm
      simd/jfdctint-mmx.asm
      simd/jidctfst-mmx.asm
      simd/jidctint-mmx.asm
      simd/jidctred-mmx.asm
      simd/jquant-mmx.asm
      simd/jfdctflt-sse.asm
      simd/jidctflt-sse.asm
      simd/jquant-sse.asm
      simd/jccolor-sse2.asm
      simd/jcgray-sse2.asm
      simd/jchuff-sse2.asm
      simd/jcsample-sse2.asm
      simd/jdcolor-sse2.asm
      simd/jdmerge-sse2.asm
      simd/jdsample-sse2.asm
      simd/jfdctfst-sse2.asm
      simd/jfdctint-sse2.asm
      simd/jidctflt-sse2.asm
      simd/jidctfst-sse2.asm
      simd/jidctint-sse2.asm
      simd/jidctred-sse2.asm
      simd/jquantf-sse2.asm
      simd/jquanti-sse2.asm
  )

  set(CMAKE_ASM_NASM_COMPILER_ARG1 "-DELF -DPIC")

elseif (${ANDROID_ABI} STREQUAL "x86_64")

  enable_language(ASM_NASM)

  set(LIBJPEG_TURBO_SOURCES
      ${LIBJPEG_TURBO_SOURCES}
      simd/jsimd_x86_64.c
      simd/jfdctflt-sse-64.asm
      simd/jccolor-sse2-64.asm
      simd/jcgray-sse2-64.asm
      simd/jchuff-sse2-64.asm
      simd/jcsample-sse2-64.asm
      simd/jdcolor-sse2-64.asm
      simd/jdmerge-sse2-64.asm
      simd/jdsample-sse2-64.asm
      simd/jfdctfst-sse2-64.asm
      simd/jfdctint-sse2-64.asm
      simd/jidctflt-sse2-64.asm
      simd/jidctfst-sse2-64.asm
      simd/jidctint-sse2-64.asm
      simd/jidctred-sse2-64.asm
      simd/jquantf-sse2-64.asm
      simd/jquanti-sse2-64.asm
  )

  set(CMAKE_ASM_NASM_COMPILER_ARG1 "-D__x86_64__ -DELF -DPIC")

else ()

  set(LIBJPEG_TURBO_SOURCES
      ${LIBJPEG_TURBO_SOURCES}
      jsimd_none.c
  )

endif ()

set(LIBJPEG_TURBO_EXPORT_LIBRARIES ${LIBJPEG_TURBO_LIBRARIES} PARENT_SCOPE)

add_library(libjpeg-turbo STATIC ${LIBJPEG_TURBO_SOURCES})
target_include_directories(libjpeg-turbo PRIVATE ${LIBJPEG_TURBO_INCLUDES})
target_compile_definitions(libjpeg-turbo PRIVATE ${LIBJPEG_TURBO_DEFINITIONS})
