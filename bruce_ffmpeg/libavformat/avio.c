/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


#include <stddef.h>

#include "url.h"
#include "avio.h"


static const char *urlcontext_to_name(void *ptr)
{
    URLContext *pstUrlCtx = (URLContext *)ptr;
    if (pstUrlCtx->pstUrlProt)
        return pstUrlCtx->pstUrlProt->url_name;
    else
        return "NULL";
}

static void *urlcontext_child_next(void *obj, void *prev)
{
    URLContext *pstUrlCtx = obj;
    if (!prev && pstUrlCtx->pstPrivData && pstUrlCtx->pstUrlProt->pstPrivDataClass)
        return pstUrlCtx->pstPrivData;
    return NULL;
}

#if 0 //编译测试暂时屏蔽


int url_handshake(URLContext *pstUrlCtx)
{
    int ret;
    if (pstUrlCtx->pstProt->url_handshake) {
        ret = pstUrlCtx->pstProt->url_handshake(pstUrlCtx);
        if (ret)
            return ret;
    }
    pstUrlCtx->is_connected = 1;
    
    return 0;
}




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

    protocols = ffurl_get_protocols(NULL, NULL);
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
        return -1;//AVERROR(EIO);
    
    /* avoid sending too big packets */
    if (pstUrlCtx->max_packet_size && size > pstUrlCtx->max_packet_size)
        return -1;//AVERROR(EIO);

    return retry_transfer_wrapper(pstUrlCtx, (unsigned char *)buf, size, size,
                                  (int (*)(struct URLContext *, uint8_t *, int))pstUrlCtx->pstUrlProt->url_write);
}

int ffurl_write(URLContext *h, const unsigned char *buf, int size)
{
    if (!(h->flags & AVIO_FLAG_WRITE))
        return AVERROR(EIO);
    
    /* avoid sending too big packets */
    if (h->max_packet_size && size > h->max_packet_size)
        return AVERROR(EIO);

    return retry_transfer_wrapper(h, (unsigned char *)buf, size, size, (int (*)(struct URLContext *, uint8_t *, int))h->prot->url_write);
}

int ffurl_get_file_handle(URLContext *h)
{
    if (!h || !h->pstUrlProt || !h->pstUrlProt->url_get_file_handle)
        return -1;
    return h->pstUrlProt->url_get_file_handle(h);
}

int ffurl_read_complete(URLContext *pstUrlCtx, unsigned char *buf, int size)
{
    if (!(pstUrlCtx->flags & AVIO_FLAG_READ))
        return AVERROR(EIO);
    return retry_transfer_wrapper(pstUrlCtx, buf, size, size, pstUrlCtx->pstUrlProt->url_read);
}

int ffurl_alloc(URLContext **puc, const char *filename, int flags, const AVIOInterruptCB *int_cb)
{
    const URLProtocol *p = NULL;

    p = url_find_protocol(filename);
    if (p)
       return url_alloc_for_protocol(puc, p, filename, flags, int_cb);

    *puc = NULL;
    if (av_strstart(filename, "https:", NULL))
        av_log(NULL, AV_LOG_WARNING, "https protocol not found, recompile FFmpeg with "
                                     "openssl, gnutls "
                                     "or securetransport enabled.\n");
    return AVERROR_PROTOCOL_NOT_FOUND;
}

