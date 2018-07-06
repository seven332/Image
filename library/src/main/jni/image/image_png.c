/*
 * Copyright 2015 Hippo Seven
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
// Created by Hippo on 12/27/2015.
//

#include "config.h"
#ifdef IMAGE_SUPPORT_PNG


#include <stdlib.h>

#include "image.h"
#include "image_png.h"
#include "image_utils.h"
#include "image_decoder.h"
#include "image_convert.h"
#include "animated_image.h"
#include "../log.h"


#define IMAGE_PNG_PREPARE_NONE 0x00
#define IMAGE_PNG_PREPARE_BACKGROUND 0x01
#define IMAGE_PNG_PREPARE_USE_BACKUP 0x02


typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t offset_x;
  uint32_t offset_y;
  uint16_t delay_num;
  uint16_t delay_den;
  uint32_t delay; // ms
  uint8_t dop;
  uint8_t bop;
  uint8_t pop;
  uint8_t* buffer;
} PngFrame;

typedef struct {
  PngFrame* frames;
  uint32_t frame_count;
  png_structp png_ptr;
  png_infop info_ptr;
  Stream* stream;
} PngData;


LIBRARY_EXPORT
bool png_init(ImageLibrary* library) {
  library->loaded = true;
  library->is_magic = png_is_magic;
  library->decode = png_decode;
  library->decode_info = png_decode_info;
  library->decode_buffer = png_decode_buffer;
  library->create = NULL;
  library->get_description = png_get_description;

  return true;
}

bool png_is_magic(Stream* stream) {
  uint8_t magic[2];

  size_t read = stream->peek(stream, magic, sizeof(magic));
  if (read != sizeof(magic)) {
    LOGE(MSG("Could not read %zu bytes from stream, only read %zu"), sizeof(magic), read);
    return false;
  }

  return magic[0] == IMAGE_PNG_MAGIC_NUMBER_0 && magic[1] == IMAGE_PNG_MAGIC_NUMBER_1;
}

const char* png_get_description() {
  return IMAGE_PNG_DECODER_DESCRIPTION;
}

/**
 * Skip some rows
 */
static void png_skip_rows(png_structrp png_ptr, png_uint_32 num_rows) {
  for (png_uint_32 i = 0; i < num_rows; ++i) {
    png_read_row(png_ptr, NULL, NULL);
  }
}

/**
 * Skip the whole image
 */
static void png_skip_image(png_structrp png_ptr, png_infop info_ptr) {
  png_uint_32 image_height;
  int pass, i, j;

  image_height = png_get_image_height(png_ptr, info_ptr);
  pass = png_set_interlace_handling(png_ptr);

  for (j = 0; j < pass; j++) {
    for (i = 0; i < image_height; i++) {
      png_read_row(png_ptr, NULL, NULL);
    }
  }
}


static void user_read_fn(png_structp png_ptr,
    png_bytep data, png_size_t length) {
  Stream* stream = png_get_io_ptr(png_ptr);
  stream->read(stream, data, length);
}

static void user_error_fn(__unused png_structp png_ptr,
    png_const_charp error_msg) {
  LOGE(MSG("%s"), error_msg);
}

static void user_warn_fn(__unused png_structp png_ptr,
    png_const_charp error_msg) {
  LOGW(MSG("%s"), error_msg);
}

static void free_frames(PngFrame** frames, size_t count) {
  PngFrame* frame_info;
  size_t i;

  if (frames == NULL || *frames == NULL) {
    return;
  }

  for (i = 0; i < count; i++) {
    frame_info = *frames + i;
    free(frame_info->buffer);
    frame_info->buffer = NULL;
  }
  free(*frames);
  *frames = NULL;
}

static inline void blend_over(uint8_t* dp, const uint8_t* sp, size_t len) {
  uint32_t i;
  uint32_t u, v, al;

  for (i = 0; i < len; i += 4, sp += 4, dp += 4) {
    if (sp[3] == 255) {
      memcpy(dp, sp, 4);
    } else if (sp[3] != 0) {
      if (dp[3] != 0) {
        u = sp[3] * 255u;
        v = (255u - sp[3]) * dp[3];
        al = u + v;
        dp[0] = (uint8_t) ((sp[0] * u + dp[0] * v) / al);
        dp[1] = (uint8_t) ((sp[1] * u + dp[1] * v) / al);
        dp[2] = (uint8_t) ((sp[2] * u + dp[2] * v) / al);
        dp[3] = (uint8_t) (al / 255u);
      } else {
        memcpy(dp, sp, 4);
      }
    }
  }
}

