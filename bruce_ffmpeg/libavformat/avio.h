/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-18
 * Description:
 
*********************************/


#ifndef AVIO_H
#define AVIO_H

#include <stdint.h>

#include "libavutil/log.h"

#define AVSEEK_SIZE 0x10000
#define AVSEEK_FORCE 0x20000

#define AVIO_FLAG_READ  1
#define AVIO_FLAG_WRITE 2
#define AVIO_FLAG_READ_WRITE (AVIO_FLAG_READ|AVIO_FLAG_WRITE)

#define AVIO_SEEKABLE_NORMAL (1 << 0)
#define AVIO_SEEKABLE_TIME   (1 << 1)

#define AVIO_FLAG_NONBLOCK 8
#define AVIO_FLAG_DIRECT 0x8000

#define IO_BUFFER_SIZE 32768
#define SHORT_SEEK_THRESHOLD 4096


enum AVIODataMarkerType {
    AVIO_DATA_MARKER_HEADER,
    AVIO_DATA_MARKER_SYNC_POINT,
    AVIO_DATA_MARKER_BOUNDARY_POINT,
    AVIO_DATA_MARKER_UNKNOWN,
    AVIO_DATA_MARKER_TRAILER
};

typedef struct AVIOInterruptCB {
    int (*callback)(void*);
    void *opaque;
}AVIOInterruptCB;

typedef struct AVIOContext {
    const AVClass *av_class;
    unsigned char *buffer;  /**< Start of the buffer. */
    int buffer_size;        /**< Maximum buffer size */
    unsigned char *buf_ptr; /**< Current position in the buffer */
    unsigned char *buf_end; 
    
    void *opaque;         
    int (*read_packet)(void *opaque, uint8_t *buf, int buf_size);
    int (*write_packet)(void *opaque, uint8_t *buf, int buf_size);
    int64_t (*seek)(void *opaque, int64_t offset, int whence);
    int64_t pos;            /**< position in the file of the current buffer */
    int must_flush;         /**< true if the next seek should flush */
    int eof_reached;        /**< true if eof reached */
    int write_flag;         /**< true if open for writing */
    int max_packet_size;
    unsigned long checksum;
    unsigned char *checksum_ptr;
    unsigned long (*update_checksum)(unsigned long checksum, const uint8_t *buf, unsigned int size);
    int error;              /**< contains the error code or 0 if no error happened */
  
    int (*read_pause)(void *opaque, int pause);
    int64_t (*read_seek)(void *opaque, int stream_index, int64_t timestamp, int flags);

    int seekable;
    int64_t maxsize;
    int direct;
    int64_t bytes_read;
    int seek_count;
    int writeout_count;
    int orig_buffer_size;
    int short_seek_threshold;
    const char *protocol_whitelist;
    const char *protocol_blacklist;
    int (*write_data_type)(void *opaque, uint8_t *buf, int buf_size, enum AVIODataMarkerType type, int64_t time);
    int ignore_boundary_point;
    enum AVIODataMarkerType current_type;
    int64_t last_time;
    int (*short_seek_get)(void *opaque);
    int64_t written;
    unsigned char *buf_ptr_max;
    int min_packet_size;
} AVIOContext;

typedef struct AVIOInternal {
    struct URLContext *h;
} AVIOInternal;

int avio_open_whitelist(AVIOContext **ppstIOCtx, const char *filename, int flags, const AVIOInterruptCB *int_cb, AVDictionary **options);
int avio_ensure_seekback(AVIOContext *s, int64_t buf_size);

int avio_close(AVIOContext *s);
int64_t avio_skip(AVIOContext *s, int64_t offset);
int64_t avio_tell(AVIOContext *s);

int avio_read(AVIOContext *s, unsigned char *buf, int size);
int avio_feof(AVIOContext *s);
void avio_flush(AVIOContext *s);
int avio_closep(AVIOContext **s);
void avio_context_free(AVIOContext **ps);
int avio_rewind_with_probe_data(AVIOContext *s, unsigned char **bufp, int buf_size);



#endif
