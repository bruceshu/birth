/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


#include <stdio.h>
#include <netdb.h>
#include <stdarg.h>

#include "libavutil/avstring.h"
#include "libavutil/error.h"
#include "libavutil/mem.h"
#include "libavutil/time.h"
#include "libavutil/common.h"
#include "libavutil/opt.h"

#include "config.h"
#include "url.h"

void url_split(char *proto, int proto_size, 
              char *authorization, int authorization_size, 
              char *hostname, int hostname_size, int *port_ptr, 
              char *path, int path_size, 
              const char *url)
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

#define URL_SCHEME_CHARS                        \
    "abcdefghijklmnopqrstuvwxyz"                \
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"                \
    "0123456789+-."

static const struct URLProtocol *url_find_protocol(const char *filename)
{
    const URLProtocol **protocols;
    char proto_str[128], proto_nested[128], *ptr;
    size_t proto_len = strspn(filename, URL_SCHEME_CHARS);
    int i;

    if (filename[proto_len] != ':' && (strncmp(filename, "subfile,", 8) || !strchr(filename + proto_len + 1, ':')))
        //|| is_dos_path(filename))
        strcpy(proto_str, "file");
    else
        av_strlcpy(proto_str, filename, FFMIN(proto_len + 1, sizeof(proto_str)));

    av_strlcpy(proto_nested, proto_str, sizeof(proto_nested));
    if ((ptr = strchr(proto_nested, '+')))
        *ptr = '\0';

    protocols = url_get_protocols(NULL, NULL);
    if (!protocols)
        return NULL;
    
    for (i = 0; protocols[i]; i++) {
        const URLProtocol *up = protocols[i];
        
        if (!strcmp(proto_str, up->name)) {
            av_freep(&protocols);
            return up;
        }
        if (up->flags & URL_PROTOCOL_FLAG_NESTED_SCHEME && !strcmp(proto_nested, up->name)) {
            av_freep(&protocols);
            return up;
        }
    }
    av_freep(&protocols);

    return NULL;
}

static const char *url_context_to_name(void *ptr)
{
    URLContext *h = (URLContext *)ptr;
    if (h->pstUrlProt)
        return h->pstUrlProt->name;
    else
        return "NULL";
}

static void *url_context_child_next(void *obj, void *prev)
{
    URLContext *h = obj;
    if (!prev && h->pstPrivData && h->pstUrlProt->pstPrivDataClass)
        return h->pstPrivData;
    return NULL;
}

#define OFFSET(x) offsetof(URLContext,x)
#define E AV_OPT_FLAG_ENCODING_PARAM
#define D AV_OPT_FLAG_DECODING_PARAM
static const AVOption url_options[] = {
    {"rw_timeout", OFFSET(rw_timeout), AV_OPT_TYPE_INT64, {.i64 = 0}, 0, INT64_MAX, D | E},
    { NULL }
};

const AVClass url_context_class = {
    .class_name       = "URLContext",
    .item_name        = url_context_to_name,
    .option           = url_options,
    .child_next       = url_context_child_next,
    .child_class_next = url_context_child_class_next,
};

static int url_alloc_for_protocol(URLContext **ppstUrlCtx, const URLProtocol *pstUrlProt, const char *filename, int flags, const AVIOInterruptCB *int_cb)
{
    URLContext *pstUrlCtx;
    int err;

    if ((flags & AVIO_FLAG_READ) && !pstUrlProt->url_read) {
        av_log(NULL, AV_LOG_ERROR, "Impossible to open the '%s' protocol for reading\n", pstUrlProt->name);
        return -1;
    }
    
    if ((flags & AVIO_FLAG_WRITE) && !pstUrlProt->url_write) {
        av_log(NULL, AV_LOG_ERROR, "Impossible to open the '%s' protocol for writing\n", pstUrlProt->name);
        return -1;
    }
    
    pstUrlCtx = av_mallocz(sizeof(URLContext) + strlen(filename) + 1);
    if (!pstUrlCtx) {
        err = -1;
        goto fail;
    }
    
    pstUrlCtx->pstClass = &url_context_class;
    pstUrlCtx->filename = (char *)&pstUrlCtx[1];
    strcpy(pstUrlCtx->filename, filename);
    pstUrlCtx->pstUrlProt      = pstUrlProt;
    pstUrlCtx->flags           = flags;
    pstUrlCtx->is_streamed     = 0; /* default = not streamed */
    pstUrlCtx->max_packet_size = 0; /* default: stream file */
    if (pstUrlProt->priv_data_size) {
        pstUrlCtx->pstPrivData = av_mallocz(pstUrlProt->priv_data_size);
        if (!pstUrlCtx->pstPrivData) {
            err = -1;
            goto fail;
        }
        
        if (pstUrlProt->pstPrivDataClass) {
            int proto_len= strlen(pstUrlProt->name);
            char *start = strchr(pstUrlCtx->filename, ',');
            *(const AVClass **)pstUrlCtx->pstPrivData = pstUrlProt->pstPrivDataClass;
            av_opt_set_defaults(pstUrlCtx->pstPrivData);
            /*if(!strncmp(pstUrlProt->url_name, pstUrlCtx->filename, proto_len) && pstUrlCtx->filename + proto_len == start){
                int ret= 0;
                char *p= start;
                char sep= *++p;
                char *key, *val;
                p++;

                if (strcmp(pstUrlProt->name, "subfile"))
                    ret = -1;

                while(ret >= 0 && (key= strchr(p, sep)) && p<key && (val = strchr(key+1, sep))){
                    *val= *key= 0;
                    if (strcmp(p, "start") && strcmp(p, "end")) {
                        ret = AVERROR_OPTION_NOT_FOUND;
                    } else {
                        //ret= av_opt_set(uc->priv_data, p, key+1, 0);
                    }
                    
                    if (ret == AVERROR_OPTION_NOT_FOUND) {
                        
                        //av_log(uc, AV_LOG_ERROR, "Key '%s' not found.\n", p);
                    }
                    
                    *val= *key= sep;
                    p= val+1;
                }
                
                if(ret<0 || p!=key){
                    //av_log(uc, AV_LOG_ERROR, "Error parsing options string %s\n", start);
                    ff_freep(&pstUrlCtx->priv_data);
                    ff_freep(&pstUrlCtx);
                    err = -1;
                    goto fail;
                }
                
                memmove(start, key+1, strlen(key));
            }*/
        }
    }
    
    if (int_cb)
        pstUrlCtx->interrupt_callback = *int_cb;

    *ppstUrlCtx = pstUrlCtx;
    return 0;
    
fail:
    *ppstUrlCtx = NULL;
    if (pstUrlCtx)
        av_freep(&pstUrlCtx->pstPrivData);
    av_freep(&pstUrlCtx);
    return err;
}

