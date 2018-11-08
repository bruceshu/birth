/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <limits.h>

#include "libavutil/error.h"
#include "libavutil/avstring.h"
#include "libavutil/mem.h"
#include "libavutil/opt.h"
#include "libavutil/time.h"
#include "libavutil/assert.h"
#include "libavutil/utils.h"
#include "libavutil/version.h"

#include "url.h"

static int avio_read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    AVIOInternal *internal = opaque;
    return url_read(internal->h, buf, buf_size);
}

static int avio_write_packet(void *opaque, uint8_t *buf, int buf_size)
{
    AVIOInternal *internal = opaque;
    return url_write(internal->h, buf, buf_size);
}

static int64_t avio_seek(void *opaque, int64_t offset, int whence)
{
    AVIOInternal *internal = opaque;
    return url_seek(internal->h, offset, whence);
}

static int avio_short_seek(void *opaque)
{
    AVIOInternal *internal = opaque;
    return url_get_short_seek(internal->h);
}

int64_t avio_tell(AVIOContext *s)
{
    return avio_seek(s, 0, SEEK_CUR);
}

static int avio_read_pause(void *opaque, int pause)
{
    AVIOInternal *internal = opaque;
    if (!internal->h->pstUrlProt->url_read_pause)
        return AVERROR(ENOSYS);
    return internal->h->pstUrlProt->url_read_pause(internal->h, pause);
}

static int64_t avio_read_seek(void *opaque, int stream_index, int64_t timestamp, int flags)
{
    AVIOInternal *internal = opaque;
    if (!internal->h->pstUrlProt->url_read_seek)
        return AVERROR(ENOSYS);
    return internal->h->pstUrlProt->url_read_seek(internal->h, stream_index, timestamp, flags);
}

static int avio_resetbuf(AVIOContext *s, int flags)
{
    av_assert1(flags == AVIO_FLAG_WRITE || flags == AVIO_FLAG_READ);

    if (flags & AVIO_FLAG_WRITE) {
        s->buf_end = s->buffer + s->buffer_size;
        s->write_flag = 1;
    } else {
        s->buf_end = s->buffer;
        s->write_flag = 0;
    }
    return 0;
}

static void *avio_child_next(void *obj, void *prev)
{
    AVIOContext *s = obj;
    AVIOInternal *internal = s->opaque;
    return prev ? NULL : internal->h;
}

static const AVClass *avio_child_class_next(const AVClass *prev)
{
    return prev ? NULL : &url_context_class;
}

#define OFFSET(x) offsetof(AVIOContext,x)
#define E AV_OPT_FLAG_ENCODING_PARAM
#define D AV_OPT_FLAG_DECODING_PARAM
static const AVOption avio_options[] = {
    {"protocol_whitelist", OFFSET(protocol_whitelist), AV_OPT_TYPE_STRING, { .str = NULL },  CHAR_MIN, CHAR_MAX, D },
    { NULL },
};

const AVClass avio_class = {
    .class_name = "AVIOContext",
    .item_name  = av_default_item_name,
    .version    = LIBAVUTIL_VERSION_INT,
    .option     = avio_options,
    .child_next = avio_child_next,
    .child_class_next = avio_child_class_next,
};

static int avio_init_context(AVIOContext *s,
                  unsigned char *buffer,
                  int buffer_size,
                  int write_flag,
                  void *opaque,
                  int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),
                  int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
                  int64_t (*seek)(void *opaque, int64_t offset, int whence))
{
    memset(s, 0, sizeof(AVIOContext));

    s->buffer      = buffer;
    s->orig_buffer_size =
    s->buffer_size = buffer_size;
    s->buf_ptr     = buffer;
    s->buf_ptr_max = buffer;
    s->opaque      = opaque;
    s->direct      = 0;

    avio_resetbuf(s, write_flag ? AVIO_FLAG_WRITE : AVIO_FLAG_READ);

    s->write_packet    = write_packet;
    s->read_packet     = read_packet;
    s->seek            = seek;
    s->pos             = 0;
    s->eof_reached     = 0;
    s->error           = 0;
    s->seekable        = seek ? AVIO_SEEKABLE_NORMAL : 0;
    s->min_packet_size = 0;
    s->max_packet_size = 0;
    s->update_checksum = NULL;
    s->short_seek_threshold = SHORT_SEEK_THRESHOLD;

    if (!read_packet && !write_flag) {
        s->pos     = buffer_size;
        s->buf_end = s->buffer + buffer_size;
    }
    s->read_pause = NULL;
    s->read_seek  = NULL;

    s->write_data_type       = NULL;
    s->ignore_boundary_point = 0;
    s->current_type          = AVIO_DATA_MARKER_UNKNOWN;
    s->last_time             = AV_NOPTS_VALUE;
    s->short_seek_get        = NULL;
    s->written               = 0;

    return 0;
}

