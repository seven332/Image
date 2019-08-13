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

#include <stdlib.h>
#include <image_info.h>

#include "image.h"
#include "image_webp.h"
#include "image_utils.h"
#include "image_decoder.h"
#include "image_convert.h"
#include "animated_image.h"
#include "../log.h"


#define IMAGE_WEBP_PREPARE_NONE 0x00
#define IMAGE_WEBP_PREPARE_BACKGROUND 0x01


typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t offset_x;
    uint32_t offset_y;
    uint32_t delay; // ms
    WebPMuxAnimDispose disposal;
    WebPMuxAnimBlend blend;
    uint8_t prepare;
    uint8_t* buffer;
} WebPFrame;

typedef struct {
    WebPFrame* frames;
    uint32_t frame_count;
    uint32_t background_color;
    Stream* stream;
    WebPDemuxer *demux;
    WebPIterator *iterator;
} WebPAnimData;


LIBRARY_EXPORT
bool webp_init(ImageLibrary* library) {
    library->loaded = true;
    library->is_magic = webp_is_magic;
    library->decode = webp_decode;
    library->decode_info = webp_decode_info;
    library->decode_buffer = webp_decode_buffer;
    library->create = NULL;
    library->get_description = webp_get_description;

    return true;
}

bool webp_is_magic(Stream* stream) {
    uint8_t magic[2];

    size_t read = stream->peek(stream, magic, sizeof(magic));
    if (read != sizeof(magic)) {
        LOGE(MSG("Could not read %zu bytes from stream, only read %zu"), sizeof(magic), read);
        return false;
    }

    return magic[0] == IMAGE_WEBP_MAGIC_NUMBER_0 && magic[1] == IMAGE_WEBP_MAGIC_NUMBER_1;
}

const char* webp_get_description() {
    return IMAGE_WEBP_DECODER_DESCRIPTION;
}


static inline void blend_over(uint8_t* dp, const uint8_t* sp, size_t len) {
    uint32_t i;
    uint32_t u, v, al;

    for (i = 0; i < len; i += 4, sp += 4, dp += 4) {
        if (sp[3] == 255) {
            memcpy(dp, sp, 4);
        } else if (sp[3] != 0) {
            if (dp[3] != 0) {
                u = sp[3] * 255u;
                v = (255u - sp[3]) * dp[3];
                al = u + v;
                dp[0] = (uint8_t) ((sp[0] * u + dp[0] * v) / al);
                dp[1] = (uint8_t) ((sp[1] * u + dp[1] * v) / al);
                dp[2] = (uint8_t) ((sp[2] * u + dp[2] * v) / al);
                dp[3] = (uint8_t) (al / 255u);
            } else {
                memcpy(dp, sp, 4);
            }
        }
    }
}

static void blend(uint8_t* dst, uint32_t dst_width, uint32_t dst_height,
                  uint8_t* src, uint32_t src_width, uint32_t src_height,
                  uint32_t offset_x, uint32_t offset_y, bool blend_op_over) {
    uint32_t i;
    uint8_t* src_ptr;
    uint8_t* dst_ptr;
    size_t len;
    uint32_t copy_width = MIN(dst_width - offset_x, src_width);
    uint32_t copy_height = MIN(dst_height - offset_y, src_height);

    for (i = 0; i < copy_height; i++) {
        src_ptr = src + (i * src_width * 4);
        dst_ptr = dst + (((offset_y + i) * dst_width + offset_x) * 4);
        len = (size_t) (copy_width * 4);

        if (blend_op_over) {
            blend_over(dst_ptr, src_ptr, len);
        } else {
            memcpy(dst_ptr, src_ptr, len);
        }
    }
}

static void read_frame(WebPIterator *iter, WebPFrame *frame, WebPFrame *pre_frame) {
    frame->buffer = WebPDecodeRGBA(iter->fragment.bytes, iter->fragment.size, (int*) &frame->width, (int*) &frame->height);
    frame->offset_x = (uint32_t) iter->x_offset;
    frame->offset_y = (uint32_t) iter->y_offset;
    frame->delay = (uint32_t) iter->duration;
    frame->disposal = iter->dispose_method;
    frame->blend = iter->blend_method;

    WebPMuxAnimDispose pre_disposal = pre_frame != NULL ? pre_frame->disposal : WEBP_MUX_DISPOSE_BACKGROUND;
    switch (pre_disposal) {
        case WEBP_MUX_DISPOSE_NONE:
            frame->prepare = IMAGE_WEBP_PREPARE_NONE;
            break;
        default:
        case WEBP_MUX_DISPOSE_BACKGROUND:
            frame->prepare = IMAGE_WEBP_PREPARE_BACKGROUND;
            break;
    }
}