static void blend(uint8_t* dst, uint32_t dst_width, uint32_t dst_height,
    uint8_t* src, uint32_t src_width, uint32_t src_height,
    uint32_t offset_x, uint32_t offset_y, bool blend_op_over) {
  uint32_t i;
  uint8_t* src_ptr;
  uint8_t* dst_ptr;
  size_t len;
  uint32_t copy_width = MIN(dst_width - offset_x, src_width);
  uint32_t copy_height = MIN(dst_height - offset_y, src_height);

  for (i = 0; i < copy_height; i++) {
    src_ptr = src + (i * src_width * 4);
    dst_ptr = dst + (((offset_y + i) * dst_width + offset_x) * 4);
    len = (size_t) (copy_width * 4);

    if (blend_op_over) {
      blend_over(dst_ptr, src_ptr, len);
    } else {
      memcpy(dst_ptr, src_ptr, len);
    }
  }
}

// Read pixels
static void read_image(png_structp png_ptr, uint8_t* buffer, uint32_t width, uint32_t height) {
  uint32_t i;
  uint8_t** image = (png_bytepp) malloc(height * sizeof(png_bytep));
  if (image == NULL) {
    png_error(png_ptr, OUT_OF_MEMORY);
  }

  for (i = 0; i < height; i++) {
    *(image + i) = buffer + (width * i * 4);
  }

  png_read_image(png_ptr, image);

  free(image);
}

// Read frame info and frame image pixel.
// Pixels buffer will be malloc.
static void read_frame(png_structp png_ptr, png_infop info_ptr, PngFrame* frame, PngFrame* pre_frame) {
  int pre_dop;

  png_read_frame_head(png_ptr, info_ptr);
  png_get_next_frame_fcTL(png_ptr, info_ptr, &frame->width, &frame->height,
      &frame->offset_x, &frame->offset_y, &frame->delay_num, &frame->delay_den,
      &frame->dop, &frame->bop);

  // If hide first frame and only one frame,
  // no fcTL chunk, so width and height will be zero.
  if (frame->width == 0 || frame->height == 0) {
    frame->width = png_get_image_width(png_ptr, info_ptr);
    frame->height = png_get_image_height(png_ptr, info_ptr);
    frame->offset_x = 0;
    frame->offset_y = 0;
    frame->delay_num = 0;
    frame->delay_den = 1000;
    frame->dop = PNG_DISPOSE_OP_NONE;
    frame->bop = PNG_BLEND_OP_SOURCE;
  }

  // Set delay
  if (frame->delay_den != 0) {
    frame->delay = 1000u * frame->delay_num / frame->delay_den;
  } else {
    frame->delay = 0;
  }

  // Set pop
  pre_dop = pre_frame != NULL ? pre_frame->dop : PNG_DISPOSE_OP_BACKGROUND;
  switch (pre_dop) {
    case PNG_DISPOSE_OP_NONE:
      frame->pop = IMAGE_PNG_PREPARE_NONE;
      break;
    default:
    case PNG_DISPOSE_OP_BACKGROUND:
      frame->pop = IMAGE_PNG_PREPARE_BACKGROUND;
      break;
    case PNG_DISPOSE_OP_PREVIOUS:
      frame->pop = IMAGE_PNG_PREPARE_USE_BACKUP;
      break;
  }

  // Malloc
  frame->buffer = (png_bytep) malloc(4 * frame->width * frame->height * sizeof(png_byte));
  if (frame->buffer == NULL) {
    png_error(png_ptr, OUT_OF_MEMORY);
  }

  // Read pixels
  read_image(png_ptr, frame->buffer, frame->width, frame->height);
}

static Stream* get_stream(AnimatedImage* image) {
  return ((PngData*) image->data)->stream;
}

