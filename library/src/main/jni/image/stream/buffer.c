/*
 * Copyright 2018 Hippo Seven
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

#include <stdint.h>
#include <string.h>
#include <malloc.h>

#include "image_utils.h"
#include "buffer.h"
#include "log.h"
#include "utils.h"

size_t buffer_read(Buffer* buffer, void* dst, size_t size) {
  size_t remain = buffer->length - buffer->position;
  size_t read = MIN(remain, size);

  if (read != 0) {
    memcpy(dst, buffer->raw + buffer->position, read);
    buffer->position += read;
  }

  return read;
}

size_t buffer_write(Buffer* buffer, const void* src, size_t size) {
  size_t remain = buffer->capacity - buffer->length;

  size_t write;
  size_t new_capacity;
  void* new_raw;

  if (remain >= size) {
    write = size;
  } else if (!buffer->extendable) {
    write = remain;
  } else {
    new_capacity = next_pow2_size_t(buffer->length + size);
    new_raw = realloc(buffer->raw, new_capacity);
    if (new_raw == NULL) {
      WTF_OOM;
      write = remain;
    } else {
      buffer->raw = new_raw;
      buffer->capacity = new_capacity;
      write = size;
    }
  }

  if (write != 0) {
    memcpy(buffer->raw + buffer->length, src, write);
    buffer->length += write;
  }

  return write;
}

size_t buffer_seek(Buffer* buffer, size_t position) {
  buffer->position = MIN(position, buffer->length);
  return buffer->position;
}

void buffer_shrink(Buffer* buffer) {
  if (buffer->position == 0) {
    // No data has been read, no need to shrink
  } else {
    buffer->length = buffer->length - buffer->position;
    if (buffer->length == 0) {
      // No need to copy
    } else if (buffer->length <= buffer->position) {
      // No overlap
      memcpy(buffer->raw, buffer->raw + buffer->position, buffer->length);
    } else {
      memmove(buffer->raw, buffer->raw + buffer->position, buffer->length);
    }
    buffer->position = 0;
  }
}

void buffer_close(Buffer** buffer) {
  if (buffer == NULL || *buffer == NULL) {
    return;
  }

  free((*buffer)->raw);
  (*buffer)->raw = NULL;
  free(*buffer);
  *buffer = NULL;
}

Buffer* buffer_new(size_t capacity, bool extendable) {
  void* raw;

  if (capacity == 0) {
    LOGE(MSG("Invalid buffer capacity: 0"));
    return NULL;
  }

  raw = malloc(capacity);
  if (raw == NULL) {
    WTF_OOM;
    free(raw);
    return NULL;
  }

  return buffer_new_from_raw(raw, 0, 0, capacity, extendable);
}

Buffer* buffer_new_from_raw(void* raw, size_t position, size_t length, size_t capacity, bool extendable) {
  Buffer* buffer;

  if (raw == NULL || position > length || length > capacity || capacity == 0) {
    LOGE(MSG("Invalid parameter"));
    return NULL;
  }

  buffer = malloc(sizeof(Buffer));
  if (buffer == NULL) {
    WTF_OOM;
    free(buffer);
    free(raw);
    return NULL;
  }

  buffer->raw = raw;
  buffer->position = position;
  buffer->length = length;
  buffer->capacity = capacity;
  buffer->extendable = extendable;

  return buffer;
}
