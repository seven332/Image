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
// Created by Hippo on 1/3/2016.
//

#ifndef IMAGE_IMAGE_JPEG_H
#define IMAGE_IMAGE_JPEG_H


#include <stdio.h>

#include "jpeglib.h"
#include "image_library.h"
#include "static_image.h"
#include "stream.h"


#define IMAGE_JPEG_DECODER_DESCRIPTION ("libjpeg-turbo " MAKESTRING(STRINGIZE, LIBJPEG_TURBO_VERSION))

#define IMAGE_JPEG_MAGIC_NUMBER_0 0xFF
#define IMAGE_JPEG_MAGIC_NUMBER_1 0xD8

bool jpeg_init(ImageLibrary* library);

bool jpeg_is_magic(Stream* stream);

const char* jpeg_get_description();

StaticImage* jpeg_decode(Stream* stream, bool unused1, bool* animated);

bool jpeg_decode_info(Stream* stream, ImageInfo* info);

bool jpeg_decode_buffer(Stream* stream, bool clip, uint32_t x, uint32_t y, uint32_t width,
    uint32_t height, int32_t config, uint32_t ratio, BufferContainer* container);


#endif // IMAGE_IMAGE_JPEG_H
