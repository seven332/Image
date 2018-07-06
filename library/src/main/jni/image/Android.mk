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

LOCAL_CFLAGS := -DIMAGE_SINGLE_SHARED_LIB

LOCAL_MODULE := image
LOCAL_SRC_FILES := \
    image.c \
    image_plain.c \
    image_bmp.c \
    image_jpeg.c \
    image_png.c \
    image_gif.c \
    image_utils.c \
    image_convert.c \
    static_image.c \
    delegate_image.c \
    bitmap_container.c \
    java_wrapper.c \
    stream/stream.c \
    stream/buffer_stream.c \
    stream/java_stream.c
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/javah \
    $(LOCAL_PATH)/stream
LOCAL_LDLIBS := -llog -ljnigraphics -lGLESv2 -ldl
LOCAL_STATIC_LIBRARIES := jpeg-turbo png gif

LOCAL_SRC_FILES_armeabi-v7a := \
    image_convert_arm.c \
    image_convert_neon.S
LOCAL_CFLAGS_armeabi-v7a := -DIMAGE_CONVERT_ARM

include $(APPLY_ABI)
include $(BUILD_SHARED_LIBRARY)