static void free_frames(WebPFrame** frames, size_t count) {
    WebPFrame* frame_info;
    size_t i;

    if (frames == NULL || *frames == NULL) {
        return;
    }

    for (i = 0; i < count; i++) {
        frame_info = *frames + i;
        free(frame_info->buffer);
        frame_info->buffer = NULL;
    }
    free(*frames);
    *frames = NULL;
}

Stream *get_stream(AnimatedImage *image) {
    return ((WebPAnimData*) image->data)->stream;
}

void complete(AnimatedImage *image) {
    WebPAnimData* data = image->data;
    WebPFrame* frame;
    uint32_t i = 1;
    uint32_t j;

    if (image->completed || data->demux == NULL || data->iterator == NULL) {
        return;
    }


}

static uint32_t get_frame_count(AnimatedImage *image) {
    return ((WebPAnimData*) image->data)->frame_count;
}

uint32_t get_delay(AnimatedImage *image, uint32_t frame) {
    WebPAnimData* data = image->data;
    if (frame >= data->frame_count) {
        LOGE(MSG("Frame count is %ud, can't get delay of index %ud"), data->frame_count, frame);
        return 0;
    }
    return (data->frames + frame)->delay;
}

uint32_t get_byte_count(AnimatedImage *image) {
    uint32_t size = 0;
    uint32_t i;
    WebPFrame* frame;
    WebPAnimData* data = image->data;
    if (image->completed) {
        for (i = 0; i < data->frame_count; i++) {
            frame = data->frames + i;
            size += frame->width * frame->height * 4;
        }
        return size;
    } else {
        LOGE(MSG("Can't call get_byte_count on Uncompleted AnimatedImage"));
        return 0;
    }
}

void advance(AnimatedImage *image, DelegateImage *dImage) {
    WebPAnimData* data = image->data;
    int32_t target_index = dImage->index + 1;
    uint32_t width = image->width;
    uint32_t height = image->height;
    WebPFrame* frame;

    if (target_index < 0 || target_index >= data->frame_count) {
        target_index = 0;
    }
    if (target_index == dImage->index) {
        return;
    }

    frame = data->frames + target_index;

    // Prepare
    switch (frame->prepare) {
        case IMAGE_WEBP_PREPARE_NONE:
            // Do nothing
            break;
        default:
        case IMAGE_WEBP_PREPARE_BACKGROUND:
            // Set transparent
            memset(dImage->buffer, data->background_color, width * height * 4);
            break;
    }

    blend(dImage->buffer, dImage->width, dImage->height,
          frame->buffer, frame->width, frame->height, frame->offset_x, frame->offset_y,
          frame->blend == WEBP_MUX_BLEND);

    delegate_image_apply(dImage);

    dImage->index = target_index;
}

void recycle(AnimatedImage **image) {
    WebPAnimData *data;

    if (image == NULL || *image == NULL) {
        return;
    }

    data = (WebPAnimData*) (*image)->data;

    if (data->iterator != NULL) {
        WebPDemuxReleaseIterator(data->iterator);
        free(data->iterator);
        data->iterator = NULL;
    }
    if (data->demux != NULL) {
        WebPDemuxDelete(data->demux);
        data->demux = NULL;
    }

    if (data->stream != NULL) {
        data->stream->close(&data->stream);
        data->stream = NULL;
    }

    free(data);
    (*image)->data = NULL;

    free(*image);
    *image = NULL;
}

