/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


#include <stdio.h>
#include <netdb.h>
#include <stdarg.h>
#include <string.h>

#include "config.h"

#include "libavutil/avstring.h"
#include "url.h"

void url_split(char *proto, int proto_size, char *authorization, int authorization_size, char *hostname, int hostname_size, int *port_ptr, char *path, int path_size, const char *url)
{
    const char *p, *ls, *ls2, *at, *at2, *col, *brk;

    if (port_ptr)
        *port_ptr = -1;
    if (proto_size > 0)
        proto[0] = 0;
    if (authorization_size > 0)
        authorization[0] = 0;
    if (hostname_size > 0)
        hostname[0] = 0;
    if (path_size > 0)
        path[0] = 0;

    /* parse protocol */
    if ((p = strchr(url, ':'))) {
        av_strlcpy(proto, url, FFMIN(proto_size, p + 1 - url));
        p++; /* skip ':' */
        if (*p == '/')
            p++;
        if (*p == '/')
            p++;
    } else {
        /* no protocol means plain filename */
        av_strlcpy(path, url, path_size);
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
        av_strlcpy(path, ls, path_size);
    else
        ls = &p[strlen(p)];  // XXX

    /* the rest is hostname, use that to parse auth/port */
    if (ls != p) {
        /* authorization (user[:pass]@hostname) */
        at2 = p;
        while ((at = strchr(p, '@')) && at < ls) {
            av_strlcpy(authorization, at2, FFMIN(authorization_size, at + 1 - at2));
            p = at + 1; /* skip '@' */
        }

        if (*p == '[' && (brk = strchr(p, ']')) && brk < ls) {
            /* [host]:port */
            av_strlcpy(hostname, p + 1, FFMIN(hostname_size, brk - p));
            if (brk[1] == ':' && port_ptr)
                *port_ptr = atoi(brk + 2);
        } else if ((col = strchr(p, ':')) && col < ls) {
            av_strlcpy(hostname, p, FFMIN(col + 1 - p, hostname_size));
            if (port_ptr)
                *port_ptr = atoi(col + 1);
        } else {
            av_strlcpy(hostname, p, FFMIN(ls + 1 - p, hostname_size));
        }
    }
}

int url_join(char *str, int size, const char *proto, const char *authorization, const char *hostname, int port, const char *fmt, ...)
{
#if CONFIG_NETWORK
    struct addrinfo hints = { 0 }, *ai;
#endif

    str[0] = '\0';
    if (proto)
        av_strlcatf(str, size, "%s://", proto);
    
    if (authorization && authorization[0])
        av_strlcatf(str, size, "%s@", authorization);
    
#if CONFIG_NETWORK && defined(AF_INET6)
    /* Determine if hostname is a numerical IPv6 address, properly escape it within [] in that case. */
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
        av_strlcatf(str, size, ":%d", port);
    
    if (fmt) {
        va_list vl;
        size_t len = strlen(str);

        va_start(vl, fmt);
        vsnprintf(str + len, size > len ? size - len : 0, fmt, vl);
        va_end(vl);
    }
    return strlen(str);
}

void make_absolute_url(char *buf, int size, const char *base, const char *rel)
{
    char *sep, *path_query;
    /* Absolute path, relative to the current server */
    if (base && strstr(base, "://") && rel[0] == '/') {
        if (base != buf)
            av_strlcpy(buf, base, size);
        sep = strstr(buf, "://");
        if (sep) {
            /* Take scheme from base url */
            if (rel[1] == '/') {
                sep[1] = '\0';
            } else {
                /* Take scheme and host from base url */
                sep += 3;
                sep = strchr(sep, '/');
                if (sep)
                    *sep = '\0';
            }
        }
        av_strlcat(buf, rel, size);
        return;
    }
    /* If rel actually is an absolute url, just copy it */
    if (!base || strstr(rel, "://") || rel[0] == '/') {
        av_strlcpy(buf, rel, size);
        return;
    }
    if (base != buf)
        av_strlcpy(buf, base, size);

    /* Strip off any query string from base */
    path_query = strchr(buf, '?');
    if (path_query)
        *path_query = '\0';

    /* Is relative path just a new query part? */
    if (rel[0] == '?') {
        av_strlcat(buf, rel, size);
        return;
    }

    /* Remove the file name from the base url */
    sep = strrchr(buf, '/');
    if (sep)
        sep[1] = '\0';
    else
        buf[0] = '\0';
    while (av_strstart(rel, "../", NULL) && sep) {
        /* Remove the path delimiter at the end */
        sep[0] = '\0';
        sep = strrchr(buf, '/');
        /* If the next directory name to pop off is "..", break here */
        if (!strcmp(sep ? &sep[1] : buf, "..")) {
            /* Readd the slash we just removed */
            av_strlcat(buf, "/", size);
            break;
        }
        /* Cut off the directory name */
        if (sep)
            sep[1] = '\0';
        else
            buf[0] = '\0';
        rel += 3;
    }
    av_strlcat(buf, rel, size);
}

int url_open_whitelist(URLContext **ppstUrlCtx, const char *filename, int flags, AVDictionary **options, URLContext *parent)
{
    AVDictionary *tmp_opts = NULL;
    AVDictionaryEntry *e;
    int ret = url_alloc(ppstUrlCtx, filename, flags, NULL);
    if (ret < 0)
        return ret;
    
    if (parent) {
        av_opt_copy(*ppstUrlCtx, parent);
    }
    
    /*
    if (options && (ret = av_opt_set_dict(*puc, options)) < 0)
        goto fail;
    
    if (options && (*puc)->prot->priv_data_class && (ret = av_opt_set_dict((*puc)->priv_data, options)) < 0)
        goto fail;

    if (!options)
        options = &tmp_opts;
        

    if ((ret = av_opt_set_dict(*puc, options)) < 0)
        goto fail;
        */

    ret = url_connect(*ppstUrlCtx, options);

    if (!ret)
        return 0;
fail:
    url_close(*ppstUrlCtx);
    *ppstUrlCtx = NULL;
    return ret;
}

int url_handshake(URLContext *c)
{
    int ret;
    if (c->pstUrlProt->url_handshake) {
        ret = c->pstUrlProt->url_handshake(c);
        if (ret)
            return ret;
    }
    
    c->is_connected = 1;
    return 0;
}

static int retry_transfer_wrapper(URLContext *pstUrlCtx, uint8_t *buf, int size, int size_min, 
    int (*transfer_func)(URLContext *pstUrlCtx, uint8_t *buf, int size))
{
    int ret, len;
    int fast_retries = 5;
    int64_t wait_since = 0;

    len = 0;
    while (len < size_min) {
        /*if (ff_check_interrupt(&h->interrupt_callback))
            return AVERROR_EXIT;*/
            
        ret = transfer_func(pstUrlCtx, buf + len, size - len);
        if (ret == AVERROR(EINTR))
            continue;
        
        if (pstUrlCtx->flags & AVIO_FLAG_NONBLOCK)
            return ret;
        
        if (ret == AVERROR(EAGAIN)) {
            ret = 0;
            if (fast_retries) {
                fast_retries--;
            } else {
                if (pstUrlCtx->rw_timeout) {
                    if (!wait_since)
                        wait_since = av_gettime_relative();
                    else if (av_gettime_relative() > wait_since + pstUrlCtx->rw_timeout)
                        return AVERROR(EIO);
                }
                av_usleep(1000);
            }
        } else if (ret < 1)
            return (ret < 0 && ret != AVERROR_EOF) ? ret : len;
            
        if (ret) {
            fast_retries = FFMAX(fast_retries, 2);
            wait_since = 0;
        }
        
        len += ret;
    }
    
    return len;
}

int url_read(URLContext *pstUrlCtx, unsigned char *buf, int size)
{
    if (!(pstUrlCtx->flags & AVIO_FLAG_READ))
        return -1;
    return retry_transfer_wrapper(pstUrlCtx, buf, size, 1, pstUrlCtx->pstUrlProt->url_read);
}

int url_write(URLContext *pstUrlCtx, const unsigned char *buf, int size)
{
    if (!(pstUrlCtx->flags & AVIO_FLAG_WRITE))
        return AVERROR(EIO);
    
    /* avoid sending too big packets */
    if (pstUrlCtx->max_packet_size && size > pstUrlCtx->max_packet_size)
        return AVERROR(EIO);

    return retry_transfer_wrapper(pstUrlCtx, (unsigned char *)buf, size, size, pstUrlCtx->pstUrlProt->url_write);
}

int url_get_file_handle(URLContext *pstUrlCtx)
{
    if (!pstUrlCtx || !pstUrlCtx->pstUrlProt || !pstUrlCtx->pstUrlProt->url_get_file_handle)
        return AVERROR(ENOSYS);
    
    return pstUrlCtx->pstUrlProt->url_get_file_handle(pstUrlCtx);
}

int url_get_multi_file_handle(URLContext *h, int **handles, int *numhandles)
{
    if (!h || !h->pstUrlProt)
        return AVERROR(ENOSYS);
    
    if (!h->pstUrlProt->url_get_multi_file_handle) {
        if (!h->pstUrlProt->url_get_file_handle)
            return AVERROR(ENOSYS);
        
        *handles = av_malloc(sizeof(**handles));
        if (!*handles)
            return AVERROR(ENOMEM);
        
        *numhandles = 1;
        *handles[0] = h->pstUrlProt->url_get_file_handle(h);
        
        return 0;
    }
    
    return h->pstUrlProt->url_get_multi_file_handle(h, handles, numhandles);
}

int url_get_short_seek(URLContext *pstUrlCtx)
{
    if (!pstUrlCtx || !pstUrlCtx->pstUrlProt || !pstUrlCtx->pstUrlProt->url_get_short_seek)
        return AVERROR(ENOSYS);
    
    return pstUrlCtx->pstUrlProt->url_get_short_seek(pstUrlCtx);
}

int url_closep(URLContext **ppstUrlCtx)
{
    URLContext *pstUrlCtx = *ppstUrlCtx;
    int ret = 0;
    if (!pstUrlCtx)
        return 0;     /* can happen when ffurl_open fails */

    if (pstUrlCtx->is_connected && pstUrlCtx->pstUrlProt->url_close)
        ret = pstUrlCtx->pstUrlProt->url_close(pstUrlCtx);

    if (pstUrlCtx->pstUrlProt->priv_data_size) {
        if (pstUrlCtx->pstUrlProt->pstPrivDataClass)
            av_opt_free(pstUrlCtx->pstPrivData);
        av_freep(&pstUrlCtx->pstPrivData);
    }
    
    av_opt_free(pstUrlCtx);
    av_freep(ppstUrlCtx);
    
    return ret;
}

int url_close(URLContext *pstUrlCtx)
{
    return url_closep(&pstUrlCtx);
}

