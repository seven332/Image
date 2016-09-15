//
// Created by Hippo on 9/14/2016.
//

#ifndef IMAGE_BUFFER_STREAM_H
#define IMAGE_BUFFER_STREAM_H


#include <stddef.h>

#include "stream.h"


Stream* buffer_stream_new(void* buffer, size_t length);

void buffer_stream_reset(Stream* stream);


#endif //IMAGE_BUFFER_STREAM_H