static AVIOContext *avio_alloc_context(
                  unsigned char *buffer,
                  int buffer_size,
                  int write_flag,
                  void *opaque,
                  int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),
                  int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
                  int64_t (*seek)(void *opaque, int64_t offset, int whence))
{
    AVIOContext *s = av_malloc(sizeof(AVIOContext));
    if (!s)
        return NULL;
    
    avio_init_context(s, buffer, buffer_size, write_flag, opaque, read_packet, write_packet, seek);
    return s;
}

static int avio_fdopen(AVIOContext **s, URLContext *h)
{
    AVIOInternal *internal = NULL;
    uint8_t *buffer = NULL;
    int buffer_size, max_packet_size;

    max_packet_size = h->max_packet_size;
    if (max_packet_size) {
        buffer_size = max_packet_size; /* no need to bufferize more than one packet */
    } else {
        buffer_size = IO_BUFFER_SIZE;
    }
    
    buffer = av_malloc(buffer_size);
    if (!buffer)
        return AVERROR(ENOMEM);

    internal = av_mallocz(sizeof(*internal));
    if (!internal)
        goto fail;

    internal->h = h;

    *s = avio_alloc_context(buffer, buffer_size, h->flags & AVIO_FLAG_WRITE,
                            internal, avio_read_packet, avio_write_packet, avio_seek);
    if (!*s)
        goto fail;

    (*s)->direct = h->flags & AVIO_FLAG_DIRECT;

    (*s)->seekable = h->is_streamed ? 0 : AVIO_SEEKABLE_NORMAL;
    (*s)->max_packet_size = max_packet_size;
    (*s)->min_packet_size = h->min_packet_size;
    if(h->pstUrlProt) {
        (*s)->read_pause = avio_read_pause;
        (*s)->read_seek  = avio_read_seek;

        if (h->pstUrlProt->url_read_seek)
            (*s)->seekable |= AVIO_SEEKABLE_TIME;
    }
    (*s)->short_seek_get = avio_short_seek;
    (*s)->av_class = &avio_class;
    return 0;
fail:
    av_freep(&internal);
    av_freep(&buffer);
    return AVERROR(ENOMEM);
}

int avio_open_whitelist(AVIOContext **ppstIOCtx, const char *filename, int flags, const AVIOInterruptCB *int_cb, AVDictionary **options)
{
    URLContext *pstUrlCtx;
    int err;

    err = url_open_whitelist(&pstUrlCtx, filename, flags, options, NULL);
    if (err < 0)
        return err;
    
    err = avio_fdopen(ppstIOCtx, pstUrlCtx);
    if (err < 0) {
        url_close(pstUrlCtx);
        return err;
    }
    
    return 0;
}

int avio_ensure_seekback(AVIOContext *s, int64_t buf_size)
{
    uint8_t *buffer;
    int max_buffer_size = s->max_packet_size ?
                          s->max_packet_size : IO_BUFFER_SIZE;
    int filled = s->buf_end - s->buffer;
    ptrdiff_t checksum_ptr_offset = s->checksum_ptr ? s->checksum_ptr - s->buffer : -1;

    buf_size += s->buf_ptr - s->buffer + max_buffer_size;

    if (buf_size < filled || s->seekable || !s->read_packet)
        return 0;
    av_assert0(!s->write_flag);

    buffer = av_malloc(buf_size);
    if (!buffer)
        return AVERROR(ENOMEM);

    memcpy(buffer, s->buffer, filled);
    av_free(s->buffer);
    s->buf_ptr = buffer + (s->buf_ptr - s->buffer);
    s->buf_end = buffer + (s->buf_end - s->buffer);
    s->buffer = buffer;
    s->buffer_size = buf_size;
    if (checksum_ptr_offset >= 0)
        s->checksum_ptr = s->buffer + checksum_ptr_offset;
    return 0;
}

