//
// Created by Hippo on 12/27/2015.
//

#ifndef IMAGE_IMAGE_UTILS_H
#define IMAGE_IMAGE_UTILS_H

#include <stdbool.h>

void copy_pixels(void* dst, int dst_w, int dst_h, int dst_x, int dst_y,
    const void* src, int src_w, int src_h, int src_x, int src_y,
    int width, int height, int ratio, bool fill_blank, int fill_color);

#endif //IMAGE_IMAGE_UTILS_H
