

#include "../log_system/log.h"
#include "dns_cache.h"

#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>


#define HAVE_PTHREADS 1
#define HAVE_STRUCT_SOCKADDR_IN6 1

static void tcp_getaddrinfo_request_free_buffer(void *opaque, uint8_t *data)
{
    TCPAddrinfoRequest *req = (TCPAddrinfoRequest *)opaque;
    tcp_getaddrinfo_request_free(req);
}


static void tcp_getaddrinfo_request_free(TCPAddrinfoRequest *req)
{
    if (req->res) {
        freeaddrinfo(req->res);
        req->res = NULL;
    }

    freep(&req->servname);
    freep(&req->hostname);
    pthread_cond_destroy(&req->cond);
    pthread_mutex_destroy(&req->mutex);
    freep(&req);
}


static int tcp_getaddrinfo_request_create(TCPAddrinfoRequest **request,
                                          const char *hostname,
                                          const char *servname,
                                          const struct addrinfo *hints)
{
    TCPAddrinfoRequest *req = (TCPAddrinfoRequest *) av_mallocz(sizeof(TCPAddrinfoRequest));
    if (!req)
        return AVERROR(ENOMEM);

    if (pthread_mutex_init(&req->mutex, NULL)) {
        freep(&req);
        return AVERROR(ENOMEM);
    }

    if (pthread_cond_init(&req->cond, NULL)) {
        pthread_mutex_destroy(&req->mutex);
        freep(&req);
        return AVERROR(ENOMEM);
    }

    /*
    if (int_cb)
        req->interrupt_callback = *int_cb;
    */

    if (hostname) {
        req->hostname = strdup(hostname);
        if (!req->hostname)
            goto fail;
    }

    if (servname) {
        req->servname = strdup(servname);
        if (!req->servname)
            goto fail;
    }

    if (hints) {
        req->hints.ai_family   = hints->ai_family;
        req->hints.ai_socktype = hints->ai_socktype;
        req->hints.ai_protocol = hints->ai_protocol;
        req->hints.ai_flags    = hints->ai_flags;
    }

    req->buffer = av_buffer_create(NULL, 0, tcp_getaddrinfo_request_free_buffer, req, 0);
    if (!req->buffer)
        goto fail;

    *request = req;
    return 0;
fail:
    tcp_getaddrinfo_request_free(req);
    return AVERROR(ENOMEM);
}


static void *tcp_getaddrinfo_worker(void *arg)
{
    TCPAddrinfoRequest *req = arg;

    getaddrinfo(req->hostname, req->servname, &req->hints, &req->res);
    
    pthread_mutex_lock(&req->mutex);
    req->finished = 1;
    pthread_cond_signal(&req->cond);
    pthread_mutex_unlock(&req->mutex);
    
    //av_buffer_unref(&req->buffer);
    return NULL;
}


int tcp_getaddrinfo_nonblock(const char *hostname, const char *servname,
                                const struct addrinfo *hints, struct addrinfo **res,
                                int64_t timeout, int one_by_one)
{
    int     ret;
    int64_t start;
    int64_t now;
    //AVBufferRef        *req_ref = NULL;
    TCPAddrinfoRequest *req     = NULL;
    pthread_t work_thread;

    if (hostname && !hostname[0])
        hostname = NULL;

    if (timeout <= 0)
        return getaddrinfo(hostname, servname, hints, res);

    ret = tcp_getaddrinfo_request_create(&req, hostname, servname, hints);
    if (ret)
        goto fail;

    /*
    req_ref = av_buffer_ref(req->buffer);
    if (req_ref == NULL) {
        ret = -1;
        goto fail;
    }
    */

    /* FIXME: using a thread pool would be better. */
    if (one_by_one)
        ret = pthread_create(&work_thread, NULL, tcp_getaddrinfo_one_by_one_worker, req);
    else
        ret = pthread_create(&work_thread, NULL, tcp_getaddrinfo_worker, req);

    if (ret) {
        ret = -1;
        goto fail;
    }

    pthread_detach(work_thread);

    start = av_gettime();
    now   = start;

    pthread_mutex_lock(&req->mutex);
    while (1) {
        int64_t wait_time = now + 100000;
        struct timespec tv = { .tv_sec  =  wait_time / 1000000,
                               .tv_nsec = (wait_time % 1000000) * 1000 };

        if (req->finished || (start + timeout < now)) {
            if (req->res) {
                ret = 0;
                *res = req->res;
                req->res = NULL;
            } else {
                ret = req->last_error ? req->last_error : AVERROR_EXIT;
            }
            break;
        }
#if defined(__ANDROID__) && defined(HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC)
        ret = pthread_cond_timedwait_monotonic_np(&req->cond, &req->mutex, &tv);
#else
        ret = pthread_cond_timedwait(&req->cond, &req->mutex, &tv);
#endif
        if (ret != 0 && ret != ETIMEDOUT) {
            //av_log(NULL, AV_LOG_ERROR, "pthread_cond_timedwait failed: %d\n", ret);
            ret = AVERROR_EXIT;
            break;
        }

        /*
        if (ff_check_interrupt(&req->interrupt_callback)) {
            ret = AVERROR_EXIT;
            break;
        }
        */

        now = av_gettime();
    }
    pthread_mutex_unlock(&req->mutex);
fail:
    //av_buffer_unref(&req_ref);
    return ret;
}


