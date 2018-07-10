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

#ifndef IMAGE_IMAGE_PNG_H
#define IMAGE_IMAGE_PNG_H


#include <stdbool.h>

#include "png.h"
#include "image_library.h"
#include "stream.h"


#define IMAGE_PNG_DECODER_DESCRIPTION ("libpng " PNG_LIBPNG_VER_STRING)

#define IMAGE_PNG_MAGIC_NUMBER_0 0x89
#define IMAGE_PNG_MAGIC_NUMBER_1 0x50


bool png_init(ImageLibrary* library);

bool png_is_magic(Stream* stream);

const char* png_get_description();

void* png_decode(Stream* stream, bool partially, bool* animated);

bool png_decode_info(Stream* stream, ImageInfo* info);

bool png_decode_buffer(Stream* stream, bool clip, uint32_t x, uint32_t y, uint32_t width,
    uint32_t height, int32_t config, uint32_t ratio, BufferContainer* container);


#endif // IMAGE_IMAGE_PNG_H
