//
// Created by Hippo on 8/5/2016.
//

#ifndef IMAGE_ANIMATED_IMAGE_H
#define IMAGE_ANIMATED_IMAGE_H


#include <stdbool.h>
#include <stdint.h>

#include "delegate_image.h"
#include "stream/stream.h"


struct ANIMATED_IMAGE;
typedef struct ANIMATED_IMAGE AnimatedImage;

struct ANIMATED_IMAGE {
  uint32_t width;
  uint32_t height;
  int8_t format;
  bool opaque;
  bool completed;
  void* data;
  Stream* (*get_stream)(AnimatedImage* image);
  void (*complete)(AnimatedImage* image);
  uint32_t (*get_frame_count)(AnimatedImage* image);
  uint32_t (*get_delay)(AnimatedImage* image, uint32_t frame); // ms
  uint32_t (*get_byte_count)(AnimatedImage* image);
  void (*advance)(AnimatedImage* image, DelegateImage* dImage);
  void (*recycle)(AnimatedImage** image);
};


#endif //IMAGE_ANIMATED_IMAGE_H
