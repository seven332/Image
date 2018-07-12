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

#ifndef IMAGE_IMAGE_BMP_H
#define IMAGE_IMAGE_BMP_H

#include "static_image.h"
#include "stream.h"


#define IMAGE_BMP_DECODER_DESCRIPTION "image_bmp 0.1.0"

#define IMAGE_BMP_MAGIC_NUMBER_0 0x42 // 'B'
#define IMAGE_BMP_MAGIC_NUMBER_1 0x4D // 'M'


StaticImage* bmp_decode(Stream* stream);


#endif //IMAGE_IMAGE_BMP_H
