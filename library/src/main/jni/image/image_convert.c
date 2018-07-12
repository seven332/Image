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

#include "image_convert.h"
#include "image_decoder.h"
#include "image_utils.h"
#include "../log.h"


#if IMAGE_CONVERT_ARM
#  include "image_convert_arm.h"
#  define IMAGE_CONVERT_SIMD_CHECK is_support_neon
#  define IMAGE_CONVERT_SIMD_RGBA8888_TO_RGBA8888_ROW_INTERNAL_2 RGBA8888_to_RGBA8888_row_internal_2_neon
#  define IMAGE_CONVERT_SIMD_RGBA8888_TO_RGB565_ROW_INTERNAL_1   RGBA8888_to_RGB565_row_internal_1_neon
#  define IMAGE_CONVERT_SIMD_RGB565_TO_RGB565_ROW_INTERNAL_2     RGB565_to_RGB565_row_internal_2_neon
#endif


#define RGB565_BLUE(c) ((c) & 0x1f)
#define RGB565_GREEN(c1, c2) ((((c1) & 0xe0) >> 5) | (((c2) & 0x7) << 3))
#define RGB565_REG(c) ((c) >> 3)


void RGBA8888_to_RGBA8888_row_internal_2(
    uint8_t* dst, const uint8_t* src1, const uint8_t* src2,
    uint32_t d_width, uint32_t ratio) {
  uint32_t i;
  uint32_t start = (ratio - 2) / 2 * 4;
  uint32_t interval = ratio * 4;

  src1 += start;
  src2 += start;
  for (i = 0; i < d_width; i++) {
    register uint16_t r, g, b, a;
    r = src1[0] + src2[0];
    g = src1[1] + src2[1];
    b = src1[2] + src2[2];
    a = src1[3] + src2[3];
    r += src1[4] + src2[4];
    g += src1[5] + src2[5];
    b += src1[6] + src2[6];
    a += src1[7] + src2[7];

    dst[0] = (uint8_t) (r / 4);
    dst[1] = (uint8_t) (g / 4);
    dst[2] = (uint8_t) (b / 4);
    dst[3] = (uint8_t) (a / 4);

    src1 += interval;
    src2 += interval;
    dst += 4;
  }
}

void RGBA8888_to_RGBA8888_row(uint8_t* dst,
    const uint8_t* src1, const uint8_t* src2,
    uint32_t d_width, uint32_t ratio) {
  if (ratio == 1) {
    memcpy(dst, src1, d_width * 4);
  } else {
#ifdef IMAGE_CONVERT_SIMD_RGBA8888_TO_RGBA8888_ROW_INTERNAL_2
#  ifdef IMAGE_CONVERT_SIMD_CHECK
    if (IMAGE_CONVERT_SIMD_CHECK()) {
#  endif
      IMAGE_CONVERT_SIMD_RGBA8888_TO_RGBA8888_ROW_INTERNAL_2(dst, src1, src2, d_width, ratio);
#  ifdef IMAGE_CONVERT_SIMD_CHECK
    } else {
      RGBA8888_to_RGBA8888_row_internal_2(dst, src1, src2, d_width, ratio);
    }
#  endif
#else
    RGBA8888_to_RGBA8888_row_internal_2(dst, src1, src2, d_width, ratio);
#endif
  }
}


static void RGBA8888_to_RGB565_row_internal_1(
    uint8_t* dst, const uint8_t* src, uint32_t width) {
  uint32_t i;
  for (i = 0; i < width; i++) {
    register uint8_t r, g, b;
    r = src[0] >> 3;
    g = src[1] >> 2;
    b = src[2] >> 3;

    dst[0] = (uint8_t) (g << 5 | b);
    dst[1] = (uint8_t) (r << 3 | g >> 3);

    src += 4;
    dst += 2;
  }
}

static void RGBA8888_to_RGB565_row_internal_2(
    uint8_t* dst, const uint8_t* src1, const uint8_t* src2,
    uint32_t d_width, uint32_t ratio) {
  uint32_t i;
  uint32_t start = (ratio - 2) / 2 * 4;
  uint32_t interval = ratio * 4;

  src1 += start;
  src2 += start;
  for (i = 0; i < d_width; i++) {
    register uint16_t r, g, b;
    r = src1[0] + src2[0];
    g = src1[1] + src2[1];
    b = src1[2] + src2[2];
    r += src1[4] + src2[4];
    g += src1[5] + src2[5];
    b += src1[6] + src2[6];
    r = (uint16_t) ((r / 4) >> 3);
    g = (uint16_t) ((g / 4) >> 2);
    b = (uint16_t) ((b / 4) >> 3);

    dst[0] = (uint8_t) (g << 5 | b);
    dst[1] = (uint8_t) (r << 3 | g >> 3);

    src1 += interval;
    src2 += interval;
    dst += 2;
  }
}

