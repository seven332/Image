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

#include <malloc.h>

#include "static_image.h"
#include "../log.h"


StaticImage* static_image_new(uint32_t width, uint32_t height) {
  StaticImage* image = malloc(sizeof(StaticImage));
  uint8_t* buffer = malloc(width * height * 4);
  if (image == NULL || buffer == NULL) {
    WTF_OOM;
    free(image);
    free(buffer);
    return NULL;
  }

  image->width = width;
  image->height = height;
  image->buffer = buffer;

  return image;
}

void static_image_delete(StaticImage** image) {
  if (image == NULL || *image == NULL) {
    return;
  }

  free((*image)->buffer);
  (*image)->buffer = NULL;
  free(*image);
  *image = NULL;
}
