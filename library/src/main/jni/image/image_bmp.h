//
// Created by Hippo on 8/10/2016.
//

#ifndef IMAGE_IMAGE_BMP_H
#define IMAGE_IMAGE_BMP_H

#include "static_image.h"
#include "stream.h"


#define IMAGE_BMP_DECODER_DESCRIPTION "image_bmp 0.1.0"

#define IMAGE_BMP_MAGIC_NUMBER_0 0x42 // 'B'
#define IMAGE_BMP_MAGIC_NUMBER_1 0x4D // 'M'


StaticImage* bmp_decode(Stream* stream);


#endif //IMAGE_IMAGE_BMP_H
