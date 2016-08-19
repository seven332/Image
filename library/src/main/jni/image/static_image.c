//
// Created by Hippo on 8/5/2016.
//

#include <malloc.h>

#include "static_image.h"
#include "../log.h"


StaticImage* static_image_new(uint32_t width, uint32_t height) {
  StaticImage* image = malloc(sizeof(StaticImage));
  uint8_t* buffer = malloc(width * height * 4);
  if (image == NULL || buffer == NULL) {
    WTF_OM;
    free(image);
    free(buffer);
    return NULL;
  }

  image->width = width;
  image->height = height;
  image->buffer = buffer;

  return image;
}

void static_image_delete(StaticImage** image) {
  if (image == NULL || *image == NULL) {
    return;
  }

  free((*image)->buffer);
  (*image)->buffer = NULL;
  free(*image);
  *image = NULL;
}
