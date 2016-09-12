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

#include <malloc.h>

#include "image.h"
#include "image_plain.h"
#include "image_bmp.h"
#include "image_jpeg.h"
#include "image_png.h"
#include "image_gif.h"
#include "patched_stream.h"
#include "../log.h"


static int8_t get_format(Stream* stream, uint8_t* magic) {
  size_t read = stream->read(stream, magic, 0, 2);

  if (read == 2) {
#ifdef IMAGE_FORMAT_BMP
    if (magic[0] == IMAGE_BMP_MAGIC_NUMBER_0 && magic[1] == IMAGE_BMP_MAGIC_NUMBER_1) {
      return IMAGE_FORMAT_BMP;
    }
#endif
#ifdef IMAGE_SUPPORT_JPEG
    if (magic[0] == IMAGE_JPEG_MAGIC_NUMBER_0 && magic[1] == IMAGE_JPEG_MAGIC_NUMBER_1) {
      return IMAGE_FORMAT_JPEG;
    }
#endif
#ifdef IMAGE_SUPPORT_PNG
    if (magic[0] == IMAGE_PNG_MAGIC_NUMBER_0 && magic[1] == IMAGE_PNG_MAGIC_NUMBER_1) {
      return IMAGE_FORMAT_PNG;
    }
#endif
#ifdef IMAGE_SUPPORT_GIF
    if (magic[0] == IMAGE_GIF_MAGIC_NUMBER_0 && magic[1] == IMAGE_GIF_MAGIC_NUMBER_1) {
      return IMAGE_FORMAT_GIF;
    }
#endif
    LOGE(MSG("Can't recognize the two magic number: %d, %d"), magic[0], magic[1]);
  } else {
    LOGE(MSG("Can't read two magic number from stream"));
  }

  return IMAGE_FORMAT_UNKNOWN;
}

bool decode(Stream* stream, bool partially, bool* animated, void** image) {
  Stream* patched_stream;
  uint8_t magic[2];
  int8_t format;

  // Get image format
  format = get_format(stream, magic);
  if (format == IMAGE_FORMAT_UNKNOWN) {
    stream->close(&stream);
    return false;
  }

  // Create patched stream
  patched_stream = patched_stream_new(stream, magic, 2);
  if (patched_stream == NULL) {
    stream->close(&stream);
    return false;
  }

  // Decode
  switch (format) {
#ifdef IMAGE_SUPPORT_BMP
    case IMAGE_FORMAT_BMP:
      // TODO
#endif
#ifdef IMAGE_SUPPORT_JPEG
    case IMAGE_FORMAT_JPEG:
      *animated = false;
      *image = jpeg_decode(patched_stream);
      break;
#endif
#ifdef IMAGE_SUPPORT_PNG
    case IMAGE_FORMAT_PNG:
      *image = png_decode(patched_stream, partially, animated);
      break;
#endif
#ifdef IMAGE_SUPPORT_GIF
    case IMAGE_FORMAT_GIF:
      *animated = true;
      *image = gif_decode(patched_stream, partially);
      break;
#endif
    default:
      *image = NULL;
      break;
  }

  // Don't close stream if animated image is no totally decoded.
  if (*image == NULL || !*animated || ((AnimatedImage*) *image)->completed) {
    patched_stream->close(&patched_stream);
  }

  return *image != NULL;
}

StaticImage* create(uint32_t width, uint32_t height, const uint8_t* data) {
#ifdef IMAGE_SUPPORT_PLAIN
    return plain_create(width, height, data);
#else
    return NULL;
#endif
}

int get_supported_formats(int *formats)
{
  int i = 0;
#ifdef IMAGE_SUPPORT_BMP
  formats[i++] = IMAGE_FORMAT_BMP;
#endif
#ifdef IMAGE_SUPPORT_JPEG
  formats[i++] = IMAGE_FORMAT_JPEG;
#endif
#ifdef IMAGE_SUPPORT_PNG
  formats[i++] = IMAGE_FORMAT_PNG;
#endif
#ifdef IMAGE_SUPPORT_GIF
  formats[i++] = IMAGE_FORMAT_GIF;
#endif
  return i;
}

const char *get_decoder_description(int format)
{
  switch (format) {
#ifdef IMAGE_SUPPORT_BMP
    case IMAGE_FORMAT_BMP:
      return IMAGE_BMP_DECODER_DESCRIPTION;
#endif
#ifdef IMAGE_SUPPORT_JPEG
    case IMAGE_FORMAT_JPEG:
      return IMAGE_JPEG_DECODER_DESCRIPTION;
#endif
#ifdef IMAGE_SUPPORT_PNG
    case IMAGE_FORMAT_PNG:
      return IMAGE_PNG_DECODER_DESCRIPTION;
#endif
#ifdef IMAGE_SUPPORT_GIF
    case IMAGE_FORMAT_GIF:
      return IMAGE_GIF_DECODER_DESCRIPTION;
#endif
    default:
      return NULL;
  }
}
