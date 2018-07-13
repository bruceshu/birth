




#ifndef URL_h
#define URL_H

#include "../log_system/log.h" //depend on AVClass


#define URL_PROTOCOL_FLAG_NESTED_SCHEME 1 /*< The protocol name can be the first part of a nested protocol scheme */
#define URL_PROTOCOL_FLAG_NETWORK       2 /*< The protocol uses network */



typedef struct URLContext {
    const AVClass *av_class;
    const URLProtocol *pstUrlProt;
    void *priv_data;
    char *filename;
    int flags;
    int max_packet_size;
    int min_packet_size;
    int is_streamed;
    int is_connected;
    int64_t rw_timeout;
    AVIOInterruptCB interrupt_callback;
}URLContext;


typedef struct URLProtocol {
    const char *url_name;
    int (*url_open)(URLContext *pstUrlCtx, const char *url);
    int (*url_open2)(URLContext *pstUrlCtx, const char *url, int flags, AVDictionary **options);
    int (*url_handshake)(URLContext *pstUrlCtx);
    int (*url_read)(URLContext *pstUrlCtx, unsigned char *buf, int size);
    int (*url_write)(URLContext *pstUrlCtx, const unsigned char *buf, int size);
    int (*url_seek)(URLContext *pstUrlCtx, int64_t pos, int whence);
    int (*url_close)(URLContext *pstUrlCtx);

    int priv_data_size;
    const AVClass *priv_data_class;
    
    int flags;
}URLProtocol;






#endif
