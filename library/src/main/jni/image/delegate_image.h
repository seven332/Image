//
// Created by Hippo on 8/10/2016.
//

#ifndef IMAGE_DELEGATE_IMAGE_H
#define IMAGE_DELEGATE_IMAGE_H


#include <stdint.h>


typedef struct {
  uint32_t width;
  uint32_t height;
  int32_t index;
  uint8_t* buffer;
  uint8_t* backup;
} DelegateImage;


DelegateImage* delegate_image_new(uint32_t width, uint32_t height);

void delegate_image_switch_data_backup(DelegateImage* image);

void delegate_image_backup(DelegateImage* image);

void delegate_image_restore(DelegateImage* image);

void delegate_image_delete(DelegateImage** image);


#endif //IMAGE_DELEGATE_IMAGE_H
