//
// Created by Hippo on 9/14/2016.
//

#include <malloc.h>
#include <memory.h>

#include "buffer_stream.h"
#include "../log.h"


typedef struct {
  void* buffer;
  size_t length;
  void* position;
  size_t read;
} BufferStreamData;


static size_t read(Stream* stream, void* buffer, size_t offset, size_t size) {
  BufferStreamData* data = (BufferStreamData*) stream->data;
  size_t remain = data->length - data->read;
  size_t len = MIN(size, remain);

  memcpy(buffer + offset, data->position, len);
  data->position += len;
  data->read += len;

  return len;
}

static void close(Stream** stream) {
  BufferStreamData* data;

  if (stream == NULL || *stream == NULL) {
    return;
  }

  data = (BufferStreamData*) (*stream)->data;
  free(data->buffer);
  data->buffer = NULL;
  data->position = NULL;
  free(data);
  (*stream)->data = NULL;
  free(*stream);
  *stream = NULL;
}

Stream* buffer_stream_new(void* buffer, size_t length) {
  Stream* stream;
  BufferStreamData* data;

  stream = (Stream*) malloc(sizeof(Stream));
  data = (BufferStreamData*) malloc(sizeof(BufferStreamData));
  if (stream == NULL || data == NULL) {
    WTF_OM;
    free(stream);
    free(data);
    return NULL;
  }

  data->buffer = buffer;
  data->length = length;
  data->position = buffer;
  data->read = 0;

  stream->data = data;
  stream->read = &read;
  stream->close = &close;

  return stream;
}

void buffer_stream_reset(Stream* stream) {
  BufferStreamData* data = stream->data;
  data->position = data->buffer;
  data->read = 0;
}