static int tcp_open(URLContext *pstUrlCtx, const char *uri){
    TCPContext *pstTcpCtx = pstUrlCtx->priv_data;
    char buf[256] = {0};
    char proto[16] = {0};
    char hostname[1024] = {0};
    char hostname_bak[1024] = {0};
    char path[1024] = {0};
    int port = -1;
    int fd = -1;
    int ret;

    char *p;
    char port_str[10] = {0};
    struct addrinfo hints = { 0 };
    struct addrinfo *ai = NULL;
    struct addrinfo *cur_ai = NULL;
    DnsCacheEntry *dns_entry = NULL;
    AppTcpIOControl *pstAppTcpIOCtx = NULL;

    if (pstTcpCtx->open_timeout < 0) {
        pstTcpCtx->open_timeout = 15000000;
    }

    if (pstTcpCtx->fastopen) {
        pstTcpCtx->tcp_connected = 0;
        strcpy(pstTcpCtx->uri, uri);
        return 0;
    }

    url_split(proto, sizeof(proto), NULL, 0, hostname, sizeof(hostname), &port, path, sizeof(path), uri);

    if (strcmp(proto, "tcp")) {
        return -1;
    }

    if (port <= 0 || port >= 65536) {
        return -1;
    }

    p = strchr(uri, '?');
    if (p) {
        if (find_info_tag(buf, sizeof(buf), "listen", p)) {
            char *endptr = NULL;
            pstTcpCtx->listen = strtol(buf, &endptr, 10);
        
            /* assume if no digits were found it is a request to enable it */
            if (buf == endptr)
                pstTcpCtx->listen = 1;
        }
        
        if (find_info_tag(buf, sizeof(buf), "timeout", p)) {
            pstTcpCtx->rw_timeout = strtol(buf, NULL, 10);
            if (pstTcpCtx->rw_timeout >= 0) {
                pstTcpCtx->open_timeout = pstTcpCtx->rw_timeout;
            }
        }
        
        if (find_info_tag(buf, sizeof(buf), "listen_timeout", p)) {
            pstTcpCtx->listen_timeout = strtol(buf, NULL, 10);
        }
    }

    if (pstTcpCtx->rw_timeout >= 0 ) {
        pstUrlCtx->rw_timeout = pstTcpCtx->rw_timeout;
    }

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (pstTcpCtx->listen) {
        hints.ai_flags |= AI_PASSIVE;
    }

    snprintf(port_str, sizeof(port_str), "%d", port);

    /* 域名解析后续研究
    if (pstTcpCtx->dns_cache_timeout > 0) {
        memcpy(hostname_bak, hostname, sizeof(hostname));
        if (pstTcpCtx->dns_cache_clear) {
            remove_dns_cache_entry(hostname);
        } else {
            dns_entry = get_dns_cache_reference(hostname);
        }
    }
    */

    if (!dns_entry) {
#ifdef HAVE_PTHREADS
        ret = tcp_getaddrinfo_nonblock(hostname, port_str, &hints, &ai, pstTcpCtx->addrinfo_timeout, pstTcpCtx->addrinfo_one_by_one);
#else

        if (!hostname[0])
            ret = getaddrinfo(NULL, port_str, &hints, &ai);
        else
            ret = getaddrinfo(hostname, port_str, &hints, &ai);
#endif

        if (ret) {
            return -1;
        }

        cur_ai = ai;
    } else {
        cur_ai = dns_entry->res;
    }

restart:
#if HAVE_STRUCT_SOCKADDR_IN6
        // workaround for IOS9 getaddrinfo in IPv6 only network use hardcode IPv4 address can not resolve port number.
        if (cur_ai->ai_family == AF_INET6){
            struct sockaddr_in6 * sockaddr_v6 = (struct sockaddr_in6 *)cur_ai->ai_addr;
            if (!sockaddr_v6->sin6_port){
                sockaddr_v6->sin6_port = htons(port);
            }
        }
#endif

    fd = ff_socket(cur_ai->ai_family, cur_ai->ai_socktype, cur_ai->ai_protocol);
    if (fd < 0) {
        ret = -1;
        goto fail;
    }

    /* Set the socket's send or receive buffer sizes, if specified.
      If unspecified or setting fails, system default is used. */
    if (pstTcpCtx->recv_buffer_size > 0) {
        setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &pstTcpCtx->recv_buffer_size, sizeof (pstTcpCtx->recv_buffer_size));
    }
    if (pstTcpCtx->send_buffer_size > 0) {
        setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &pstTcpCtx->send_buffer_size, sizeof (pstTcpCtx->send_buffer_size));
    }

    if (pstTcpCtx->listen == 2) {
            // multi-client
            if ((ret = ff_listen(fd, cur_ai->ai_addr, cur_ai->ai_addrlen)) < 0)
                goto fail1;
    } else if (pstTcpCtx->listen == 1) {
            // single client
            if ((ret = ff_listen_bind(fd, cur_ai->ai_addr, cur_ai->ai_addrlen, pstTcpCtx->listen_timeout,  pstUrlCtx)) < 0)
                goto fail1;
            // Socket descriptor already closed here. Safe to overwrite to client one.
            fd = ret;
    } else {
            /*ret = application_on_tcp_will_open(pstTcpCtx->app_ctx);
            if (ret) {
                //av_log(NULL, AV_LOG_WARNING, "terminated by application in AVAPP_CTRL_WILL_TCP_OPEN");
                printf("terminated by application in AVAPP_CTRL_WILL_TCP_OPEN");
                goto fail1;
            }
            */

            ret = ff_listen_connect(fd, cur_ai->ai_addr, cur_ai->ai_addrlen, pstTcpCtx->open_timeout / 1000, pstUrlCtx, !!cur_ai->ai_next);
            if (ret < 0) {
                /*if (application_on_tcp_did_open(pstTcpCtx->app_ctx, ret, fd, &pstAppTcpIOCtx))
                    goto fail1;*/
                    
                if (ret == AVERROR_EXIT)
                    goto fail1;
                else
                    goto fail;
            } /*else {
                ret = application_on_tcp_did_open(pstTcpCtx->app_ctx, 0, fd, &pstAppTcpIOCtx);
                if (ret) {
                    //av_log(NULL, AV_LOG_WARNING, "terminated by application in AVAPP_CTRL_DID_TCP_OPEN");
                    goto fail1;
                } else if (!dns_entry && strcmp(pstAppTcpIOCtx.ip, hostname_bak)) {
                    add_dns_cache_entry(hostname_bak, cur_ai, s->dns_cache_timeout);
                    //av_log(NULL, AV_LOG_INFO, "Add dns cache hostname = %s, ip = %s\n", hostname_bak , control.ip);
                }
            }
            */
    }

    pstUrlCtx->is_streamed = 1;
    pstTcpCtx->fd = fd;

    if (dns_entry) {
        release_dns_cache_reference(hostname_bak, &dns_entry);
    } else {
        freeaddrinfo(ai);
    }
    return 0;

