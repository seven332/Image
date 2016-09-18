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

#include "config.h"
#ifdef IMAGE_SUPPORT_GIF

#include <stdlib.h>

#include "image.h"
#include "image_gif.h"
#include "../log.h"


#define IMAGE_GIF_PREPARE_NONE 0x00
#define IMAGE_GIF_PREPARE_BACKGROUND 0x01
#define IMAGE_GIF_PREPARE_USE_BACKUP 0x02


typedef struct {
  int tran;
  int disposal;
  int delay;
  int prepare;
} GifFrame;

typedef struct {
  GifFileType* gif_file;
  GifFrame* frames;
  Stream* stream;
} GifData;


typedef struct {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned char alpha;
} RGBA;


static int error_code = 0;


static int custom_read_fun(GifFileType* gif, GifByteType* bytes, int size) {
  Stream* stream = gif->UserData;
  return (int) stream->read(stream, bytes, (size_t) size);
}

static void read_gcb(GifFileType* gif_file, int index, GifFrame* frame, GifFrame* pre_frame) {
  GraphicsControlBlock gcb;
  int pre_disposal;

  // Read gcb
  if (DGifSavedExtensionToGCB(gif_file, index, &gcb) == GIF_OK) {
    frame->tran = gcb.TransparentColor;
    frame->delay = gcb.DelayTime * 10;
    frame->disposal = gcb.DisposalMode;
  } else {
    frame->tran = -1;
    frame->delay = 0;
    frame->disposal = DISPOSE_DO_NOT;
  }

  // Set pop
  pre_disposal = pre_frame != NULL ? pre_frame->disposal : DISPOSE_BACKGROUND;
  switch (pre_disposal) {
    case DISPOSAL_UNSPECIFIED:
    case DISPOSE_DO_NOT:
    default:
      frame->prepare = IMAGE_GIF_PREPARE_NONE;
      break;
    case DISPOSE_BACKGROUND:
      frame->prepare = IMAGE_GIF_PREPARE_BACKGROUND;
      break;
    case DISPOSE_PREVIOUS:
      frame->prepare = IMAGE_GIF_PREPARE_USE_BACKUP;
      break;
  }
}

static bool get_color_from_table(const ColorMapObject* cmap, int index, RGBA* color) {
  if (cmap == NULL || index < 0 || index >= cmap->ColorCount) {
    return false;
  } else {
    GifColorType gct = cmap->Colors[index];
    color->red = gct.Red;
    color->green = gct.Green;
    color->blue = gct.Blue;
    color->alpha = 0xff;
    return true;
  }
}

static void clear(RGBA* pixels, int num, RGBA color) {
  RGBA* ptr = pixels;
  int i;
  for (i = 0; i < num; i++) {
    *(ptr++) = color;
  }
}

static void clear_bg(GifFileType* gif_file, void* pixels) {
  RGBA color;
  if (!get_color_from_table(gif_file->SColorMap, gif_file->SBackGroundColor, &color)) {
    color.red = 0x00;
    color.green = 0x00;
    color.blue = 0x00;
    color.alpha = 0x00;
  }
  clear(pixels, gif_file->SWidth * gif_file->SHeight, color);
}

static void copy_line(GifByteType* src, RGBA* dst, const ColorMapObject* cmap, int tran, size_t len) {
  for (; len > 0; len--, src++, dst++) {
    int index = *src;
    if (tran == -1 || index != tran) {
      get_color_from_table(cmap, index, dst);
    }
  }
}

static void blend(GifFileType* gif_file, int index, void* pixels, int tran) {
  int width = gif_file->SWidth;
  int height = gif_file->SHeight;
  SavedImage* cur = gif_file->SavedImages + index;
  GifImageDesc desc = cur->ImageDesc;
  int copy_width = MIN(width - desc.Left, desc.Width);
  int copy_height = MIN(height - desc.Top, desc.Height);
  ColorMapObject *cmap = desc.ColorMap;
  GifByteType* src = cur->RasterBits;
  RGBA* dst = pixels;
  GifByteType* src_ptr;
  RGBA* dst_ptr;
  size_t len;
  int i;

  if (cmap == NULL) {
    cmap = gif_file->SColorMap;
  }
  if (cmap != NULL) {
    for (i = 0; i < copy_height; i++) {
      src_ptr = src + (i * desc.Width);
      dst_ptr = dst + ((desc.Top + i) * width + desc.Left);
      len = (size_t) copy_width;
      copy_line(src_ptr, dst_ptr, cmap, tran, len);
    }
  } else {
    LOGW(MSG("Can't find color map"));
  }
}

