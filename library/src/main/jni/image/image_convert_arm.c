//
// Created by Hippo on 9/20/2016.
//


#include "image_convert_arm.h"
#include "image_utils.h"
#include "cpu-features.h"


static volatile unsigned int support_neon = ~0U;

bool is_support_neon() {
  if (support_neon == ~0U) {
    support_neon = (android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) ? 1 : 0;
  }
  return support_neon == 1;
}


void RGBA8888_to_RGB565_row_internal_1_neon(
    const uint8_t* src, uint32_t src_x,
    uint8_t* dst, uint32_t dst_width) {
  const uint8_t* src_pos;
  uint8_t* dst_pos;
  uint32_t i;

  src_pos = src + src_x * 4;
  dst_pos = dst;

  uint32_t align_width = floor_uint32_t(dst_width, 8);
  rgba8888_to_rgb565_neon(dst_pos, src_pos, align_width);
  src_pos += align_width * 4;
  dst_pos += align_width * 2;

  for (i = align_width; i < dst_width; i++) {
    dst_pos[0] = (uint8_t) ((src_pos[1] >> 2) << 5 | (src_pos[2] >> 3));
    dst_pos[1] = (uint8_t) ((src_pos[0] >> 3) << 3 | (src_pos[1] >> 2) >> 3);
    src_pos += 4;
    dst_pos += 2;
  }
}
