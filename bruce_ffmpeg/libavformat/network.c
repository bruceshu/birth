/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-09
 * Description:
 
*********************************/




int ff_network_init(void)
{
#if HAVE_WINSOCK2_H
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(1,1), &wsaData))
        return 0;
#endif
    return 1;
}

int ff_socket(int af, int type, int proto)
{
    int fd;

#ifdef SOCK_CLOEXEC
    fd = socket(af, type | SOCK_CLOEXEC, proto);
    if (fd == -1 && errno == EINVAL)
#endif
    {
        fd = socket(af, type, proto);
#if HAVE_FCNTL
        if (fd != -1) {
            if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1)
                av_log(NULL, AV_LOG_DEBUG, "Failed to set close on exec\n");
        }
#endif
    }
#ifdef SO_NOSIGPIPE
    if (fd != -1) {
        if (setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &(int){1}, sizeof(int))) {
             av_log(NULL, AV_LOG_WARNING, "setsockopt(SO_NOSIGPIPE) failed\n");
        }
    }
#endif
    return fd;
}

void ff_log_net_error(void *ctx, int level, const char* prefix)
{
    char errbuf[100];
    av_strerror(ff_neterrno(), errbuf, sizeof(errbuf));
    av_log(ctx, level, "%s: %s\n", prefix, errbuf);
}

int ff_listen(int fd, const struct sockaddr *addr, socklen_t addrlen)
{
    int ret;
    int reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))) {
        av_log(NULL, AV_LOG_WARNING, "setsockopt(SO_REUSEADDR) failed\n");
    }
    
    ret = bind(fd, addr, addrlen);
    if (ret) {
        return ff_neterrno();
    }

    ret = listen(fd, 1);
    if (ret) {
        return ff_neterrno();
    }
    
    return ret;
}

static int ff_poll_interrupt(struct pollfd *p, nfds_t nfds, int timeout, AVIOInterruptCB *cb)
{
    int runs = timeout / POLLING_TIME;
    int ret = 0;

    do {
        if (ff_check_interrupt(cb))
            return AVERROR_EXIT;
        
        ret = poll(p, nfds, POLLING_TIME);
        if (ret != 0) {
            if (ret < 0)
                ret = ff_neterrno();
            if (ret == AVERROR(EINTR))
                continue;
            break;
        }
    } while (timeout <= 0 || runs-- > 0);

    if (!ret)
        return AVERROR(ETIMEDOUT);
    
    return ret;
}

int ff_accept(int fd, int timeout, URLContext *h)
{
    int ret;
    struct pollfd lp = { fd, POLLIN, 0 };

    ret = ff_poll_interrupt(&lp, 1, timeout, &h->interrupt_callback);
    if (ret < 0)
        return ret;

    ret = accept(fd, NULL, NULL);
    if (ret < 0)
        return ff_neterrno();
    
    if (ff_socket_nonblock(ret, 1) < 0)
        av_log(NULL, AV_LOG_DEBUG, "ff_socket_nonblock failed\n");

    return ret;
}


int ff_listen_bind(int fd, const struct sockaddr *addr, socklen_t addrlen, int timeout, URLContext *h)
{
    int ret;
    if ((ret = ff_listen(fd, addr, addrlen)) < 0) {
        return ret;
    }
    
    if ((ret = ff_accept(fd, timeout, h)) < 0) {
        return ret;
    }
    
    closesocket(fd);
    return ret;
}

void ff_network_close(void)
{
#if HAVE_WINSOCK2_H
    WSACleanup();
#endif
}

int ff_network_wait_fd(int fd, int write)
{
    int ev = write ? POLLOUT : POLLIN;
    struct pollfd p = { .fd = fd, .events = ev, .revents = 0 };
    int ret;
    ret = poll(&p, 1, POLLING_TIME);
    return ret < 0 ? ff_neterrno() : p.revents & (ev | POLLERR | POLLHUP) ? 0 : AVERROR(EAGAIN);
}

int ff_network_wait_fd_timeout(int fd, int write, int64_t timeout, AVIOInterruptCB *int_cb)
{
    int ret;
    int64_t wait_start = 0;

    while (1) {
        if (ff_check_interrupt(int_cb))
            return AVERROR_EXIT;
        
        ret = ff_network_wait_fd(fd, write);
        if (ret != AVERROR(EAGAIN))
            return ret;
        
        if (timeout > 0) {
            if (!wait_start) {
                wait_start = av_gettime_relative();
            }
            else if (av_gettime_relative() - wait_start > timeout) {
                return AVERROR(ETIMEDOUT);
            }
        }
    }
}


