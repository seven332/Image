//
// Created by Hippo on 9/20/2016.
//

#ifndef IMAGE_IMAGE_CONVERT_NEON_H
#define IMAGE_IMAGE_CONVERT_NEON_H


#include <stdlib.h>
#include <stdbool.h>


bool is_support_neon();


// count should be a multiple of 8
void rgba8888_to_rgba8888_neon_2(uint8_t* dst,
    const uint8_t* src1, const uint8_t* src2, uint32_t count);

// count should be a multiple of 8
void rgba8888_to_rgb565_neon(uint8_t* dst,
    const uint8_t* src, uint32_t count);

// count should be a multiple of 8
void rgb565_to_rgb565_neon_2(uint8_t* dst,
    const uint8_t* src1, const uint8_t* src2, uint32_t count);


void RGBA8888_to_RGBA8888_row_internal_2_neon(
    uint8_t* dst, const uint8_t* src1, const uint8_t* src2,
    uint32_t d_width, uint32_t ratio);

void RGBA8888_to_RGB565_row_internal_1_neon(
    uint8_t* dst, const uint8_t* src, uint32_t width);

void RGB565_to_RGB565_row_internal_2_neon(
    uint8_t* dst, const uint8_t* src1, const uint8_t* src2,
    uint32_t d_width, uint32_t ratio);


#endif //IMAGE_IMAGE_CONVERT_NEON_H
