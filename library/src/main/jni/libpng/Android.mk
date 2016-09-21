# Copyright 2015 Hippo Seven
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

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
include $(CLEAR_ABI)

LOCAL_MODULE := png
LOCAL_SRC_FILES := \
    png.c \
    pngerror.c \
    pngget.c \
    pngmem.c \
    pngpread.c \
    pngread.c \
    pngrio.c \
    pngrtran.c \
    pngrutil.c \
    pngset.c \
    pngtrans.c
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
LOCAL_EXPORT_LDLIBS := -lz

LOCAL_SRC_FILES_armeabi-v7a := \
    arm/arm_init.c \
    arm/filter_neon.S \
    arm/filter_neon_intrinsics.c
LOCAL_CFLAGS_armeabi-v7a := -DPNG_ARM_NEON_OPT=2
LOCAL_STATIC_LIBRARIES_armeabi-v7a := cpufeatures

LOCAL_SRC_FILES_arm64-v8a := \
    arm/arm_init.c \
    arm/filter_neon.S \
    arm/filter_neon_intrinsics.c
LOCAL_CFLAGS_arm64-v8a := -DPNG_ARM_NEON_OPT=2
LOCAL_STATIC_LIBRARIES_arm64-v8a := cpufeatures

include $(APPLY_ABI)
include $(BUILD_STATIC_LIBRARY)