static void free_last_frame(GifFileType* gif_file) {
  SavedImage* last_image = gif_file->SavedImages + gif_file->ImageCount;

  if (last_image->ImageDesc.ColorMap != NULL) {
    GifFreeMapObject(last_image->ImageDesc.ColorMap);
    last_image->ImageDesc.ColorMap = NULL;
  }

  if (last_image->RasterBits != NULL) {
    free(last_image->RasterBits);
    last_image->RasterBits = NULL;
  }

  GifFreeExtensions(&last_image->ExtensionBlockCount, &last_image->ExtensionBlocks);

  --gif_file->ImageCount;
}

// Remove last frame if it is invalid
static void fix_gif_file(GifFileType* gif_file) {
  if (gif_file->ImageCount == 0) {
    return;
  }

  // If the last RasterBits is NULL, discard it
  if ((gif_file->SavedImages + (gif_file->ImageCount - 1))->RasterBits == NULL) {
    free_last_frame(gif_file);
  }
}

static Stream* get_stream(AnimatedImage* image) {
  return ((GifData*) image->data)->stream;
}

static void complete(AnimatedImage* image) {
  GifData* data = image->data;
  GifFrame* frames;
  uint32_t i;

  if (image->completed || data->gif_file == NULL || data->stream == NULL) {
    return;
  }

  if (DGifSlurp(data->gif_file) == GIF_ERROR) {
    fix_gif_file(data->gif_file);
  }

  // Close stream
  data->stream->close(&data->stream);

  frames = realloc(data->frames, data->gif_file->ImageCount * sizeof(GifFrame));
  if (frames != NULL) {
    // Read gcb
    for (i = 0; i < data->gif_file->ImageCount; i++) {
      read_gcb(data->gif_file, i, frames + i, i == 0 ? NULL : frames + (i - 1));
    }
    // Assign
    data->frames = frames;
  } else {
    WTF_OOM;
    // Make it only one frame
    while (data->gif_file->ImageCount > 1) {
      free_last_frame(data->gif_file);
    }
    data->frames->delay = INT32_MAX;
  }

  // Clean up
  data->stream = NULL;

  // Completed
  image->completed = true;
}

static uint32_t get_frame_count(AnimatedImage* image) {
  return (uint32_t) ((GifData*)image->data)->gif_file->ImageCount;
}

static uint32_t get_delay(AnimatedImage* image, uint32_t frame) {
  GifData* data = image->data;
  if (frame >= data->gif_file->ImageCount) {
    LOGE(MSG("Frame count is %ud, can't get delay of index %ud"), data->gif_file->ImageCount, frame);
    return 0;
  }
  return (uint32_t) (data->frames + frame)->delay;
}

static uint32_t get_byte_count(AnimatedImage* image) {
  GifFileType* gif_file = ((GifData*) image->data)->gif_file;
  SavedImage* saved_image;
  uint32_t size = 0;
  uint32_t i;
  if (gif_file->SavedImages != NULL) {
    for (i = 0; i < gif_file->ImageCount; i++) {
      saved_image = gif_file->SavedImages + i;
      size += saved_image->ImageDesc.Width * saved_image->ImageDesc.Height;
    }
  }
  return size;
}

static void advance(AnimatedImage* image, DelegateImage* dImage) {
  GifData* data = image->data;
  GifFrame* frame;
  GifFileType* gif_file = data->gif_file;
  int32_t target_index = dImage->index + 1;

  if (target_index < 0 || target_index >= gif_file->ImageCount) {
    target_index = 0;
  }
  if (target_index == dImage->index) {
    return;
  }

  frame = data->frames + target_index;

  if (frame->disposal == DISPOSE_PREVIOUS && frame->prepare == IMAGE_GIF_PREPARE_USE_BACKUP) {
    delegate_image_switch_data_backup(dImage);
  } else {
    // Backup
    if (frame->disposal == DISPOSE_PREVIOUS) {
      delegate_image_backup(dImage);
    }

    // Prepare
    switch (frame->prepare) {
      case IMAGE_GIF_PREPARE_NONE:
        // Do nothing
        break;
      default:
      case IMAGE_GIF_PREPARE_BACKGROUND:
        clear_bg(gif_file, dImage->buffer);
        break;
      case IMAGE_GIF_PREPARE_USE_BACKUP:
        delegate_image_restore(dImage);
        break;
    }
  }

  blend(gif_file, target_index, dImage->buffer, frame->tran);

  delegate_image_apply(dImage);

  dImage->index = target_index;
}

