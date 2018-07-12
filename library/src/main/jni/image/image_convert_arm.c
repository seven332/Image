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

#include "cpu-features.h"
#include "image_convert_arm.h"
#include "../log.h"


#define RGB565_BLUE(c) ((c) & 0x1f)
#define RGB565_GREEN(c1, c2) ((((c1) & 0xe0) >> 5) | (((c2) & 0x7) << 3))
#define RGB565_REG(c) ((c) >> 3)


static volatile unsigned int support_neon = ~0U;

bool is_support_neon() {
  if (support_neon == ~0U) {
    support_neon = (android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) ? 1 : 0;
  }
  return support_neon == 1;
}

static inline void RGBA8888_align_pixels(uint8_t* src1, uint8_t* src2, uint32_t d_width, uint32_t ratio) {
  const uint32_t start = (ratio - 2) / 2;
  const uint32_t interval = ratio;
  uint32_t* s1 = (uint32_t *) src1;
  uint32_t* s2 = (uint32_t *) src2;
  uint32_t* d1 = s1;
  uint32_t* d2 = s2;
  uint32_t i;

  s1 += start;
  s2 += start;
  for (i = 0; i < d_width; i++) {
    d1[0] = s1[0];
    d1[1] = s1[1];
    d2[0] = s2[0];
    d2[1] = s2[1];

    s1 += interval;
    s2 += interval;
    d1 += 2;
    d2 += 2;
  }
}

static inline void RGB565_align_pixels(uint8_t* src1, uint8_t* src2, uint32_t d_width, uint32_t ratio) {
  const uint32_t start = (ratio - 2) / 2;
  const uint32_t interval = ratio;
  uint16_t* s1 = (uint16_t *) src1;
  uint16_t* s2 = (uint16_t *) src2;
  uint16_t* d1 = s1;
  uint16_t* d2 = s2;
  uint32_t i;

  s1 += start;
  s2 += start;
  for (i = 0; i < d_width; i++) {
    d1[0] = s1[0];
    d1[1] = s1[1];
    d2[0] = s2[0];
    d2[1] = s2[1];

    s1 += interval;
    s2 += interval;
    d1 += 2;
    d2 += 2;
  }
}


void RGBA8888_to_RGBA8888_row_internal_2_neon(
    uint8_t* dst, const uint8_t* src1, const uint8_t* src2,
    uint32_t d_width, uint32_t ratio) {
  uint32_t i;

  // Align
  if (ratio != 2) {
    RGBA8888_align_pixels((uint8_t *) src1, (uint8_t *) src2, d_width, ratio);
  }

  // align_width is multiple of 8
  uint32_t align_width = (d_width >> 3) << 3;
  rgba8888_to_rgba8888_neon_2(dst, src1, src2, d_width);
  src1 += align_width * 2 * 4;
  src2 += align_width * 2 * 4;
  dst  += align_width * 4;

  for (i = align_width; i < d_width; i++) {
    uint16_t r, g, b, a;
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

    src1 += 2 * 4;
    src2 += 2 * 4;
    dst += 4;
  }
}

void RGBA8888_to_RGB565_row_internal_1_neon(
    uint8_t* dst, const uint8_t* src, uint32_t width) {
  uint32_t i;

  // align_width is multiple of 8
  uint32_t align_width = (width >> 3) << 3;
  rgba8888_to_rgb565_neon(dst, src, align_width);
  src += align_width * 4;
  dst += align_width * 2;

  for (i = align_width; i < width; i++) {
    uint8_t r, g, b;
    r = src[0] >> 3;
    g = src[1] >> 2;
    b = src[2] >> 3;
    dst[0] = (uint8_t) (g << 5 | b);
    dst[1] = (uint8_t) (r << 3 | g >> 3);
    src += 4;
    dst += 2;
  }
}

void RGB565_to_RGB565_row_internal_2_neon(
    uint8_t* dst, const uint8_t* src1, const uint8_t* src2,
    uint32_t d_width, uint32_t ratio) {
  uint32_t i;

  // Align
  if (ratio != 2) {
    RGB565_align_pixels((uint8_t *) src1, (uint8_t *) src2, d_width, ratio);
  }

  // align_width is multiple of 8
  uint32_t align_width = (d_width >> 3) << 3;
  rgb565_to_rgb565_neon_2(dst, (uint8_t *) src1, (uint8_t *) src2, d_width);
  src1 += align_width * 2 * 2;
  src2 += align_width * 2 * 2;
  dst  += align_width * 2;

  for (i = align_width; i < d_width; i++) {
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

    src1 += 2 * 2;
    src2 += 2 * 2;
    dst += 2;
  }
}

