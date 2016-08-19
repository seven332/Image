//
// Created by Hippo on 8/6/2016.
//

#ifndef IMAGE_PATCHED_STREAM_H
#define IMAGE_PATCHED_STREAM_H


#include "stream.h"


Stream* patched_stream_new(Stream* stream, void* patch, size_t patch_length);

Stream* patched_stream_get_stream(Stream* stream);


#endif //IMAGE_PATCHED_STREAM_H
