//
// Created by Hippo on 9/18/2016.
//

#include "image_convert.h"
#include "image_decoder.h"
#include "image_utils.h"
#include "../utils.h"


#if IMAGE_CONVERT_ARM
#  include "image_convert_arm.h"
#  define IMAGE_CONVERT_SIMD_CHECK is_support_neon
#  define IMAGE_CONVERT_SIMD_RGBA8888_TO_RGB565_ROW_INTERNAL_1 RGBA8888_to_RGB565_row_internal_1_neon
#endif


static void RGBA8888_to_RGBA8888_row_internal_1(
    const uint8_t* src, uint32_t src_x,
    uint8_t* dst, uint32_t dst_width) {
  memcpy(dst, src + src_x * 4, dst_width * 4);
}

static void RGBA8888_to_RGBA8888_row_internal_2(Converter* conv,
    const uint8_t* src, uint32_t src_x, uint32_t src_width,
    uint8_t* dst, uint32_t dst_width, uint32_t ratio) {
  const uint8_t* src_pos;
  uint8_t* dst_pos;
  const uint32_t left_rem = src_x;
  const uint32_t right_rem = src_width - src_x - dst_width * ratio;
  uint32_t* r = conv->r;
  uint32_t* g = conv->g;
  uint32_t* b = conv->b;
  uint32_t* a = conv->a;
  uint32_t divisor = ratio * ratio;

  // Clear r, g, b, a
  CLEAR(r, dst_width * 4);
  CLEAR(g, dst_width * 4);
  CLEAR(b, dst_width * 4);
  CLEAR(a, dst_width * 4);

  // src to r, g, b, a
  src_pos = src;
  // Each src row
  for (uint32_t i = 0; i < ratio; ++i) {
    src_pos += left_rem * 4;
    // Each src pixel bundle
    for (uint32_t j = 0; j < dst_width; ++j) {
      // Each src pixel
      for (uint32_t k = 0; k < ratio; ++k) {
        r[j] += src_pos[0];
        g[j] += src_pos[1];
        b[j] += src_pos[2];
        a[j] += src_pos[3];
        src_pos += 4;
      }
    }
    src_pos += right_rem * 4;
  }

  // r, g, b to dst
  dst_pos = dst;
  for (uint32_t i = 0; i < dst_width; i++) {
    dst_pos[0] = (uint8_t) (r[i] / divisor);
    dst_pos[1] = (uint8_t) (g[i] / divisor);
    dst_pos[2] = (uint8_t) (b[i] / divisor);
    dst_pos[3] = (uint8_t) (a[i] / divisor);
    dst_pos += 4;
  }
}

static void RGBA8888_to_RGBA8888_row(Converter* conv,
    const uint8_t* src, uint32_t src_x, uint32_t src_width,
    uint8_t* dst, uint32_t dst_width, uint32_t ratio) {
  if (ratio == 1) {
    RGBA8888_to_RGBA8888_row_internal_1(src, src_x, dst, dst_width);
  } else {
    RGBA8888_to_RGBA8888_row_internal_2(conv, src, src_x, src_width, dst, dst_width, ratio);
  }
}


static void RGBA8888_to_RGB565_row_internal_1(
    const uint8_t* src, uint32_t src_x,
    uint8_t* dst, uint32_t dst_width) {
  const uint8_t* src_pos;
  uint8_t* dst_pos;

  src_pos = src + src_x * 4;
  dst_pos = dst;
  for (uint32_t i = 0; i < dst_width; i++) {
    dst_pos[0] = (uint8_t) ((src_pos[1] >> 2) << 5 | (src_pos[2] >> 3));
    dst_pos[1] = (uint8_t) ((src_pos[0] >> 3) << 3 | (src_pos[1] >> 2) >> 3);
    src_pos += 4;
    dst_pos += 2;
  }
}