void RGBA8888_to_RGB565_row(uint8_t* dst,
    const uint8_t* src1, const uint8_t* src2,
    uint32_t d_width, uint32_t ratio) {
  if (ratio == 1) {
#ifdef IMAGE_CONVERT_SIMD_RGBA8888_TO_RGB565_ROW_INTERNAL_1
#  ifdef IMAGE_CONVERT_SIMD_CHECK
    if (IMAGE_CONVERT_SIMD_CHECK()) {
#  endif
      IMAGE_CONVERT_SIMD_RGBA8888_TO_RGB565_ROW_INTERNAL_1(dst, src1, d_width);
#  ifdef IMAGE_CONVERT_SIMD_CHECK
    } else {
      RGBA8888_to_RGB565_row_internal_1(dst, src1, d_width);
    }
#  endif
#else
    RGBA8888_to_RGB565_row_internal_1(dst, src1, d_width);
#endif
  } else {
    RGBA8888_to_RGB565_row_internal_2(dst, src1, src2, d_width, ratio);
  }
}


static void RGB565_to_RGB565_row_internal_2(
    uint8_t* dst, const uint8_t* src1, const uint8_t* src2,
    uint32_t d_width, uint32_t ratio) {
  uint32_t i;
  uint32_t start = (ratio - 2) / 2 * 2;
  uint32_t interval = ratio * 2;

  src1 += start;
  src2 += start;
  for (i = 0; i < d_width; i++) {
    register uint8_t r, g, b;

    b = (uint8_t) RGB565_BLUE(src1[0]) + (uint8_t) RGB565_BLUE(src2[0]);
    g = (uint8_t) RGB565_GREEN(src1[0], src1[1]) + (uint8_t) RGB565_GREEN(src2[0], src2[1]);
    r = RGB565_REG(src1[1]) + RGB565_REG(src2[1]);
    b += (uint8_t) RGB565_BLUE(src1[2]) + (uint8_t) RGB565_BLUE(src2[2]);
    g += (uint8_t) RGB565_GREEN(src1[2], src1[3]) + (uint8_t) RGB565_GREEN(src2[2], src2[3]);
    r += RGB565_REG(src1[3]) + RGB565_REG(src2[3]);

    b /= 4;
    g /= 4;
    r /= 4;

    dst[0] = b | g << 5;
    dst[1] = g >> 3 | r << 3;

    src1 += interval;
    src2 += interval;
    dst += 2;
  }
}

void RGB565_to_RGB565_row(uint8_t* dst,
    const uint8_t* src1, const uint8_t* src2,
    uint32_t d_width, uint32_t ratio) {
  if (ratio == 1) {
    memcpy(dst, src1, d_width * 2);
  } else {
#ifdef IMAGE_CONVERT_SIMD_RGB565_TO_RGB565_ROW_INTERNAL_2
#  ifdef IMAGE_CONVERT_SIMD_CHECK
    if (IMAGE_CONVERT_SIMD_CHECK()) {
#  endif
      IMAGE_CONVERT_SIMD_RGB565_TO_RGB565_ROW_INTERNAL_2(dst, src1, src2, d_width, ratio);
#  ifdef IMAGE_CONVERT_SIMD_CHECK
    } else {
      RGB565_to_RGB565_row_internal_2(dst, src1, src2, d_width, ratio);
    }
#  endif
#else
    RGB565_to_RGB565_row_internal_2(dst, src1, src2, d_width, ratio);
#endif
  }
}


static void memset_color(uint8_t* dst, uint8_t* color, size_t depth, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    for (size_t j = 0; j < depth; ++j) {
      dst[j] = color[j];
    }
    dst += depth;
  }
}

