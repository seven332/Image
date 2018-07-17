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
#include <stdbool.h>

#include "java_stream.h"
#include "buffer.h"
#include "image_utils.h"
#include "log.h"

static bool INIT_SUCCEED = false;

static jmethodID METHOD_READ = NULL;
static jmethodID METHOD_CLOSE = NULL;

struct JAVA_STREAM_DATA;
typedef struct JAVA_STREAM_DATA JavaStreamData;

struct JAVA_STREAM_DATA {
  JNIEnv* env;
  jobject is;
  jbyteArray j_buffer;

  void* buffer;
  size_t buffer_size;
  size_t buffer_pos;

  size_t (*read_internal)(JavaStreamData* data, void* dst, size_t size);

  Buffer* backup;
};


static size_t read_internal_with_buffer(JavaStreamData* data, void* dst, size_t size) {
  JNIEnv* env = data->env;
  size_t remain = size;
  size_t read = 0;
  int len;

  while (remain > 0) {
    if (data->buffer_pos == data->buffer_size) {
      // Read from java InputStream to java buffer
      // Always read DEFAULT_BUFFER_SIZE
      len = (*env)->CallIntMethod(env, data->is, METHOD_READ, data->j_buffer, 0, DEFAULT_BUFFER_SIZE);
      if ((*env)->ExceptionCheck(env)) {
        LOGE(MSG("Catch exception"));
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        len = -1;
      }

      // end of the stream or catch exception
      if (len <= 0) { break; }

      // Copy from java buffer to c buffer
      (*env)->GetByteArrayRegion(env, data->j_buffer, 0, len, (jbyte *) (data->buffer));

      // Update buffer info
      data->buffer_size = (size_t) len;
      data->buffer_pos = 0;
    }

    // Copy from c buffer to target buffer
    len = MIN((int) (data->buffer_size - data->buffer_pos), (int) remain);
    memcpy(dst, data->buffer + data->buffer_pos, (size_t) len);

    // Update parameters
    remain -= len;
    read += len;
    dst += len;
    data->buffer_pos += len;
  }

  return read;
}

static size_t read_internal_without_buffer(JavaStreamData* data, void* dst, size_t size) {
  JNIEnv* env = data->env;
  size_t remain = size;
  size_t read = 0;
  int len;

  while (remain > 0) {
    // Read from java InputStream to java buffer
    len = MIN(DEFAULT_BUFFER_SIZE, (int) remain);
    len = (*env)->CallIntMethod(env, data->is, METHOD_READ, data->j_buffer, 0, len);
    if ((*env)->ExceptionCheck(env)) {
      LOGE(MSG("Catch exception"));
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
      len = -1;
    }

    // end of the stream or catch exception
    if (len <= 0) { break; }

    // Copy from java buffer to c buffer
    (*env)->GetByteArrayRegion(env, data->j_buffer, 0, len, (jbyte *) dst);

    // Update parameters
    remain -= len;
    read += len;
    dst += len;
  }

  return read;
}

static size_t read(Stream* stream, void* dst, size_t size) {
  JavaStreamData* data = stream->data;
  size_t read = 0;

  if (dst == NULL || size == 0) {
    return 0;
  }

  if (data->backup != NULL) {
    read += buffer_read(data->backup, dst, size);
    dst += read;
    size -= read;

    if (size == 0) {
      return read;
    }
  }

  // Read from stream
  read += data->read_internal(stream->data, dst, size);

  return read;
}

