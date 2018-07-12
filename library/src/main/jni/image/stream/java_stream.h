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

#ifndef IMAGE_JAVA_STREAM_H
#define IMAGE_JAVA_STREAM_H


#include <jni.h>

#include "stream.h"


void java_stream_init(JNIEnv* env);

Stream* java_stream_new(JNIEnv* env, jobject* is, bool with_buffer);

void java_stream_set_env(Stream* stream, JNIEnv* env);


#endif //IMAGE_JAVA_STREAM_H
