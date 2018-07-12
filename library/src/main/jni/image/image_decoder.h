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

#ifndef IMAGE_IMAGE_DECODER_H
#define IMAGE_IMAGE_DECODER_H


#include "com_hippo_image_BitmapDecoder.h"


#define IMAGE_CONFIG_INVALID   -1;
#define IMAGE_CONFIG_AUTO      com_hippo_image_BitmapDecoder_CONFIG_AUTO
#define IMAGE_CONFIG_RGB_565   com_hippo_image_BitmapDecoder_CONFIG_RGB_565
#define IMAGE_CONFIG_RGBA_8888 com_hippo_image_BitmapDecoder_CONFIG_RGBA_8888


static inline bool is_explicit_config(int32_t config) {
  return config == IMAGE_CONFIG_RGB_565 || config == IMAGE_CONFIG_RGBA_8888;
}


static inline uint32_t get_depth_for_config(int32_t config) {
  switch (config) {
    case IMAGE_CONFIG_RGB_565:
      return 2;
    case IMAGE_CONFIG_RGBA_8888:
      return 4;
    default:
      return 0;
  }
}


#endif //IMAGE_IMAGE_DECODER_H
