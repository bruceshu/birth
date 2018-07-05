/*
copyright bruceshu

author:bruceshu
date:2018-07-15
description:

*/


#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "log_system/log.h"
#include "libavutils/dict.h"


typedef struct URLContext {
    const AVClass *av_class;
    const URLProtocol * prot;
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
    const char *name;
    int (*url_open)(URLContext *url_ctx, const char *url, int flags);
    int (*url_open2)(URLContext *url_ctx, const char *url, int flags, AVDictionary **options);

    int priv_data_size;
    const AVClass *priv_data_class;
    int flags;
}URLProtocol;




#endif