void* webp_decode(Stream* stream, bool partially, bool* animated) {
    void* result = NULL;
    StaticImage* static_image = NULL;
    AnimatedImage* animated_image = NULL;
    WebPAnimData* webp_anim_data = NULL;
    WebPDemuxer *demux = NULL;
    WebPIterator *iter = NULL;
    WebPFrame *frames = NULL;

    size_t data_size = 0;
    uint32_t frame_count = 1;
    uint32_t background_color = 0;
    uint32_t width;
    uint32_t height;

    uint8_t *data = stream_read_all(stream, &data_size);
    if (data == NULL) {
        WTF_OOM;
        return NULL;
    }

    // Get features
    WebPBitstreamFeatures features;
    if (WebPGetFeatures(data, data_size, &features) != VP8_STATUS_OK) {
        LOGW("Invalid webp info");
        goto end;
    }

    width = (uint32_t) features.width;
    height = (uint32_t) features.height;

    // Init demuxer if the image is animated
    if (features.has_animation) {
        WebPData webp_data = {.bytes = data, .size = data_size};
        demux = WebPDemux(&webp_data);

        frame_count = WebPDemuxGetI(demux, WEBP_FF_FRAME_COUNT);
        background_color = WebPDemuxGetI(demux, WEBP_FF_BACKGROUND_COLOR);
    }

    // Check there's at least one frame
    if (frame_count == 0) {
        LOGW("Invalid frame count");
        goto end;
    }

    // If only one frame, decode all
    if (frame_count == 1) {
        partially = false;
    }

    if (features.has_animation) {
        // Malloc iterator and frames
        iter = (WebPIterator*) malloc(sizeof(WebPIterator));
        frames = (WebPFrame*) malloc(frame_count * sizeof(WebPFrame));
        if (iter == NULL || frames == NULL) {
            WTF_OOM;
            goto end;
        }

        // Set frame buffer NULL for safety
        for (int i = 0; i < frame_count; i++) {
            (frames + i)->buffer = NULL;
        }

        // Init iterator
        if (!WebPDemuxGetFrame(demux, 1, iter)) {
            goto end;
        }

        // Read first frame
        read_frame(iter, frames, NULL);

        // Read remaining frames if not partial
        if (!partially) {
            for (int i = 1; i < frame_count; i++) {
                if (WebPDemuxNextFrame(iter)) {
                    read_frame(iter, frames + i, frames + i - 1);
                }
            }
        }

        if (frame_count == 1) {
            // For one-frame webp, use StaticImage
            static_image = static_image_new(width, height);
            if (static_image == NULL) {
                WTF_OOM;
                goto end;
            }
            memset(static_image->buffer, background_color, width * height * 4);
            blend(static_image->buffer, width, height,
                  frames->buffer, frames->width, frames->height,
                  frames->offset_x, frames->offset_y, false);

            // Free frames, don't need it anymore
            free_frames(&frames, 1);

            // Set final result
            static_image->format = IMAGE_FORMAT_WEBP;
            static_image->opaque = !features.has_alpha;
            *animated = false;
            result = static_image;
            goto end;
        } else {
            // For multi-frame webp, use AnimatedImage
            animated_image = (AnimatedImage *) malloc(sizeof(AnimatedImage));
            webp_anim_data = (WebPAnimData *) malloc(sizeof(WebPAnimData));
            if (animated_image == NULL || webp_anim_data == NULL) {
                WTF_OOM;
                goto end;
            }

            animated_image->width = width;
            animated_image->height = height;
            animated_image->format = IMAGE_FORMAT_WEBP;
            animated_image->opaque = !features.has_alpha;
            animated_image->completed = !partially;
            animated_image->data = webp_anim_data;

            animated_image->get_stream = &get_stream;
            animated_image->complete = &complete;
            animated_image->get_frame_count = &get_frame_count;
            animated_image->get_delay = &get_delay;
            animated_image->get_byte_count = &get_byte_count;
            animated_image->advance = &advance;
            animated_image->recycle = &recycle;

            webp_anim_data->frames = frames;
            webp_anim_data->frame_count = frame_count;
            webp_anim_data->background_color = background_color;

            if (partially) {
                webp_anim_data->stream = stream;
                webp_anim_data->iterator = iter;
                webp_anim_data->demux = demux;
            } else {
                webp_anim_data->stream = NULL;
                webp_anim_data->iterator = NULL;
                webp_anim_data->demux = NULL;
            }

            *animated = true;
            result = animated_image;
            goto end;
        }
    } else {
        // New static image
        static_image = static_image_new(width, height);
        if (static_image == NULL) {
            goto end;
        }

        int row_size = features.width * 4;
        size_t total_size = (size_t) (features.height * row_size);

        // Start decompress
        WebPDecodeRGBAInto(data, data_size, static_image->buffer, total_size, row_size);

        static_image->format = IMAGE_FORMAT_WEBP;
        static_image->opaque = !features.has_alpha;
        result = static_image;
        goto end;
    }

    end:

    if (!partially || result == NULL) {
        WebPDemuxReleaseIterator(iter);
        free(iter);
        WebPDemuxDelete(demux);
    }

    if (result == NULL) {
        free_frames(&frames, frame_count);
        free(animated_image);
        free(webp_anim_data);
        free(static_image);
        if (stream != NULL) {
            stream->close(&stream);
        }
    }

    return result;
}