static void complete(AnimatedImage* image) {
  PngData* data = image->data;
  PngFrame* frame;
  uint32_t i = 1;
  uint32_t j;
  if (image->completed || data->png_ptr == NULL ||
      data->info_ptr == NULL || data->stream == NULL) {
    return;
  }

  // Set new jmp
  if (setjmp(png_jmpbuf(data->png_ptr))) {
    // i is decoded frame count now.
    for (j = i; j < data->frame_count; j++) {
      frame = data->frames + j;
      if (frame->buffer == NULL) {
        break;
      } else {
        free(frame->buffer);
        frame->buffer = NULL;
      }
    }
    data->frame_count = i;
    png_destroy_read_struct(&data->png_ptr, &data->info_ptr, NULL);
    return;
  }

  for (i = 1; i < data->frame_count; i++) {
    read_frame(data->png_ptr, data->info_ptr, data->frames + i, data->frames + i - 1);
  }

  // End read
  png_read_end(data->png_ptr, data->info_ptr);
  png_destroy_read_struct(&data->png_ptr, &data->info_ptr, NULL);

  // Close stream
  data->stream->close(&data->stream);

  // Clean up
  data->png_ptr = NULL;
  data->info_ptr = NULL;
  data->stream = NULL;

  // Completed
  image->completed = true;
}

static uint32_t get_frame_count(AnimatedImage* image) {
  return ((PngData*)image->data)->frame_count;
}

static uint32_t get_delay(AnimatedImage* image, uint32_t frame) {
  PngData* data = image->data;
  if (frame >= data->frame_count) {
    LOGE(MSG("Frame count is %ud, can't get delay of index %ud"), data->frame_count, frame);
    return 0;
  }
  return (data->frames + frame)->delay;
}

static uint32_t get_byte_count(AnimatedImage* image) {
  uint32_t size = 0;
  uint32_t i;
  PngFrame* frame;
  PngData* data = image->data;
  if (image->completed) {
    for (i = 0; i < data->frame_count; i++) {
      frame = data->frames + i;
      size += frame->width * frame->height * 4;
    }
    return size;
  } else {
    LOGE(MSG("Can't call get_byte_count on Uncompleted AnimatedImage"));
    return 0;
  }
}

static void advance(AnimatedImage* image, DelegateImage* dImage) {
  PngData* data = image->data;
  int32_t target_index = dImage->index + 1;
  uint32_t width = image->width;
  uint32_t height = image->height;
  PngFrame* frame;

  if (target_index < 0 || target_index >= data->frame_count) {
    target_index = 0;
  }
  if (target_index == dImage->index) {
    return;
  }

  frame = data->frames + target_index;

  if (frame->dop == PNG_DISPOSE_OP_PREVIOUS && frame->pop == IMAGE_PNG_PREPARE_USE_BACKUP) {
    delegate_image_switch_data_backup(dImage);
  } else {
    // Backup
    if (frame->dop == PNG_DISPOSE_OP_PREVIOUS) {
      delegate_image_backup(dImage);
    }

    // Prepare
    switch (frame->pop) {
      case IMAGE_PNG_PREPARE_NONE:
        // Do nothing
        break;
      default:
      case IMAGE_PNG_PREPARE_BACKGROUND:
        // Set transparent
        memset(dImage->buffer, '\0', width * height * 4);
        break;
      case IMAGE_PNG_PREPARE_USE_BACKUP:
        delegate_image_restore(dImage);
        break;
    }
  }

  blend(dImage->buffer, dImage->width, dImage->height,
      frame->buffer, frame->width, frame->height, frame->offset_x, frame->offset_y,
      frame->bop == PNG_BLEND_OP_OVER);

  delegate_image_apply(dImage);

  dImage->index = target_index;
}

static void recycle(AnimatedImage** image) {
  PngData* data;

  if (image == NULL || *image == NULL) {
    return;
  }

  data = (PngData*) (*image)->data;

  free_frames(&data->frames, data->frame_count);

  if (data->png_ptr != NULL || data->info_ptr != NULL) {
    png_destroy_read_struct(&data->png_ptr, &data->info_ptr, NULL);
  }
  data->png_ptr = NULL;
  data->info_ptr = NULL;

  if (data->stream != NULL) {
    data->stream->close(&data->stream);
    data->stream = NULL;
  }

  free(data);
  (*image)->data = NULL;

  free(*image);
  *image = NULL;
}

