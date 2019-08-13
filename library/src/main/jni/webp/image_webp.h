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

#ifndef IMAGE_IMAGE_WEBP_H
#define IMAGE_IMAGE_WEBP_H


#include <stdbool.h>

#include "src/webp/decode.h"
#include "src/webp/mux_types.h"
#include "src/webp/demux.h"
#include "image_library.h"
#include "stream.h"


#define IMAGE_WEBP_DECODER_DESCRIPTION ("libwebp " MAKESTRING(STRINGIZE, DEC_MAJ_VERSION) "." \
                                        MAKESTRING(STRINGIZE, DEC_MIN_VERSION) "." \
                                        MAKESTRING(STRINGIZE, DEC_REV_VERSION))

#define IMAGE_WEBP_MAGIC_NUMBER_0 0x52
#define IMAGE_WEBP_MAGIC_NUMBER_1 0x49


bool webp_init(ImageLibrary* library);

bool webp_is_magic(Stream* stream);

const char* webp_get_description();

void* webp_decode(Stream* stream, bool partially, bool* animated);

bool webp_decode_info(Stream* stream, ImageInfo* info);

bool webp_decode_buffer(Stream* stream, bool clip, uint32_t x, uint32_t y, uint32_t width,
                       uint32_t height, int32_t config, uint32_t ratio, BufferContainer* container);


#endif // IMAGE_IMAGE_WEBP_H
