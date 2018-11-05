/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/

#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

#define AV_BUFFER_FLAG_READONLY (1 << 0)
#define BUFFER_FLAG_READONLY      (1 << 0)

typedef struct AVBuffer {
    uint8_t *data; /**< data described by this buffer */
    int      size; /**< size of data in bytes */
    uint32_t refcount;
    void (*free)(void *opaque, uint8_t *data);
    void *opaque;
    int flags;
} AVBuffer;

typedef struct AVBufferRef {
    AVBuffer *buffer;
    uint8_t *data;
    int      size;
} AVBufferRef;


void av_buffer_default_free(void *opaque, uint8_t *data);
AVBufferRef *av_buffer_create(uint8_t *data, int size, void (*free)(void *opaque, uint8_t *data), void *opaque, int flags);
void buffer_replace(AVBufferRef **dst, AVBufferRef **src);
void av_buffer_unref(AVBufferRef **buf);


#endif
