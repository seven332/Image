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

#ifndef IMAGE_IMAGE_GIF_H
#define IMAGE_IMAGE_GIF_H

#include <stdbool.h>

#include "gif_lib.h"
#include "animated_image.h"
#include "image_library.h"
#include "stream.h"


#define IMAGE_GIF_DECODER_DESCRIPTION ("giflib " MAKESTRING(STRINGIZE, GIFLIB_MAJOR) "." \
                                       MAKESTRING(STRINGIZE, GIFLIB_MINOR) "." \
                                       MAKESTRING(STRINGIZE, GIFLIB_RELEASE))

#define IMAGE_GIF_MAGIC_NUMBER_0 0x47
#define IMAGE_GIF_MAGIC_NUMBER_1 0x49

bool gif_init(ImageLibrary* library);

bool gif_is_magic(Stream* stream);

const char* gif_get_description();

AnimatedImage* gif_decode(Stream* stream, bool partially, bool* animated);

bool gif_decode_info(Stream* stream, ImageInfo* info);


#endif // IMAGE_IMAGE_GIF_H
