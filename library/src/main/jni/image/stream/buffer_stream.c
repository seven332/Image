/*
 * Copyright 2016 Hippo Seven
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <malloc.h>
#include <string.h>

#include "buffer_stream.h"
#include "../log.h"


typedef struct {
  void* buffer;
  size_t length;
  void* pos;
  size_t read;
} BufferStreamData;


static size_t read(Stream* stream, void* dst, size_t size) {
  BufferStreamData* data = (BufferStreamData*) stream->data;
  size_t remain = data->length - data->read;
  size_t len = MIN(size, remain);

  memcpy(dst, data->pos, len);
  data->pos += len;
  data->read += len;

  return len;
}

static size_t peek(Stream* stream, void* dst, size_t size) {
  BufferStreamData* data = (BufferStreamData*) stream->data;
  void* pos_bak = data->pos;
  size_t read_bak = data->read;

  size_t len = read(stream, dst, size);
  data->pos = pos_bak;
  data->read = read_bak;

  return len;
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
    WTF_OOM;
    free(stream);
    free(data);
    return NULL;
  }

  data->buffer = buffer;
  data->length = length;
  data->pos = buffer;
  data->read = 0;

  stream->data = data;
  stream->read = read;
  stream->peek = peek;
  stream->close = close;

  return stream;
}

void buffer_stream_reset(Stream* stream) {
  BufferStreamData* data = stream->data;
  data->pos = data->buffer;
  data->read = 0;
}