static void RGBA8888_to_RGB565_row_internal_2(Converter* conv,
    const uint8_t* src, uint32_t src_x, uint32_t src_width,
    uint8_t* dst, uint32_t dst_width, uint32_t ratio) {
  const uint8_t* src_pos;
  uint8_t* dst_pos;
  const uint32_t left_rem = src_x;
  const uint32_t right_rem = src_width - src_x - dst_width * ratio;
  uint32_t* r = conv->r;
  uint32_t* g = conv->g;
  uint32_t* b = conv->b;
  uint32_t divisor = ratio * ratio;

  // Clear r, g, b
  CLEAR(r, dst_width * 4);
  CLEAR(g, dst_width * 4);
  CLEAR(b, dst_width * 4);

  // src to r, g, b
  src_pos = src;
  // Each src row
  for (uint32_t i = 0; i < ratio; ++i) {
    src_pos += left_rem * 4;
    // Each src pixel bundle
    for (uint32_t j = 0; j < dst_width; ++j) {
      // Each src pixel
      for (uint32_t k = 0; k < ratio; ++k) {
        r[j] += src_pos[0];
        g[j] += src_pos[1];
        b[j] += src_pos[2];
        src_pos += 4;
      }
    }
    src_pos += right_rem * 4;
  }

  // r, g, b to dst
  dst_pos = dst;
  for (uint32_t i = 0; i < dst_width; i++) {
    r[i] /= divisor;
    g[i] /= divisor;
    b[i] /= divisor;
    dst_pos[0] = (uint8_t) ((g[i] >> 2) << 5 | (b[i] >> 3));
    dst_pos[1] = (uint8_t) ((r[i] >> 3) << 3 | (g[i] >> 2) >> 3);
    dst_pos += 2;
  }
}

static void RGBA8888_to_RGB565_row(Converter* conv,
    const uint8_t* src, uint32_t src_x, uint32_t src_width,
    uint8_t* dst, uint32_t dst_width, uint32_t ratio) {
  if (ratio == 1) {
#ifdef IMAGE_CONVERT_SIMD_RGBA8888_TO_RGB565_ROW_INTERNAL_1
#  ifdef IMAGE_CONVERT_SIMD_CHECK
    if (IMAGE_CONVERT_SIMD_CHECK()) {
#  endif
      IMAGE_CONVERT_SIMD_RGBA8888_TO_RGB565_ROW_INTERNAL_1(src, src_x, dst, dst_width);
#  ifdef IMAGE_CONVERT_SIMD_CHECK
    } else {
      RGBA8888_to_RGB565_row_internal_1(src, src_x, dst, dst_width);
    }
#  endif
#else
    RGBA8888_to_RGB565_row_internal_1(src, src_x, dst, dst_width);
#endif
  } else {
    RGBA8888_to_RGB565_row_internal_2(conv, src, src_x, src_width, dst, dst_width, ratio);
  }
}


static void RGB565_to_RGBA8888_row_internal_1(
    const uint8_t* src, uint32_t src_x,
    uint8_t* dst, uint32_t dst_width) {
  const uint8_t* src_pos;
  uint8_t* dst_pos;

  src_pos = src + src_x * 2;
  dst_pos = dst;
  for (uint32_t i = 0; i < dst_width; i++) {
    dst_pos[0] = (uint8_t) (src_pos[1] & 0xf8);
    dst_pos[1] = (uint8_t) (((src_pos[1] & 0x7) << 5) | ((src_pos[0] & 0xe0) >> 3));
    dst_pos[2] = (uint8_t) ((src_pos[0] & 0x1f) << 3);
    dst_pos[4] = 0xff;
    src_pos += 2;
    dst_pos += 4;
  }
}

