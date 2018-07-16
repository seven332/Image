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

#include "image_utils.h"


uint32_t floor_uint32_t(uint32_t num, uint32_t multiple) {
  uint32_t remainder = num % multiple;
  if (remainder == 0) {
    return num;
  } else {
    return num - remainder;
  }
}

uint32_t ceil_uint32_t(uint32_t num, uint32_t multiple) {
  uint32_t remainder = num % multiple;
  if (remainder == 0) {
    return num;
  } else {
    return num - remainder + multiple;
  }
}

#if __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#  define __builtin_clzst __builtin_clz
#  define SIZE_T_ONE 1U
#elif __SIZEOF_SIZE_T__ == __SIZEOF_LONG__
#  define __builtin_clzst __builtin_clzl
#  define SIZE_T_ONE 1UL
#elif __SIZEOF_SIZE_T__ == __SIZEOF_LONG_LONG__
#  define __builtin_clzst __builtin_clzll
#  define SIZE_T_ONE 1ULL
#else
#  error BAD size_t
#endif

size_t next_pow2_size_t(size_t x) {
  return SIZE_T_ONE << (__SIZEOF_SIZE_T__ * 8 - __builtin_clzst(x - SIZE_T_ONE));
}
