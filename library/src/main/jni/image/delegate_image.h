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

#ifndef IMAGE_DELEGATE_IMAGE_H
#define IMAGE_DELEGATE_IMAGE_H


#include <stdint.h>


typedef struct {
  uint32_t width;
  uint32_t height;
  int32_t index;
  uint8_t* buffer;
  uint8_t* shown;
  uint8_t* backup;
} DelegateImage;


DelegateImage* delegate_image_new(uint32_t width, uint32_t height);

void delegate_image_switch_data_backup(DelegateImage* image);

void delegate_image_backup(DelegateImage* image);

void delegate_image_restore(DelegateImage* image);

void delegate_image_apply(DelegateImage* image);

void delegate_image_delete(DelegateImage** image);


#endif //IMAGE_DELEGATE_IMAGE_H
