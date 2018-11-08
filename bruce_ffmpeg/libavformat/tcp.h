/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-08
 * Description:
 
*********************************/


#ifndef TCP_H
#define TCP_H

#include "libavutil/log.h"

#include "config.h"

typedef struct TCPContext {
    const AVClass *class;
    int fd;
    int listen;
    int open_timeout;
    int rw_timeout;
    int listen_timeout;
    int recv_buffer_size;
    int send_buffer_size;
    int tcp_nodelay;
#if !HAVE_WINSOCK2_H
    int tcp_mss;
#endif
} TCPContext;

#endif
