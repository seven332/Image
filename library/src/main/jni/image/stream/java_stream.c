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
#include "../../utils.h"
#include "../../log.h"


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

  size_t (*read_internal)(JavaStreamData* data, void* buffer, size_t size);

  void* backup;
  size_t backup_alloc;
  size_t backup_size;
  size_t backup_pos;
};


static size_t read_internal_with_buffer(JavaStreamData* data, void* buffer, size_t size) {
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
    memcpy(buffer, data->buffer + data->buffer_pos, (size_t) len);

    // Update parameters
    remain -= len;
    read += len;
    buffer += len;
    data->buffer_pos += len;
  }

  return read;
}

static size_t read_internal_without_buffer(JavaStreamData* data, void* buffer, size_t size) {
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
    (*env)->GetByteArrayRegion(env, data->j_buffer, 0, len, (jbyte *) buffer);

    // Update parameters
    remain -= len;
    read += len;
    buffer += len;
  }

  return read;
}

static size_t read(Stream* stream, void* buffer, size_t size) {
  JavaStreamData* data = stream->data;
  size_t len, read = 0;

  if (buffer == NULL || size == 0) {
    return 0;
  }

  // Read from backup
  if (data->backup != NULL && data->backup_pos < data->backup_size) {
    read = MIN(size, data->backup_size - data->backup_pos);
    memcpy(buffer, data->backup + data->backup_pos, read);

    // Update data
    buffer += read;
    size -= read;
    data->backup_pos += read;

    if (size == 0) {
      return read;
    }
  }

  // Read from stream
  read += data->read_internal(stream->data, buffer, size);

  return read;
}

size_t peek(Stream* stream, void* buffer, size_t size) {
  JavaStreamData* data = stream->data;

  // Get amount of data in the backup before reading
  size_t pre_read_backup_data = data->backup != NULL ? data->backup_size - data->backup_pos : 0;

  size_t len = read(stream, buffer, size);

  size_t prev_backup_remain = 0;
  size_t new_backup_len = len;

  // If there is a previous backup, include remainder into new backup.
  if (data->backup != NULL && data->backup_pos < data->backup_size) {
    prev_backup_remain = data->backup_size - data->backup_pos;
    new_backup_len += prev_backup_remain;
  }

  if (data->backup != NULL && len <= pre_read_backup_data) {
    // Reset backup pos if the read was purely within the backup buffer
    data->backup_pos -= len;
  } else if (data->backup != NULL && data->backup_alloc >= new_backup_len) {
    // Reuse backup buffer if it is large enough
    if (prev_backup_remain != 0) {
      // Append remaining backup to the end
      memmove(data->backup + len, data->backup + data->backup_pos, prev_backup_remain);
    }

    memcpy(data->backup, buffer, len);
    data->backup_pos = 0;
    data->backup_size = new_backup_len;
  } else {
    // Generate a new backup buffer
    void* new_backup = malloc(new_backup_len);
    if (new_backup == NULL) {
      WTF_OOM;
      free(data->backup);
      data->backup = NULL;
      data->backup_alloc = 0;
      data->backup_size = 0;
      data->backup_pos = 0;
      return 0;
    }

    // Append remaining backup to the end
    if (prev_backup_remain != 0) {
      memcpy(new_backup + len, data->backup + data->backup_pos, prev_backup_remain);
      free(data->backup);
    }

    memcpy(new_backup, buffer, len);
    data->backup = new_backup;
    data->backup_pos = 0;
    data->backup_size = new_backup_len;
    data->backup_alloc = new_backup_len;
  }

  return len;
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
  free(data->backup);
  data->backup = NULL;
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
  data->backup_alloc = 0;
  data->backup_size = 0;
  data->backup_pos = 0;

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