fail:
    if (cur_ai->ai_next) {
        /* Retry with the next sockaddr */
        cur_ai = cur_ai->ai_next;
        if (fd >= 0)
            closesocket(fd);
        ret = 0;
        goto restart;
    }
 fail1:
    if (fd >= 0)
        closesocket(fd);

    if (dns_entry) {
        //av_log(NULL, AV_LOG_ERROR, "Hit dns cache but connect fail hostname = %s, ip = %s\n", hostname , control.ip);
        release_dns_cache_reference(hostname_bak, &dns_entry);
        remove_dns_cache_entry(hostname_bak);
    } else {
        freeaddrinfo(ai);
    }

    return ret;
}

static int tcp_read(URLContext *pstUrlCtx, uint8_t *buf, int size)
{
    TCPContext *pstTcpCtx = pstUrlCtx->priv_data;
    int ret;

    if (!(pstUrlCtx->flags & AVIO_FLAG_NONBLOCK)) {
        ret = network_wait_fd_timeout(pstTcpCtx->fd, 0, pstUrlCtx->rw_timeout, &pstUrlCtx->interrupt_callback);
        if (ret)
            return ret;
    }
    ret = recv(pstTcpCtx->fd, buf, size, 0);
    if (ret > 0)
        application_did_io_tcp_read(pstTcpCtx->app_ctx, (void*)pstUrlCtx, ret);
    return ret < 0 ? ff_neterrno() : ret;
}

