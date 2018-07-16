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

#include "test_buffer.h"
#include "stream/buffer.h"

START_TEST(test_buffer_new) {
    Buffer* buffer = buffer_new(8, false);
    ck_assert_int_eq(0, buffer->position);
    ck_assert_int_eq(0, buffer->length);
    ck_assert_int_eq(8, buffer->capacity);
    buffer_close(&buffer);

    ck_assert_ptr_null(buffer_new(0, false));
  }
END_TEST

START_TEST(test_buffer_new_from_raw) {
    void* raw = malloc(8);
    Buffer* buffer = buffer_new_from_raw(raw, 0, 8, 8, false);
    ck_assert_int_eq(0, buffer->position);
    ck_assert_int_eq(8, buffer->length);
    ck_assert_int_eq(8, buffer->capacity);
    buffer_close(&buffer);

    raw = malloc(8);
    ck_assert_ptr_null(buffer_new_from_raw(NULL, 0, 1, 2, false));
    ck_assert_ptr_null(buffer_new_from_raw(raw, 2, 1, 2, false));
    ck_assert_ptr_null(buffer_new_from_raw(raw, 0, 3, 2, false));
    ck_assert_ptr_null(buffer_new_from_raw(raw, 0, 0, 0, false));
    free(raw);
  }
END_TEST

START_TEST(test_buffer_read) {
    Buffer* buffer = buffer_new(8, false);
    void* src = malloc(4);
    void* dst = malloc(4);
    *((uint*) src) = 0x01234567U;
    *((uint*) dst) = 0x76543210U;

    ck_assert_int_eq(0, buffer_read(buffer, dst, 4));
    ck_assert_int_eq(0, buffer->position);
    ck_assert_int_eq(0, buffer->length);
    ck_assert_int_eq(8, buffer->capacity);

    buffer_write(buffer, src, 4);
    ck_assert_int_eq(2, buffer_read(buffer, dst, 2));
    ck_assert_mem_eq(src, dst, 2);
    ck_assert_int_eq(2, buffer->position);
    ck_assert_int_eq(4, buffer->length);
    ck_assert_int_eq(8, buffer->capacity);

    ck_assert_int_eq(2, buffer_read(buffer, dst + 2, 4));
    ck_assert_mem_eq(src, dst, 4);
    ck_assert_int_eq(4, buffer->position);
    ck_assert_int_eq(4, buffer->length);
    ck_assert_int_eq(8, buffer->capacity);

    buffer_close(&buffer);
  }
END_TEST

START_TEST(test_buffer_write_not_extendable) {
    Buffer* buffer = buffer_new(4, false);
    void* src = malloc(4);
    *((uint*) src) = 0x01234567U;

    ck_assert_int_eq(2, buffer_write(buffer, src, 2));
    ck_assert_int_eq(0, buffer->position);
    ck_assert_int_eq(2, buffer->length);
    ck_assert_int_eq(4, buffer->capacity);

    ck_assert_int_eq(2, buffer_write(buffer, src + 2, 3));
    ck_assert_int_eq(0, buffer->position);
    ck_assert_int_eq(4, buffer->length);
    ck_assert_int_eq(4, buffer->capacity);

    ck_assert_int_eq(0, buffer_write(buffer, src + 2, 3));
    ck_assert_int_eq(0, buffer->position);
    ck_assert_int_eq(4, buffer->length);
    ck_assert_int_eq(4, buffer->capacity);

    buffer_close(&buffer);
  }
END_TEST

START_TEST(test_buffer_write_extendable) {
    Buffer* buffer = buffer_new(2, true);
    void* src = malloc(4);
    *((uint*) src) = 0x01234567U;

    ck_assert_int_eq(2, buffer_write(buffer, src, 2));
    ck_assert_int_eq(0, buffer->position);
    ck_assert_int_eq(2, buffer->length);
    ck_assert_int_eq(2, buffer->capacity);

    ck_assert_int_eq(2, buffer_write(buffer, src + 2, 2));
    ck_assert_int_eq(0, buffer->position);
    ck_assert_int_eq(4, buffer->length);
    ck_assert_int_eq(4, buffer->capacity);

    buffer_close(&buffer);
  }
END_TEST

START_TEST(test_buffer_seek) {
    Buffer* buffer = buffer_new(4, false);
    void* src = malloc(4);
    *((uint*) src) = 0x01234567U;
    buffer_write(buffer, src, 2);

    ck_assert_int_eq(1, buffer_seek(buffer, 1));
    ck_assert_int_eq(1, buffer->position);

    ck_assert_int_eq(0, buffer_seek(buffer, 0));
    ck_assert_int_eq(0, buffer->position);

    ck_assert_int_eq(2, buffer_seek(buffer, 3));
    ck_assert_int_eq(2, buffer->position);

    buffer_close(&buffer);
  }
END_TEST

START_TEST(test_buffer_shrink) {
    Buffer* buffer = buffer_new(4, false);
    void* src = malloc(4);
    void* dst = malloc(2);
    *((uint*) src) = 0x01234567U;

    buffer_write(buffer, src, 4);
    buffer_read(buffer, dst, 2);
    buffer_shrink(buffer);

    ck_assert_mem_eq(src + 2, buffer->raw, 2);
    ck_assert_int_eq(0, buffer->position);
    ck_assert_int_eq(2, buffer->length);
    ck_assert_int_eq(4, buffer->capacity);

    buffer_close(&buffer);
  }
END_TEST

TCase* buffer_case() {
  TCase* t_case = tcase_create("Buffer");

  tcase_add_test(t_case, test_buffer_new);
  tcase_add_test(t_case, test_buffer_new_from_raw);
  tcase_add_test(t_case, test_buffer_read);
  tcase_add_test(t_case, test_buffer_write_not_extendable);
  tcase_add_test(t_case, test_buffer_write_extendable);
  tcase_add_test(t_case, test_buffer_seek);
  tcase_add_test(t_case, test_buffer_shrink);

  return t_case;
}
