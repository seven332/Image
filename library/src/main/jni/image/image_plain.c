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
// Created by Hippo on 5/3/2016.
//

#include "config.h"
#ifdef IMAGE_SUPPORT_PLAIN


#include <stdlib.h>

#include "image.h"
#include "image_plain.h"
#include "../log.h"


StaticImage* plain_create(uint32_t width, uint32_t height, const uint8_t* buffer) {
  StaticImage* image;

  image = static_image_new(width, height);
  if (image == NULL) { WTF_OOM; return NULL; }

  memcpy(image->buffer, buffer, width * height * 4);

  image->format = IMAGE_FORMAT_PLAIN;
  // Set opaque to false to safe
  image->opaque = false;
  return image;
}


#endif // IMAGE_SUPPORT_PLAIN