// Use int32_t instead of uint32_t to avoid
// the type hide negative number.
static bool convert_internal(
    uint8_t* dst, int32_t dst_config,
    int32_t dst_w, int32_t dst_h,
    int32_t dst_x, int32_t dst_y,
    uint8_t* src, int32_t src_config,
    int32_t src_w, int32_t src_h,
    int32_t src_x, int32_t src_y,
    int32_t width, int32_t height,
    int32_t ratio, bool fill_blank, uint8_t* fill_color) {

  int32_t temp;
  uint32_t len;
  uint8_t* line1;
  uint8_t* line2;
  RowFunc row_func = NULL;

  // Make width and height is multiple of ratio
  width = floor_uint32_t((uint32_t) width, (uint32_t) ratio);
  height = floor_uint32_t((uint32_t) height, (uint32_t) ratio);

  // Avoid ratio is too big to render
  if (ratio > width || ratio > height) { return false; }

  // Make sure x >= 0
  if (src_x < 0) {
    temp = ceil_uint32_t((uint32_t) -src_x, (uint32_t) ratio);
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
  if (width <= 0) { return false; }

  // Make sure y >= 0
  if (src_y < 0) {
    temp = ceil_uint32_t((uint32_t) -src_y, (uint32_t) ratio);
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
  if (height <= 0) { return false; }

  // Make sure x + width <= w
  temp = src_x + width - src_w;
  if (temp > 0) {
    width -= ceil_uint32_t((uint32_t) temp, (uint32_t) ratio);
  }
  temp = dst_x + width / ratio - dst_w;
  if (temp > 0) {
    width -= temp * ratio;
  }
  if (width <= 0) { return false; }

  // Make sure y + height <= h
  temp = src_y + height - src_h;
  if (temp > 0) {
    height -= ceil_uint32_t((uint32_t) temp, (uint32_t) ratio);
  }
  temp = dst_y + height / ratio - dst_h;
  if (temp > 0) {
    height -= temp * ratio;
  }
  if (height <= 0) { return false; }

  // Assign depth
  const uint32_t src_depth = get_depth_for_config(src_config);
  const uint32_t dst_depth = get_depth_for_config(dst_config);

  // Row function
  if (src_config == IMAGE_CONFIG_RGBA_8888) {
    if (dst_config == IMAGE_CONFIG_RGBA_8888) {
      row_func = &RGBA8888_to_RGBA8888_row;
    } else if (dst_config == IMAGE_CONFIG_RGB_565) {
      row_func = &RGBA8888_to_RGB565_row;
    } else {
      LOGE("Invalid dst config: %d", dst_config);
      return false;
    }
  } else if (src_config == IMAGE_CONFIG_RGB_565) {
    if (dst_config == IMAGE_CONFIG_RGB_565) {
      row_func = &RGB565_to_RGB565_row;
    } else {
      LOGE("Invalid dst config: %d", dst_config);
      return false;
    }
  } else {
    LOGE("Invalid src config: %d", dst_config);
  }

  // Fill start blank lines
  len = (uint32_t) (dst_y * dst_w);
  if (fill_blank && len > 0) {
    memset_color(dst, fill_color, dst_depth, len);
  }
  dst += len * dst_depth;

  // Copy lines
  const uint32_t w = (uint32_t) (width / ratio);
  const uint32_t h = (uint32_t) (height / ratio);
  const uint32_t skip = (const uint32_t) ((ratio - 2) / 2);
  src += src_y * src_w * src_depth;
  for (uint32_t i = 0; i < h; ++i) {
    // Fill line start blank
    len = (uint32_t) dst_x;
    if (fill_blank && len > 0) {
      memset_color(dst, fill_color, dst_depth, len);
    }
    dst += len * dst_depth;

    // Convert
    line1 = src + ((skip * src_w + src_x) * src_depth);
    line2 = line1 + (src_w * src_depth);
    row_func(dst, line1, line2, w, (uint32_t) ratio);
    dst += w * dst_depth;

    // Fill line end blank
    len = dst_w - dst_x - w;
    if (fill_blank && len > 0) {
      memset_color(dst, fill_color, dst_depth, len);
    }
    dst += len * dst_depth;

    src += ratio * src_w * src_depth;
  }

  // Fill end blank lines
  len = (dst_h - dst_y - h) * dst_w;
  if (fill_blank && len > 0) {
    memset_color(dst, fill_color, dst_depth, len);
  }

  return true;
}

void convert(uint8_t* dst, int32_t dst_config,
    uint32_t dst_w, uint32_t dst_h,
    int32_t dst_x, int32_t dst_y,
    uint8_t* src, int32_t src_config,
    uint32_t src_w, uint32_t src_h,
    int32_t src_x, int32_t src_y,
    uint32_t width, uint32_t height,
    uint32_t ratio, bool fill_blank, uint32_t fill_color) {
  // Can't convert for not explicit config
  if (!is_explicit_config(src_config) || !is_explicit_config(dst_config)) {
    return;
  }

  // 4 is enough
  uint8_t color[4];
  size_t color_depth = get_depth_for_config(dst_config);

  if (dst_config == IMAGE_CONFIG_RGBA_8888) {
    memcpy(color, &fill_color, 4);
  } else if (dst_config == IMAGE_CONFIG_RGB_565) {
    uint8_t* p = (uint8_t *) &fill_color;
    color[0] = (uint8_t) ((p[1] >> 2) << 5 | (p[2] >> 3));
    color[1] = (uint8_t) ((p[0] >> 3) << 3 | (p[1] >> 2) >> 3);
  }

  if (!convert_internal(dst, dst_config, dst_w, dst_h, dst_x, dst_y,
      src, src_config, src_w, src_h, src_x, src_y, width, height,
      ratio, fill_blank, color) && fill_blank) {
    memset_color(dst, color, color_depth, (size_t) (dst_w * dst_h));
  }
}
