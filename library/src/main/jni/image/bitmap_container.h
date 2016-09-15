//
// Created by Hippo on 9/13/2016.
//

#ifndef IMAGE_BITMAP_CONTAINER_H
#define IMAGE_BITMAP_CONTAINER_H


#include <jni.h>

#include "buffer_container.h"


BufferContainer* bitmap_container_new(JNIEnv* env, jclass clazz, jmethodID method);

jobject bitmap_container_fetch_bitmap(BufferContainer *container);

void bitmap_container_recycle(BufferContainer** container);


#endif //IMAGE_BITMAP_CONTAINER_H
