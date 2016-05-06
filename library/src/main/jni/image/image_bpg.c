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

#include "config.h"
#ifdef IMAGE_SUPPORT_BPG

#include <stdlib.h>

#include "image_bpg.h"
#include "image_utils.h"
#include "../log.h"

#define BPG_FRAME_INFO_INC_SIZE 10

void free_bpg_frame_info_array(BPG_FRAME_INFO* frame_info_array, unsigned int frame_count)
{
  int i;
  BPG_FRAME_INFO* frame_info;
  for (i = 0; i < frame_count; i++) {
    frame_info = frame_info_array + i;
    if (frame_info->buffer != NULL) {
      free(frame_info->buffer);
      frame_info->buffer = NULL;
    }
  }
  free(frame_info_array);
}

void* BPG_decode(JNIEnv* env, PatchHeadInputStream* patch_head_input_stream, bool partially)
{
  void* data_buffer = NULL;
  size_t data_length;
  BPG* bpg = NULL;
  BPGDecoderContext* bdc = NULL;
  BPGImageInfo bii;
  unsigned int width;
  unsigned int height;
  unsigned int frame_count = 0;
  BPG_FRAME_INFO* frame_info_array;
  BPG_FRAME_INFO* temp;
  BPG_FRAME_INFO* frame_info;
  size_t frame_info_array_size;
  unsigned char* buffer;
  bool decode_over;
  int delay_num;
  int delay_den;
  int i;

  // Read stream all, libbpg need all data at once
  data_buffer = read_patch_head_input_stream_all(env, patch_head_input_stream, &data_length);
  close_patch_head_input_stream(env, patch_head_input_stream);
  destroy_patch_head_input_stream(env, &patch_head_input_stream);
  if (data_buffer == NULL) {
    WTF_OM;
    return NULL;
  }

  bpg = (BPG *) malloc(sizeof(BPG));
  if (bpg == NULL) {
    WTF_OM;
    free(data_buffer);
    return NULL;
  }

  bdc = bpg_decoder_open();
  if (bdc == NULL) {
    WTF_OM;
    free(bpg);
    free(data_buffer);
    return NULL;
  }

  if (bpg_decoder_decode(bdc, data_buffer, data_length) < 0) {
    bpg_decoder_close(bdc);
    free(bpg);
    free(data_buffer);
    return NULL;
  } else {
    // data buffer can be free
    free(data_buffer);
    data_buffer = NULL;
  }

  bpg_decoder_get_info(bdc, &bii);
  width = bii.width;
  height = bii.height;
  decode_over = !partially || !bii.has_animation;

  if (bii.has_animation) {
    frame_info_array = malloc(sizeof(BPG_FRAME_INFO) * BPG_FRAME_INFO_INC_SIZE);
    frame_info_array_size = BPG_FRAME_INFO_INC_SIZE;
  } else {
    frame_info_array = malloc(sizeof(BPG_FRAME_INFO));
    frame_info_array_size = 1;
  }
  if (frame_info_array == NULL) {
    bpg_decoder_close(bdc);
    free(bpg);
    return NULL;
  }
  // Clear frame_info_array
  memset(frame_info_array, 0, sizeof(BPG_FRAME_INFO) * frame_info_array_size);

  for (;;) {
    if (bpg_decoder_start(bdc, BPG_OUTPUT_FORMAT_RGBA32) < 0) {
      break;
    }

    // Check frame info array size
    if (frame_count == frame_info_array_size) {
      temp = realloc(frame_info_array,
          sizeof(BPG_FRAME_INFO) * (frame_info_array_size + BPG_FRAME_INFO_INC_SIZE));
      if (temp == NULL) {
        WTF_OM;
        free_bpg_frame_info_array(frame_info_array, frame_count);
        bpg_decoder_close(bdc);
        free(bpg);
        return NULL;
      }

      frame_info_array = temp;
      // Clear new frame_info_array
      memset(frame_info_array + frame_info_array_size, 0, sizeof(BPG_FRAME_INFO) * BPG_FRAME_INFO_INC_SIZE);
      frame_info_array_size += BPG_FRAME_INFO_INC_SIZE;
    }

    frame_info = frame_info_array + frame_count;

    // Get animation info
    bpg_decoder_get_frame_duration(bdc, &delay_num, &delay_den);
    frame_info->delay = delay_num * 1000 / delay_den;

    // Create frame buffer
    buffer = malloc(width * height * 4);
    if (buffer == NULL) {
      WTF_OM;
      free_bpg_frame_info_array(frame_info_array, frame_count);
      bpg_decoder_close(bdc);
      free(bpg);
      return NULL;
    }
    frame_info->buffer = buffer;

    // Fill frame buffer
    for(i = 0; i < height; i++) {
      bpg_decoder_get_line(bdc, buffer + i * width * 4);
    }

    frame_count++;

    if (frame_count == 1 && !decode_over) {
      break;
    }
  }

  // Check frame count is 0
  if (frame_count == 0) {
    LOGE(MSG("No frame"));
    free_bpg_frame_info_array(frame_info_array, frame_count);
    bpg_decoder_close(bdc);
    free(bpg);
    return NULL;
  }

  bpg->width = width;
  bpg->height = height;
  bpg->opaque = !bii.has_alpha;
  bpg->frame_index = 0;
  bpg->frame_count = frame_count;
  bpg->frame_info_array = frame_info_array;
  bpg->frame_info_array_size = frame_info_array_size;
  if (decode_over) {
    bpg_decoder_close(bdc);
    bpg->bdc = NULL;
  } else {
    bpg->bdc = bdc;
  }

  return bpg;
}

