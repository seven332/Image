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

#include <dlfcn.h>
#include <malloc.h>

#include "image.h"
#include "../log.h"

#ifdef IMAGE_SINGLE_SHARED_LIB
  #include "image_plain.h"
  #include "image_bmp.h"
  #include "image_jpeg.h"
  #include "image_png.h"
  #include "image_gif.h"
#endif

static ImageLibrary image_libraries[IMAGE_FORMAT_MAX_COUNT] = { 0 };

#ifdef IMAGE_SINGLE_SHARED_LIB
// Load a local library and call its initializer to populate the ImageLibrary struct
static void load_library_local(const char* init_func_name, ImageLibraryInitFunc init_func, ImageLibrary* library) {
    if (!init_func || !init_func(library)) {
        LOGE(MSG("Library init func %s failed"), init_func_name);
        library->loaded = false;
        return;
    }

    LOGI(MSG("Library loaded successfully with %s()"), init_func_name);
}
#else
// Load a library from lib_name, and call its initializer to populate the ImageLibrary struct
static void load_library(const char* lib_name, const char* init_func_name, ImageLibrary* library) {
  void* handle = dlopen(lib_name, RTLD_LAZY | RTLD_LOCAL);
  if (handle == NULL) {
    LOGE(MSG("Cannot find library %s"), lib_name);
    library->loaded = false;
    return;
  }

  ImageLibraryInitFunc init_func = dlsym(handle, init_func_name);
  if (init_func == NULL) {
    LOGE(MSG("Cannot find library %s's init func %s"), lib_name, init_func_name);
    library->loaded = false;
    dlclose(handle);
    return;
  }

  if (!init_func(library)) {
    LOGE(MSG("Library %s's init func %s failed"), lib_name, init_func_name);
    library->loaded = false;
    dlclose(handle);
    return;
  }

  LOGI(MSG("Library %s loaded successfully with %s()"), lib_name, init_func_name);
}
#endif

// Dynamically load any available decoders from their shared libraries
void init_image_libraries() {
#ifdef IMAGE_SINGLE_SHARED_LIB
  load_library_local("plain_init", plain_init, &image_libraries[IMAGE_FORMAT_PLAIN]);
  load_library_local("bmp_init",   NULL,       &image_libraries[IMAGE_FORMAT_BMP]);
  load_library_local("jpeg_init",  jpeg_init,  &image_libraries[IMAGE_FORMAT_JPEG]);
  load_library_local("png_init",   png_init,   &image_libraries[IMAGE_FORMAT_PNG]);
  load_library_local("gif_init",   gif_init,   &image_libraries[IMAGE_FORMAT_GIF]);
#else
  load_library("libimage.so",      "plain_init", &image_libraries[IMAGE_FORMAT_PLAIN]);
  load_library("libimage.so",      "bmp_init",   &image_libraries[IMAGE_FORMAT_BMP]);
  load_library("libimage-jpeg.so", "jpeg_init",  &image_libraries[IMAGE_FORMAT_JPEG]);
  load_library("libimage-png.so",  "png_init",   &image_libraries[IMAGE_FORMAT_PNG]);
  load_library("libimage-gif.so",  "gif_init",   &image_libraries[IMAGE_FORMAT_GIF]);
#endif
}

static ImageLibrary* get_library_for_format(int8_t format) {
  if (format < 0 || format >= IMAGE_FORMAT_MAX_COUNT) {
    return NULL;
  }

  if (!image_libraries[format].loaded) {
    return NULL;
  }

  return &image_libraries[format];
}

static ImageLibrary* get_library_for_image(Stream* stream) {
  for (size_t i = 0; i < IMAGE_FORMAT_MAX_COUNT; i++) {
    ImageLibrary* library = &image_libraries[i];
    if (!library->loaded || library->is_magic == NULL) {
      continue;
    }

    if (library->is_magic(stream)) {
      return library;
    }
  }

  // Read several bytes for error message
  uint8_t magic[2];
  size_t read = stream->peek(stream, magic, sizeof(magic));
  if (read != sizeof(magic)) {
    LOGE(MSG("Can't read %zu magic numbers from stream"), sizeof(magic));
    return NULL;
  }

  LOGE(MSG("Can't recognize the stream with starting bytes: 0x%02x 0x%02x"), magic[0], magic[1]);
  return NULL;
}

void decode(Stream* stream, bool partially, bool* animated, void** image) {
  ImageLibrary* library = get_library_for_image(stream);
  if (library == NULL || library->decode == NULL) {
    LOGE(MSG("No valid image decode could be found"));
    *image = NULL;
    return;
  }

  *image = library->decode(stream, partially, animated);
}

bool decode_info(Stream* stream, ImageInfo* info) {
  ImageLibrary* library = get_library_for_image(stream);
  if (library == NULL || library->decode_info == NULL) {
    LOGE(MSG("No valid image decode_info could be found"));
    return false;
  }

  return library->decode_info(stream, info);
}

bool decode_buffer(Stream* stream, bool clip, uint32_t x, uint32_t y, uint32_t width,
    uint32_t height, int32_t config, uint32_t ratio, BufferContainer* container) {
  ImageLibrary* library = get_library_for_image(stream);
  if (library == NULL || library->decode_buffer == NULL) {
    LOGE(MSG("No valid image decode_buffer could be found"));
    return false;
  }

  return library->decode_buffer(stream, clip, x, y, width, height, config, ratio, container);
}

StaticImage* create(uint32_t width, uint32_t height, const uint8_t* data) {
  ImageLibrary* library = get_library_for_format(IMAGE_FORMAT_PLAIN);
  if (library == NULL || library->create == NULL) {
    LOGE(MSG("No valid image create could be found"));
    return NULL;
  }

  return library->create(width, height, data);
}

int get_supported_formats(int *formats) {
  int n = 0;
  for (size_t i = 0; i < IMAGE_FORMAT_MAX_COUNT; i++) {
    if (image_libraries[i].loaded) {
      formats[n++] = i;
    }
  }
  return n;
}

const char *get_library_description(int format) {
  ImageLibrary* library = get_library_for_format(format);
  if (library == NULL || library->get_description == NULL) {
    LOGE(MSG("No valid image get_library_description could be found"));
    return NULL;
  }

  return library->get_description();
}
