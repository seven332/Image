//
// Created by Hippo on 8/5/2016.
//

#ifndef IMAGE_STATIC_IMAGE_H
#define IMAGE_STATIC_IMAGE_H


#include <stdbool.h>
#include <stdint.h>


typedef struct {
  uint32_t width;
  uint32_t height;
  int8_t format;
  bool opaque;
  uint8_t* buffer;
} StaticImage;


StaticImage* static_image_new(uint32_t width, uint32_t height);

void static_image_delete(StaticImage** image);


#endif //IMAGE_STATIC_IMAGE_H