bool BPG_complete(BPG* bpg)
{
  BPG_FRAME_INFO* temp;
  BPG_FRAME_INFO* frame_info;
  unsigned char* buffer;
  int delay_num;
  int delay_den;
  int i;
  bool result = true;

  if (bpg->bdc == NULL) {
    return true;
  }
  if (bpg->frame_info_array == NULL) {
    LOGE(MSG("No frame info array"));
    return false;
  }

  for (;;) {
    if (bpg_decoder_start(bpg->bdc, BPG_OUTPUT_FORMAT_RGBA32) < 0) {
      break;
    }

    // Check frame info array size
    if (bpg->frame_count == bpg->frame_info_array_size) {
      temp = realloc(bpg->frame_info_array,
          sizeof(BPG_FRAME_INFO) * (bpg->frame_info_array_size + BPG_FRAME_INFO_INC_SIZE));
      if (temp == NULL) {
        WTF_OM;
        result = false;
        break;
      }

      bpg->frame_info_array = temp;
      // Clear new frame_info_array
      memset(bpg->frame_info_array + bpg->frame_info_array_size, 0, sizeof(BPG_FRAME_INFO) * BPG_FRAME_INFO_INC_SIZE);
      bpg->frame_info_array_size += BPG_FRAME_INFO_INC_SIZE;
    }

    frame_info = bpg->frame_info_array + bpg->frame_count;

    // Get animation info
    bpg_decoder_get_frame_duration(bpg->bdc, &delay_num, &delay_den);
    frame_info->delay = delay_num * 1000 / delay_den;

    // Create frame buffer
    buffer = malloc(bpg->width * bpg->height * 4);
    if (buffer == NULL) {
      WTF_OM;
      result = false;
      break;
    }
    frame_info->buffer = buffer;

    // Fill frame buffer
    for(i = 0; i < bpg->height; i++) {
      bpg_decoder_get_line(bpg->bdc, buffer + i * bpg->width * 4);
    }

    bpg->frame_count++;
  }

  bpg_decoder_close(bpg->bdc);
  bpg->bdc = NULL;
  return result;
}

bool BPG_is_completed(BPG* bpg)
{
  return bpg->bdc == NULL;
}

int BPG_get_width(BPG* bpg)
{
  return bpg->width;
}

int BPG_get_height(BPG* bpg)
{
  return bpg->height;
}

int BPG_get_byte_count(BPG* bpg)
{
  return bpg->frame_count * bpg->width * bpg->height * 4;
}

void BPG_render(BPG* bpg, int src_x, int src_y,
    void* dst, int dst_w, int dst_h, int dst_x, int dst_y,
    int width, int height, bool fill_blank, int default_color)
{
  copy_pixels((bpg->frame_info_array + bpg->frame_index)->buffer,
      bpg->width, bpg->height, src_x, src_y,
      dst, dst_w, dst_h, dst_x, dst_y,
      width, height, fill_blank, default_color);
}

void BPG_advance(BPG* bpg)
{
  bpg->frame_index = ++bpg->frame_index % bpg->frame_count;
}

int BPG_get_delay(BPG* bpg)
{
  return (bpg->frame_info_array + bpg->frame_index)->delay;
}

int BPG_get_frame_count(BPG* bpg)
{
  return bpg->frame_count;
}

bool BPG_is_opaque(BPG* bpg)
{
  return bpg->opaque;
}

void BPG_recycle(BPG* bpg)
{
  if (bpg->bdc != NULL) {
    bpg_decoder_close(bpg->bdc);
    bpg->bdc = NULL;
  }
  if (bpg->frame_info_array != NULL) {
    free_bpg_frame_info_array(bpg->frame_info_array, bpg->frame_count);
    bpg->frame_info_array = NULL;
  }
  free(bpg);
}

#endif // IMAGE_SUPPORT_BPG