int ffurl_connect(URLContext *uc, AVDictionary **options)
{
    int err;
    AVDictionary *tmp_opts = NULL;
    AVDictionaryEntry *e;

    if (!options)
        options = &tmp_opts;

    // Check that URLContext was initialized correctly and lists are matching if set
    av_assert0(!(e=av_dict_get(*options, "protocol_whitelist", NULL, 0)) ||
               (uc->protocol_whitelist && !strcmp(uc->protocol_whitelist, e->value)));
    av_assert0(!(e=av_dict_get(*options, "protocol_blacklist", NULL, 0)) ||
               (uc->protocol_blacklist && !strcmp(uc->protocol_blacklist, e->value)));

    if (uc->protocol_whitelist && av_match_list(uc->pstUrlProt->url_name, uc->protocol_whitelist, ',') <= 0) {
        av_log(uc, AV_LOG_ERROR, "Protocol '%s' not on whitelist '%s'!\n", uc->pstUrlProt->url_name, uc->protocol_whitelist);
        return AVERROR(EINVAL);
    }

    if (uc->protocol_blacklist && av_match_list(uc->pstUrlProt->url_name, uc->protocol_blacklist, ',') > 0) {
        av_log(uc, AV_LOG_ERROR, "Protocol '%s' on blacklist '%s'!\n", uc->pstUrlProt->url_name, uc->protocol_blacklist);
        return AVERROR(EINVAL);
    }

    if (!uc->protocol_whitelist && uc->pstUrlProt->default_whitelist) {
        av_log(uc, AV_LOG_DEBUG, "Setting default whitelist '%s'\n", uc->pstUrlProt->default_whitelist);
        uc->protocol_whitelist = av_strdup(uc->pstUrlProt->default_whitelist);
        if (!uc->protocol_whitelist) {
            return AVERROR(ENOMEM);
        }
    } else if (!uc->protocol_whitelist)
        av_log(uc, AV_LOG_DEBUG, "No default whitelist set\n"); // This should be an error once all declare a default whitelist

    if ((err = av_dict_set(options, "protocol_whitelist", uc->protocol_whitelist, 0)) < 0)
        return err;
    if ((err = av_dict_set(options, "protocol_blacklist", uc->protocol_blacklist, 0)) < 0)
        return err;

    err =
        uc->pstUrlProt->url_open2 ? uc->pstUrlProt->url_open2(uc, uc->filename, uc->flags, options) :
        uc->pstUrlProt->url_open(uc, uc->filename, uc->flags);

    av_dict_set(options, "protocol_whitelist", NULL, 0);
    av_dict_set(options, "protocol_blacklist", NULL, 0);

    if (err)
        return err;
    uc->is_connected = 1;
    /* We must be careful here as ffurl_seek() could be slow,
     * for example for http */
    if ((uc->flags & AVIO_FLAG_WRITE) || !strcmp(uc->prot->name, "file"))
        if (!uc->is_streamed && ffurl_seek(uc, 0, SEEK_SET) < 0)
            uc->is_streamed = 1;
    return 0;
}

int ff_check_interrupt(AVIOInterruptCB *cb)
{
    if (cb && cb->callback)
        return cb->callback(cb->opaque);
    return 0;
}

int ffurl_closep(URLContext **hh)
{
    URLContext *h= *hh;
    int ret = 0;
    if (!h)
        return 0;

    if (h->is_connected && h->pstUrlProt->url_close)
        ret = h->pstUrlProt->url_close(h);
#if CONFIG_NETWORK
    if (h->pstUrlProt->flags & URL_PROTOCOL_FLAG_NETWORK) {
        ff_network_close();
    }
#endif

    if (h->pstUrlProt->priv_data_size) {
        if (h->pstUrlProt->pstPrivDataClass)
            av_opt_free(h->pstPrivData);
        av_freep(&h->pstPrivData);
    }
    
    av_opt_free(h);
    av_freep(hh);
    return ret;
}

int ffurl_open_whitelist(URLContext **puc, const char *filename, int flags, const AVIOInterruptCB *int_cb, AVDictionary **options, const char *whitelist, const char* blacklist, URLContext *parent)
{
    AVDictionary *tmp_opts = NULL;
    AVDictionaryEntry *e;
    int ret = ffurl_alloc(puc, filename, flags, int_cb);
    if (ret < 0)
        return ret;
    if (parent)
        av_opt_copy(*puc, parent);
    if (options &&
        (ret = av_opt_set_dict(*puc, options)) < 0)
        goto fail;
    if (options && (*puc)->prot->priv_data_class &&
        (ret = av_opt_set_dict((*puc)->priv_data, options)) < 0)
        goto fail;

    if (!options)
        options = &tmp_opts;

    av_assert0(!whitelist ||
               !(e=av_dict_get(*options, "protocol_whitelist", NULL, 0)) ||
               !strcmp(whitelist, e->value));
    av_assert0(!blacklist ||
               !(e=av_dict_get(*options, "protocol_blacklist", NULL, 0)) ||
               !strcmp(blacklist, e->value));

    if ((ret = av_dict_set(options, "protocol_whitelist", whitelist, 0)) < 0)
        goto fail;

    if ((ret = av_dict_set(options, "protocol_blacklist", blacklist, 0)) < 0)
        goto fail;

    if ((ret = av_opt_set_dict(*puc, options)) < 0)
        goto fail;

    ret = ffurl_connect(*puc, options);

    if (!ret)
        return 0;
fail:
    ffurl_close(*puc);
    *puc = NULL;
    return ret;
}