static void RGB565_to_RGBA8888_row_internal_2(Converter* conv,
    const uint8_t* src, uint32_t src_x, uint32_t src_width,
    uint8_t* dst, uint32_t dst_width, uint32_t ratio) {
  const uint8_t* src_pos;
  uint8_t* dst_pos;
  const uint32_t left_rem = src_x;
  const uint32_t right_rem = src_width - src_x - dst_width * ratio;
  uint32_t* r = conv->r;
  uint32_t* g = conv->g;
  uint32_t* b = conv->b;
  uint32_t divisor = ratio * ratio;

  // Clear r, g, b
  CLEAR(r, dst_width * 4);
  CLEAR(g, dst_width * 4);
  CLEAR(b, dst_width * 4);

  // src to r, g, b
  src_pos = src;
  // Each src row
  for (uint32_t i = 0; i < ratio; ++i) {
    src_pos += left_rem * 2;
    // Each src pixel bundle
    for (uint32_t j = 0; j < dst_width; ++j) {
      // Each src pixel
      for (uint32_t k = 0; k < ratio; ++k) {
        r[j] += src_pos[1] & 0xf8;
        g[j] += ((src_pos[1] & 0x7) << 5) | ((src_pos[0] & 0xe0) >> 3);
        b[j] += (src_pos[0] & 0x1f) << 3;
        src_pos += 2;
      }
    }
    src_pos += right_rem * 2;
  }

  // r, g, b to dst
  dst_pos = dst;
  for (uint32_t i = 0; i < dst_width; i++) {
    dst_pos[0] = (uint8_t) (r[i] / divisor);
    dst_pos[1] = (uint8_t) (g[i] / divisor);
    dst_pos[2] = (uint8_t) (b[i] / divisor);
    dst_pos[3] = 0xff;
    dst_pos += 4;
  }
}

static void RGB565_to_RGBA8888_row(Converter* conv,
    const uint8_t* src, uint32_t src_x, uint32_t src_width,
    uint8_t* dst, uint32_t dst_width, uint32_t ratio) {
  if (ratio == 1) {
    RGB565_to_RGBA8888_row_internal_1(src, src_x, dst, dst_width);
  } else {
    RGB565_to_RGBA8888_row_internal_2(conv, src, src_x, src_width, dst, dst_width, ratio);
  }
}


static void RGB565_to_RGB565_row_internal_1(
    const uint8_t* src, uint32_t src_x,
    uint8_t* dst, uint32_t dst_width) {
  memcpy(dst, src + src_x * 2, dst_width * 2);
}

static void RGB565_to_RGB565_row_internal_2(Converter* conv,
    const uint8_t* src, uint32_t src_x, uint32_t src_width,
    uint8_t* dst, uint32_t dst_width, uint32_t ratio) {
  const uint8_t* src_pos;
  uint8_t* dst_pos;
  const uint32_t left_rem = src_x;
  const uint32_t right_rem = src_width - src_x - dst_width * ratio;
  uint32_t* r = conv->r;
  uint32_t* g = conv->g;
  uint32_t* b = conv->b;
  uint32_t divisor = ratio * ratio;

  // Clear r, g, b
  CLEAR(r, dst_width * 4);
  CLEAR(g, dst_width * 4);
  CLEAR(b, dst_width * 4);

  // src to r, g, b
  src_pos = src;
  // Each src row
  for (uint32_t i = 0; i < ratio; ++i) {
    src_pos += left_rem * 2;
    // Each src pixel bundle
    for (uint32_t j = 0; j < dst_width; ++j) {
      // Each src pixel
      for (uint32_t k = 0; k < ratio; ++k) {
        r[j] += src_pos[1] >> 3;
        g[j] += ((src_pos[1] & 0x7) << 3) | ((src_pos[0] & 0xe0) >> 5);
        b[j] += src_pos[0] & 0x1f;
        src_pos += 2;
      }
    }
    src_pos += right_rem * 2;
  }

  // r, g, b to dst
  dst_pos = dst;
  for (uint32_t i = 0; i < dst_width; i++) {
    r[i] /= divisor;
    g[i] /= divisor;
    b[i] /= divisor;
    dst_pos[0] = (uint8_t) (g[i] << 5 | b[i]);
    dst_pos[1] = (uint8_t) (r[i] << 3 | g[i] >> 3);
    dst_pos += 2;
  }
}