int64_t avio_skip(AVIOContext *s, int64_t offset)
{
    return avio_seek(s, offset, SEEK_CUR);
}

static int avio_read_packet_wrapper(AVIOContext *s, uint8_t *buf, int size)
{
    int ret;

    if (!s->read_packet)
        return AVERROR(EINVAL);
    
    ret = s->read_packet(s->opaque, buf, size);
    
    return ret;
}

static int avio_set_buf_size(AVIOContext *s, int buf_size)
{
    uint8_t *buffer;
    buffer = av_malloc(buf_size);
    if (!buffer)
        return AVERROR(ENOMEM);

    av_free(s->buffer);
    s->buffer = buffer;
    s->orig_buffer_size = s->buffer_size = buf_size;
    s->buf_ptr = s->buf_ptr_max = buffer;
    avio_resetbuf(s, s->write_flag ? AVIO_FLAG_WRITE : AVIO_FLAG_READ);
    return 0;
}

static void avio_fill_buffer(AVIOContext *s)
{
    int max_buffer_size = s->max_packet_size ? s->max_packet_size : IO_BUFFER_SIZE;
    uint8_t *dst = s->buf_end - s->buffer + max_buffer_size < s->buffer_size ? s->buf_end : s->buffer;
    int len = s->buffer_size - (dst - s->buffer);

    /* can't fill the buffer without read_packet, just set EOF if appropriate */
    if (!s->read_packet && s->buf_ptr >= s->buf_end)
        s->eof_reached = 1;

    /* no need to do anything if EOF already reached */
    if (s->eof_reached)
        return;

    if (s->update_checksum && dst == s->buffer) {
        if (s->buf_end > s->checksum_ptr)
            s->checksum = s->update_checksum(s->checksum, s->checksum_ptr, s->buf_end - s->checksum_ptr);
        s->checksum_ptr = s->buffer;
    }

    /* make buffer smaller in case it ended up large after probing */
    if (s->read_packet && s->orig_buffer_size && s->buffer_size > s->orig_buffer_size) {
        if (dst == s->buffer && s->buf_ptr != dst) {
            int ret = avio_set_buf_size(s, s->orig_buffer_size);
            if (ret < 0) {
                av_log(s, AV_LOG_WARNING, "Failed to decrease buffer size\n");
            }

            s->checksum_ptr = dst = s->buffer;
        }
        av_assert0(len >= s->orig_buffer_size);
        len = s->orig_buffer_size;
    }

    len = avio_read_packet_wrapper(s, dst, len);
    if (len == AVERROR_EOF) {
        /* do not modify buffer if EOF reached so that a seek back can be done without rereading data */
        s->eof_reached = 1;
    } else if (len < 0) {
        s->eof_reached = 1;
        s->error= len;
    } else {
        s->pos += len;
        s->buf_ptr = dst;
        s->buf_end = dst + len;
        s->bytes_read += len;
    }
}

int avio_feof(AVIOContext *s)
{
    if(!s)
        return 0;
    if(s->eof_reached){
        s->eof_reached=0;
        avio_fill_buffer(s);
    }
    return s->eof_reached;
}

int avio_read(AVIOContext *s, unsigned char *buf, int size)
{
    int len, size1;

    size1 = size;
    while (size > 0) {
        len = FFMIN(s->buf_end - s->buf_ptr, size);
        if (len == 0 || s->write_flag) {
            if((s->direct || size > s->buffer_size) && !s->update_checksum) {
                // bypass the buffer and read data directly into buf
                len = avio_read_packet_wrapper(s, buf, size);
                if (len == AVERROR_EOF) {
                    /* do not modify buffer if EOF reached so that a seek back can be done without rereading data */
                    s->eof_reached = 1;
                    break;
                } else if (len < 0) {
                    s->eof_reached = 1;
                    s->error= len;
                    break;
                } else {
                    s->pos += len;
                    s->bytes_read += len;
                    size -= len;
                    buf += len;
                    // reset the buffer
                    s->buf_ptr = s->buffer;
                    s->buf_end = s->buffer/* + len*/;
                }
            } else {
                avio_fill_buffer(s);
                len = s->buf_end - s->buf_ptr;
                if (len == 0)
                    break;
            }
        } else {
            memcpy(buf, s->buf_ptr, len);
            buf += len;
            s->buf_ptr += len;
            size -= len;
        }
    }
    if (size1 == size) {
        if (s->error)      return s->error;
        if (avio_feof(s))  return AVERROR_EOF;
    }
    return size1 - size;
}

