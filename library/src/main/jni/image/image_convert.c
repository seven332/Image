//
// Created by Hippo on 9/18/2016.
//

#include "image_convert.h"
#include "image_decoder.h"
#include "../utils.h"


Converter* converter_new(uint32_t dst_width, uint8_t src_config, uint8_t dst_config, uint32_t ratio) {
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

void convert_delete(Converter** conv) {
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


void RGBA8888_to_RGBA8888_row_internal_1(
    const uint8_t* src, uint32_t src_x,
    uint8_t* dst, uint32_t dst_width) {
  memcpy(dst, src + src_x * 4, dst_width * 4);
}

void RGBA8888_to_RGBA8888_row_internal_2(Converter* conv,
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

void RGBA8888_to_RGBA8888_row(Converter* conv,
    const uint8_t* src, uint32_t src_x, uint32_t src_width,
    uint8_t* dst, uint32_t dst_width, uint32_t ratio) {
  if (ratio == 1) {
    RGBA8888_to_RGBA8888_row_internal_1(src, src_x, dst, dst_width);
  } else {
    RGBA8888_to_RGBA8888_row_internal_2(conv, src, src_x, src_width, dst, dst_width, ratio);
  }
}


void RGBA8888_to_RGB565_row_internal_1(
    const uint8_t* src, uint32_t src_x,
    uint8_t* dst, uint32_t dst_width) {
  const uint8_t* src_pos;
  uint8_t* dst_pos;

  src_pos = src + src_x * 4;
  dst_pos = dst;
  for (uint32_t i = 0; i < dst_width; i++) {
    dst_pos[0] = (uint8_t) ((src_pos[1] >> 2) << 5 | (src_pos[3] >> 3));
    dst_pos[1] = (uint8_t) ((src_pos[0] >> 3) << 3 | (src_pos[1] >> 2) >> 3);
    src_pos += 4;
    dst_pos += 2;
  }
}

void RGBA8888_to_RGB565_row_internal_2(Converter* conv,
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

void RGBA8888_to_RGB565_row(Converter* conv,
    const uint8_t* src, uint32_t src_x, uint32_t src_width,
    uint8_t* dst, uint32_t dst_width, uint32_t ratio) {
  if (ratio == 1) {
    RGBA8888_to_RGB565_row_internal_1(src, src_x, dst, dst_width);
  } else {
    RGBA8888_to_RGB565_row_internal_2(conv, src, src_x, src_width, dst, dst_width, ratio);
  }
}


void RGB565_to_RGBA8888_row_internal_1(
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

void RGB565_to_RGBA8888_row_internal_2(Converter* conv,
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

void RGB565_to_RGBA8888_row(Converter* conv,
    const uint8_t* src, uint32_t src_x, uint32_t src_width,
    uint8_t* dst, uint32_t dst_width, uint32_t ratio) {
  if (ratio == 1) {
    RGB565_to_RGBA8888_row_internal_1(src, src_x, dst, dst_width);
  } else {
    RGB565_to_RGBA8888_row_internal_2(conv, src, src_x, src_width, dst, dst_width, ratio);
  }
}


void RGB565_to_RGB565_row_internal_1(
    const uint8_t* src, uint32_t src_x,
    uint8_t* dst, uint32_t dst_width) {
  memcpy(dst, src + src_x * 2, dst_width * 2);
}

void RGB565_to_RGB565_row_internal_2(Converter* conv,
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

void RGB565_to_RGB565_row(Converter* conv,
    const uint8_t* src, uint32_t src_x, uint32_t src_width,
    uint8_t* dst, uint32_t dst_width, uint32_t ratio) {
  if (ratio == 1) {
    RGB565_to_RGB565_row_internal_1(src, src_x, dst, dst_width);
  } else {
    RGB565_to_RGB565_row_internal_2(conv, src, src_x, src_width, dst, dst_width, ratio);
  }
}