int ffurl_read(URLContext *h, unsigned char *buf, int size)
{
   if (!(h->flags & AVIO_FLAG_READ))
       return AVERROR(EIO);
   
   return retry_transfer_wrapper(h, buf, size, 1, h->pstUrlProt->url_read);
}

int ffurl_close(URLContext *h)
{
    return ffurl_closep(&h);
}

#endif

int64_t url_seek(URLContext *pstUrlCtx, int64_t pos, int whence)
{
    int64_t ret;

    if (!pstUrlCtx->pstUrlProt->url_seek)
        return -1;
    ret = pstUrlCtx->pstUrlProt->url_seek(pstUrlCtx, pos, whence & ~AVSEEK_FORCE);
    
    return ret;
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
    if ((pstUrlCtx->flags & AVIO_FLAG_WRITE) || !strcmp(pstUrlCtx->pstUrlProt->url_name, "file")) {
        if (!pstUrlCtx->is_streamed && url_seek(pstUrlCtx, 0, SEEK_SET) < 0) {
            pstUrlCtx->is_streamed = 1;
        }
    }
        
    return 0;
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

    protocols = ffurl_get_protocols(NULL, NULL);
    if (!protocols)
        return NULL;
    
    for (i = 0; protocols[i]; i++) {
        const URLProtocol *up = protocols[i];
        
        if (!strcmp(proto_str, up->url_name)) {
            av_freep(&protocols);
            return up;
        }
        if (up->flags & URL_PROTOCOL_FLAG_NESTED_SCHEME && !strcmp(proto_nested, up->url_name)) {
            av_freep(&protocols);
            return up;
        }
    }
    av_freep(&protocols);

    return NULL;
}

#define OFFSET(x) offsetof(URLContext,x)
#define E AV_OPT_FLAG_ENCODING_PARAM
#define D AV_OPT_FLAG_DECODING_PARAM
static const AVOption options[] = {
    {"rw_timeout", OFFSET(rw_timeout), AV_OPT_TYPE_INT64, {.i64 = 0}, 0, INT64_MAX, D | E},
    { NULL }
};

const AVClass url_context_class = {
    .class_name       = "URLContext",
    .item_name        = urlcontext_to_name,
    .option           = options,
    .child_next       = urlcontext_child_next,
    .child_class_next = ff_urlcontext_child_class_next,
};

static int url_alloc_for_protocol(URLContext **ppstUrlCtx, const URLProtocol *pstUrlProt, const char *filename, int flags, const AVIOInterruptCB *int_cb)
{
    URLContext *pstUrlCtx;
    int err;

    if ((flags & AVIO_FLAG_READ) && !pstUrlProt->url_read) {
        av_log(NULL, AV_LOG_ERROR, "Impossible to open the '%s' protocol for reading\n", pstUrlProt->url_name);
        return -1;
    }
    
    if ((flags & AVIO_FLAG_WRITE) && !pstUrlProt->url_write) {
        av_log(NULL, AV_LOG_ERROR, "Impossible to open the '%s' protocol for writing\n", pstUrlProt->url_name);
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
            int proto_len= strlen(pstUrlProt->url_name);
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

int url_open_whitelist(URLContext **ppstUrlCtx, const char *filename, int flags, AVDictionary **options, URLContext *parent)
{
    AVDictionary *tmp_opts = NULL;
    AVDictionaryEntry *e;
    int ret = url_alloc(ppstUrlCtx, filename, flags, NULL);
    if (ret < 0)
        return ret;
    
    if (parent) {
        //av_opt_copy(*puc, parent);
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

int ffurl_handshake(URLContext *c)
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



