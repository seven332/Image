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

//
// Created by Hippo on 1/3/2016.
//

#include "config.h"

#ifdef IMAGE_SUPPORT_WEBP

#include <setjmp.h>
#include <stdlib.h>

#include "image_webp.h"
#include "image_utils.h"
#include "java_wrapper.h"
#include "../log.h"
#include "../libwebp/libwebp/src/webp/decode.h"


void *WEBP_decode(JNIEnv *env, PatchHeadInputStream *patch_head_input_stream, bool partially) {
    WEBP* webp = NULL;
    uint8_t *result = NULL;
    size_t data_size = 0;
    const uint8_t* data = NULL;

    webp = (WEBP*) malloc(sizeof(WEBP));
    data = read_patch_head_input_stream_all(env,patch_head_input_stream,&data_size);
    if (!data) {
        WTF_OM;
        free(webp);
        close_patch_head_input_stream(env, patch_head_input_stream);
        destroy_patch_head_input_stream(env, &patch_head_input_stream);
        return NULL;
    }

    result = WebPDecodeRGBA(data,data_size,&webp->width,&webp->height);
    LOGE(MSG("decode done height %d width %d"), webp->height,webp->width);
    webp->buffer = result;
    // free(result);
    return webp;
}

bool WEBP_complete(WEBP *webp) {
    return true;
}

bool WEBP_is_completed(WEBP *webp) {
    return true;
}

void *WEBP_get_pixels(WEBP *webp) {
    return webp->buffer;
}

int WEBP_get_width(WEBP *webp) {
    return webp->width;
}

int WEBP_get_height(WEBP *webp) {
    return webp->height;
}

int WEBP_get_byte_count(WEBP *webp) {
    return webp->width * webp->height * 4;
}

void WEBP_render(WEBP *webp, int src_x, int src_y,
                 void *dst, int dst_w, int dst_h, int dst_x, int dst_y,
                 int width, int height, bool fill_blank, int default_color) {
    copy_pixels(webp->buffer, webp->width, webp->height, src_x, src_y,
                dst, dst_w, dst_h, dst_x, dst_y,
                width, height, fill_blank, default_color);
}

void WEBP_advance(WEBP *webp) {
}

int WEBP_get_delay(WEBP *webp) {
    return 0;
}

int WEBP_get_frame_count(WEBP *webp) {
    return 1;
}

bool WEBP_is_opaque(WEBP *webp) {
    return true;
}

void WEBP_recycle(WEBP *webp) {
    free(webp->buffer);
    webp->buffer = NULL;
    free(webp);
}

#endif // IMAGE_SUPPORT_JPEG
