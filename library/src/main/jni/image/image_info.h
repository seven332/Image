//
// Created by Hippo on 9/13/2016.
//

#ifndef IMAGE_IMAGE_INFO_H
#define IMAGE_IMAGE_INFO_H


#include <stdbool.h>
#include <stdint.h>


typedef struct {
  uint32_t width;
  uint32_t height;
  int32_t format;
  bool opaque;
  int32_t frame_count;
} ImageInfo;


#endif //IMAGE_IMAGE_INFO_H