void* png_decode(Stream* stream, bool partially, bool* animated) {
  StaticImage* static_image = NULL;
  AnimatedImage* animated_image = NULL;
  PngData* png_data = NULL;
  PngFrame* frames = NULL;
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  bool apng;
  uint32_t width;
  uint32_t height;
  uint8_t color_type;
  uint8_t bit_depth;
  uint32_t frame_count = 1;
  bool hide_first_frame = false;
  bool opaque;
  int i;

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, &user_error_fn, &user_warn_fn);
  if (png_ptr == NULL) {
    WTF_OOM;
    return NULL;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    WTF_OOM;
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    return NULL;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    static_image_delete(&static_image);
    free(animated_image);
    free(png_data);
    free_frames(&frames, frame_count);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return NULL;
  }

  // Init
  png_set_read_fn(png_ptr, stream, &user_read_fn);
  png_read_info(png_ptr, info_ptr);

  // Get info
  apng = (bool) png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL);
  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);
  if (apng) {
    frame_count = png_get_num_frames(png_ptr, info_ptr);
    // Don't hide first frame for only-one-frame png
    if (frame_count > 1) {
      hide_first_frame = png_get_first_frame_is_hidden(png_ptr, info_ptr);
      if (hide_first_frame) {
        frame_count -= 1;
      }
    }
  }
  // If only one frame, decode all
  if (frame_count == 1) {
    partially = false;
  }
  // Check info invalid
  if (width == 0 || height == 0 || frame_count == 0) {
    png_error(png_ptr, "Invalid png info");
    return NULL;
  }

  // Configure to ARGB
  png_set_expand(png_ptr);
  if (bit_depth == 16) {
    png_set_scale_16(png_ptr);
  }
  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
    png_set_gray_to_rgb(png_ptr);
  }
  if (color_type & PNG_COLOR_MASK_ALPHA) {
    opaque = false;
  } else {
    opaque = true;
    png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
  }

  if (apng) {
    // Skip first frame if necessary
    if (hide_first_frame && frame_count > 0) {
      png_skip_image(png_ptr, info_ptr);
    }

    // Malloc frames
    frames = (PngFrame*) malloc(frame_count * sizeof(PngFrame));
    if (frames == NULL) {
      png_error(png_ptr, OUT_OF_MEMORY);
      return NULL;
    }
    // Set frame buffer NULL for safety
    for (i = 0; i < frame_count; i++) {
      (frames + i)->buffer = NULL;
    }

    // Read first frame
    read_frame(png_ptr, info_ptr, frames, NULL);
    // Fix first frame dop
    if (frames->dop == PNG_DISPOSE_OP_PREVIOUS) {
      frames->dop = PNG_DISPOSE_OP_BACKGROUND;
    }

    // Read next frame
    if (!partially) {
      for (i = 1; i < frame_count; i++) {
        read_frame(png_ptr, info_ptr, frames + i, frames + i - 1);
      }

      // End read
      png_read_end(png_ptr, info_ptr);
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    }

    if (frame_count == 1) {
      // For one-frame apng, use StaticImage
      static_image = static_image_new(width, height);
      if (static_image == NULL) {
        png_error(png_ptr, OUT_OF_MEMORY);
        return NULL;
      }
      memset(static_image->buffer, '\0', width * height * 4);
      blend(static_image->buffer, width, height,
          frames->buffer, frames->width, frames->height,
          frames->offset_x, frames->offset_y, false);

      // Free frames, don't need it anymore
      free_frames(&frames, 1);

      // Set final result
      static_image->format = IMAGE_FORMAT_PNG;
      static_image->opaque = opaque;
      *animated = false;
      return static_image;
    } else {
      // For multi-frame apng, use AnimatedImage
      animated_image = (AnimatedImage*) malloc(sizeof(AnimatedImage));
      png_data = (PngData*) malloc(sizeof(PngData));
      if (animated_image == NULL || png_data == NULL) {
        png_error(png_ptr, OUT_OF_MEMORY);
        return NULL;
      }
      animated_image->width = width;
      animated_image->height = height;
      animated_image->format = IMAGE_FORMAT_PNG;
      animated_image->opaque = opaque;
      animated_image->completed = !partially;
      animated_image->data = png_data;

      animated_image->get_stream = &get_stream;
      animated_image->complete = &complete;
      animated_image->get_frame_count = &get_frame_count;
      animated_image->get_delay = &get_delay;
      animated_image->get_byte_count = &get_byte_count;
      animated_image->advance = &advance;
      animated_image->recycle = &recycle;

      png_data->frames = frames;
      png_data->frame_count = frame_count;
      if (partially) {
        png_data->png_ptr = png_ptr;
        png_data->info_ptr = info_ptr;
        png_data->stream = stream;
      } else {
        png_data->png_ptr = NULL;
        png_data->info_ptr = NULL;
        png_data->stream = NULL;
      }

      return animated_image;
    }
  } else {
    // For png, use StaticImage
    static_image = static_image_new(width, height);
    if (static_image == NULL) {
      png_error(png_ptr, OUT_OF_MEMORY);
      return NULL;
    }

    // Read pixel
    read_image(png_ptr, static_image->buffer, width, height);

    // End read
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    // Set final result
    static_image->format = IMAGE_FORMAT_PNG;
    static_image->opaque = opaque;
    *animated = false;
    return static_image;
  }
}

