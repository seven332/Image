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

#include "image_utils.h"
#include "../log.h"


#define LITTLE_ENDIAN  0x00
#define BIG_ENDIAN 0x01

static unsigned int endian = ~0U;

static bool is_big_endian() {
  unsigned int x = 1;

  if (endian == ~0U) {
    if (1 == ((char *)&x)[0]) {
      endian = LITTLE_ENDIAN;
    } else {
      endian = BIG_ENDIAN;
    }
  }

  return endian == BIG_ENDIAN;
}


static int convert_color(int origin) {
  int result;
  unsigned char* orPtr = (unsigned char *) &origin;
  unsigned char* rePtr = (unsigned char *) &result;

  if (is_big_endian()) {
    rePtr[0] = orPtr[1];
    rePtr[1] = orPtr[2];
    rePtr[2] = orPtr[3];
    rePtr[3] = orPtr[0];
  } else {
    rePtr[0] = orPtr[2];
    rePtr[1] = orPtr[1];
    rePtr[2] = orPtr[0];
    rePtr[3] = orPtr[3];
  }

  return result;
}

static inline void memset_int(int* dst, int val, size_t size) {
  int* maxPtr = dst + size;
  int* ptr = dst;
  while(ptr < maxPtr) {
    *ptr++ = val;
  }
}

static inline int ceil(int num, int multiple) {
  int remainder = num % multiple;
  if (remainder == 0) {
    return num;
  } else {
    return num - remainder + multiple;
  }
}

static inline int floor_int(int num, int multiple) {
  int remainder = num % multiple;
  if (remainder == 0) {
    return num;
  } else {
    return num - remainder;
  }
}

static inline void rgb565_color_to_rgb565(uint8_t *color, uint8_t *r, uint8_t *g, uint8_t *b) {
  *r = color[1] >> 3;
  *g = (uint8_t) (((color[1] & 0x7) << 3) | ((color[0] & 0xe0) >> 5));
  *b = (uint8_t) (color[0] & 0x1f);
}

static inline void rgb565_to_rgb565_color(uint8_t r, uint8_t g, uint8_t b, uint8_t *color) {
  color[1] = r << 3 | g >> 3;
  color[0] = g << 5 | b;
}

static inline void rgb888_to_rgba8888_color(uint8_t r, uint8_t g, uint8_t b, uint8_t *color) {
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = 0xff;
}

static inline void rgb888_to_rgb565_color(uint8_t r, uint8_t g, uint8_t b, uint8_t *color) {
  rgb565_to_rgb565_color(r >> 3, g >> 2, b >> 3, color);
}

// TODO not support if 2 * count - 2 > UINT8_MAX
static inline void average_step(uint8_t num, uint32_t count, uint8_t* x, uint8_t* y) {
  *x += num / count;
  *y += num % count;
  if (*y >= count) {
    ++(*x);
    *y -= count;
  }
}

static inline void rgba_to_color(unsigned char r, unsigned char g,
    unsigned char b, unsigned char a, int* color) {
  unsigned char* p = (unsigned char*) color;
  p[0] = r;
  p[1] = g;
  p[2] = b;
  p[3] = a;
}

static inline void color_to_rgba(int color,
    unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a) {
  unsigned char* p = (unsigned char*) &color;
  *r = p[0];
  *g = p[1];
  *b = p[2];
  *a = p[3];
}

static inline void bilinear_color(const int* ptr, int w, int ratio, int* result) {
  int i, j, count = ratio * ratio;
  unsigned char r, g, b, a;
  unsigned char r1 = 0, r2 = 0, g1 = 0, g2 = 0, b1 = 0, b2 = 0, a1 = 0, a2 = 0;

  for (i = 0; i < ratio; i++) {
    for (j = 0; j < ratio; j++) {
      color_to_rgba(*(ptr + (w * i) + j), &r, &g, &b, &a);
      average_step(r, count, &r1, &r2);
      average_step(g, count, &g1, &g2);
      average_step(b, count, &b1, &b2);
      average_step(a, count, &a1, &a2);
    }
  }

  rgba_to_color(r1, g1, b1, a1, result);
}

static inline void copy_color_internal(int* dst, const int* src, int src_w, int count, int ratio) {
  int i = 0;
  for (i = 0; i < count; ++i, ++dst, src += ratio) {
    bilinear_color(src, src_w, ratio, dst);
  }
}