static void recycle(AnimatedImage** image) {
  GifData* data;

  if (image == NULL || *image == NULL) {
    return;
  }

  data = (*image)->data;

  DGifCloseFile(data->gif_file, &error_code);
  data->gif_file = NULL;

  free(data->frames);
  data->frames = NULL;

  if (data->stream != NULL) {
    data->stream->close(&data->stream);
    data->stream = NULL;
  }

  free(data);
  (*image)->data = NULL;

  free(*image);
  *image = NULL;
}

AnimatedImage* gif_decode(Stream* stream, bool partially) {
  AnimatedImage* animated_image = NULL;
  GifData* gif_data = NULL;
  GifFrame* frames = NULL;
  GifFileType* gif_file = NULL;
  int i;

  animated_image = malloc(sizeof(AnimatedImage));
  gif_data = malloc(sizeof(GifData));
  if (animated_image == NULL || gif_data == NULL) {
    WTF_OOM;
    free(animated_image);
    free(gif_data);
    return NULL;
  }

  // Open
  gif_file = DGifOpen(stream, &custom_read_fun, &error_code);
  if (gif_file == NULL) {
    WTF_OOM;
    free(animated_image);
    free(gif_data);
    return NULL;
  }

  if (partially) {
    // Glance
    if (DGifGlance(gif_file) != GIF_OK) {
      LOGE(MSG("GIF error code %d"), error_code);
      DGifCloseFile(gif_file, &error_code);
      free(animated_image);
      free(gif_data);
      return NULL;
    }

    // Frame info
    frames = malloc(sizeof(GifFrame));
    if (frames == NULL) {
      WTF_OOM;
      DGifCloseFile(gif_file, &error_code);
      free(animated_image);
      free(gif_data);
      return NULL;
    }

    // Read gcb
    read_gcb(gif_file, 0, frames, NULL);
  } else {
    // Slurp
    if (DGifSlurp(gif_file) == GIF_ERROR) {
      fix_gif_file(gif_file);
    }
    if (gif_file->ImageCount <= 0) {
      LOGE(MSG("No frame"));
      DGifCloseFile(gif_file, &error_code);
      free(animated_image);
      free(gif_data);
      return NULL;
    }

    // Frame info
    frames = malloc(gif_file->ImageCount * sizeof(GifFrame));
    if (frames == NULL) {
      WTF_OOM;
      DGifCloseFile(gif_file, &error_code);
      free(animated_image);
      free(gif_data);
      return NULL;
    }

    // Read gcb
    for (i = 0; i < gif_file->ImageCount; i++) {
      read_gcb(gif_file, i, frames + i, i == 0 ? NULL : frames + (i - 1));
    }
  }

  gif_data->gif_file = gif_file;
  gif_data->frames = frames;
  gif_data-> stream = partially ? stream : NULL;

  animated_image->width = (uint32_t) gif_file->SWidth;
  animated_image->height = (uint32_t) gif_file->SHeight;
  animated_image->format = IMAGE_FORMAT_GIF;
  animated_image->opaque = frames->tran < 0;
  animated_image->completed = !partially;
  animated_image->data = gif_data;

  animated_image->get_stream = &get_stream;
  animated_image->complete = &complete;
  animated_image->get_frame_count = &get_frame_count;
  animated_image->get_delay = &get_delay;
  animated_image->get_byte_count = &get_byte_count;
  animated_image->advance = &advance;
  animated_image->recycle = &recycle;

  return animated_image;
}

bool gif_decode_info(Stream* stream, ImageInfo* info) {
  GifFileType* gif_file = NULL;

  // Open
  gif_file = DGifOpen(stream, &custom_read_fun, &error_code);
  if (gif_file == NULL) {
    WTF_OOM;
    return NULL;
  }

  info->width = (uint32_t) gif_file->SWidth;
  info->height = (uint32_t) gif_file->SHeight;
  info->format = IMAGE_FORMAT_GIF;
  info->opaque = false; // Can't get opaque state, set false
  info->frame_count = -1; // For gif, must decode all frame to get frame count.

  DGifCloseFile(gif_file, &error_code);
  return true;
}


#endif // IMAGE_SUPPORT_GIF
