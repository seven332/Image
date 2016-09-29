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

#include "config.h"
#ifdef IMAGE_SUPPORT_JPEG


#include <setjmp.h>
#include <malloc.h>
#include <string.h>

#include "image.h"
#include "image_jpeg.h"
#include "image_decoder.h"
#include "image_convert.h"
#include "image_utils.h"
#include "../log.h"


struct my_error_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;


static char emsg[JMSG_LENGTH_MAX];


static void my_error_exit(j_common_ptr cinfo) {
  my_error_ptr myerr = (my_error_ptr) cinfo->err;
  (*cinfo->err->format_message)(cinfo, emsg);
  longjmp(myerr->setjmp_buffer, 1);
}

static size_t custom_read(void * custom_stuff, unsigned char * buffer, size_t size) {
  Stream* stream = (Stream*) custom_stuff;
  return stream->read(stream, buffer, size);
}

StaticImage* jpeg_decode(Stream* stream) {
  StaticImage* image = NULL;
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  uint8_t* buffer = NULL;
  size_t stride;
  uint8_t* line_buffer_array[3];
  uint32_t read_lines;
  bool result = false;

  // Init
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if (setjmp(jerr.setjmp_buffer)) { LOGE(MSG("%s"), emsg); goto end; }
  jpeg_create_decompress(&cinfo);
  jpeg_custom_src(&cinfo, &custom_read, stream);
  jpeg_read_header(&cinfo, TRUE);

  // Start decompress
  cinfo.out_color_space = JCS_EXT_RGBA;
  jpeg_start_decompress(&cinfo);

  // New static image
  image = static_image_new(cinfo.output_width, cinfo.output_height);
  if (image == NULL) { goto end; }

  // Set buffer to image->buffer
  buffer = image->buffer;

  // Copy to buffer
  stride = cinfo.output_components * cinfo.output_width;
  line_buffer_array[0] = buffer;
  line_buffer_array[1] = line_buffer_array[0] + stride;
  line_buffer_array[2] = line_buffer_array[1] + stride;
  while (cinfo.output_scanline < cinfo.output_height) {
    read_lines = jpeg_read_scanlines(&cinfo, line_buffer_array, 3);
    line_buffer_array[0] += stride * read_lines;
    line_buffer_array[1] = line_buffer_array[0] + stride;
    line_buffer_array[2] = line_buffer_array[1] + stride;
  }

  // It's not necessary to call jpeg_finish_decompress().
  // Skip it will increase decode speed.

  // Fill jpeg
  image->format = IMAGE_FORMAT_JPEG;
  image->opaque = true;

  // Done!
  result = true;

end:
  if (!result) {
    static_image_delete(&image);
  }
  jpeg_destroy_decompress(&cinfo);
  return image;
}

bool jpeg_decode_info(Stream* stream, ImageInfo* info) {
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  bool result = false;

  // Init
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if (setjmp(jerr.setjmp_buffer)) { LOGE(MSG("%s"), emsg); goto end; }
  jpeg_create_decompress(&cinfo);
  jpeg_custom_src(&cinfo, &custom_read, stream);
  jpeg_read_header(&cinfo, TRUE);

  // Assign image info
  info->width = cinfo.image_width;
  info->height = cinfo.image_height;
  info->format = IMAGE_FORMAT_JPEG;
  info->opaque = true;
  info->frame_count = 1;

  // Done
  result = true;

end:
  jpeg_destroy_decompress(&cinfo);
  return result;
}

