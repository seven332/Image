/*
 * Copyright 2015 Hippo Seven
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

//
// Created by Hippo on 12/27/2015.
//

#include <stdlib.h>
#include <android/bitmap.h>
#include <GLES2/gl2.h>

#include "com_hippo_image_Image.h"
#include "com_hippo_image_StaticImage.h"
#include "com_hippo_image_StaticDelegateImage.h"
#include "com_hippo_image_AnimatedImage.h"
#include "com_hippo_image_AnimatedDelegateImage.h"
#include "image.h"
#include "image_utils.h"
#include "animated_image.h"
#include "java_stream.h"
#include "patched_stream.h"
#include "../log.h"


static bool INIT_SUCCEED = false;

static jclass CLASS_STATIC_IMAGE = NULL;
static jclass CLASS_ANIMATED_IMAGE = NULL;

static jmethodID CONSTRUCTOR_STATIC_IMAGE = NULL;
static jmethodID CONSTRUCTOR_ANIMATED_IMAGE = NULL;
static jmethodID METHOD_ANIMATED_IMAGE_ON_COMPLETE = NULL;


static jobject static_image_object_new(JNIEnv* env, StaticImage* image) {
  return (*env)->NewObject(env, CLASS_STATIC_IMAGE, CONSTRUCTOR_STATIC_IMAGE,
      (jlong) image, (jint) image->width, (jint) image->height, (jint) image->format,
      (jboolean) image->opaque, (jint) (image->width * image->height * 4));
}

static jobject animated_image_object_new(JNIEnv* env, AnimatedImage* image) {
  return (*env)->NewObject(env, CLASS_ANIMATED_IMAGE, CONSTRUCTOR_ANIMATED_IMAGE,
      (jlong) image, (jint) image->width, (jint) image->height,
      (jint) image->format, (jboolean) image->opaque);
}

static void animated_image_object_on_complete(JNIEnv* env, jobject obj, AnimatedImage* image) {
  uint32_t frame_count;
  uint32_t byte_count;
  jintArray delay_array;
  jint delay;
  uint32_t i;

  frame_count = image->get_frame_count(image);
  byte_count = image->get_byte_count(image);

  delay_array = (*env)->NewIntArray(env, frame_count);
  for (i = 0; i < frame_count; i++) {
    delay = image->get_delay(image, i);
    (*env)->SetIntArrayRegion(env, delay_array, i, 1, &delay);
  }

  (*env)->CallVoidMethod(env, obj, METHOD_ANIMATED_IMAGE_ON_COMPLETE,
      frame_count, delay_array, byte_count);
}


////////////////////////////////
// Image
////////////////////////////////

JNIEXPORT jobject JNICALL
Java_com_hippo_image_Image_nativeDecode(JNIEnv* env, __unused jclass clazz, jobject is, jboolean partially) {
  bool animated;
  void* image = NULL;
  Stream* stream = NULL;
  jobject obj;

  if (!INIT_SUCCEED) {
    return NULL;
  }

  stream = java_stream_new(env, is);
  if (stream == NULL) {
    LOGE(MSG("Can't create java stream"));
    return NULL;
  }

  if (!decode(stream, partially, &animated, &image)) {
    LOGE(MSG("Can't decode image"));
    return NULL;
  }

  if (!animated) {
    obj = static_image_object_new(env, image);
  } else {
    obj = animated_image_object_new(env, image);
    if (((AnimatedImage*) image)->completed) {
      animated_image_object_on_complete(env, obj, image);
    }
  }

  return obj;
}

JNIEXPORT jobject JNICALL
Java_com_hippo_image_Image_nativeCreate(JNIEnv* env, __unused jclass clazz, jobject bitmap) {
#ifdef IMAGE_SUPPORT_PLAIN
  AndroidBitmapInfo info;
  void* pixels = NULL;
  StaticImage* image = NULL;
  jobject image_object;

  if (!INIT_SUCCEED) {
    return NULL;
  }

  AndroidBitmap_getInfo(env, bitmap, &info);
  AndroidBitmap_lockPixels(env, bitmap, &pixels);
  if (pixels == NULL) {
    LOGE(MSG("Can't lock bitmap pixels"));
    return NULL;
  }

  image = create(info.width, info.height, pixels);

  AndroidBitmap_unlockPixels(env, bitmap);

  if (image == NULL) {
    return NULL;
  }

  image_object = static_image_object_new(env, image);
  if (image_object == NULL) {
    static_image_delete(&image);
    return NULL;
  } else {
    return image_object;
  }
#else
  return NULL;
#endif
}

JNIEXPORT jlong JNICALL
Java_com_hippo_image_Image_nativeCreateBuffer(__unused JNIEnv* env, __unused jclass clazz, jint size) {
  return (jlong) malloc((size_t) (size * 4));
}

JNIEXPORT void JNICALL
Java_com_hippo_image_Image_nativeDestroyBuffer(__unused JNIEnv* env, __unused jclass clazz, jlong buffer) {
  free((void*) buffer);
}

JNIEXPORT jintArray JNICALL
Java_com_hippo_image_Image_nativeGetSupportedImageFormats(JNIEnv *env, __unused jclass clazz) {
  int formats[IMAGE_MAX_SUPPORTED_FORMAT_COUNT];
  int count = get_supported_formats(formats);
  jintArray array = (*env)->NewIntArray(env, count);
  if (array == NULL) {
    return NULL;
  }
  (*env)->SetIntArrayRegion(env, array, 0, count, formats);
  return array;
}

JNIEXPORT jstring JNICALL
Java_com_hippo_image_Image_nativeGetDecoderDescription(JNIEnv *env, __unused jclass clazz, jint format) {
  const char *description = get_decoder_description(format);
  if (description == NULL) {
    return NULL;
  } else {
    return (*env)->NewStringUTF(env, description);
  }
}


////////////////////////////////
// StaticImage
////////////////////////////////

JNIEXPORT void JNICALL
Java_com_hippo_image_StaticImage_nativeRecycle(__unused JNIEnv* env, __unused jclass clazz, jlong ptr) {
  StaticImage* image = (StaticImage *) ptr;
  static_image_delete(&image);
}


////////////////////////////////
// StaticDelegateImage
////////////////////////////////

JNIEXPORT void JNICALL
Java_com_hippo_image_StaticDelegateImage_nativeRender(JNIEnv* env, __unused jclass clazz,
    jlong ptr, jobject bitmap, jint dst_x, jint dst_y, jint src_x, jint src_y,
    jint width, jint height, jint ratio, jboolean fill_blank, jint fill_color) {
  AndroidBitmapInfo info;
  void *pixels = NULL;
  StaticImage* image = (StaticImage *) ptr;

  AndroidBitmap_getInfo(env, bitmap, &info);
  AndroidBitmap_lockPixels(env, bitmap, &pixels);
  if (pixels == NULL) {
    LOGE(MSG("Can't lock bitmap pixels"));
    return;
  }

  copy_pixels(pixels, info.width, info.height, dst_x, dst_y,
      image->buffer, (int) image->width, (int) image->height, src_x, src_y,
      width, height, ratio, fill_blank, fill_color);

  AndroidBitmap_unlockPixels(env, bitmap);

  return;
}

JNIEXPORT void JNICALL
Java_com_hippo_image_StaticDelegateImage_nativeGlTex(__unused JNIEnv* env, __unused jclass clazz,
    jlong image_ptr, jlong buffer_ptr, jboolean init, jint tex_w, jint tex_h,
    jint dst_x, jint dst_y, jint src_x, jint src_y, jint width, jint height, jint ratio) {
  StaticImage* image = (StaticImage*) image_ptr;
  void* buffer = (void*) buffer_ptr;

  copy_pixels(buffer, tex_w, tex_h, dst_x, dst_y,
      image->buffer, (int) image->width, (int) image->height, src_x, src_y,
      width, height, ratio, false, 0);

  if (init) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_w, tex_h,
        0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  } else {
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_w, tex_h,
        GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  }
}


////////////////////////////////
// AnimatedImage
////////////////////////////////

JNIEXPORT void JNICALL
Java_com_hippo_image_AnimatedImage_nativeRecycle(__unused JNIEnv* env, __unused jclass clazz, jlong image_ptr) {
  AnimatedImage* image = (AnimatedImage *) image_ptr;
  image->recycle(&image);
}

JNIEXPORT void JNICALL
Java_com_hippo_image_AnimatedImage_nativeComplete(JNIEnv* env, __unused jclass clazz, jobject obj, jlong image_ptr) {
  AnimatedImage* image = (AnimatedImage *) image_ptr;
  Stream* stream = image->get_stream(image);
  if (stream == NULL) {
    LOGE(MSG("Can't get stream from image data"));
    return;
  }

  // Set new env to ensure works in new thread
  java_stream_set_env(patched_stream_get_stream(stream), env);

  image->complete(image);
  if (!image->completed) {
    LOGE(MSG("Can't complete the image"));
    return;
  }

  animated_image_object_on_complete(env, obj, image);
}


////////////////////////////////
// AnimatedDelegateImage
////////////////////////////////

JNIEXPORT jlong JNICALL Java_com_hippo_image_AnimatedDelegateImage_nativeNew(
    __unused JNIEnv* env, __unused jclass clazz, jint width, jint height) {
  return (jlong) delegate_image_new((uint32_t) width, (uint32_t) height);
}

JNIEXPORT void JNICALL Java_com_hippo_image_AnimatedDelegateImage_nativeRecycle(
    __unused JNIEnv* env, __unused jclass clazz, jlong ptr) {
  DelegateImage* image = (DelegateImage *) ptr;
  delegate_image_delete(&image);
}

JNIEXPORT jint JNICALL
Java_com_hippo_image_AnimatedDelegateImage_nativeGetCurrentDelay(
    __unused JNIEnv* env, __unused jclass clazz, jlong delegate_ptr, jlong image_ptr) {
  AnimatedImage* image = (AnimatedImage *) image_ptr;
  DelegateImage* delegate = (DelegateImage *) delegate_ptr;

  if (delegate->index < 0) {
    return 0;
  } else {
    return image->get_delay(image, (uint32_t) delegate->index);
  }
}

JNIEXPORT void JNICALL Java_com_hippo_image_AnimatedDelegateImage_nativeRender(
    JNIEnv* env, __unused jclass clazz, jlong ptr, jobject bitmap, jint dst_x, jint dst_y,
    jint src_x, jint src_y, jint width, jint height, jint ratio,
    jboolean fill_blank, jint fill_color) {
  AndroidBitmapInfo info;
  void *pixels = NULL;
  DelegateImage* image = (DelegateImage *) ptr;

  AndroidBitmap_getInfo(env, bitmap, &info);
  AndroidBitmap_lockPixels(env, bitmap, &pixels);
  if (pixels == NULL) {
    LOGE(MSG("Can't lock bitmap pixels"));
    return;
  }

  copy_pixels(pixels, info.width, info.height, dst_x, dst_y,
      image->buffer, (int) image->width, (int) image->height, src_x, src_y,
      width, height, ratio, fill_blank, fill_color);

  AndroidBitmap_unlockPixels(env, bitmap);

  return;
}

JNIEXPORT void JNICALL Java_com_hippo_image_AnimatedDelegateImage_nativeGlTex(
    __unused JNIEnv* env, __unused jclass clazz, jlong image_ptr, jlong buffer_ptr, jboolean init,
    jint tex_w, jint tex_h, jint dst_x, jint dst_y, jint src_x, jint src_y,
    jint width, jint height, jint ratio) {
  DelegateImage* image = (DelegateImage*) image_ptr;
  void* buffer = (void*) buffer_ptr;

  copy_pixels(buffer, tex_w, tex_h, dst_x, dst_y,
      image->buffer, (int) image->width, (int) image->height, src_x, src_y,
      width, height, ratio, false, 0);

  if (init) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_w, tex_h,
        0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  } else {
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_w, tex_h,
        GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  }
}

JNIEXPORT void JNICALL Java_com_hippo_image_AnimatedDelegateImage_nativeAdvance(
    __unused JNIEnv* env, __unused jclass clazz, jlong delegate_ptr, jlong image_ptr) {
  AnimatedImage* image = (AnimatedImage *) image_ptr;
  DelegateImage* delegate = (DelegateImage *) delegate_ptr;
  image->advance(image, delegate);
}

JNIEXPORT void JNICALL Java_com_hippo_image_AnimatedDelegateImage_nativeReset(
    __unused JNIEnv* env, __unused jclass clazz, jlong delegate_ptr, jlong image_ptr) {
  AnimatedImage* image = (AnimatedImage *) image_ptr;
  DelegateImage* delegate = (DelegateImage *) delegate_ptr;
  if (delegate->index == 0) {
    // Already first frame
    return;
  } else {
    // Set index to -1 to make next frame is 0
    delegate->index = -1;
    image->advance(image, delegate);
  }
}


__unused
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, __unused void* reserved) {
  JNIEnv* env = NULL;
  if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_6) != JNI_OK) {
    LOGE(MSG("Can't get env in JNI_OnLoad"));
    INIT_SUCCEED = false;
    return JNI_VERSION_1_6;
  }

  CLASS_STATIC_IMAGE = (*env)->FindClass(env, "com/hippo/image/StaticImage");
  CLASS_STATIC_IMAGE = (*env)->NewGlobalRef(env, CLASS_STATIC_IMAGE);
  if (CLASS_STATIC_IMAGE != NULL) {
    CONSTRUCTOR_STATIC_IMAGE = (*env)->GetMethodID(env, CLASS_STATIC_IMAGE, "<init>", "(JIIIZI)V");
  }

  CLASS_ANIMATED_IMAGE = (*env)->FindClass(env, "com/hippo/image/AnimatedImage");
  CLASS_ANIMATED_IMAGE = (*env)->NewGlobalRef(env, CLASS_ANIMATED_IMAGE);
  if (CLASS_ANIMATED_IMAGE != NULL) {
    CONSTRUCTOR_ANIMATED_IMAGE = (*env)->GetMethodID(env, CLASS_ANIMATED_IMAGE, "<init>", "(JIIIZ)V");
    METHOD_ANIMATED_IMAGE_ON_COMPLETE = (*env)->GetMethodID(env, CLASS_ANIMATED_IMAGE, "onComplete", "(I[II)V");
  }

  INIT_SUCCEED = CLASS_STATIC_IMAGE != NULL && CONSTRUCTOR_STATIC_IMAGE != NULL
      && CLASS_ANIMATED_IMAGE != NULL && CONSTRUCTOR_ANIMATED_IMAGE != NULL
      && METHOD_ANIMATED_IMAGE_ON_COMPLETE != NULL;

  if (INIT_SUCCEED) {
    java_stream_init(env);
  }

  if (!INIT_SUCCEED) {
    LOGE(MSG("Can't init image java wrapper"));
  }

  return JNI_VERSION_1_6;
}

__unused
JNIEXPORT void JNICALL
JNI_OnUnload(__unused JavaVM *vm, __unused void* reserved) {}
