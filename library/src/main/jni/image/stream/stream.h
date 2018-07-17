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

#ifndef IMAGE_STREAM_H
#define IMAGE_STREAM_H


#include <stdbool.h>
#include <stdint.h>


#define DEFAULT_BUFFER_SIZE 4096 // 1024 * 4


struct STREAM;
typedef struct STREAM Stream;

typedef size_t (*StreamReadFunc) (Stream* stream, void* dst, size_t size);
typedef size_t (*StreamPeekFunc) (Stream* stream, void* dst, size_t size);
typedef void   (*StreamCloseFunc)(Stream** stream);

struct STREAM {
  void* data;
  StreamReadFunc  read;
  StreamPeekFunc peek;
  StreamCloseFunc close;
};


/**
 * Read all data from this stream.
 */
void* stream_read_all(Stream* stream, size_t* size);


#endif //IMAGE_STREAM_H
