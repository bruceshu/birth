/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/

#ifndef URL_H
#define URL_H

#include "avio.h"

#define URL_PROTOCOL_FLAG_NESTED_SCHEME 1 /*< The protocol name can be the first part of a nested protocol scheme */
#define URL_PROTOCOL_FLAG_NETWORK       2 /*< The protocol uses network */

typedef struct URLContext {
    const AVClass *pstClass;    /**< information for av_log(). Set by url_open(). */
    const struct URLProtocol *pstUrlProt;
    void *pstPrivData;
    char *filename;             /**< specified URL */
    int flags;
    int max_packet_size;        /**< if non zero, the stream is packetized with this max packet size */
    int is_streamed;            /**< true if streamed (no seek possible), default = false */
    int is_connected;
    AVIOInterruptCB interrupt_callback;
    int64_t rw_timeout;         /**< maximum time to wait for (network) read/write operation completion, in mcs */
    const char *protocol_whitelist;
    const char *protocol_blacklist;
    int min_packet_size;        /**< if non zero, the stream is packetized with this min packet size */
} URLContext;

typedef struct URLProtocol {
    const char *url_name;
    const AVClass *pstPrivDataClass;
    int priv_data_size;
    int flags;
    const char *default_whitelist;
    
    int     (*url_open)( URLContext *pstUrlCtx, const char *pUrl, int flags);
    int     (*url_open2)(URLContext *pstUrlCtx, const char *pUrl, int flags, AVDictionary **ppstOptions);
    int     (*url_accept)(URLContext *pstUrlServerCtx, URLContext **ppstUrlClientCtx);
    int     (*url_handshake)(URLContext *pstUrlCtx);

    /**
     * Read data from the protocol.
     * If data is immediately available (even less than size), EOF is
     * reached or an error occurs (including EINTR), return immediately.
     * Otherwise:
     * In non-blocking mode, return AVERROR(EAGAIN) immediately.
     * In blocking mode, wait for data/EOF/error with a short timeout (0.1s),
     * and return AVERROR(EAGAIN) on timeout.
     * Checking interrupt_callback, looping on EINTR and EAGAIN and until
     * enough data has been read is left to the calling function; see
     * retry_transfer_wrapper in avio.c.
     */
    int     (*url_read)( URLContext *pstUrlCtx, unsigned char *pBuf, int size);
    int     (*url_write)(URLContext *pstUrlCtx, unsigned char *pBuf, int size);
    int64_t (*url_seek)( URLContext *pstUrlCtx, int64_t pos, int whence);
    int     (*url_close)(URLContext *pstUrlCtx);
    int (*url_read_pause)(URLContext *h, int pause);
    int64_t (*url_read_seek)(URLContext *h, int stream_index, int64_t timestamp, int flags);

    int (*url_get_file_handle)(URLContext *pstUrlCtx);
    int (*url_get_multi_file_handle)(URLContext *pstUrlCtx, int **handles, int *numhandles);
    int (*url_get_short_seek)(URLContext *pstUrlCtx);
    int (*url_shutdown)(URLContext *pstUrlCtx, int flags);

    
    //int (*url_check)(URLContext *h, int mask);
    //int (*url_open_dir)(URLContext *h);
    //int (*url_read_dir)(URLContext *h, AVIODirEntry **next);
    //int (*url_close_dir)(URLContext *h);
    //int (*url_delete)(URLContext *h);
    //int (*url_move)(URLContext *h_src, URLContext *h_dst);
} URLProtocol;

const URLProtocol **url_get_protocols(const char *whitelist, const char *blacklist);
const AVClass *url_context_child_class_next(const AVClass *prev);

void url_split(char *proto, int proto_size, char *authorization, int authorization_size, 
    char *hostname, int hostname_size, int *port_ptr, char *path, int path_size, const char *url);
int url_join(char *str, int size, const char *proto, const char *authorization, 
    const char *hostname, int port, const char *fmt, ...);
void make_absolute_url(char *buf, int size, const char *base, const char *rel);


int url_alloc(URLContext **ppstUrlCtx, const char *filename, int flags, const AVIOInterruptCB *int_cb);
int url_connect(URLContext *pstUrlCtx, AVDictionary **options);

int url_open_whitelist(URLContext **ppstUrlCtx, const char *filename, int flags, AVDictionary **options, URLContext *parent);
int url_handshake(URLContext *c);
int url_write(URLContext *pstUrlCtx, const unsigned char *buf, int size);
int url_read(URLContext *pstUrlCtx, unsigned char *buf, int size);
int64_t url_seek(URLContext *pstUrlCtx, int64_t pos, int whence);

int url_get_file_handle(URLContext *pstUrlCtx);
int url_get_multi_file_handle(URLContext *h, int **handles, int *numhandles);
int url_get_short_seek(URLContext *pstUrlCtx);


int url_closep(URLContext **ppstUrlCtx);
int url_close(URLContext *pstUrlCtx);

extern const AVClass url_context_class;


#endif
