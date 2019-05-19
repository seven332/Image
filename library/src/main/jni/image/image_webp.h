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
// Created by Hippo on 1/3/2016.
//

#ifndef IMAGE_IMAGE_WEBP_H
#define IMAGE_IMAGE_WEBP_H

#include "config.h"
#ifdef IMAGE_SUPPORT_WEBP

#include <stdio.h>
#include <stdbool.h>

#include "webp/decode.h"
#include "patch_head_input_stream.h"
#include "../utils.h"


#define IMAGE_WEBP_DECODER_DESCRIPTION ("libwebp " MAKESTRING(STRINGIZE, LIBWEBP_VERSION))

#define IMAGE_WEBP_MAGIC_NUMBER_0 0x82
#define IMAGE_WEBP_MAGIC_NUMBER_1 0x73

typedef struct
{
   int width;
   int height;
   void* buffer;
} WEBP;

void* WEBP_decode(JNIEnv* env, PatchHeadInputStream* patch_head_input_stream, bool partially);
bool WEBP_complete(WEBP* webp);
bool WEBP_is_completed(WEBP* webp);
void* WEBP_get_pixels(WEBP* webp);
int WEBP_get_width(WEBP* webp);
int WEBP_get_height(WEBP* webp);
int WEBP_get_byte_count(WEBP* webp);
void WEBP_render(WEBP* jpeg, int src_x, int src_y,
    void* dst, int dst_w, int dst_h, int dst_x, int dst_y,
    int width, int height, bool fill_blank, int default_color);
void WEBP_advance(WEBP* webp);
int WEBP_get_delay(WEBP* webp);
int WEBP_get_frame_count(WEBP* webp);
bool WEBP_is_opaque(WEBP* webp);
void WEBP_recycle(WEBP* webp);

#endif // IMAGE_SUPPORT_WEBP

#endif // IMAGE_IMAGE_WEBP_H
