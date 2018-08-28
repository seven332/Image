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

#ifndef IMAGE_IMAGE_H
#define IMAGE_IMAGE_H


#include <stdbool.h>

#include "com_hippo_image_Image.h"
#include "static_image.h"
#include "image_info.h"
#include "buffer_container.h"
#include "stream.h"
#include "image_library.h"

#define IMAGE_FORMAT_UNKNOWN com_hippo_image_Image_FORMAT_UNKNOWN

#define IMAGE_FORMAT_PLAIN com_hippo_image_Image_FORMAT_PLAIN

#define IMAGE_FORMAT_BMP com_hippo_image_Image_FORMAT_BMP

#define IMAGE_FORMAT_JPEG com_hippo_image_Image_FORMAT_JPEG

#define IMAGE_FORMAT_PNG com_hippo_image_Image_FORMAT_PNG

#define IMAGE_FORMAT_GIF com_hippo_image_Image_FORMAT_GIF

#define IMAGE_FORMAT_WEBP com_hippo_image_Image_FORMAT_WEBP

#define IMAGE_FORMAT_MAX_COUNT 6

void init_image_libraries();

void decode(Stream* stream, bool partially, bool* animated, void** image);

bool decode_info(Stream* stream, ImageInfo* info);

bool decode_buffer(Stream* stream, bool clip, uint32_t x, uint32_t y, uint32_t width,
    uint32_t height, int32_t config, uint32_t ratio, BufferContainer* container);

StaticImage* create(uint32_t width, uint32_t height, const uint8_t* data);

int get_supported_formats(int *formats);

const char *get_library_description(int format);


#endif //IMAGE_IMAGE_H
