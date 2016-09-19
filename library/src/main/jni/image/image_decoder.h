//
// Created by Hippo on 9/13/2016.
//

#ifndef IMAGE_IMAGE_DECODER_H
#define IMAGE_IMAGE_DECODER_H


#include "com_hippo_image_BitmapDecoder.h"


#define IMAGE_CONFIG_INVALID   -1;
#define IMAGE_CONFIG_AUTO      com_hippo_image_BitmapDecoder_CONFIG_AUTO
#define IMAGE_CONFIG_RGB_565   com_hippo_image_BitmapDecoder_CONFIG_RGB_565
#define IMAGE_CONFIG_RGBA_8888 com_hippo_image_BitmapDecoder_CONFIG_RGBA_8888


inline bool is_explicit_config(int32_t config) {
  return config == IMAGE_CONFIG_RGB_565 || config == IMAGE_CONFIG_RGBA_8888;
}


inline uint32_t get_depth_for_config(int32_t config) {
  switch (config) {
    case IMAGE_CONFIG_RGB_565:
      return 2;
    case IMAGE_CONFIG_RGBA_8888:
      return 4;
    default:
      return 0;
  }
}


#endif //IMAGE_IMAGE_DECODER_H
