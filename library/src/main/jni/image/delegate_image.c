//
// Created by Hippo on 8/10/2016.
//

#include <malloc.h>
#include <string.h>

#include "delegate_image.h"
#include "../log.h"


DelegateImage* delegate_image_new(uint32_t width, uint32_t height) {
  DelegateImage* image = malloc(sizeof(DelegateImage));
  uint8_t* buffer = malloc(width * height * 4);
  if (image == NULL || buffer == NULL) {
    WTF_OM;
    free(image);
    free(buffer);
    return NULL;
  }

  image->width = width;
  image->height = height;
  image->index = -1;
  image->buffer = buffer;
  image->backup = NULL;

  return image;
}

void delegate_image_switch_data_backup(DelegateImage* image) {
  void* temp;

  if (image->backup == NULL) {
    delegate_image_backup(image);
  } else {
    temp = image->buffer;
    image->buffer = image->backup;
    image->backup = temp;
  }
}

void delegate_image_backup(DelegateImage* image) {
  if (image->backup == NULL) {
    image->backup = malloc(image->width * image->height * 4);
    if (image->backup == NULL) {
      WTF_OM;
      return;
    }
  }

  memcpy(image->backup, image->buffer, image->width * image->height * 4);
}

void delegate_image_restore(DelegateImage* image) {
  if (image->backup == NULL) {
    LOGE(MSG("Can't restore on null backup"));
  } else {
    memcpy(image->buffer, image->backup, image->width * image->height * 4);
  }
}

void delegate_image_delete(DelegateImage** image) {
  if (image == NULL || *image == NULL) {
    return;
  }

  free((*image)->buffer);
  (*image)->buffer = NULL;
  free((*image)->backup);
  (*image)->backup = NULL;
  free(*image);
  *image = NULL;
}