static inline void copy_color(void* dst, const void* src, int src_w, int count, int ratio) {
  if (ratio == 1) {
    memcpy(dst, src, (size_t) (count * 4));
  } else {
    copy_color_internal(dst, src, src_w, count / ratio, ratio);
  }
}

static inline bool copy_pixels_internal(void* dst, int dst_w, int dst_h, int dst_x, int dst_y,
    const void* src, int src_w, int src_h, int src_x, int src_y,
    int width, int height, int ratio, bool fill_blank, int fill_color) {
  int temp;
  int line;
  int src_stride;
  int src_pos;
  int dst_pos;
  size_t dst_blank_length;

  // Check ratio
  if (ratio <= 0) {
    return false;
  }

  // Make width and height is multiple of ratio
  width = floor_int(width, ratio);
  height = floor_int(height, ratio);

  // Avoid ratio is too big to render
  if (ratio > width || ratio > height) {
    return false;
  }

  // Make sure x >= 0
  if (src_x < 0) {
    temp = ceil(-src_x, ratio);
    src_x += temp;
    dst_x += temp / ratio;
    width -= temp;
  }
  if (dst_x < 0) {
    temp = -dst_x * ratio;
    src_x += temp;
    dst_x = 0;
    width -= temp;
  }
  if (width <= 0) {
    return false;
  }

  // Make sure y >= 0
  if (src_y < 0) {
    temp = ceil(-src_y, ratio);
    src_y += temp;
    dst_y += temp / ratio;
    height -= temp;
  }
  if (dst_y < 0) {
    temp = -dst_y * ratio;
    src_y += temp;
    dst_y = 0;
    height -= temp;
  }
  if (height <= 0) {
    return false;
  }

  // Make sure x + width <= w
  temp = src_x + width - src_w;
  if (temp > 0) {
    width -= ceil(temp, ratio);
  }
  temp = dst_x + width / ratio - dst_w;
  if (temp > 0) {
    width -= temp * ratio;
  }
  if (width <= 0) {
    return false;
  }

  // Make sure y + height <= h
  temp = src_y + height - src_h;
  if (temp > 0) {
    height -= ceil(temp, ratio);
  }
  temp = dst_y + height / ratio - dst_h;
  if (temp > 0) {
    height -= temp * ratio;
  }
  if (height <= 0) {
    return false;
  }

  // Init
  src_pos = (src_y * src_w + src_x) * 4;
  dst_pos = 0;
  src_stride = src_w * 4 * ratio;

  // Start blank
  dst_blank_length = (size_t) (dst_y * dst_w + dst_x) * 4;
  // Fill start blank if necessary
  if (fill_blank) {
    memset_int((int *) (dst + dst_pos), fill_color, dst_blank_length / 4);
  }

  // First line
  dst_pos += dst_blank_length;
  copy_color(dst + dst_pos, src + src_pos, src_w, width, ratio);
  dst_pos += width * 4;
  src_pos += src_stride;

  // Other lines
  dst_blank_length = (size_t) ((dst_w - width) * 4);
  for (line = 1; line < height / ratio; line++) {
    if (fill_blank && dst_blank_length != 0) {
      memset_int((int *) (dst + dst_pos), fill_color, dst_blank_length / 4);
    }
    dst_pos += dst_blank_length;
    copy_color(dst + dst_pos, src + src_pos, src_w, width, ratio);
    dst_pos += width * 4;
    src_pos += src_stride;
  }

  // Fill left blank if necessary
  if (fill_blank) {
    memset_int((int *) (dst + dst_pos), fill_color, (size_t) (dst_w * dst_h * 4 - dst_pos) / 4);
  }

  return true;
}

uint32_t floor_uint32_t(uint32_t num, uint32_t multiple) {
  uint32_t remainder = num % multiple;
  if (remainder == 0) {
    return num;
  } else {
    return num - remainder;
  }
}

