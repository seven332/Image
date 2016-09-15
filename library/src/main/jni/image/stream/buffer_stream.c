//
// Created by Hippo on 9/14/2016.
//

#include <malloc.h>
#include <string.h>

#include "buffer_stream.h"
#include "../log.h"


typedef struct {
  void* buffer;
  size_t length;
  void* pos;
  size_t read;
  size_t mark;
} BufferStreamData;


static size_t read(Stream* stream, void* buffer, size_t size) {
  BufferStreamData* data = (BufferStreamData*) stream->data;
  size_t remain = data->length - data->read;
  size_t len = MIN(size, remain);

  memcpy(buffer, data->pos, len);
  data->pos += len;
  data->read += len;

  return len;
}

static bool mark(Stream* stream, size_t limit) {
  BufferStreamData* data = stream->data;
  data->mark = data->read;
  return true;
}

static void reset(Stream* stream) {
  BufferStreamData* data = stream->data;
  data->pos = data->buffer + data->mark;
  data->read = data->mark;
}

static void close(Stream** stream) {
  if (stream == NULL || *stream == NULL) {
    return;
  }

  BufferStreamData* data = (*stream)->data;
  free(data->buffer);
  data->buffer = NULL;
  data->pos = NULL;
  free(data);
  (*stream)->data = NULL;
  free(*stream);
  *stream = NULL;
}

Stream* buffer_stream_new(void* buffer, size_t length) {
  Stream* stream;
  BufferStreamData* data;

  stream = malloc(sizeof(Stream));
  data = malloc(sizeof(BufferStreamData));
  if (stream == NULL || data == NULL) {
    WTF_OM;
    free(stream);
    free(data);
    return NULL;
  }

  data->buffer = buffer;
  data->length = length;
  data->pos = buffer;
  data->read = 0;
  data->mark = 0;

  stream->data = data;
  stream->read = &read;
  stream->mark = &mark;
  stream->reset = &reset;
  stream->close = &close;

  return stream;
}

void buffer_stream_reset(Stream* stream) {
  BufferStreamData* data = stream->data;
  data->pos = data->buffer;
  data->read = 0;
}
