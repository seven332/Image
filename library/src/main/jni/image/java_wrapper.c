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
#include "com_hippo_image_BitmapDecoder.h"
#include "com_hippo_image_BitmapRegionDecoder.h"
#include "image.h"
#include "image_utils.h"
#include "animated_image.h"
#include "bitmap_container.h"
#include "java_stream.h"
#include "buffer_stream.h"
#include "../log.h"


static bool INIT_SUCCEED = false;

static jclass CLASS_STATIC_IMAGE = NULL;
static jclass CLASS_ANIMATED_IMAGE = NULL;
static jclass CLASS_BITMAP_DECODER = NULL;
static jclass CLASS_BITMAP_REGION_DECODER = NULL;

static jmethodID CONSTRUCTOR_STATIC_IMAGE = NULL;
static jmethodID CONSTRUCTOR_ANIMATED_IMAGE = NULL;
static jmethodID CONSTRUCTOR_BITMAP_REGION_DECODER = NULL;

static jmethodID METHOD_ANIMATED_IMAGE_ON_COMPLETE = NULL;
static jmethodID METHOD_IMAGE_INFO_SET = NULL;
static jmethodID METHOD_BITMAP_DECODER_CREATE_BITMAP = NULL;
static jmethodID METHOD_BITMAP_RECYCLE = NULL;


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

