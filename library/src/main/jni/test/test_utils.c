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

#include <malloc.h>

#include "test_utils.h"
#include "utils.h"

START_TEST(test_max) {
    ck_assert_int_eq(100, MAX(10, 100));
    ck_assert_int_eq(100, MAX(-10, 100));
  }
END_TEST

START_TEST(test_min) {
    ck_assert_int_eq(10, MIN(10, 100));
    ck_assert_int_eq(-10, MIN(-10, 100));
  }
END_TEST

START_TEST(test_clear) {
    void* block1 = malloc(100);
    void* block2 = calloc(1, 100);

    ck_assert_mem_ne(block1, block2, 100);

    CLEAR(block1, 100);
    ck_assert_mem_eq(block1, block2, 100);
  }
END_TEST

START_TEST(test_clear_null) {
    CLEAR(NULL, 100);
  }
END_TEST

TCase* utils_case() {
  TCase* t_case = tcase_create("Utils");

  tcase_add_test(t_case, test_max);
  tcase_add_test(t_case, test_min);
  tcase_add_test(t_case, test_clear);
  tcase_add_test(t_case, test_clear_null);

  return t_case;
}
