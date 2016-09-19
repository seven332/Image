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

Converter* converter_new(uint32_t dst_width, int32_t src_config, int32_t dst_config, uint32_t ratio);

void converter_delete(Converter** conv);

void convert(uint8_t* dst, int32_t dst_config,
    uint32_t dst_w, uint32_t dst_h,
    int32_t dst_x, int32_t dst_y,
    uint8_t* src, int32_t src_config,
    uint32_t src_w, uint32_t src_h,
    int32_t src_x, int32_t src_y,
    uint32_t width, uint32_t height,
    uint32_t ratio, bool fill_blank, uint32_t fill_color);


#endif //IMAGE_IMAGE_CONVERT_H
