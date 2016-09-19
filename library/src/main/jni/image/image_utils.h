//
// Created by Hippo on 12/27/2015.
//

#ifndef IMAGE_IMAGE_UTILS_H
#define IMAGE_IMAGE_UTILS_H


#include <stdlib.h>
#include <stdbool.h>


void memset_uint32_t(uint32_t* dst, uint32_t val, size_t size);

uint32_t floor_uint32_t(uint32_t num, uint32_t multiple);

uint32_t ceil_uint32_t(uint32_t num, uint32_t multiple);


#endif //IMAGE_IMAGE_UTILS_H