bool png_decode_info(Stream* stream, ImageInfo* info) {
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, &user_error_fn, &user_warn_fn);
  if (png_ptr == NULL) {
    WTF_OOM;
    return false;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    WTF_OOM;
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    return false;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return false;
  }

  // Init
  png_set_read_fn(png_ptr, stream, &user_read_fn);
  png_read_info(png_ptr, info_ptr);

  // Assign
  info->width = png_get_image_width(png_ptr, info_ptr);
  info->height = png_get_image_height(png_ptr, info_ptr);
  info->format = IMAGE_FORMAT_PNG;
  info->opaque = !(png_get_color_type(png_ptr, info_ptr) & PNG_COLOR_MASK_ALPHA);
  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL)) {
    // APNG
    info->frame_count = png_get_num_frames(png_ptr, info_ptr);
    if (info->frame_count > 1 && png_get_first_frame_is_hidden(png_ptr, info_ptr)) {
      --info->frame_count;
    }
  } else {
    info->frame_count = 1;
  }

  // End read
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

  return true;
}

bool png_decode_buffer(Stream* stream, bool clip, uint32_t x, uint32_t y, uint32_t width,
    uint32_t height, int32_t config, uint32_t ratio, BufferContainer* container) {
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  uint32_t i;
  bool result = false;

  uint32_t i_width;
  uint32_t i_height;
  uint8_t  i_color_type;
  uint8_t  i_bit_depth;
  bool     i_opaque;
  const uint32_t i_components = 4; // Always rgba8888

  uint32_t d_width;
  uint32_t d_height;
  uint32_t d_stride;
  bool     d_too_small;
  uint32_t d_components;

  int32_t  pass;

  uint32_t r_stride;
  uint32_t r_start_stride;
  uint8_t* r_buffer = NULL;
  // Don't free it
  uint8_t* r_line   = NULL;
  uint8_t* r_line_1 = NULL;
  uint8_t* r_line_2 = NULL;
  uint8_t* d_buffer = NULL;
  uint8_t* d_line = NULL;

  RowFunc row_func = NULL;

  // Prepare
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, &user_error_fn, &user_warn_fn);
  if (png_ptr == NULL) { WTF_OOM; goto end; }
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) { goto end; }
  if (setjmp(png_jmpbuf(png_ptr))) { goto end; }

  // Init
  png_set_read_fn(png_ptr, stream, &user_read_fn);
  png_read_info(png_ptr, info_ptr);

  // Get png info
  i_width = png_get_image_width(png_ptr, info_ptr);
  i_height = png_get_image_height(png_ptr, info_ptr);
  i_color_type = png_get_color_type(png_ptr, info_ptr);
  i_bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  // Set clip info
  if (!clip) {
    // Decode full image
    x = 0; y = 0; width = i_width; height = i_height;
  }

  // Configure output
  png_set_expand(png_ptr);
  if (i_bit_depth == 16) {
    png_set_scale_16(png_ptr);
  }
  if (i_color_type == PNG_COLOR_TYPE_GRAY ||
      i_color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
    png_set_gray_to_rgb(png_ptr);
  }
  if (!(i_color_type & PNG_COLOR_MASK_ALPHA)) {
    i_opaque = true;
    png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
  } else {
    i_opaque = false;
  }
  pass = png_set_interlace_handling(png_ptr);

  // Resolve config
  if (config == IMAGE_CONFIG_AUTO) {
    config = (int32_t) (i_opaque ? IMAGE_CONFIG_RGB_565 : IMAGE_CONFIG_RGBA_8888);
  } else if (!is_explicit_config(config)) {
    LOGE("Invalid config: %d", config);
    goto end;
  }

  // Fix width and height
  width = floor_uint32_t(width, ratio);
  height = floor_uint32_t(height, ratio);
  d_width = width / ratio;
  d_height = height / ratio;
  d_too_small = d_width == 0 || d_height == 0;

  // Create buffer
  d_buffer = container->create_buffer(container, MAX(d_width, 1), MAX(d_height, 1), config);
  if (d_buffer == NULL) { goto end; }

  // Check image ratio too large
  if (d_too_small) {
    // Ratio is too large, no need to decode.
    // Still treat it as success.
    LOGE("Ratio is too large!");
    result = true;
    goto end;
  }

  d_components = get_depth_for_config(config);
  d_stride = d_width * d_components;
  r_start_stride = x * i_components;
  r_stride = i_width * i_components;

  // Row function
  row_func = config == IMAGE_CONFIG_RGBA_8888 ? &RGBA8888_to_RGBA8888_row : &RGBA8888_to_RGB565_row;

  // Read data
  if (pass > 1) {
    // Interlaced PNG, read all rows to r_buffer,
    // then transfer them to d_buffer

    uint32_t remain_y = i_height - y - height;

    if (ratio == 1) {
      r_buffer = malloc(r_stride * height);
      if (r_buffer == NULL) { WTF_OOM; goto end; }

      // Read all rows to r_buffer
      while (--pass >= 0) {
        // Skip start lines
        png_skip_rows(png_ptr, y);
        // Read rows
        r_line = r_buffer;
        for (i = 0; i < height; ++i) {
          png_read_row(png_ptr, r_line, NULL);
          r_line += r_stride;
        }
        // Skip end lines
        png_skip_rows(png_ptr, remain_y);
      }

      // r_buffer to d_buffer
      d_line = d_buffer;
      r_line = r_buffer;
      for (i = 0; i < height; ++i) {
        row_func(d_line, r_line + r_start_stride, NULL, d_width, 1);
        d_line += d_stride;
        r_line += r_stride;
      }
    } else {
      r_buffer = malloc(r_stride * d_height * 2);
      if (r_buffer == NULL) { WTF_OOM; goto end; }

      uint32_t temp = ratio - 2;
      uint32_t skip_start = temp / 2;
      uint32_t skip_end = temp - skip_start;

      // Read all rows to r_buffer
      while (--pass >= 0) {
        // Skip start lines
        png_skip_rows(png_ptr, y);
        // Read rows
        r_line = r_buffer;
        for (i = 0; i < d_height; ++i) {
          png_skip_rows(png_ptr, skip_start);
          png_read_row(png_ptr, r_line, NULL);
          r_line += r_stride;
          png_read_row(png_ptr, r_line, NULL);
          r_line += r_stride;
          png_skip_rows(png_ptr, skip_end);
        }
        // Skip end lines
        png_skip_rows(png_ptr, remain_y);
      }

      // r_buffer to d_buffer
      d_line = d_buffer;
      r_line = r_buffer;
      for (i = 0; i < d_height; ++i) {
        row_func(d_line, r_line + r_start_stride,
            r_line + r_stride + r_start_stride, d_width, ratio);
        d_line += d_stride;
        r_line += r_stride * 2;
      }
    }
  } else if (ratio == 1) {
    r_line_1 = malloc(r_stride);
    if (r_line_1 == NULL) { WTF_OOM; goto end; }

    // Skip start lines
    png_skip_rows(png_ptr, y);

    // Read lines
    d_line = d_buffer;
    for (i = 0; i < d_height; ++i) {
      png_read_row(png_ptr, r_line_1, NULL);
      row_func(d_line, r_line_1 + r_start_stride, NULL, d_width, 1);
      d_line += d_stride;
    }
  } else {
    r_line_1 = malloc(r_stride);
    r_line_2 = malloc(r_stride);
    if (r_line_1 == NULL || r_line_2 == NULL) { WTF_OOM; goto end; }

    // Skip start lines
    png_skip_rows(png_ptr, y);

    // Read lines
    uint32_t temp = ratio - 2;
    uint32_t skip_start = temp / 2;
    uint32_t skip_end = temp - skip_start;

    d_line = d_buffer;
    for (i = 0; i < d_height; ++i) {
      png_skip_rows(png_ptr, skip_start);

      png_read_row(png_ptr, r_line_1, NULL);
      png_read_row(png_ptr, r_line_2, NULL);
      row_func(d_line, r_line_1 + r_start_stride,
          r_line_2 + r_start_stride, d_width, ratio);

      png_skip_rows(png_ptr, skip_end);

      d_line += d_stride;
    }
  }

  // It's not necessary to call png_read_end().
  // Also the png might not be totally decoded.
  // Skip it will increase decode speed.

  // Done
  result = true;

end:
  free(r_line_1);
  free(r_line_2);
  free(r_buffer);
  if (d_buffer != NULL) {
    container->release_buffer(container, d_buffer);
  }
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  return result;
}


#endif // IMAGE_SUPPORT_PNG
