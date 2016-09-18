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
    uint32_t height, uint8_t config, uint32_t ratio, BufferContainer* container) {
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  void* buffer = NULL;
  bool too_small;
  bool result = false;
  uint32_t i;

  uint32_t r_x;
  uint32_t r_y;
  uint32_t r_width;
  uint32_t r_height;

  uint32_t d_width;
  uint32_t d_height;

  uint32_t components;
  uint32_t channels;

  void (*average_step) (uint8_t*, uint8_t*, uint8_t*, uint32_t, uint32_t);
  void (*fill_line) (uint8_t*, const uint8_t*, uint32_t);

  uint8_t* r_line = NULL;
  uint8_t* m_line_quotient = NULL;
  uint8_t* m_line_remainder = NULL;
  uint8_t* d_line = NULL;

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
  if (config == IMAGE_CONFIG_ARGB_8888) {
    cinfo.out_color_space = JCS_EXT_RGBA;
    components = 4;
    channels = 4;
    average_step = &average_step_RGBA_8888;
    fill_line = &RGBA_8888_fill_RGBA_8888;
  } else {
    config = IMAGE_CONFIG_RGB_565;
    cinfo.out_color_space = JCS_RGB565;
    components = 2;
    channels = 3;
    average_step = &average_step_RGB_565;
    fill_line = &RGB_565_plain_fill_RGB_565;
  }

  // Fix width and height
  width = floor_uint32_t(width, ratio);
  height = floor_uint32_t(height, ratio);
  d_width = width / ratio;
  d_height = height / ratio;
  too_small = d_width == 0 || d_height == 0;

  // Create buffer
  buffer = container->create_buffer(container, MAX(d_width, 1), MAX(d_height, 1), config);
  if (buffer == NULL) { goto end; }

  // Check image ratio too large
  if (too_small) {
    // Ratio is too large, no need to decode.
    // Still treat it as success.
    LOGE("Ratio is too large!");
    result = true;
    goto end;
  }

  // Assign read info
  r_x = x, r_y = y, r_width = width, r_height = height;

  // Start decompress
  jpeg_start_decompress(&cinfo);
  jpeg_crop_scanline(&cinfo, &r_x, &r_width);
  jpeg_skip_scanlines(&cinfo, r_y);

  // Malloc
  r_line = malloc(r_width * components);
  if (r_line == NULL) { WTF_OM; goto end; }
  if (ratio != 1) {
    m_line_quotient = malloc(d_width * channels);
    m_line_remainder = malloc(d_width * channels);
    if (m_line_quotient == NULL || m_line_remainder == NULL) { WTF_OM; goto end; }
  }

  // Decompress
  if (ratio == 1) {
    // No subsample, just copy
    for (i = 0, d_line = buffer; i < r_height; ++i, d_line += d_width * components) {
      jpeg_read_scanlines(&cinfo, &r_line, 1);
      memcpy(d_line, r_line + (x - r_x) * components, d_width * components);
    }
  } else {
    // subsample
    d_line = buffer;
    memset(m_line_quotient, 0, d_width * channels);
    memset(m_line_remainder, 0, d_width * channels);
    for (i = 0; i < r_height; ++i) {
      jpeg_read_scanlines(&cinfo, &r_line, 1);
      average_step(r_line + (x - r_x) * components, m_line_quotient, m_line_remainder, width, ratio);

      if (i % ratio == ratio - 1) {
        fill_line(d_line, m_line_quotient, d_width);
        d_line += d_width * components;

        // Clear line_quotient and line_remainder
        memset(m_line_quotient, 0, d_width * channels);
        memset(m_line_remainder, 0, d_width * channels);
      }
    }
  }

  // It's not necessary to call jpeg_finish_decompress().
  // Also the jpeg might not be totally decoded.
  // Skip it will increase decode speed.

  // Done
  result = true;

end:
  free(r_line);
  free(m_line_quotient);
  free(m_line_remainder);
  if (buffer != NULL) {
    container->release_buffer(container, buffer);
  }
  jpeg_destroy_decompress(&cinfo);

  return result;
}

#endif // IMAGE_SUPPORT_JPEG
