//
// Created by Hippo on 9/18/2016.
//

#ifndef IMAGE_IMAGE_CONVERT_H
#define IMAGE_IMAGE_CONVERT_H


#include <stdint.h>
#include <string.h>
#include <stdbool.h>


struct CONVERTER;
typedef struct CONVERTER Converter;

typedef void (*ConvertFunc)(Converter* conv,
    const uint8_t* src, uint32_t src_x, uint32_t src_width,
    uint8_t* dst, uint32_t dst_width, uint32_t ratio);

struct CONVERTER {
  uint32_t* r;
  uint32_t* g;
  uint32_t* b;
  uint32_t* a;
  ConvertFunc convert_func;
};

Converter* converter_new(uint32_t dst_width, uint8_t src_config, uint8_t dst_config, uint32_t ratio);

void convert_delete(Converter** conv);

void RGBA8888_to_RGBA8888_row(Converter* conv,
    const uint8_t* src, uint32_t src_x, uint32_t src_width,
    uint8_t* dst, uint32_t dst_width, uint32_t ratio);

void RGBA8888_to_RGB565_row(Converter* conv,
    const uint8_t* src, uint32_t src_x, uint32_t src_width,
    uint8_t* dst, uint32_t dst_width, uint32_t ratio);

void RGB565_to_RGBA8888_row(Converter* conv,
    const uint8_t* src, uint32_t src_x, uint32_t src_width,
    uint8_t* dst, uint32_t dst_width, uint32_t ratio);

void RGB565_to_RGB565_row(Converter* conv,
    const uint8_t* src, uint32_t src_x, uint32_t src_width,
    uint8_t* dst, uint32_t dst_width, uint32_t ratio);

#endif //IMAGE_IMAGE_CONVERT_H
