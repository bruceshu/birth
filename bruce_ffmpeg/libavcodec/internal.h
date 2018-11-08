/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-23
 * Description:
 
*********************************/


#ifndef AVCODEC_INTERNAL_H
#define AVCODEC_INTERNAL_H

#include <stddef.h>

#include "libavutil/internal.h"
#include "libavutil/thread.h"
#include "libavutil/frame.h"

typedef struct DecodeSimpleContext {
    AVPacket *in_pkt;
    AVFrame  *out_frame;
} DecodeSimpleContext;

typedef struct DecodeFilterContext {
    //AVBSFContext **bsfs;
    int         nb_bsfs;
} DecodeFilterContext;

typedef struct BufferPoolEntry {
    uint8_t *data;
    void *opaque;
    void (*free)(void *opaque, uint8_t *data);
    struct AVBufferPool *pool;
    struct BufferPoolEntry *next;
} BufferPoolEntry;

typedef struct AVBufferPool {
    AVMutex mutex;
    BufferPoolEntry *pool;
    uint32_t refcount;
    int size;
    void *opaque;
    AVBufferRef* (*alloc)(int size);
    AVBufferRef* (*alloc2)(void *opaque, int size);
    void (*pool_free)(void *opaque);
} AVBufferPool;

typedef struct FramePool {
    AVBufferPool *pools[4];
    int format;
    int width, height;
    int stride_align[AV_NUM_DATA_POINTERS];
    int linesize[4];
    int planes;
    int channels;
    int samples;
} FramePool;

typedef struct AVCodecInternal {
    int is_copy;
    int allocate_progress;
    int last_audio_frame;
    AVFrame *to_free;
    FramePool *pool;
    void *thread_ctx;
    DecodeSimpleContext ds;
    DecodeFilterContext filter;
    AVPacket *last_pkt_props;
    uint8_t *byte_buffer;
    unsigned int byte_buffer_size;
    void *frame_thread_encoder;
    int skip_samples;
    void *hwaccel_priv_data;
    int draining;
    AVPacket *buffer_pkt;
    int buffer_pkt_valid; // encoding: packet without data can be valid
    AVFrame *buffer_frame;
    int draining_done;
    int compat_decode;
    int compat_decode_warned;
    size_t compat_decode_consumed;
    size_t compat_decode_partial_size;
    AVFrame *compat_decode_frame;
    int showed_multi_packet_warning;
    int skip_samples_multiplier;
    int nb_draining_errors;
} AVCodecInternal;

#endif