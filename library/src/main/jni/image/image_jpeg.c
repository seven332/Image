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

#include "image.h"
#include "image_jpeg.h"
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
  return stream->read(stream, buffer, 0, size);
}

StaticImage* jpeg_decode(Stream* stream) {
  StaticImage* image = NULL;
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  uint8_t* buffer = NULL;
  size_t stride;
  uint8_t* line_buffer_array[3];
  uint32_t read_lines;

  // Init
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if (setjmp(jerr.setjmp_buffer)) {
    LOGE(MSG("%s"), emsg);
    static_image_delete(&image);
    jpeg_destroy_decompress(&cinfo);
    return NULL;
  }
  jpeg_create_decompress(&cinfo);
  jpeg_custom_src(&cinfo, &custom_read, stream);
  jpeg_read_header(&cinfo, TRUE);

  // Start decompress
  cinfo.out_color_space = JCS_EXT_RGBA;
  jpeg_start_decompress(&cinfo);

  // New static image
  image = static_image_new(cinfo.output_width, cinfo.output_height);
  if (image == NULL) {
    jpeg_destroy_decompress(&cinfo);
    return NULL;
  }

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

  // Finish decompress
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  // Fill jpeg
  image->format = IMAGE_FORMAT_JPEG;
  image->opaque = true;

  return image;
}


#endif // IMAGE_SUPPORT_JPEG
