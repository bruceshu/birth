
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
                //av_log(NULL, AV_LOG_DEBUG, "Failed to set close on exec\n");
                printf("Failed to set close on exec\n");
        }
#endif
    }
#ifdef SO_NOSIGPIPE
    if (fd != -1)
        setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &(int){1}, sizeof(int));
#endif
    return fd;
}


int ff_listen(int fd, const struct sockaddr *addr, socklen_t addrlen)
{
    int ret;
    int reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))) {
        av_log(NULL, AV_LOG_WARNING, "setsockopt(SO_REUSEADDR) failed\n");
    }
    ret = bind(fd, addr, addrlen);
    if (ret)
        return -1;

    ret = listen(fd, 1);
    if (ret)
        return -1;
    return ret;
}

static int ff_poll_interrupt(struct pollfd *p, nfds_t nfds, int timeout)
{
    int runs = timeout / POLLING_TIME;
    int ret = 0;

    do {
        /*if (ff_check_interrupt(cb))
            return AVERROR_EXIT;*/
            
        ret = poll(p, nfds, POLLING_TIME);
        if (ret != 0)
            break;
    } while (timeout <= 0 || runs-- > 0);

    if (!ret)
        return -1;
    if (ret < 0)
        return -1;
    return ret;
}


int ff_accept(int fd, int timeout, URLContext *pstUrlCtx)
{
    int ret;
    struct pollfd lp = { fd, POLLIN, 0 };

    ret = ff_poll_interrupt(&lp, 1, timeout);
    if (ret < 0)
        return ret;

    ret = accept(fd, NULL, NULL);
    if (ret < 0)
        return -1;
    if (ff_socket_nonblock(ret, 1) < 0)
        //av_log(NULL, AV_LOG_DEBUG, "ff_socket_nonblock failed\n");
        printf("ff_socket_nonblock failed\n");

    return ret;
}


int ff_listen_bind(int fd, const struct sockaddr *addr, socklen_t addrlen, int timeout, URLContext *pstUrlCtx)
{
    int ret;
    if ((ret = ff_listen(fd, addr, addrlen)) < 0)
        return ret;
    if ((ret = ff_accept(fd, timeout, pstUrlCtx)) < 0)
        return ret;
    close(fd);
    return ret;
}

int ff_listen_connect(int fd, const struct sockaddr *addr, socklen_t addrlen, int timeout, URLContext *pstUrlCtx, int will_try_next)
{
    struct pollfd p = {fd, POLLOUT, 0};
    int ret;
    int optlen;

    if (ff_socket_nonblock(fd, 1) < 0)
        //av_log(NULL, AV_LOG_DEBUG, "ff_socket_nonblock failed\n");
        printf("ff_socket_nonblock failed\n");

    while ((ret = connect(fd, addr, addrlen))) {
        ret = ff_neterrno();
        switch (ret) {
        case AVERROR(EINTR):
            /*if (ff_check_interrupt(&pstUrlCtx->interrupt_callback))
                return AVERROR_EXIT;*/
            continue;
        case AVERROR(EINPROGRESS):
        case AVERROR(EAGAIN):
            ret = ff_poll_interrupt(&p, 1, timeout, &pstUrlCtx->interrupt_callback);
            if (ret < 0)
                return ret;
            optlen = sizeof(ret);
            if (getsockopt (fd, SOL_SOCKET, SO_ERROR, &ret, &optlen))
                ret = AVUNERROR(ff_neterrno());
            if (ret != 0) {
                char errbuf[100];
                ret = AVERROR(ret);
                ff_strerror(ret, errbuf, sizeof(errbuf));
                if (will_try_next)
                    //av_log(h, AV_LOG_WARNING, "Connection to %s failed (%s), trying next address\n", pstUrlCtx->filename, errbuf);
                    printf("Connection to %s failed (%s), trying next address\n", pstUrlCtx->filename, errbuf);
                else
                    //av_log(h, AV_LOG_ERROR, "Connection to %s failed: %s\n", pstUrlCtx->filename, errbuf);
                    printf("Connection to %s failed: %s\n", pstUrlCtx->filename, errbuf);
            }
        default:
            return ret;
        }
    }
    return ret;
}

static int match_host_pattern(const char *pattern, const char *hostname)
{
    int len_p, len_h;
    
    if (!strcmp(pattern, "*"))
        return 1;
    
    // Skip a possible *. at the start of the pattern
    if (pattern[0] == '*')
        pattern++;
    if (pattern[0] == '.')
        pattern++;
    
    len_p = strlen(pattern);
    len_h = strlen(hostname);
    if (len_p > len_h)
        return 0;
    
    // Simply check if the end of hostname is equal to 'pattern'
    if (!strcmp(pattern, &hostname[len_h - len_p])) {
        if (len_h == len_p)
            return 1; // Exact match
            
        if (hostname[len_h - len_p - 1] == '.')
            return 1; // The matched substring is a domain and not just a substring of a domain
    }
    return 0;
}

int ff_http_match_no_proxy(const char *no_proxy, const char *hostname)
{
    char *buf = NULL;
    char *start = NULL;
    int ret = 0;
    
    if (!no_proxy || !hostname)
        return 0;

    buf = strdup(no_proxy);
    if (!buf)
        return 0;
    
    start = buf;
    while (start) {
        char *sep, *next = NULL;
        start += strspn(start, " ,");
        sep = start + strcspn(start, " ,");
        if (*sep) {
            next = sep + 1;
            *sep = '\0';
        }
        if (match_host_pattern(start, hostname)) {
            ret = 1;
            break;
        }
        start = next;
    }
    
    ff_free(buf);
    return ret;
}

int network_wait_fd(int fd, int write)
{
    int ev = write ? POLLOUT : POLLIN;
    struct pollfd p = { .fd = fd, .events = ev, .revents = 0 };
    int ret;
    ret = poll(&p, 1, POLLING_TIME);
    return ret < 0 ? ff_neterrno() : p.revents & (ev | POLLERR | POLLHUP) ? 0 : AVERROR(EAGAIN);
}


int network_wait_fd_timeout(int fd, int write, int64_t timeout, AVIOInterruptCB *int_cb)
{
    int ret;
    int64_t wait_start = 0;

    while (1) {
        /*if (ff_check_interrupt(int_cb))
            return AVERROR_EXIT;
            */
            
        ret = network_wait_fd(fd, write);
        if (ret != AVERROR(EAGAIN))
            return ret;
        
        if (timeout > 0) {
            if (!wait_start)
                wait_start = av_gettime_relative();
            else if (av_gettime_relative() - wait_start > timeout)
                return AVERROR(ETIMEDOUT);
        }
    }
}

