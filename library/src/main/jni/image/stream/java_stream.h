//
// Created by Hippo on 8/8/2016.
//

#ifndef IMAGE_JAVA_STREAM_H
#define IMAGE_JAVA_STREAM_H


#include <jni.h>

#include "stream.h"


void java_stream_init(JNIEnv* env);

Stream* java_stream_new(JNIEnv* env, jobject* is, bool with_buffer);

void java_stream_set_env(Stream* stream, JNIEnv* env);


#endif //IMAGE_JAVA_STREAM_H