int ff_connect_parallel(struct addrinfo *addrs, int timeout_ms_per_address,
                        int parallel, URLContext *h, int *fd,
                        void (*customize_fd)(void *, int), void *customize_ctx)
{
    struct ConnectionAttempt attempts[3];
    struct pollfd pfd[3];
    int nb_attempts = 0, i, j;
    int64_t next_attempt_us = av_gettime_relative(), next_deadline_us;
    int last_err = AVERROR(EIO);
    socklen_t optlen;
    char errbuf[100], hostbuf[100], portbuf[20];

    if (parallel > FF_ARRAY_ELEMS(attempts))
        parallel = FF_ARRAY_ELEMS(attempts);

    print_address_list(h, addrs, "Original list of addresses");
    // This mutates the list, but the head of the list is still the same
    // element, so the caller, who owns the list, doesn't need to get
    // an updated pointer.
    interleave_addrinfo(addrs);
    print_address_list(h, addrs, "Interleaved list of addresses");

    while (nb_attempts > 0 || addrs) {
        // Start a new connection attempt, if possible.
        if (nb_attempts < parallel && addrs) {
            getnameinfo(addrs->ai_addr, addrs->ai_addrlen,
                        hostbuf, sizeof(hostbuf), portbuf, sizeof(portbuf),
                        NI_NUMERICHOST | NI_NUMERICSERV);
            
            av_log(h, AV_LOG_VERBOSE, "Starting connection attempt to %s port %s\n",
                                      hostbuf, portbuf);
            last_err = start_connect_attempt(&attempts[nb_attempts], &addrs,
                                             timeout_ms_per_address, h,
                                             customize_fd, customize_ctx);
            if (last_err < 0) {
                av_strerror(last_err, errbuf, sizeof(errbuf));
                av_log(h, AV_LOG_VERBOSE, "Connected attempt failed: %s\n",
                                          errbuf);
                continue;
            }
            if (last_err > 0) {
                for (i = 0; i < nb_attempts; i++)
                    closesocket(attempts[i].fd);
                *fd = attempts[nb_attempts].fd;
                return 0;
            }
            pfd[nb_attempts].fd = attempts[nb_attempts].fd;
            pfd[nb_attempts].events = POLLOUT;
            next_attempt_us = av_gettime_relative() + NEXT_ATTEMPT_DELAY_MS * 1000;
            nb_attempts++;
        }

        av_assert0(nb_attempts > 0);
        // The connection attempts are sorted from oldest to newest, so the
        // first one will have the earliest deadline.
        next_deadline_us = attempts[0].deadline_us;
        // If we can start another attempt in parallel, wait until that time.
        if (nb_attempts < parallel && addrs)
            next_deadline_us = FFMIN(next_deadline_us, next_attempt_us);
        last_err = ff_poll_interrupt(pfd, nb_attempts,
                                     (next_deadline_us - av_gettime_relative())/1000,
                                     &h->interrupt_callback);
        if (last_err < 0 && last_err != AVERROR(ETIMEDOUT))
            break;

        // Check the status from the poll output.
        for (i = 0; i < nb_attempts; i++) {
            last_err = 0;
            if (pfd[i].revents) {
                // Some sort of action for this socket, check its status (either
                // a successful connection or an error).
                optlen = sizeof(last_err);
                if (getsockopt(attempts[i].fd, SOL_SOCKET, SO_ERROR, &last_err, &optlen))
                    last_err = ff_neterrno();
                else if (last_err != 0)
                    last_err = AVERROR(last_err);
                if (last_err == 0) {
                    // Everything is ok, we seem to have a successful
                    // connection. Close other sockets and return this one.
                    for (j = 0; j < nb_attempts; j++)
                        if (j != i)
                            closesocket(attempts[j].fd);
                    *fd = attempts[i].fd;
                    getnameinfo(attempts[i].addr->ai_addr, attempts[i].addr->ai_addrlen,
                                hostbuf, sizeof(hostbuf), portbuf, sizeof(portbuf),
                                NI_NUMERICHOST | NI_NUMERICSERV);
                    av_log(h, AV_LOG_VERBOSE, "Successfully connected to %s port %s\n",
                                              hostbuf, portbuf);
                    return 0;
                }
            }
            if (attempts[i].deadline_us < av_gettime_relative() && !last_err)
                last_err = AVERROR(ETIMEDOUT);
            if (!last_err)
                continue;
            // Error (or timeout) for this socket; close the socket and remove
            // it from the attempts/pfd arrays, to let a new attempt start
            // directly.
            getnameinfo(attempts[i].addr->ai_addr, attempts[i].addr->ai_addrlen,
                        hostbuf, sizeof(hostbuf), portbuf, sizeof(portbuf),
                        NI_NUMERICHOST | NI_NUMERICSERV);
            av_strerror(last_err, errbuf, sizeof(errbuf));
            av_log(h, AV_LOG_VERBOSE, "Connection attempt to %s port %s "
                                      "failed: %s\n", hostbuf, portbuf, errbuf);
            closesocket(attempts[i].fd);
            memmove(&attempts[i], &attempts[i + 1],
                    (nb_attempts - i - 1) * sizeof(*attempts));
            memmove(&pfd[i], &pfd[i + 1],
                    (nb_attempts - i - 1) * sizeof(*pfd));
            i--;
            nb_attempts--;
        }
    }
    for (i = 0; i < nb_attempts; i++)
        closesocket(attempts[i].fd);
    if (last_err >= 0)
        last_err = AVERROR(ECONNREFUSED);
    if (last_err != AVERROR_EXIT) {
        av_strerror(last_err, errbuf, sizeof(errbuf));
        av_log(h, AV_LOG_ERROR, "Connection to %s failed: %s\n",
               h->filename, errbuf);
    }
    return last_err;
}