static void writeout(AVIOContext *s, const uint8_t *data, int len)
{
    if (!s->error) {
        int ret = 0;
        if (s->write_data_type)
            ret = s->write_data_type(s->opaque, (uint8_t *)data, len, s->current_type, s->last_time);
        else if (s->write_packet)
            ret = s->write_packet(s->opaque, (uint8_t *)data, len);
        
        if (ret < 0) {
            s->error = ret;
        } else {
            if (s->pos + len > s->written)
                s->written = s->pos + len;
        }
    }
    
    if (s->current_type == AVIO_DATA_MARKER_SYNC_POINT || s->current_type == AVIO_DATA_MARKER_BOUNDARY_POINT) {
        s->current_type = AVIO_DATA_MARKER_UNKNOWN;
    }
    
    s->last_time = AV_NOPTS_VALUE;
    s->writeout_count ++;
    s->pos += len;
}

static void avio_flush_buffer(AVIOContext *s)
{
    s->buf_ptr_max = FFMAX(s->buf_ptr, s->buf_ptr_max);
    if (s->write_flag && s->buf_ptr_max > s->buffer) {
        writeout(s, s->buffer, s->buf_ptr_max - s->buffer);
        if (s->update_checksum) {
            s->checksum     = s->update_checksum(s->checksum, s->checksum_ptr, s->buf_ptr_max - s->checksum_ptr);
            s->checksum_ptr = s->buffer;
        }
    }
    
    s->buf_ptr = s->buf_ptr_max = s->buffer;
    if (!s->write_flag)
        s->buf_end = s->buffer;
}

void avio_flush(AVIOContext *s)
{
    int seekback = s->write_flag ? FFMIN(0, s->buf_ptr - s->buf_ptr_max) : 0;
    avio_flush_buffer(s);
    if (seekback)
        avio_seek(s, seekback, SEEK_CUR);
}

int avio_close(AVIOContext *s)
{
    AVIOInternal *internal;
    URLContext *h;

    if (!s)
        return 0;

    avio_flush(s);
    internal = s->opaque;
    h = internal->h;

    av_freep(&s->opaque);
    av_freep(&s->buffer);
    if (s->write_flag)
        av_log(s, AV_LOG_VERBOSE, "Statistics: %d seeks, %d writeouts\n", s->seek_count, s->writeout_count);
    else
        av_log(s, AV_LOG_VERBOSE, "Statistics: %"PRId64" bytes read, %d seeks\n", s->bytes_read, s->seek_count);
    av_opt_free(s);

    avio_context_free(&s);

    return url_close(h);
}

int avio_closep(AVIOContext **s)
{
    int ret = avio_close(*s);
    *s = NULL;
    return ret;
}

void avio_context_free(AVIOContext **ps)
{
    av_freep(ps);
}

int avio_rewind_with_probe_data(AVIOContext *s, unsigned char **bufp, int buf_size)
{
    int64_t buffer_start;
    int buffer_size;
    int overlap, new_size, alloc_size;
    uint8_t *buf = *bufp;

    if (s->write_flag) {
        av_freep(bufp);
        return AVERROR(EINVAL);
    }

    buffer_size = s->buf_end - s->buffer;

    /* the buffers must touch or overlap */
    if ((buffer_start = s->pos - buffer_size) > buf_size) {
        av_freep(bufp);
        return AVERROR(EINVAL);
    }

    overlap = buf_size - buffer_start;
    new_size = buf_size + buffer_size - overlap;

    alloc_size = FFMAX(s->buffer_size, new_size);
    if (alloc_size > buf_size)
        if (!(buf = (*bufp) = av_realloc_f(buf, 1, alloc_size)))
            return AVERROR(ENOMEM);

    if (new_size > buf_size) {
        memcpy(buf + buf_size, s->buffer + overlap, buffer_size - overlap);
        buf_size = new_size;
    }

    av_free(s->buffer);
    s->buf_ptr = s->buffer = buf;
    s->buffer_size = alloc_size;
    s->pos = buf_size;
    s->buf_end = s->buf_ptr + buf_size;
    s->eof_reached = 0;

    return 0;
}
