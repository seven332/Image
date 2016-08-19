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


#include "config.h"
#ifdef IMAGE_SUPPORT_JPEG


#include "static_image.h"
#include "stream/stream.h"


#define IMAGE_JPEG_DECODER_DESCRIPTION ("libjpeg-turbo " MAKESTRING(STRINGIZE, LIBJPEG_TURBO_VERSION))

#define IMAGE_JPEG_MAGIC_NUMBER_0 0xFF
#define IMAGE_JPEG_MAGIC_NUMBER_1 0xD8


StaticImage* jpeg_decode(Stream* stream);


#endif // IMAGE_SUPPORT_JPEG


#endif // IMAGE_IMAGE_JPEG_H
