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

#ifndef IMAGE_ANIMATED_IMAGE_H
#define IMAGE_ANIMATED_IMAGE_H


#include <stdbool.h>
#include <stdint.h>

#include "delegate_image.h"
#include "stream.h"


struct ANIMATED_IMAGE;
typedef struct ANIMATED_IMAGE AnimatedImage;

struct ANIMATED_IMAGE {
  uint32_t width;
  uint32_t height;
  int32_t format;
  bool opaque;
  bool completed;
  void* data;
  Stream* (*get_stream)(AnimatedImage* image);
  void (*complete)(AnimatedImage* image);
  uint32_t (*get_frame_count)(AnimatedImage* image);
  uint32_t (*get_delay)(AnimatedImage* image, uint32_t frame); // ms
  uint32_t (*get_byte_count)(AnimatedImage* image);
  void (*advance)(AnimatedImage* image, DelegateImage* dImage);
  void (*recycle)(AnimatedImage** image);
};


#endif //IMAGE_ANIMATED_IMAGE_H
