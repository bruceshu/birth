


#define CONFIG_NETWORK 1

void url_split(char *proto, int proto_size, char *auth, int auth_size, char *hostname, int hostname_size, int *port, char *path, int path_size, const char *url)
{
    const char *p;
    char *ls, *ls2;
    char *at, *at2, *col, *brk;

    if (port)
        *port = -1;
    
    /* parse protocol */
    if ((p = strchr(url, ':'))) {
        strlcpy(proto, url, FFMIN(proto_size, p - url + 1));
        p++;
        if (*p == '/')
            p++;
        if (*p == '/')
            p++;
    } else {
        /* no protocol means plain filename */
        strlcpy(path, url, FFMIN(path_size, strlen(url) + 1));
        return;
    }

    /* separate path from hostname */
    ls = strchr(p, '/');
    ls2 = strchr(p, '?');
    if (!ls)
        ls = ls2;
    else if (ls && ls2)
        ls = FFMIN(ls, ls2);
    
    if (ls)
        strlcpy(path, ls, FFMIN(path_size, strlen(ls) + 1));
    else
        ls = &p[strlen(p)];  // XXX

    /* the rest is hostname, use that to parse auth/port */
    if (ls != p) {
        /* authorization (user[:pass]@hostname) */
        at2 = p;
        while ((at = strchr(p, '@')) && at < ls) {
            strlcpy(auth, at2, FFMIN(auth_size, at - at2 + 1));
            p = at + 1;
        }

        if (*p == '[' && (brk = strchr(p, ']')) && brk < ls) {
            /* [host]:port */
            strlcpy(hostname, p + 1,  FFMIN(hostname_size, brk - p));
            if (brk[1] == ':' && port)
                *port = atoi(brk + 2);
        } else if ((col = strchr(p, ':')) && col < ls) {
            strlcpy(hostname, p, FFMIN(col + 1 - p, hostname_size));
            if (port)
                *port = atoi(col + 1);
        } else
            strlcpy(hostname, p, FFMIN(hostname_size, ls - p + 1));
    }
}


int url_join(char *str, int size, const char *proto, const char *auth, const char *hostname, int port, const char *fmt, ...)
{
#if CONFIG_NETWORK
    struct addrinfo hints = { 0 };
    struct addrinfo *ai;
#endif

    str[0] = '\0';
    if (proto)
        av_strlcat(str, size, "%s://", proto);
    
    if (auth && auth[0])
        av_strlcat(str, size, "%s@", auth);
    
#if CONFIG_NETWORK && defined(AF_INET6) && 0
    hints.ai_flags = AI_NUMERICHOST;
    if (!getaddrinfo(hostname, NULL, &hints, &ai)) {
        if (ai->ai_family == AF_INET6) {
            av_strlcat(str, "[", size);
            av_strlcat(str, hostname, size);
            av_strlcat(str, "]", size);
        } else {
            av_strlcat(str, hostname, size);
        }
        freeaddrinfo(ai);
    } else
#endif
        /* Not an IPv6 address, just output the plain string. */
        av_strlcat(str, hostname, size);

    if (port >= 0)
        av_strlcat(str, size, ":%d", port);
    
    if (fmt) {
        va_list vl;
        size_t len = strlen(str);

        va_start(vl, fmt);
        vsnprintf(str + len, size > len ? size - len : 0, fmt, vl);
        va_end(vl);
    }
    
    return strlen(str);
}

                