static void RGB565_to_RGB565_row(Converter* conv,
    const uint8_t* src, uint32_t src_x, uint32_t src_width,
    uint8_t* dst, uint32_t dst_width, uint32_t ratio) {
  if (ratio == 1) {
    RGB565_to_RGB565_row_internal_1(src, src_x, dst, dst_width);
  } else {
    RGB565_to_RGB565_row_internal_2(conv, src, src_x, src_width, dst, dst_width, ratio);
  }
}


Converter* converter_new(uint32_t dst_width, int32_t src_config, int32_t dst_config, uint32_t ratio) {
  Converter* conv = NULL;
  uint32_t* r = NULL;
  uint32_t* g = NULL;
  uint32_t* b = NULL;
  uint32_t* a = NULL;

  conv = malloc(sizeof(Converter));
  if (conv == NULL) { goto fail; }
  // Only use r, g, b, a for resize
  if (ratio > 1) {
    r = malloc(dst_width * sizeof(uint32_t));
    g = malloc(dst_width * sizeof(uint32_t));
    b = malloc(dst_width * sizeof(uint32_t));
    if (r == NULL || g == NULL || b == NULL) { goto fail; }
    // Only use a for IMAGE_CONFIG_RGBA_8888
    if (src_config == IMAGE_CONFIG_RGBA_8888 && dst_config == IMAGE_CONFIG_RGBA_8888) {
      a = malloc(dst_width * sizeof(uint32_t));
      if (a == NULL) { goto fail; }
    }
  }

  conv->r = r;
  conv->g = g;
  conv->b = b;
  conv->a = a;
  if (src_config == IMAGE_CONFIG_RGBA_8888) {
    if (dst_config == IMAGE_CONFIG_RGBA_8888) {
      conv->convert_func = &RGBA8888_to_RGBA8888_row;
    } else if (dst_config == IMAGE_CONFIG_RGB_565) {
      conv->convert_func = &RGBA8888_to_RGB565_row;
    } else {
      goto fail;
    }
  } else if (src_config == IMAGE_CONFIG_RGB_565) {
    if (dst_config == IMAGE_CONFIG_RGBA_8888) {
      conv->convert_func = &RGB565_to_RGBA8888_row;
    } else if (dst_config == IMAGE_CONFIG_RGB_565) {
      conv->convert_func = &RGB565_to_RGB565_row;
    } else {
      goto fail;
    }
  } else {
    goto fail;
  }

  return conv;

  fail:
  free(conv);
  free(r);
  free(g);
  free(b);
  free(a);
  return NULL;
}

void converter_delete(Converter** conv) {
  if (conv == NULL || *conv == NULL) {
    return;
  }

  free((*conv)->r);
  (*conv)->r = NULL;
  free((*conv)->g);
  (*conv)->g = NULL;
  free((*conv)->b);
  (*conv)->b = NULL;
  free((*conv)->a);
  (*conv)->a = NULL;
  free(*conv);
  *conv = NULL;
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
  uint32_t w, h;
  uint32_t src_depth, dst_depth;
  Converter* conv;

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

  // Create converter
  conv = converter_new((uint32_t) (width / ratio), src_config, dst_config, (uint32_t) ratio);
  if (conv == NULL) { return false; }

  // Assign depth
  src_depth = get_depth_for_config(src_config);
  dst_depth = get_depth_for_config(dst_config);

  // Fill start blank lines
  len = (uint32_t) (dst_y * dst_w);
  if (fill_blank && len > 0) {
    memset_color(dst, fill_color, dst_depth, len);
  }
  dst += len * dst_depth;

  // Copy lines
  w = (uint32_t) (width / ratio);
  h = (uint32_t) (height / ratio);
  src += src_y * src_w * src_depth;
  for (uint32_t i = 0; i < h; ++i) {
    // Fill line start blank
    len = (uint32_t) dst_x;
    if (fill_blank && len > 0) {
      memset_color(dst, fill_color, dst_depth, len);
    }
    dst += len * dst_depth;

    // Convert
    conv->convert_func(conv, src, (uint32_t) src_x, (uint32_t) src_w, dst, w, (uint32_t) ratio);
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

  converter_delete(&conv);
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
