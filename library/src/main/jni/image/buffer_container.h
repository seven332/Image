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

#ifndef IMAGE_BUFFER_CONTAINER_H
#define IMAGE_BUFFER_CONTAINER_H


#include <stdint.h>


struct BUFFER_CONTAINER;
typedef struct BUFFER_CONTAINER BufferContainer;

struct BUFFER_CONTAINER {
  void* data;
  void* (*create_buffer)(BufferContainer* container, uint32_t width, uint32_t height, int32_t config);
  void (*release_buffer)(BufferContainer* container, void* buffer);
};


#endif //IMAGE_BUFFER_CONTAINER_H
