//
// Created by Hippo on 8/8/2016.
//

#include <malloc.h>
#include <string.h>
#include <stdbool.h>

#include "java_stream.h"
#include "../../utils.h"
#include "../../log.h"


static bool INIT_SUCCEED = false;

static jmethodID METHOD_READ = NULL;
static jmethodID METHOD_CLOSE = NULL;


typedef struct {
  JNIEnv* env;
  jobject is;
  jbyteArray j_buffer;

  void* buffer;
  size_t buffer_size;
  size_t buffer_pos;

  void* backup;
  size_t backup_limit;
  size_t backup_size;
  size_t backup_pos;
  bool is_backup;
} JavaStreamData;


static size_t read_internal(JavaStreamData* data, void* buffer, size_t size) {
  JNIEnv* env = data->env;
  size_t remain = size;
  size_t offset = 0;
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
    memcpy(buffer + offset, data->buffer + data->buffer_pos, (size_t) len);

    // Update parameters
    remain -= len;
    offset += len;
    data->buffer_pos += len;
  }

  return offset;
}

static size_t read(Stream* stream, void* buffer, size_t size) {
  JavaStreamData* data = stream->data;
  size_t len, read = 0;

  if (buffer == NULL || size == 0) {
    return 0;
  }

  // Read from backup
  if (!data->is_backup && data->backup != NULL && data->backup_pos < data->backup_size) {
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
  read += read_internal(stream->data, buffer, size);

  // Backup
  if (data->is_backup && data->backup != NULL && data->backup_pos < data->backup_limit) {
    len = MIN(read, data->backup_limit - data->backup_pos);
    memcpy(data->backup + data->backup_pos, buffer, len);

    // Update data
    data->backup_size += len;
    data->backup_pos += len;
  }

  return read;
}

static bool mark(Stream* stream, size_t limit) {
  JavaStreamData* data = stream->data;
  void* bak;
  size_t remain;

  bak = data->backup;
  remain = data->backup_size - data->backup_pos;
  if (!data->is_backup && data->backup != NULL && remain > 0) {
    data->backup = malloc(remain + limit);
    if (data->backup == NULL) { WTF_OM; goto fail; }
    memcpy(data->backup, bak + data->backup_pos, remain);
    free(bak);
    data->is_backup = true;
    data->backup_limit = remain + limit;
    data->backup_size = remain;
    data->backup_pos = remain;
  } else {
    data->backup = malloc(limit);
    if (data->backup == NULL) { WTF_OM; goto fail; }
    free(bak);
    data->is_backup = true;
    data->backup_limit = limit;
    data->backup_size = 0;
    data->backup_pos = 0;
  }
  return true;

fail:
  free(bak);
  data->is_backup = false;
  data->backup_limit = 0;
  data->backup_size = 0;
  data->backup_pos = 0;
  return false;
}

static void reset(Stream* stream) {
  JavaStreamData* data = stream->data;
  data->is_backup = false;
  data->backup_pos = 0;
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

Stream* java_stream_new(JNIEnv* env, jobject* is) {
  Stream* stream = NULL;
  JavaStreamData* data = NULL;
  void* buffer;
  jbyteArray j_buffer;

  if (!INIT_SUCCEED) {
    return NULL;
  }

  stream = malloc(sizeof(Stream));
  data = malloc(sizeof(JavaStreamData));
  buffer = malloc(DEFAULT_BUFFER_SIZE);
  if (stream == NULL || data == NULL || buffer == NULL) { WTF_OM; goto fail; }

  j_buffer = (*env)->NewByteArray(env, DEFAULT_BUFFER_SIZE);
  j_buffer = (*env)->NewGlobalRef(env, j_buffer);
  if (j_buffer == NULL) { LOGE(MSG("Can't create buffer")); goto fail; }

  data->env = env;
  data->is = (*env)->NewGlobalRef(env, is);
  data->j_buffer = j_buffer;

  data->buffer = buffer;
  data->buffer_size = 0;
  data->buffer_pos = 0;

  data->backup = NULL;
  data->backup_limit = 0;
  data->backup_size = 0;
  data->backup_pos = 0;
  data->is_backup = false;

  stream->data = data;
  stream->read = &read;
  stream->mark = &mark;
  stream->reset = &reset;
  stream->close = &close;

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
