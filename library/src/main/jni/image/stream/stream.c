//
// Created by Hippo on 9/14/2016.
//

#include <malloc.h>

#include "stream.h"
#include "../log.h"


void* stream_read_all(Stream* stream, size_t* size) {
  size_t len = 0;
  size_t read;
  void* buffer = NULL;
  void* buffer_bak = NULL;

  buffer = malloc(DEFAULT_BUFFER_SIZE);
  if (buffer == NULL) {
    WTF_OM;
    return NULL;
  }

  for (;;) {
    // Read from stream
    read = stream->read(stream, buffer + len, DEFAULT_BUFFER_SIZE);
    len += read;
    // Check stream end
    if (read < DEFAULT_BUFFER_SIZE) {
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
    buffer_bak = buffer;
    buffer = realloc(buffer, len + DEFAULT_BUFFER_SIZE);
    if (buffer == NULL) {
      WTF_OM;
      free(buffer_bak);
      return NULL;
    }
  }
}