static jobject bitmap_region_decoder_object_new(JNIEnv* env, Stream* stream, ImageInfo* info) {
  return (*env)->NewObject(env, CLASS_BITMAP_REGION_DECODER, CONSTRUCTOR_BITMAP_REGION_DECODER,
      (jlong) stream, (jint) info->width, (jint) info->height,
      (jint) info->format, (jboolean) info->opaque);
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

  stream = java_stream_new(env, is, true);
  if (stream == NULL) {
    LOGE(MSG("Can't create java stream"));
    return NULL;
  }

  // Decode
  decode(stream, partially, &animated, &image);

  // Close stream is necessary
  if (image == NULL || !animated || ((AnimatedImage*) image)->completed) {
    stream->close(&stream);
  }

  if (image != NULL) {
    if (!animated) {
      obj = static_image_object_new(env, image);
    } else {
      obj = animated_image_object_new(env, image);
      if (((AnimatedImage*) image)->completed) {
        animated_image_object_on_complete(env, obj, image);
      }
    }
  } else {
    obj = NULL;
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
  java_stream_set_env(stream, env);

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
      image->shown, (int) image->width, (int) image->height, src_x, src_y,
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
      image->shown, (int) image->width, (int) image->height, src_x, src_y,
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


////////////////////////////////
// BitmapDecoder
////////////////////////////////

JNIEXPORT jboolean JNICALL
Java_com_hippo_image_BitmapDecoder_nativeDecodeInfo(JNIEnv* env, __unused jclass clazz, jobject is, jobject info) {
  Stream* stream = NULL;
  ImageInfo iInfo;
  bool result;

  if (!INIT_SUCCEED) {
    return false;
  }

  stream = java_stream_new(env, is, true);
  if (stream == NULL) {
    LOGE(MSG("Can't create java stream"));
    return false;
  }

  result = decode_info(stream, &iInfo);

  if (result) {
    (*env)->CallVoidMethod(env, info, METHOD_IMAGE_INFO_SET, iInfo.width, iInfo.height,
        iInfo.format, iInfo.opaque, iInfo.frame_count);
  }

  stream->close(&stream);

  return (jboolean) result;
}

JNIEXPORT jobject JNICALL
Java_com_hippo_image_BitmapDecoder_nativeDecodeBitmap(JNIEnv* env, __unused jclass clazz, jobject is, jint config, jint ratio) {
  Stream* stream;
  BufferContainer* container;
  jobject bitmap;
  bool result;

  if (!INIT_SUCCEED) {
    return false;
  }

  stream = java_stream_new(env, is, true);
  if (stream == NULL) {
    LOGE(MSG("Can't create java stream"));
    return NULL;
  }

  container = bitmap_container_new(env, CLASS_BITMAP_DECODER, METHOD_BITMAP_DECODER_CREATE_BITMAP);
  if (container == NULL) {
    LOGE(MSG("Can't create bitmap container"));
    stream->close(&stream);
    return NULL;
  }

  result = decode_buffer(stream, false, 0, 0, 0, 0, (uint8_t) config, ratio < 1 ? 1 : (uint32_t) ratio, container);
  bitmap = bitmap_container_fetch_bitmap(container);
  bitmap_container_recycle(&container);
  stream->close(&stream);

  if (!result && bitmap != NULL) {
    // Decode failed and the bitmap is not NULL
    // Recycle it!
    (*env)->CallVoidMethod(env, bitmap, METHOD_BITMAP_RECYCLE);
    bitmap = NULL;
  }

  return bitmap;
}


////////////////////////////////
// BitmapDecoder
////////////////////////////////

JNIEXPORT jobject JNICALL
Java_com_hippo_image_BitmapRegionDecoder_nativeNewInstance(JNIEnv* env, __unused jclass clazz, jobject is) {
  Stream* java_stream = NULL;
  Stream* buffer_stream = NULL;
  void* buffer = NULL;
  size_t buffer_size;
  ImageInfo info;
  jobject obj = NULL;

  if (!INIT_SUCCEED) {
    return NULL;
  }

  java_stream = java_stream_new(env, is, false);
  if (java_stream == NULL) { goto end; }
  buffer = stream_read_all(java_stream, &buffer_size);
  if (buffer == NULL) { goto end; }
  buffer_stream = buffer_stream_new(buffer, buffer_size);
  if (buffer_stream == NULL) { goto end; }
  // Clear buffer to avoid free twice
  buffer = NULL;

  // Decode image info
  if (!decode_info(buffer_stream, &info)) { goto end; }

  // Create java object
  obj = bitmap_region_decoder_object_new(env, buffer_stream, &info);

end:
  if (java_stream != NULL) {
    java_stream->close(&java_stream);
  }
  // Only close buffer stream if create BitmapRegionDecoder failed
  if (obj == NULL && buffer_stream != NULL) {
    buffer_stream->close(&buffer_stream);
  }
  free(buffer);
  return obj;
}

JNIEXPORT jobject JNICALL
Java_com_hippo_image_BitmapRegionDecoder_nativeDecodeRegion(JNIEnv* env, __unused jclass clazz, jlong ptr,
    jint x, jint y , jint width, jint height, jint config, jint ratio) {
  BufferContainer* container = NULL;
  Stream* stream = NULL;
  jobject bitmap = NULL;
  bool result;

  stream = (Stream *) ptr;

  container = bitmap_container_new(env, CLASS_BITMAP_DECODER, METHOD_BITMAP_DECODER_CREATE_BITMAP);
  if (container == NULL) { goto end; }

  // Decode
  buffer_stream_reset(stream);
  result = decode_buffer(stream, true, (uint32_t) x, (uint32_t) y,
      (uint32_t) width, (uint32_t) height, (uint8_t) config, (uint32_t) ratio, container);
  bitmap = bitmap_container_fetch_bitmap(container);

  if (!result && bitmap != NULL) {
    // Decode failed and the bitmap is not NULL
    // Recycle it!
    (*env)->CallVoidMethod(env, bitmap, METHOD_BITMAP_RECYCLE);
    bitmap = NULL;
  }

end:
  if (container != NULL) {
    bitmap_container_recycle(&container);
  }
  return bitmap;
}

JNIEXPORT void JNICALL
Java_com_hippo_image_BitmapRegionDecoder_nativeRecycle(__unused JNIEnv* env, __unused jclass clazz, jlong ptr) {
  Stream* stream = (Stream *) ptr;
  stream->close(&stream);
}


__unused
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, __unused void* reserved) {
  JNIEnv* env = NULL;
  jclass class_image_info;
  jclass class_bitmap;

  if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_6) != JNI_OK) {
    LOGE(MSG("Can't get env in JNI_OnLoad."));
    INIT_SUCCEED = false;
    return JNI_VERSION_1_6;
  }

  CLASS_STATIC_IMAGE = (*env)->FindClass(env, "com/hippo/image/StaticImage");
  CLASS_STATIC_IMAGE = (*env)->NewGlobalRef(env, CLASS_STATIC_IMAGE);
  if (CLASS_STATIC_IMAGE != NULL) {
    CONSTRUCTOR_STATIC_IMAGE = (*env)->GetMethodID(env, CLASS_STATIC_IMAGE, "<init>", "(JIIIZI)V");
  }
  if (CLASS_STATIC_IMAGE == NULL || CONSTRUCTOR_STATIC_IMAGE == NULL) {
    LOGE(MSG("Can't find StaticImage or its constructor."));
    INIT_SUCCEED = false;
    return JNI_VERSION_1_6;
  }

  CLASS_ANIMATED_IMAGE = (*env)->FindClass(env, "com/hippo/image/AnimatedImage");
  CLASS_ANIMATED_IMAGE = (*env)->NewGlobalRef(env, CLASS_ANIMATED_IMAGE);
  if (CLASS_ANIMATED_IMAGE != NULL) {
    CONSTRUCTOR_ANIMATED_IMAGE = (*env)->GetMethodID(env, CLASS_ANIMATED_IMAGE, "<init>", "(JIIIZ)V");
    METHOD_ANIMATED_IMAGE_ON_COMPLETE = (*env)->GetMethodID(env, CLASS_ANIMATED_IMAGE, "onComplete", "(I[II)V");
  }
  if (CLASS_STATIC_IMAGE == NULL || CONSTRUCTOR_STATIC_IMAGE == NULL || METHOD_ANIMATED_IMAGE_ON_COMPLETE == NULL) {
    LOGE(MSG("Can't find AnimatedImage or its constructor or its onComplete()."));
    INIT_SUCCEED = false;
    return JNI_VERSION_1_6;
  }

  class_image_info = (*env)->FindClass(env, "com/hippo/image/ImageInfo");
  if (class_image_info != NULL) {
    METHOD_IMAGE_INFO_SET = (*env)->GetMethodID(env, class_image_info, "set", "(IIIZI)V");
  }
  if (class_image_info == NULL || METHOD_IMAGE_INFO_SET == NULL) {
    LOGE(MSG("Can't find ImageInfo or its set()."));
    INIT_SUCCEED = false;
    return JNI_VERSION_1_6;
  }

  CLASS_BITMAP_DECODER = (*env)->FindClass(env, "com/hippo/image/BitmapDecoder");
  CLASS_BITMAP_DECODER = (*env)->NewGlobalRef(env, CLASS_BITMAP_DECODER);
  if (CLASS_BITMAP_DECODER != NULL) {
    METHOD_BITMAP_DECODER_CREATE_BITMAP = (*env)->GetStaticMethodID(env, CLASS_BITMAP_DECODER, "createBitmap", "(III)Landroid/graphics/Bitmap;");
  }
  if (CLASS_BITMAP_DECODER == NULL || METHOD_BITMAP_DECODER_CREATE_BITMAP == NULL) {
    LOGE(MSG("Can't find BitmapDecoder or its createBitmap()."));
    INIT_SUCCEED = false;
    return JNI_VERSION_1_6;
  }

  CLASS_BITMAP_REGION_DECODER = (*env)->FindClass(env, "com/hippo/image/BitmapRegionDecoder");
  CLASS_BITMAP_REGION_DECODER = (*env)->NewGlobalRef(env, CLASS_BITMAP_REGION_DECODER);
  if (CLASS_BITMAP_REGION_DECODER != NULL) {
    CONSTRUCTOR_BITMAP_REGION_DECODER = (*env)->GetMethodID(env, CLASS_BITMAP_REGION_DECODER, "<init>", "(JIIIZ)V");
  }
  if (CLASS_BITMAP_REGION_DECODER == NULL || CONSTRUCTOR_BITMAP_REGION_DECODER == NULL) {
    LOGE(MSG("Can't find BitmapRegionDecoder or its constructor."));
    INIT_SUCCEED = false;
    return JNI_VERSION_1_6;
  }

  class_bitmap = (*env)->FindClass(env, "android/graphics/Bitmap");
  if (class_bitmap != NULL) {
    METHOD_BITMAP_RECYCLE = (*env)->GetMethodID(env, class_bitmap, "recycle", "()V");
  }
  if (class_bitmap == NULL || METHOD_BITMAP_RECYCLE == NULL) {
    LOGE(MSG("Can't find Bitmap or its recycle()."));
    INIT_SUCCEED = false;
    return JNI_VERSION_1_6;
  }

  java_stream_init(env);

  INIT_SUCCEED = true;
  return JNI_VERSION_1_6;
}

__unused
JNIEXPORT void JNICALL
JNI_OnUnload(__unused JavaVM *vm, __unused void* reserved) {}
