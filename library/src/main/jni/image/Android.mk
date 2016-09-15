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

IMAGE_SRC_FILES := \
    image.c \
    image_plain.c \
    image_bmp.c \
    image_jpeg.c \
    image_png.c \
    image_gif.c \
    image_utils.c \
    static_image.c \
    delegate_image.c \
    bitmap_container.c \
    java_wrapper.c \
    stream/stream.c \
    stream/buffer_stream.c \
    stream/patched_stream.c \
    stream/java_stream.c
IMAGE_C_INCLUDES := \
    $(LOCAL_PATH)/javah \
    $(LOCAL_PATH)/stream
IMAGE_LDLIBS := -llog -ljnigraphics -lGLESv2
IMAGE_LIBRARIES := jpeg-turbo png gif

include $(CLEAR_VARS)

LOCAL_MODULE := image
LOCAL_SRC_FILES := $(IMAGE_SRC_FILES)
LOCAL_C_INCLUDES := $(IMAGE_C_INCLUDES)
LOCAL_LDLIBS := $(IMAGE_LDLIBS)
LOCAL_STATIC_LIBRARIES := $(IMAGE_LIBRARIES)

include $(BUILD_SHARED_LIBRARY)
