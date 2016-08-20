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

#ifndef IMAGE_IMAGE_H
#define IMAGE_IMAGE_H


#include <stdbool.h>

#include "config.h"
#include "com_hippo_image_Image.h"
#include "static_image.h"
#include "stream.h"


#define IMAGE_FORMAT_UNKNOWN com_hippo_image_Image_FORMAT_UNKNOWN

#ifdef IMAGE_SUPPORT_PLAIN
#define IMAGE_FORMAT_PLAIN com_hippo_image_Image_FORMAT_PLAIN
#endif

#ifdef IMAGE_SUPPORT_BMP
#define IMAGE_FORMAT_BMP com_hippo_image_Image_FORMAT_BMP
#endif

#ifdef IMAGE_SUPPORT_JPEG
#define IMAGE_FORMAT_JPEG com_hippo_image_Image_FORMAT_JPEG
#endif

#ifdef IMAGE_SUPPORT_PNG
#define IMAGE_FORMAT_PNG com_hippo_image_Image_FORMAT_PNG
#endif

#ifdef IMAGE_SUPPORT_GIF
#define IMAGE_FORMAT_GIF com_hippo_image_Image_FORMAT_GIF
#endif

#define IMAGE_MAX_SUPPORTED_FORMAT_COUNT 4


bool decode(Stream* stream, bool partially, bool* animated, void** image);

StaticImage* create(uint32_t width, uint32_t height, const uint8_t* data);

int get_supported_formats(int *formats);

const char *get_decoder_description(int format);


#endif //IMAGE_IMAGE_H
