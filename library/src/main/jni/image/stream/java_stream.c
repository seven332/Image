//
// Created by Hippo on 8/8/2016.
//

#include <malloc.h>
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
  jbyteArray buffer;
} JavaStreamData;


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

static size_t read(Stream* stream, void* buffer, size_t offset, size_t size) {
  JavaStreamData* data = (JavaStreamData*) stream->data;
  JNIEnv* env = data->env;
  size_t remainSize = size;
  size_t readSize = 0;
  size_t bufferOffset = offset;
  int len;

  while (remainSize > 0) {
    // Read from java InputStream to java buffer
    len = MIN(DEFAULT_BUFFER_SIZE, (int) remainSize);
    len = (*env)->CallIntMethod(env, data->is, METHOD_READ, data->buffer, 0, len);
    if ((*env)->ExceptionCheck(env)) {
      LOGE(MSG("Catch exception"));
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
      len = -1;
    }

    // end of the stream or catch exception
    if (len <= 0) {
      break;
    }

    // Copy from java buffer to c buffer
    (*env)->GetByteArrayRegion(env, data->buffer, 0, len, (jbyte *) (buffer + bufferOffset));

    // Update parameters
    remainSize -= len;
    readSize += len;
    bufferOffset += len;
  }

  return readSize;
}

static void close(Stream** stream) {
  if (stream == NULL || *stream == NULL) {
    return;
  }

  JavaStreamData* data = (JavaStreamData*) (*stream)->data;
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
  (*env)->DeleteGlobalRef(env, data->buffer);

  // Free
  free(data);
  (*stream)->data = NULL;
  free(*stream);
  *stream = NULL;
}

Stream* java_stream_new(JNIEnv* env, jobject* is) {
  Stream* stream;
  JavaStreamData* data;
  jbyteArray buffer;

  if (!INIT_SUCCEED) {
    return NULL;
  }

  stream = (Stream*) malloc(sizeof(Stream));
  if (stream == NULL) {
    WTF_OM;
    return NULL;
  }

  data = (JavaStreamData*) malloc(sizeof(JavaStreamData));
  if (data == NULL) {
    WTF_OM;
    free(stream);
    return NULL;
  }

  buffer = (*env)->NewByteArray(env, DEFAULT_BUFFER_SIZE);
  buffer = (*env)->NewGlobalRef(env, buffer);
  if (buffer == NULL) {
    LOGE(MSG("Can't create buffer"));
    free(stream);
    free(data);
    return NULL;
  }

  data->env = env;
  data->is = (*env)->NewGlobalRef(env, is);
  data->buffer = buffer;

  stream->data = data;
  stream->read = &read;
  stream->close = &close;
  return stream;
}

void java_stream_set_env(Stream* stream, JNIEnv* env) {
  ((JavaStreamData*) stream->data)->env = env;
}