static int tcp_write(URLContext *pstUrlCtx, const uint8_t *buf, int size)
{
    TCPContext *pstTcpCtx = pstUrlCtx->priv_data;
    int ret;

    if (!(pstUrlCtx->flags & AVIO_FLAG_NONBLOCK)) {
        ret = network_wait_fd_timeout(pstTcpCtx->fd, 1, pstUrlCtx->rw_timeout, &pstUrlCtx->interrupt_callback);
        if (ret)
            return ret;
    }
    
    ret = send(pstTcpCtx->fd, buf, size, MSG_NOSIGNAL);
    return ret < 0 ? ff_neterrno() : ret;
}


#define OFFSET(x) offsetof(TCPContext, x)
#define D AV_OPT_FLAG_DECODING_PARAM
#define E AV_OPT_FLAG_ENCODING_PARAM

static const AVOption tcp_options[] = {
    {"listem", OFFSET(listen), AV_OPT_TYPE_INT, {.i64 = 0}, 0, 2, .flags = D|E},
    {"timeout", OFFSET(rw_timeout), AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX, .flags = D|E},
    {"connect_timeout", OFFSET(open_timeout), AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX, .flags = D|E},
    {"listen_timeout", OFFSET(listen_timeout), AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX, .flags = D|E},
    {"send_buffer_size", OFFSET(send_buffer_size), AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX, .flags = D|E},
    {"recv_buffer_size", OFFSET(recv_buffer_size), AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX, .flags = D|E},
    {"ijkapplication", OFFSET(app_ctx_intptr), AV_OPT_TYPE_INT64, {.i64 = 0}, INT64_MIN, INT64_MAX, .flags = D},
    {"addrinfo_one_by_one", OFFSET(addrinfo_one_by_one), AV_OPT_TYPE_INT, {.i64 = 0}, 0, 1, .flags = D|E},
    {"addrinfo_timeout", OFFSET(addrinfo_timeout), AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX, .flags = D|E},
    {"dns_cache_timeout", OFFSET(dns_cache_timeout), AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT64_MAX, .flags = D|E},
    {"dns_cache_clear", OFFSET(dns_cache_clear), AV_OPT_TYPE_INT, {.i64 = 0}, -1, INT_MAX, .flags = D|E},
    {"fastopen", OFFSET(fastopen), AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX, .flags = D|E},
    {NULL}
};

static const AVClass tcp_class = {
    .class_name = "tcp",
    .item_name = default_item_name,
    .option = tcp_options,
};

const URLProtocol tcp_protocol = {
    .url_name = "tcp",
    .url_open = tcp_open;
    .url_read = tcp_read;
    .url_write = tcp_write;
    .priv_data_size = sizeof(TCPContext);
    .flags = URL_PROTOCOL_FLAG_NETWORK
    .priv_data_class = &tcp_class;
};