void average_step_RGBA_8888(uint8_t* line, uint8_t* quotient,
    uint8_t* remainder, uint32_t width, uint32_t ratio) {
  uint32_t count = ratio * ratio;
  uint32_t d_width = width / ratio;
  uint32_t i, j;

  for (i = 0; i < d_width; ++i) {
    for (j = 0; j < ratio; ++j) {
      // R
      average_step(line[(i * ratio + j) * 4], count, quotient + (i * 4), remainder + (i * 4));
      // G
      average_step(line[(i * ratio + j) * 4 + 1], count, quotient + (i * 4 + 1), remainder + (i * 4 + 1));
      // B
      average_step(line[(i * ratio + j) * 4 + 2], count, quotient + (i * 4 + 2), remainder + (i * 4 + 2));
      // A
      average_step(line[(i * ratio + j) * 4 + 3], count, quotient + (i * 4 + 3), remainder + (i * 4 + 3));
    }
  }
}

void average_step_RGB_888(uint8_t* line, uint8_t* quotient,
    uint8_t* remainder, uint32_t width, uint32_t ratio) {
  uint32_t count = ratio * ratio;
  uint32_t d_width = width / ratio;

  for (uint32_t i = 0; i < d_width; ++i) {
    for (uint32_t j = 0; j < ratio; ++j) {
      // R
      average_step(line[(i * ratio + j) * 3], count, quotient + (i * 3), remainder + (i * 3));
      // G
      average_step(line[(i * ratio + j) * 3 + 1], count, quotient + (i * 3 + 1), remainder + (i * 3 + 1));
      // B
      average_step(line[(i * ratio + j) * 3 + 2], count, quotient + (i * 3 + 2), remainder + (i * 3 + 2));
    }
  }
}

void average_step_RGB_565(uint8_t* line, uint8_t* quotient,
    uint8_t* remainder, uint32_t width, uint32_t ratio) {
  uint32_t count = ratio * ratio;
  uint32_t d_width = width / ratio;
  uint8_t r, g, b;
  uint32_t i, j;

  for (i = 0; i < d_width; ++i) {
    for (j = 0; j < ratio; ++j) {
      rgb565_color_to_rgb565(line + ((i * ratio + j) * 2), &r, &g, &b);
      average_step(r, count, quotient + (i * 3), remainder + (i * 3));
      average_step(g, count, quotient + (i * 3 + 1), remainder + (i * 3 + 1));
      average_step(b, count, quotient + (i * 3 + 2), remainder + (i * 3 + 2));
    }
  }
}

void RGBA_8888_fill_RGBA_8888(uint8_t* dst, const uint8_t* src, uint32_t size) {
  memcpy(dst, src, size * 4);
}

void RGBA_8888_fill_RGB_565(uint8_t* dst, const uint8_t* src, uint32_t size) {
  for (uint32_t i = 0; i < size; ++i) {
    rgb888_to_rgb565_color(src[i * 4], src[i * 4 + 1], src[i * 4 + 2], dst + (i * 2));
  }
}

void RGB_888_fill_RGB_565(uint8_t* dst, const uint8_t* src, uint32_t size) {
  for (uint32_t i = 0; i < size; ++i) {
    rgb888_to_rgb565_color(src[i * 3], src[i * 3 + 1], src[i * 3 + 2], dst + (i * 2));
  }
}

void RGB_888_fill_RGBA_8888(uint8_t* dst, const uint8_t* src, uint32_t size) {
  for (uint32_t i = 0; i < size; ++i) {
    rgb888_to_rgba8888_color(src[i * 3], src[i * 3 + 1], src[i * 3 + 2], dst + (i * 4));
  }
}

void RGB_565_plain_fill_RGB_565(uint8_t *dst, const uint8_t *src, uint32_t size) {
  for (uint32_t i = 0; i < size; ++i) {
    rgb565_to_rgb565_color(src[i * 3], src[i * 3 + 1], src[i * 3 + 2], dst + (i * 2));
  }
}

void copy_pixels(void* dst, int dst_w, int dst_h, int dst_x, int dst_y,
    const void* src, int src_w, int src_h, int src_x, int src_y,
    int width, int height, int ratio, bool fill_blank, int fill_color) {
  int color = 0;
  if (fill_blank) {
    color = convert_color(fill_color);
  }

  if (!copy_pixels_internal(dst, dst_w, dst_h, dst_x, dst_y, src, src_w, src_h, src_x,
      src_y, width, height, ratio, fill_blank, color) && fill_blank) {
    memset_int(dst, color, (size_t) (dst_w * dst_h));
  }
}
