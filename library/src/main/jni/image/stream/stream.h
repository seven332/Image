//
// Created by Hippo on 8/6/2016.
//

#ifndef IMAGE_STREAM_H
#define IMAGE_STREAM_H


#include <stdbool.h>
#include <stdint.h>


#define DEFAULT_BUFFER_SIZE 4096 // 1024 * 4


struct STREAM;
typedef struct STREAM Stream;

typedef size_t (*StreamReadFunc) (Stream* stream, void* buffer, size_t size);
typedef size_t (*StreamPeekFunc) (Stream* stream, void* buffer, size_t size);
typedef void   (*StreamCloseFunc)(Stream** stream);

struct STREAM {
  void* data;
  StreamReadFunc  read;
  StreamPeekFunc peek;
  StreamCloseFunc close;
};


/**
 * Read all data from this stream.
 */
void* stream_read_all(Stream* stream, size_t* size);


#endif //IMAGE_STREAM_H