bool jpeg_decode_buffer(Stream* stream, bool clip, uint32_t x, uint32_t y, uint32_t width,
    uint32_t height, int32_t config, uint32_t ratio, BufferContainer* container) {
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  bool too_small;
  bool result = false;
  uint32_t i;

  uint32_t ur_x;
  uint32_t r_x;
  uint32_t r_y;
  uint32_t r_width;
  uint32_t r_height;

  uint32_t d_width;
  uint32_t d_height;

  uint32_t components;

  bool soft_scale;

  uint32_t r_stride;
  uint32_t r_start_stride;
  uint32_t d_stride;

  uint8_t* r_line_1 = NULL;
  uint8_t* r_line_2 = NULL;
  uint8_t* d_buffer = NULL;
  uint8_t* d_line = NULL;

  RowFunc row_func = NULL;

  // Init
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if (setjmp(jerr.setjmp_buffer)) { LOGE(MSG("%s"), emsg); goto end; }
  jpeg_create_decompress(&cinfo);
  jpeg_custom_src(&cinfo, &custom_read, stream);
  jpeg_read_header(&cinfo, TRUE);

  // Set clip info
  if (!clip) {
    // Decode full image
    x = 0; y = 0; width = cinfo.image_width; height = cinfo.image_height;
  }

  // Set out color space
  // Set config to IMAGE_CONFIG_RGB_565 as default
  if (config == IMAGE_CONFIG_AUTO) {
    config = IMAGE_CONFIG_RGB_565;
  }
  if (config == IMAGE_CONFIG_RGBA_8888) {
    cinfo.out_color_space = JCS_EXT_RGBA;
    components = 4;
    row_func = &RGBA8888_to_RGBA8888_row;
  } else if (config == IMAGE_CONFIG_RGB_565) {
    config = IMAGE_CONFIG_RGB_565;
    cinfo.out_color_space = JCS_RGB565;
    components = 2;
    row_func = &RGB565_to_RGB565_row;
  } else {
    LOGE("Invalid config: %d", config);
    goto end;
  }

  // Fix width and height
  width = floor_uint32_t(width, ratio);
  height = floor_uint32_t(height, ratio);
  d_width = width / ratio;
  d_height = height / ratio;
  too_small = d_width == 0 || d_height == 0;

  // Create buffer
  d_buffer = container->create_buffer(container, MAX(d_width, 1), MAX(d_height, 1), config);
  if (d_buffer == NULL) { goto end; }

  // Check image ratio too large
  if (too_small) {
    // Ratio is too large, no need to decode.
    // Still treat it as success.
    LOGE("Ratio is too large!");
    result = true;
    goto end;
  }

  // Assign read info

  if (ratio == 2 || ratio == 4 || ratio == 8) {
    soft_scale = false;
    cinfo.scale_num = 8 / ratio;
    cinfo.scale_denom = 8;
    r_x = ur_x = x / ratio;
    r_y = y / ratio;
    r_width = width / ratio;
    r_height = height / ratio;
  } else {
    soft_scale = ratio != 1;
    r_x = ur_x = x, r_y = y, r_width = width, r_height = height;
  }

  // Start decompress
  jpeg_start_decompress(&cinfo);
  jpeg_crop_scanline(&cinfo, &r_x, &r_width);
  jpeg_skip_scanlines(&cinfo, r_y);

  r_stride = r_width * components;
  r_start_stride = (ur_x - r_x) * components;
  d_stride = d_width * components;

  if (!soft_scale) {
    r_line_1 = malloc(r_stride);
    if (r_line_1 == NULL) { WTF_OOM; goto end; }

    // Read lines
    d_line = d_buffer;
    for (i = 0; i < r_height; ++i) {
      jpeg_read_scanlines(&cinfo, &r_line_1, 1);
      // No soft scale need, set ratio as 1
      row_func(d_line, r_line_1 + r_start_stride, NULL, d_width, 1);
      d_line += d_stride;
    }
  } else {
    r_line_1 = malloc(r_stride);
    r_line_2 = malloc(r_stride);
    if (r_line_1 == NULL || r_line_2 == NULL) { WTF_OOM; goto end; }

    // Read lines
    uint32_t temp = ratio - 2;
    uint32_t skip_start = temp / 2;
    uint32_t skip_end = (temp + 1) / 2;

    d_line = d_buffer;
    for (i = 0; i < d_height; ++i) {
      jpeg_skip_scanlines(&cinfo, skip_start);

      jpeg_read_scanlines(&cinfo, &r_line_1, 1);
      jpeg_read_scanlines(&cinfo, &r_line_2, 1);
      row_func(d_line, r_line_1 + r_start_stride,
          r_line_2 + r_start_stride, d_width, ratio);

      jpeg_skip_scanlines(&cinfo, skip_end);

      d_line += d_stride;
    }
  }

  // It's not necessary to call jpeg_finish_decompress().
  // Also the jpeg might not be totally decoded.
  // Skip it will increase decode speed.

  // Done
  result = true;

end:
  free(r_line_1);
  free(r_line_2);
  if (d_buffer != NULL) {
    container->release_buffer(container, d_buffer);
  }
  jpeg_destroy_decompress(&cinfo);

  return result;
}

#endif // IMAGE_SUPPORT_JPEG
