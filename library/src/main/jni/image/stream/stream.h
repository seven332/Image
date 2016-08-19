//
// Created by Hippo on 8/6/2016.
//

#ifndef IMAGE_STREAM_H
#define IMAGE_STREAM_H


#include <stdint.h>


struct STREAM;
typedef struct STREAM Stream;

struct STREAM {
  void* data;
  size_t (*read)(Stream* stream, void* buffer, size_t offset, size_t size);
  void (*close)(Stream** stream);
};


#endif //IMAGE_STREAM_H
