/*
 * Copyright 2018 Hippo Seven
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
// Created by msm595 on 7/04/2018.
//

#ifndef IMAGE_LIBRARY_IMAGE_H
#define IMAGE_LIBRARY_IMAGE_H

#define LIBRARY_EXPORT __attribute__ ((visibility ("default")))

struct ImageLibrary;
typedef struct ImageLibrary ImageLibrary;

typedef bool (*ImageLibraryInitFunc)(ImageLibrary* library);
typedef bool (*ImageLibraryIsMagic)(Stream* stream);
typedef void* (*ImageLibraryDecodeFunc)(Stream* stream, bool partially, bool* animated);
typedef bool (*ImageLibraryDecodeInfoFunc)(Stream* stream, ImageInfo* info);
typedef bool (*ImageLibraryDecodeBufferFunc)(Stream* stream, bool clip, uint32_t x, uint32_t y,
    uint32_t width, uint32_t height, int32_t config, uint32_t ratio, BufferContainer* container);
typedef StaticImage* (*ImageLibraryCreateFunc)(uint32_t width, uint32_t height, const uint8_t* data);
typedef const char* (*ImageLibraryGetDescription)(void);

struct ImageLibrary {
  bool loaded;

  ImageLibraryIsMagic is_magic;
  ImageLibraryDecodeFunc decode;
  ImageLibraryDecodeInfoFunc decode_info;
  ImageLibraryDecodeBufferFunc decode_buffer;
  ImageLibraryCreateFunc create;
  ImageLibraryGetDescription get_description;
};

#endif //IMAGE_LIBRARY_IMAGE_H
