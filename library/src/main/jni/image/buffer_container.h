//
// Created by Hippo on 9/13/2016.
//

#ifndef IMAGE_BUFFER_CONTAINER_H
#define IMAGE_BUFFER_CONTAINER_H


#include <stdint.h>


struct BUFFER_CONTAINER;
typedef struct BUFFER_CONTAINER BufferContainer;

struct BUFFER_CONTAINER {
  void* data;
  void* (*create_buffer)(BufferContainer* container, uint32_t width, uint32_t height, uint8_t config);
  void (*release_buffer)(BufferContainer* container, void* buffer);
};


#endif //IMAGE_BUFFER_CONTAINER_H