int url_alloc(URLContext **ppstUrlCtx, const char *filename, int flags, const AVIOInterruptCB *int_cb)
{
    const URLProtocol *p = NULL;

    p = url_find_protocol(filename);
    if (p)
       return url_alloc_for_protocol(ppstUrlCtx, p, filename, flags, int_cb);

    *ppstUrlCtx = NULL;
    
    return AVERROR_PROTOCOL_NOT_FOUND;
}

int url_connect(URLContext *pstUrlCtx, AVDictionary **options)
{
    int err;
    //AVDictionary *tmp_opts = NULL;
    //AVDictionaryEntry *e;

    /*
    if (!options)
        options = &tmp_opts;

    
    if (uc->protocol_whitelist && av_match_list(uc->prot->name, uc->protocol_whitelist, ',') <= 0) {
        av_log(uc, AV_LOG_ERROR, "Protocol '%s' not on whitelist '%s'!\n", uc->prot->name, uc->protocol_whitelist);
        return AVERROR(EINVAL);
    }

    if (uc->protocol_blacklist && av_match_list(uc->prot->name, uc->protocol_blacklist, ',') > 0) {
        av_log(uc, AV_LOG_ERROR, "Protocol '%s' on blacklist '%s'!\n", uc->prot->name, uc->protocol_blacklist);
        return AVERROR(EINVAL);
    }

    if (!uc->protocol_whitelist && uc->prot->default_whitelist) {
        av_log(uc, AV_LOG_DEBUG, "Setting default whitelist '%s'\n", uc->prot->default_whitelist);
        uc->protocol_whitelist = av_strdup(uc->prot->default_whitelist);
        if (!uc->protocol_whitelist) {
            return AVERROR(ENOMEM);
        }
    } else if (!uc->protocol_whitelist)
        av_log(uc, AV_LOG_DEBUG, "No default whitelist set\n"); // This should be an error once all declare a default whitelist

    if ((err = av_dict_set(options, "protocol_whitelist", uc->protocol_whitelist, 0)) < 0)
        return err;
    if ((err = av_dict_set(options, "protocol_blacklist", uc->protocol_blacklist, 0)) < 0)
        return err;
        */

    err = pstUrlCtx->pstUrlProt->url_open2 ? pstUrlCtx->pstUrlProt->url_open2(pstUrlCtx, pstUrlCtx->filename, pstUrlCtx->flags, options) 
                                           : pstUrlCtx->pstUrlProt->url_open(pstUrlCtx, pstUrlCtx->filename, pstUrlCtx->flags);

    //av_dict_set(options, "protocol_whitelist", NULL, 0);
    //av_dict_set(options, "protocol_blacklist", NULL, 0);

    if (err)
        return err;
    
    pstUrlCtx->is_connected = 1;
    
    /* We must be careful here as ffurl_seek() could be slow, for example for http */
    if ((pstUrlCtx->flags & AVIO_FLAG_WRITE) || !strcmp(pstUrlCtx->pstUrlProt->name, "file")) {
        if (!pstUrlCtx->is_streamed && url_seek(pstUrlCtx, 0, SEEK_SET) < 0) {
            pstUrlCtx->is_streamed = 1;
        }
    }

    return 0;
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

static int retry_transfer_wrapper_read(URLContext *pstUrlCtx, unsigned char *buf, int size, int size_min, 
    int (*transfer_func)(URLContext *pstUrlCtx, unsigned char *buf, int size))
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

    static int retry_transfer_wrapper_write(URLContext *pstUrlCtx, unsigned char *buf, int size, int size_min, 
        int (*transfer_func)(URLContext *pstUrlCtx, const unsigned char *buf, int size))
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
    return retry_transfer_wrapper_read(pstUrlCtx, buf, size, 1, pstUrlCtx->pstUrlProt->url_read);
}

int url_write(URLContext *pstUrlCtx, const unsigned char *buf, int size)
{
    if (!(pstUrlCtx->flags & AVIO_FLAG_WRITE))
        return AVERROR(EIO);
    
    /* avoid sending too big packets */
    if (pstUrlCtx->max_packet_size && size > pstUrlCtx->max_packet_size)
        return AVERROR(EIO);

    return retry_transfer_wrapper_write(pstUrlCtx, (unsigned char *)buf, size, size, pstUrlCtx->pstUrlProt->url_write);
}

int64_t url_seek(URLContext *pstUrlCtx, int64_t pos, int whence)
{
    int64_t ret;

    if (!pstUrlCtx->pstUrlProt->url_seek)
        return -1;
    
    ret = pstUrlCtx->pstUrlProt->url_seek(pstUrlCtx, pos, whence & ~AVSEEK_FORCE);
    
    return ret;
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

