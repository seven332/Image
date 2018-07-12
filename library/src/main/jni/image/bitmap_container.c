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