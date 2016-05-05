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

LOCAL_MODULE := bpg
LOCAL_CFLAGS := \
    -DHAVE_AV_CONFIG_H \
    -D_GNU_SOURCE=1 \
    -DUSE_VAR_BIT_DEPTH \
    -DUSE_PRED
LOCAL_SRC_FILES := \
    libbpg.c \
    libavcodec/hevc_cabac.c \
    libavcodec/hevc_filter.c \
    libavcodec/hevc.c \
    libavcodec/hevcpred.c \
    libavcodec/hevc_refs.c \
    libavcodec/hevcdsp.c \
    libavcodec/hevc_mvs.c \
    libavcodec/hevc_ps.c \
    libavcodec/hevc_sei.c \
    libavcodec/utils.c \
    libavcodec/cabac.c \
    libavcodec/golomb.c \
    libavcodec/videodsp.c \
    libavutil/buffer.c \
    libavutil/frame.c \
    libavutil/log2_tab.c \
    libavutil/md5.c \
    libavutil/mem.c \
    libavutil/pixdesc.c
LOCAL_EXPORT_LDLIBS := -latomic

include $(BUILD_STATIC_LIBRARY)
