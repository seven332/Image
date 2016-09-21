//
// Created by Hippo on 9/20/2016.
//

#ifndef IMAGE_IMAGE_CONVERT_NEON_H
#define IMAGE_IMAGE_CONVERT_NEON_H


#include <stdlib.h>
#include <stdbool.h>


bool is_support_neon();


// count must be a multiple of 8
void rgba8888_to_rgb565_neon(uint8_t* dst, const uint8_t* src, uint32_t count);


void RGBA8888_to_RGB565_row_internal_1_neon(const uint8_t* src, uint32_t src_x, uint8_t* dst, uint32_t dst_width);


#endif //IMAGE_IMAGE_CONVERT_NEON_H
