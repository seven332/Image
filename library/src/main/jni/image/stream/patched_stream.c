//
// Created by Hippo on 8/6/2016.
//

#include <string.h>

#include "patched_stream.h"
#include "../../log.h"


typedef struct {
  Stream* stream;
  void* patch;
  size_t patch_length;
  size_t read_count;
} PatchedStreamData;

static size_t read(Stream* stream, void* buffer, size_t offset, size_t size) {
  PatchedStreamData* data = (PatchedStreamData*) stream->data;
  size_t len = MIN(size, data->patch_length - data->read_count);
  size_t buffer_offset = offset;

  // Read from patch
  if (len > 0) {
    memcpy(buffer + buffer_offset, data->patch + data->read_count, len);
    data->read_count += len;
    buffer_offset += len;
  }

  // Read from stream
  if (size > len) {
    len += data->stream->read(data->stream, buffer, buffer_offset, size - len);
  }

  return len;
}

static void close(Stream** stream) {
  if (stream == NULL || *stream == NULL) {
    return;
  }

  PatchedStreamData* data = (PatchedStreamData*) (*stream)->data;
  data->stream->close(&data->stream);
  free(data->patch);
  data->patch = NULL;
  free(data);
  (*stream)->data = NULL;
  free(*stream);
  *stream = NULL;
}

Stream* patched_stream_new(Stream* stream, void* patch, size_t patch_length) {
  Stream* new_stream;
  PatchedStreamData* new_data;
  void* new_patch;

  new_stream = (Stream*) malloc(sizeof(Stream));
  if (new_stream == NULL) {
    WTF_OM;
    return NULL;
  }

  new_data = (PatchedStreamData*) malloc(sizeof(PatchedStreamData));
  if (new_data == NULL) {
    WTF_OM;
    free(new_stream);
    return NULL;
  }

  new_patch = malloc(patch_length);
  if (new_patch == NULL) {
    WTF_OM;
    free(new_stream);
    free(new_data);
    return NULL;
  }
  memcpy(new_patch, patch, patch_length);

  new_data->stream = stream;
  new_data->patch = new_patch;
  new_data->patch_length = patch_length;
  new_data->read_count = 0;

  new_stream->data = new_data;
  new_stream->read = &read;
  new_stream->close = &close;
  return new_stream;
}

Stream* patched_stream_get_stream(Stream* stream) {
  return ((PatchedStreamData*) stream->data)->stream;
}
