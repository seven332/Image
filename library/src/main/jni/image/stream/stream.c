//
// Created by Hippo on 9/14/2016.
//

#include <malloc.h>

#include "stream.h"
#include "../log.h"


void* stream_read_all(Stream* stream, size_t* size) {
  size_t len = 0;
  size_t limit = DEFAULT_BUFFER_SIZE;
  size_t read;
  void* buffer = NULL;
  void* buffer_bak = NULL;

  buffer = malloc(limit);
  if (buffer == NULL) {
    WTF_OOM;
    return NULL;
  }

  for (;;) {
    // Read from stream
    read = stream->read(stream, buffer + len, limit - len);
    len += read;
    // Check stream end
    if (len < limit) {
      // Get the end, shrink the buffer
      buffer_bak = buffer;
      buffer = realloc(buffer, len);
      if (buffer == NULL) {
        LOGE("Failed to shrink the buffer");
        free(buffer_bak);
        return NULL;
      } else {
        if (size != NULL) {
          *size = len;
        }
        return buffer;
      }
    }
    // Extent buffer
    limit *= 2;
    buffer_bak = buffer;
    buffer = realloc(buffer, limit);
    if (buffer == NULL) {
      WTF_OOM;
      free(buffer_bak);
      return NULL;
    }
  }
}