bool webp_decode_info(Stream* stream, ImageInfo* info) {
    size_t bytes_to_read = 30;
    uint8_t data[bytes_to_read];

    stream->read(stream, data, bytes_to_read);

    WebPBitstreamFeatures features;
    if (WebPGetFeatures(data, bytes_to_read, &features) != VP8_STATUS_OK) {
        LOGE("Failed to parse webp");
        return false;
    }

    info->width = (uint32_t) features.width;
    info->height = (uint32_t) features.height;
    info->opaque = !features.has_alpha;

    return true;
}

bool webp_decode_buffer(Stream* stream, bool clip, uint32_t x, uint32_t y, uint32_t width,
                        uint32_t height, int32_t config, uint32_t ratio, BufferContainer* container) {

    bool result = false;
    uint8_t* d_buffer = NULL;

    uint32_t d_width;
    uint32_t d_height;

    // Init a configuration object
    WebPDecoderConfig webp_config;
    if (!WebPInitDecoderConfig(&webp_config)) {
        return false;
    }

    // Read the data from the stream
    size_t buffer_size = 0;
    uint8_t *buffer = stream_read_all(stream, &buffer_size);

    // Set clip info
    if (!clip) {
        // Read image headers
        uint32_t image_width = 0, image_height = 0;

        if (!WebPGetInfo(buffer, buffer_size, (int *) &image_width, (int *) &image_height)) {
            goto end;
        }

        // Decode full image
        x = 0; y = 0; width = image_width; height = image_height;
    }

    // Set config to IMAGE_CONFIG_RGB_565 as default
    if (config == IMAGE_CONFIG_AUTO) {
        config = IMAGE_CONFIG_RGB_565;
    }

    // Set out color space
    if (config == IMAGE_CONFIG_RGBA_8888) {
        webp_config.output.colorspace = MODE_RGBA;
    } else if (config == IMAGE_CONFIG_RGB_565) {
        webp_config.output.colorspace = MODE_RGB_565;
    } else {
        LOGE("Invalid config: %d", config);
        goto end;
    }

    // Fix width and height
    width = floor_uint32_t(width, ratio);
    height = floor_uint32_t(height, ratio);
    d_width = width / ratio;
    d_height = height / ratio;

    if (d_width == 0 || d_height == 0) {
        LOGE("Ratio is too large!");
        result = true;
        goto end;
    }

    // Set region to decode
    webp_config.options.use_cropping = clip;
    webp_config.options.crop_left = x;
    webp_config.options.crop_top = y;
    webp_config.options.crop_width = width;
    webp_config.options.crop_height = height;

    // Set scaled size
    webp_config.options.use_scaling = ratio != 1;
    webp_config.options.scaled_width = d_width;
    webp_config.options.scaled_height = d_height;

    // Create buffer
    d_buffer = container->create_buffer(container, d_width, d_height, config);
    if (d_buffer == NULL) { goto end; }

    uint32_t row_size = d_width * get_depth_for_config(config);

    // Save decoded data in external buffer
    webp_config.output.u.RGBA.rgba = d_buffer;
    webp_config.output.u.RGBA.size = row_size * d_height;
    webp_config.output.u.RGBA.stride = row_size;
    webp_config.output.is_external_memory = 1;

    // Decode image
    WebPDecode(buffer, buffer_size, &webp_config);

    result = true;

    end:
    if (d_buffer != NULL) {
        container->release_buffer(container, d_buffer);
    }

    WebPFreeDecBuffer(&webp_config.output);

    return result;
}