size_t peek(Stream* stream, void* dst, size_t size) {
  JavaStreamData* data = stream->data;
  Buffer* backup = data->backup;
  size_t backup_read = 0;
  size_t stream_read = 0;
  size_t backup_write = 0;

  if (backup != NULL) {
    backup_read = buffer_read(backup, dst, size);
    dst += backup_read;
    size -= backup_read;

    if (size == 0) {
      // Just peek backup
      buffer_seek(backup, backup->position - backup_read);
      return backup_read;
    }
  }

  stream_read = read(stream, dst, size);

  if (backup != NULL) {
    buffer_seek(backup, backup->position - backup_read);
    // Only shrink the backup if it needs more space
    if (backup->capacity - backup->length < stream_read) {
      buffer_shrink(backup);
    }
  } else {
    // Create a backup
    backup = data->backup = buffer_new(next_pow2_size_t(stream_read), true);
    if (backup == NULL) {
      LOGE(MSG("Can't create backup for java stream."));
    }
  }

  if (backup != NULL) {
    backup_write = buffer_write(backup, dst, stream_read);
    if (backup_write != stream_read) {
      LOGE(MSG("Can't write bytes to backup."));
    }
  }

  return backup_read + stream_read;
}

static void close(Stream** stream) {
  if (stream == NULL || *stream == NULL) {
    return;
  }

  JavaStreamData* data = (*stream)->data;
  JNIEnv* env = data->env;

  // Close java InputStream
  (*env)->CallVoidMethod(env, data->is, METHOD_CLOSE);
  if ((*env)->ExceptionCheck(env)) {
    LOGE(MSG("Catch exception"));
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
  }

  // Delete java object global reference
  (*env)->DeleteGlobalRef(env, data->is);
  (*env)->DeleteGlobalRef(env, data->j_buffer);

  // Free
  free(data->buffer);
  data->buffer = NULL;
  if (data->backup != NULL) {
    buffer_close(&data->backup);
  }
  free(data);
  (*stream)->data = NULL;
  free(*stream);
  *stream = NULL;
}

void java_stream_init(JNIEnv* env) {
  jclass CLAZZ = (*env)->FindClass(env, "java/io/InputStream");

  if (CLAZZ != NULL) {
    METHOD_READ = (*env)->GetMethodID(env, CLAZZ, "read", "([BII)I");
    METHOD_CLOSE = (*env)->GetMethodID(env, CLAZZ, "close", "()V");
    INIT_SUCCEED = METHOD_READ != NULL && METHOD_CLOSE != NULL;
  } else {
    INIT_SUCCEED = false;
  }

  if (!INIT_SUCCEED) {
    LOGE(MSG("Can't init java stream"));
  }
}

Stream* java_stream_new(JNIEnv* env, jobject* is, bool with_buffer) {
  Stream* stream = NULL;
  JavaStreamData* data = NULL;
  void* buffer = NULL;
  jbyteArray j_buffer;

  if (!INIT_SUCCEED) {
    return NULL;
  }

  stream = malloc(sizeof(Stream));
  data = malloc(sizeof(JavaStreamData));
  if (stream == NULL || data == NULL) { WTF_OOM; goto fail; }
  if (with_buffer) {
    buffer = malloc(DEFAULT_BUFFER_SIZE);
    if (buffer == NULL) { WTF_OOM; goto fail; }
  }

  j_buffer = (*env)->NewByteArray(env, DEFAULT_BUFFER_SIZE);
  j_buffer = (*env)->NewGlobalRef(env, j_buffer);
  if (j_buffer == NULL) { LOGE(MSG("Can't create buffer")); goto fail; }

  data->env = env;
  data->is = (*env)->NewGlobalRef(env, is);
  data->j_buffer = j_buffer;

  data->buffer = buffer;
  data->buffer_size = 0;
  data->buffer_pos = 0;

  data->read_internal = with_buffer ? &read_internal_with_buffer : &read_internal_without_buffer;

  data->backup = NULL;

  stream->data = data;
  stream->read = read;
  stream->peek = peek;
  stream->close = close;

  return stream;

fail:
  free(stream);
  free(data);
  free(buffer);
  return NULL;
}

void java_stream_set_env(Stream* stream, JNIEnv* env) {
  ((JavaStreamData*) stream->data)->env = env;
}
