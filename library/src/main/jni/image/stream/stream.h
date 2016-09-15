//
// Created by Hippo on 8/6/2016.
//

#ifndef IMAGE_STREAM_H
#define IMAGE_STREAM_H


#include <stdint.h>


#define DEFAULT_BUFFER_SIZE 4096 // 1024 * 4


struct STREAM;
typedef struct STREAM Stream;

struct STREAM {
  void* data;
  size_t (*read)(Stream* stream, void* buffer, size_t offset, size_t size);
  void (*close)(Stream** stream);
};


void* stream_read_all(Stream* stream, size_t* length);


#endif //IMAGE_STREAM_H
