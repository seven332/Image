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
typedef bool   (*StreamMarkFunc) (Stream* stream, size_t limit);
typedef void   (*StreamResetFunc)(Stream* stream);
typedef void   (*StreamCloseFunc)(Stream** stream);

struct STREAM {
  void* data;
  StreamReadFunc  read;
  StreamMarkFunc  mark;
  StreamResetFunc reset;
  StreamCloseFunc close;
};


/**
 * Read all data from this stream.
 */
void* stream_read_all(Stream* stream, size_t* size);


#endif //IMAGE_STREAM_H
