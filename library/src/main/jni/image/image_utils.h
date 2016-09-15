//
// Created by Hippo on 12/27/2015.
//

#ifndef IMAGE_IMAGE_UTILS_H
#define IMAGE_IMAGE_UTILS_H

#include <stdbool.h>

uint32_t floor_uint32_t(uint32_t num, uint32_t multiple);

void average_step_RGBA_8888(uint8_t* line, uint8_t* quotient,
    uint8_t* remainder, uint32_t width, uint32_t ratio);

void average_step_RGB_565(uint8_t* line, uint8_t* quotient,
    uint8_t* remainder, uint32_t width, uint32_t ratio);

void RGBA_8888_fill_RGBA_8888(uint8_t* dst, const uint8_t* src, uint32_t size);

void RGB_565_888_fill_RGB_565(uint8_t* dst, const uint8_t* src, uint32_t size);

void copy_pixels(void* dst, int dst_w, int dst_h, int dst_x, int dst_y,
    const void* src, int src_w, int src_h, int src_x, int src_y,
    int width, int height, int ratio, bool fill_blank, int fill_color);

#endif //IMAGE_IMAGE_UTILS_H
