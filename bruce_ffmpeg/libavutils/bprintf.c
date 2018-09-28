/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/



#include <stdlib.h>

#define av_bprint_room(buf) ((buf)->size - FFMIN((buf)->len, (buf)->size))
#define av_bprint_is_allocated(buf) ((buf)->str != (buf)->reserved_internal_buffer)


static int av_bprint_is_complete(const AVBPrint *buf)
{
    return buf->len < buf->size;
}

static int av_bprint_alloc(AVBPrint *buf, unsigned room)
{
    char *old_str, *new_str;
    unsigned min_size, new_size;

    if (buf->size == buf->size_max)
        return AVERROR(EIO);
    
    if (!av_bprint_is_complete(buf))
        return AVERROR_INVALIDDATA; /* it is already truncated anyway */
    
    min_size = buf->len + 1 + FFMIN(UINT_MAX - buf->len - 1, room);
    new_size = buf->size > buf->size_max / 2 ? buf->size_max : buf->size * 2;
    
    if (new_size < min_size)
        new_size = FFMIN(buf->size_max, min_size);
    
    old_str = av_bprint_is_allocated(buf) ? buf->str : NULL;
    new_str = av_realloc(old_str, new_size);
    
    if (!new_str)
        return AVERROR(ENOMEM);
    
    if (!old_str)
        memcpy(new_str, buf->str, buf->len + 1);
    
    buf->str  = new_str;
    buf->size = new_size;
    
    return 0;
}


void av_bprint_init(AVBPrint *buf, unsigned size_init, unsigned size_max)
{
    unsigned size_auto = (char *)buf + sizeof(*buf) - buf->reserved_internal_buffer;

    if (size_max == 1)
        size_max = size_auto;
    
    buf->str      = buf->reserved_internal_buffer;
    buf->len      = 0;
    buf->size     = FFMIN(size_auto, size_max);
    buf->size_max = size_max;
    *buf->str = 0;
    
    if (size_init > buf->size)
        av_bprint_alloc(buf, size_init - 1);
}

static void av_bprint_grow(AVBPrint *buf, unsigned extra_len)
{
    /* arbitrary margin to avoid small overflows */
    extra_len = FFMIN(extra_len, UINT_MAX - 5 - buf->len);
    buf->len += extra_len;
    
    if (buf->size)
        buf->str[FFMIN(buf->len, buf->size - 1)] = 0;
}

void av_bprintf(AVBPrint *buf, const char *fmt, ...)
{
    unsigned room;
    char *dst;
    va_list vl;
    int extra_len;

    while (1) {
        room = av_bprint_room(buf);
        dst = room ? buf->str + buf->len : NULL;
        va_start(vl, fmt);
        extra_len = vsnprintf(dst, room, fmt, vl);
        va_end(vl);
        if (extra_len <= 0)
            return;
        if (extra_len < room)
            break;
        if (av_bprint_alloc(buf, extra_len))
            break;
    }
    av_bprint_grow(buf, extra_len);
}

