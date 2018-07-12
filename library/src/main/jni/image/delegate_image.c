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
#include <string.h>

#include "delegate_image.h"
#include "../log.h"


DelegateImage* delegate_image_new(uint32_t width, uint32_t height) {
  DelegateImage* image = malloc(sizeof(DelegateImage));
  uint8_t* buffer = malloc(width * height * 4);
  uint8_t* shown = malloc(width * height * 4);
  if (image == NULL || buffer == NULL) {
    WTF_OOM;
    free(image);
    free(buffer);
    free(shown);
    return NULL;
  }

  image->width = width;
  image->height = height;
  image->index = -1;
  image->buffer = buffer;
  image->shown = shown;
  image->backup = NULL;

  return image;
}

void delegate_image_switch_data_backup(DelegateImage* image) {
  void* temp;

  if (image->backup == NULL) {
    delegate_image_backup(image);
  } else {
    temp = image->buffer;
    image->buffer = image->backup;
    image->backup = temp;
  }
}

void delegate_image_backup(DelegateImage* image) {
  if (image->backup == NULL) {
    image->backup = malloc(image->width * image->height * 4);
    if (image->backup == NULL) {
      WTF_OOM;
      return;
    }
  }

  memcpy(image->backup, image->buffer, image->width * image->height * 4);
}

void delegate_image_restore(DelegateImage* image) {
  if (image->backup == NULL) {
    LOGE(MSG("Can't restore on null backup"));
  } else {
    memcpy(image->buffer, image->backup, image->width * image->height * 4);
  }
}

void delegate_image_apply(DelegateImage* image) {
  memcpy(image->shown, image->buffer, image->width * image->height * 4);
}

void delegate_image_delete(DelegateImage** image) {
  if (image == NULL || *image == NULL) {
    return;
  }

  free((*image)->buffer);
  (*image)->buffer = NULL;
  free((*image)->shown);
  (*image)->shown = NULL;
  free((*image)->backup);
  (*image)->backup = NULL;
  free(*image);
  *image = NULL;
}
