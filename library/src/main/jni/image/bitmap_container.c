//
// Created by Hippo on 9/13/2016.
//

#include <malloc.h>
#include <android/bitmap.h>

#include "bitmap_container.h"
#include "../log.h"


typedef struct {
  JNIEnv* env;
  jclass clazz;
  jmethodID method;
  jobject bitmap;
} BitmapContainerData;


static void* create_buffer(BufferContainer* container, uint32_t width, uint32_t height, int32_t config) {
  BitmapContainerData* data = container->data;
  void *pixels = NULL;

  if (data->bitmap != NULL) {
    LOGE("Can't call create_buffer twice.");
    return NULL;
  }

  data->bitmap = (*data->env)->CallStaticObjectMethod(data->env, data->clazz, data->method, width, height, config);
  if (data->bitmap == NULL) {
    LOGE("Can't create bitmap.");
    return NULL;
  }

  AndroidBitmap_lockPixels(data->env, data->bitmap, &pixels);
  if (pixels == NULL) {
    LOGE("Can't lock pixels of Bitmap.");
  }

  return pixels;
}

static void release_buffer(BufferContainer* container, void* buffer) {
  BitmapContainerData* data = container->data;

  if (buffer == NULL) {
    LOGE("Can't release NULL buffer.");
    return;
  }

  if (data->bitmap == NULL) {
    LOGE("Can't release buffer when bitmap is NULL.");
    return;
  }

  AndroidBitmap_unlockPixels(data->env, data->bitmap);
}

BufferContainer* bitmap_container_new(JNIEnv* env, jclass clazz, jmethodID method) {
  BufferContainer* container;
  BitmapContainerData* data;

  container = malloc(sizeof(BufferContainer));
  data = malloc(sizeof(BitmapContainerData));
  if (container == NULL || data == NULL) {
    WTF_OOM;
    free(container);
    free(data);
    return NULL;
  }

  data->env = env;
  data->clazz = clazz;
  data->method = method;
  data->bitmap = NULL;

  container->data = data;
  container->create_buffer = &create_buffer;
  container->release_buffer = &release_buffer;

  return container;
}

jobject bitmap_container_fetch_bitmap(BufferContainer *container) {
  BitmapContainerData* data = container->data;
  jobject bitmap = data->bitmap;
  data->bitmap = NULL;
  return bitmap;
}

void bitmap_container_recycle(BufferContainer** container) {
  BitmapContainerData* data;

  if (container == NULL || *container == NULL) {
    return;
  }

  data = (BitmapContainerData*) (*container)->data;
  if (data->bitmap != NULL) {
    LOGE("Call bitmap_container_fetch_bitmap() before recycle()");
  }

  free(data);
  (*container)->data = NULL;
  free(*container);
  *container = NULL;
}