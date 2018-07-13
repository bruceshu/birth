





#ifndef TCP_H
#define TCP_H

#include <pthread.h>

#include "application.h"



typedef struct TCPContext {
    const AVClass *av_class;
    int fd;
    int listen;
    int open_timeout;
    int rw_timeout;
    int listen_timeout;
    int recv_buffer_size;
    int send_buffer_size;
    int dns_cache_timeout;
    int tcp_connected;
    int dns_cache_clear;
    int fastopen;
    int fastopen_success;
    
    int addinfo_one_by_one;
    int addrinfo_timeout;
    int addrinfo_one_by_one;

    char uri[1024];
    int64_t dns_cache_timeout;
    int64_t app_ctx_intptr;
    ApplicationContext *app_ctx;
}TCPContext;

typedef struct TCPAddrinfoRequest
{
    AVBufferRef *buffer;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    char            *hostname;
    char            *servname;
    struct addrinfo  hints;
    struct addrinfo *res;

    volatile int     finished;
    int              last_error;
} TCPAddrinfoRequest;





#endif
