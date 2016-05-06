/*
 * Copyright 2016 Hippo Seven
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// Created by Hippo on 5/4/2016.
//

#ifndef IMAGE_IMAGE_BPG_H
#define IMAGE_IMAGE_BPG_H

#include "config.h"
#ifdef IMAGE_SUPPORT_BPG

#include <stdbool.h>

#include "libbpg.h"
#include "patch_head_input_stream.h"

#define IMAGE_BPG_MAGIC_NUMBER_0 0x42
#define IMAGE_BPG_MAGIC_NUMBER_1 0x50

typedef struct
{
  int delay; // ms
  void* buffer;
} BPG_FRAME_INFO;

typedef struct
{
  unsigned int width;
  unsigned int height;
  bool opaque;
  unsigned int frame_index;
  unsigned int frame_count;
  BPG_FRAME_INFO* frame_info_array;
  size_t frame_info_array_size;
  BPGDecoderContext* bdc;
} BPG;

void* BPG_decode(JNIEnv* env, PatchHeadInputStream* patch_head_input_stream, bool partially);
bool BPG_complete(BPG* bpg);
bool BPG_is_completed(BPG* bpg);
int BPG_get_width(BPG* bpg);
int BPG_get_height(BPG* bpg);
int BPG_get_byte_count(BPG* bpg);
void BPG_render(BPG* bpg, int src_x, int src_y,
    void* dst, int dst_w, int dst_h, int dst_x, int dst_y,
    int width, int height, bool fill_blank, int default_color);
void BPG_advance(BPG* bpg);
int BPG_get_delay(BPG* bpg);
int BPG_get_frame_count(BPG* bpg);
bool BPG_is_opaque(BPG* bpg);
void BPG_recycle(BPG* bpg);

#endif // IMAGE_SUPPORT_BPG

#endif // IMAGE_IMAGE_BPG_H
