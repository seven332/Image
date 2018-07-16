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

#ifndef IMAGE_BUFFER_H
#define IMAGE_BUFFER_H

#include <stdbool.h>

struct BUFFER;
typedef struct BUFFER Buffer;

/**
 * Buffer provides some functions to control a memory block.
 *
 * The memory looks like this.
 * |-------|-----------|---------|
 * 0    position    length    capacity
 */
struct BUFFER {
  /**
   * The memory block.
   * Never access it directly.
   */
  void* raw;
  /**
   * The position of next read. It moves toward after reading. But it can't go beyond length.
   * Read-only.
   */
  size_t position;
  /**
   * The position of next write. It moves toward after writing. But it can't go beyond capacity.
   * Read-only.
   */
  size_t length;
  /**
   * The size of the memory block.
   * Read-only.
   */
  size_t capacity;
  /**
   * Whether the memory block can be extended.
   * Read-only.
   */
  bool extendable;
};

/**
 * Create a buffer with a newly malloc memory block.
 *
 * @param capacity The initially size of the memory block.
 * @param extendable True if the memory block can be extended.
 * @return A buffer, or NULL if capacity is zero or OOM.
 */
Buffer* buffer_new(size_t capacity, bool extendable);

/**
 * Create a buffer with an existed memory block.
 *
 * @param raw The memory block. The buffer handles the freedom of the memory block.
 * @param position The position of next read.
 * @param length The position of next write.
 * @param capacity The initially size of the memory block.
 * @param extendable True if the memory block can be extended.
 * @return A buffer, or NULL if raw is NULL or position > length or length > capacity or capacity is zero or OOM.
 */
Buffer* buffer_new_from_raw(void* raw, size_t position, size_t length, size_t capacity, bool extendable);

/**
 * Read bytes from the memory block to dst. The starting point of the memory block is position.
 * Position moves toward while reading. At most size bytes can be read,
 * or until position reaches length.
 *
 * @param buffer The buffer to read.
 * @param dst The reading destination.
 * @param size The maximum number of bytes to read.
 * @return How many bytes has been read.
 */
size_t buffer_read(Buffer* buffer, void* dst, size_t size);

/**
 * Write bytes from src to the memory block. The starting point of the memory block is length.
 * Length moves toward while reading. At most size bytes can be read,
 * or until length reaches capacity if the buffer is not extendable.
 *
 * @param buffer The buffer to write.
 * @param src The writing source.
 * @param size The maximum number of bytes to write.
 * @return How many bytes has been written.
 */
size_t buffer_write(Buffer* buffer, const void* src, size_t size);

/**
 * Change position of the buffer.
 *
 * @param buffer The buffer to seek.
 * @param position The new position. If it's bigger than length. The new position is length.
 * @return The actual position after seek. It's always between 0 and length.
 */
size_t buffer_seek(Buffer* buffer, size_t position);

/**
 * Discard bytes from 0 to position and move bytes from position to length ahead.
 *
 * @param buffer The buffer to buffer.
 */
void buffer_shrink(Buffer* buffer);

/**
 * Free all resources associated with this buffer and itself.
 *
 * @param buffer The buffer to close.
 */
void buffer_close(Buffer** buffer);

#endif //IMAGE_BUFFER_H
