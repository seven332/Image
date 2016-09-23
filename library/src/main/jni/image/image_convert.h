//
// Created by Hippo on 9/18/2016.
//

#ifndef IMAGE_IMAGE_CONVERT_H
#define IMAGE_IMAGE_CONVERT_H


#include <stdint.h>
#include <string.h>
#include <stdbool.h>


typedef void (*RowFunc)(uint8_t* dst,
    const uint8_t* src1, const uint8_t* src2,
    uint32_t d_width, uint32_t ratio);


void RGBA8888_to_RGBA8888_row(uint8_t* dst,
    const uint8_t* src1, const uint8_t* src2,
    uint32_t d_width, uint32_t ratio);

void RGBA8888_to_RGB565_row(uint8_t* dst,
    const uint8_t* src1, const uint8_t* src2,
    uint32_t d_width, uint32_t ratio);

void RGB565_to_RGB565_row(uint8_t* dst,
    const uint8_t* src1, const uint8_t* src2,
    uint32_t d_width, uint32_t ratio);


void convert(uint8_t* dst, int32_t dst_config,
    uint32_t dst_w, uint32_t dst_h,
    int32_t dst_x, int32_t dst_y,
    uint8_t* src, int32_t src_config,
    uint32_t src_w, uint32_t src_h,
    int32_t src_x, int32_t src_y,
    uint32_t width, uint32_t height,
    uint32_t ratio, bool fill_blank, uint32_t fill_color);


#endif //IMAGE_IMAGE_CONVERT_H
