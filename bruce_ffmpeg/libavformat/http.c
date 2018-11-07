/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>

#include "libavutil/error.h"
#include "libavutil/assert.h"
#include "libavutil/mem.h"
#include "libavutil/opt.h"
#include "libavutil/dict.h"
#include "libavutil/avstring.h"
#include "libavutil/time.h"
#include "libavutil/log.h"

#include "url.h"
#include "http.h"
#include "internal.h"
#include "version.h"
#include "avio.h"

static int http_averror(int status_code, int default_averror)
{
    switch (status_code) {
        case 400: return AVERROR_HTTP_BAD_REQUEST;
        case 401: return AVERROR_HTTP_UNAUTHORIZED;
        case 403: return AVERROR_HTTP_FORBIDDEN;
        case 404: return AVERROR_HTTP_NOT_FOUND;
        default: break;
    }
    
    if (status_code >= 400 && status_code <= 499)
        return AVERROR_HTTP_OTHER_4XX;
    else if (status_code >= 500)
        return AVERROR_HTTP_SERVER_ERROR;
    else
        return default_averror;
}

static int http_write_reply(URLContext* h, int status_code)
{
    int ret, body = 0, reply_code, message_len;
    const char *reply_text, *content_type;
    HTTPContext *s = h->pstPrivData;
    char message[BUFFER_SIZE];
    content_type = "text/plain";

    if (status_code < 0)
        body = 1;
    switch (status_code) {
        case AVERROR_HTTP_BAD_REQUEST:
        case 400:
            reply_code = 400;
            reply_text = "Bad Request";
            break;
        case AVERROR_HTTP_FORBIDDEN:
        case 403:
            reply_code = 403;
            reply_text = "Forbidden";
            break;
        case AVERROR_HTTP_NOT_FOUND:
        case 404:
            reply_code = 404;
            reply_text = "Not Found";
            break;
        case 200:
            reply_code = 200;
            reply_text = "OK";
            content_type = s->content_type ? s->content_type : "application/octet-stream";
            break;
        case AVERROR_HTTP_SERVER_ERROR:
        case 500:
            reply_code = 500;
            reply_text = "Internal server error";
            break;
        default:
            return AVERROR(EINVAL);
    }
    
    if (body) {
        s->chunked_post = 0;
        message_len = snprintf(message, sizeof(message),
                 "HTTP/1.1 %03d %s\r\n"
                 "Content-Type: %s\r\n"
                 "Content-Length: %zu\r\n"
                 "%s"
                 "\r\n"
                 "%03d %s\r\n",
                 reply_code,
                 reply_text,
                 content_type,
                 strlen(reply_text) + 6, // 3 digit status code + space + \r\n
                 s->headers ? s->headers : "",
                 reply_code,
                 reply_text);
    } else {
        s->chunked_post = 1;
        message_len = snprintf(message, sizeof(message),
                 "HTTP/1.1 %03d %s\r\n"
                 "Content-Type: %s\r\n"
                 "Transfer-Encoding: chunked\r\n"
                 "%s"
                 "\r\n",
                 reply_code,
                 reply_text,
                 content_type,
                 s->headers ? s->headers : "");
    }
    
    av_log(h, AV_LOG_TRACE, "HTTP reply header: \n%s----\n", message);
    if ((ret = url_write(s->hd, message, message_len)) < 0)
        return ret;
    
    return 0;
}

static void http_handle_error_code(URLContext *h, int error)
{
    av_assert0(error < 0);
    http_write_reply(h, error);
}

static int http_getc(HTTPContext *s)
{
    int len;
    if (s->buf_ptr >= s->buf_end) {
        len = url_read(s->hd, s->buffer, BUFFER_SIZE);
        if (len < 0) {
            return len;
        } else if (len == 0) {
            return AVERROR_EOF;
        } else {
            s->buf_ptr = s->buffer;
            s->buf_end = s->buffer + len;
        }
    }
    return *s->buf_ptr++;
}

static int http_get_line(HTTPContext *s, char *line, int line_size)
{
    int ch;
    char *q;

    q = line;
    for (;;) {
        ch = http_getc(s);
        if (ch < 0)
            return ch;
        
        if (ch == '\n') {
            /* process line */
            if (q > line && q[-1] == '\r')
                q--;
            *q = '\0';

            return 0;
        } else {
            if ((q - line) < line_size - 1)
                *q++ = ch;
        }
    }
}

static int http_check_code(URLContext *h, int http_code, const char *end)
{
    HTTPContext *s = h->pstPrivData;
    /* error codes are 4xx and 5xx, but regard 401 as a success, so we
     * don't abort until all headers have been parsed. */
    if (http_code >= 400 && http_code < 600 &&
        (http_code != 401 || s->auth_state.auth_type != HTTP_AUTH_NONE) &&
        (http_code != 407 || s->proxy_auth_state.auth_type != HTTP_AUTH_NONE)) {
        end += strspn(end, SPACE_CHARS);
        av_log(h, AV_LOG_WARNING, "HTTP error %d %s\n", http_code, end);
        return http_averror(http_code, AVERROR(EIO));
    }
    return 0;
}

static void http_parse_content_range(URLContext *h, const char *p)
{
    HTTPContext *s = h->pstPrivData;
    const char *slash;

    if (!strncmp(p, "bytes ", 6)) {
        p += 6;
        s->off = strtoull(p, NULL, 10);
        if ((slash = strchr(p, '/')) && strlen(slash) > 0)
            s->filesize = strtoull(slash + 1, NULL, 10);
    }
    
    if (s->seekable == -1 && (!s->is_akamai || s->filesize != 2147483647))
        h->is_streamed = 0; /* we _can_ in fact seek */
}

static int http_parse_cookie(HTTPContext *s, const char *p, AVDictionary **cookies)
{
    char *eql, *name;

    // duplicate the cookie name (dict will dupe the value)
    if (!(eql = strchr(p, '='))) 
        return AVERROR(EINVAL);
    
    if (!(name = av_strndup(p, eql - p))) 
        return AVERROR(ENOMEM);

    // add the cookie to the dictionary
    av_dict_set(cookies, name, eql, AV_DICT_DONT_STRDUP_KEY);

    return 0;
}

static int http_parse_icy(HTTPContext *s, const char *tag, const char *p)
{
    int len = 4 + strlen(p) + strlen(tag);
    int is_first = !s->icy_metadata_headers;
    int ret;

    av_dict_set(&s->metadata, tag, p, 0);

    if (s->icy_metadata_headers)
        len += strlen(s->icy_metadata_headers);

    if ((ret = av_reallocp(&s->icy_metadata_headers, len)) < 0)
        return ret;

    if (is_first)
        *s->icy_metadata_headers = '\0';

    av_strlcatf(s->icy_metadata_headers, len, "%s: %s\n", tag, p);

    return 0;
}

static int http_parse_content_encoding(URLContext *h, const char *p)
{
    if (!av_strncasecmp(p, "gzip", 4) || !av_strncasecmp(p, "deflate", 7)) {
#if 0 //CONFIG_ZLIB
        HTTPContext *s = h->priv_data;

        s->compressed = 1;
        inflateEnd(&s->inflate_stream);
        if (inflateInit2(&s->inflate_stream, 32 + 15) != Z_OK) {
            av_log(h, AV_LOG_WARNING, "Error during zlib initialisation: %s\n",
                   s->inflate_stream.msg);
            return AVERROR(ENOSYS);
        }
        if (zlibCompileFlags() & (1 << 17)) {
            av_log(h, AV_LOG_WARNING,
                   "Your zlib was compiled without gzip support.\n");
            return AVERROR(ENOSYS);
        }
#else
        av_log(h, AV_LOG_WARNING, "Compressed (%s) content, need zlib with gzip support\n", p);
        return AVERROR(ENOSYS);
#endif /* CONFIG_ZLIB */
    } else if (!av_strncasecmp(p, "identity", 8)) {
        // The normal, no-encoding case (although servers shouldn't include
        // the header at all if this is the case).
    } else {
        av_log(h, AV_LOG_WARNING, "Unknown content coding: %s\n", p);
    }
    
    return 0;
}

static int http_parse_location(HTTPContext *s, const char *p)
{
    char redirected_location[MAX_URL_SIZE], *new_loc;
    make_absolute_url(redirected_location, sizeof(redirected_location), s->location, p);
    new_loc = av_strdup(redirected_location);
    if (!new_loc)
        return AVERROR(ENOMEM);
    av_free(s->location);
    s->location = new_loc;
    return 0;
}

static int http_process_line(URLContext *h, char *line, int line_count, int *new_location)
{
    HTTPContext *s = h->pstPrivData;
    const char *auto_method =  h->flags & AVIO_FLAG_READ ? "POST" : "GET";
    char *tag, *p, *end, *method, *resource, *version;
    int ret;

    /* end of header */
    if (line[0] == '\0') {
        s->end_header = 1;
        return 0;
    }

    p = line;
    if (line_count == 0) {
        if (s->is_connected_server) {
            // HTTP method
            method = p;
            while (*p && !av_isspace(*p))
                p++;
            *(p++) = '\0';
            
            av_log(h, AV_LOG_TRACE, "Received method: %s\n", method);
            if (s->method) {
                if (av_strcasecmp(s->method, method)) {
                    av_log(h, AV_LOG_ERROR, "Received and expected HTTP method do not match. (%s expected, %s received)\n", s->method, method);
                    return http_averror(400, AVERROR(EIO));
                }
            } else {
                // use autodetected HTTP method to expect
                av_log(h, AV_LOG_TRACE, "Autodetected %s HTTP method\n", auto_method);
                if (av_strcasecmp(auto_method, method)) {
                    av_log(h, AV_LOG_ERROR, "Received and autodetected HTTP method did not match "
                           "(%s autodetected %s received)\n", auto_method, method);
                    return http_averror(400, AVERROR(EIO));
                }
                
                if (!(s->method = av_strdup(method)))
                    return AVERROR(ENOMEM);
            }

            // HTTP resource
            while (av_isspace(*p))
                p++;
            
            resource = p;
            while (!av_isspace(*p))
                p++;
            *(p++) = '\0';
            
            av_log(h, AV_LOG_TRACE, "Requested resource: %s\n", resource);
            if (!(s->resource = av_strdup(resource)))
                return AVERROR(ENOMEM);

            // HTTP version
            while (av_isspace(*p))
                p++;
            version = p;
            
            while (*p && !av_isspace(*p))
                p++;
            *p = '\0';
            
            if (av_strncasecmp(version, "HTTP/", 5)) {
                av_log(h, AV_LOG_ERROR, "Malformed HTTP version string.\n");
                return http_averror(400, AVERROR(EIO));
            }
            
            av_log(h, AV_LOG_TRACE, "HTTP version string: %s\n", version);
        } else {
            while (!av_isspace(*p) && *p != '\0')
                p++;
            while (av_isspace(*p))
                p++;
            s->http_code = strtol(p, &end, 10);

            av_log(h, AV_LOG_TRACE, "http_code=%d\n", s->http_code);

            if ((ret = http_check_code(h, s->http_code, end)) < 0)
                return ret;
        }
    } else {
        while (*p != '\0' && *p != ':')
            p++;
        if (*p != ':')
            return 1;

        *p  = '\0';
        tag = line;
        p++;
        while (av_isspace(*p))
            p++;
        if (!av_strcasecmp(tag, "Location")) {
            if ((ret = http_parse_location(s, p)) < 0)
                return ret;
            *new_location = 1;
        } else if (!av_strcasecmp(tag, "Content-Length") && s->filesize == UINT64_MAX) {
            s->filesize = strtoull(p, NULL, 10);
        } else if (!av_strcasecmp(tag, "Content-Range")) {
            http_parse_content_range(h, p);
        } else if (!av_strcasecmp(tag, "Accept-Ranges") && !strncmp(p, "bytes", 5) && s->seekable == -1) {
            h->is_streamed = 0;
        } else if (!av_strcasecmp(tag, "Transfer-Encoding") && !av_strncasecmp(p, "chunked", 7)) {
            s->filesize  = UINT64_MAX;
            s->chunksize = 0;
        } else if (!av_strcasecmp(tag, "WWW-Authenticate")) {
            //ff_http_auth_handle_header(&s->auth_state, tag, p);
        } else if (!av_strcasecmp(tag, "Authentication-Info")) {
            //ff_http_auth_handle_header(&s->auth_state, tag, p);
        } else if (!av_strcasecmp(tag, "Proxy-Authenticate")) {
            //ff_http_auth_handle_header(&s->proxy_auth_state, tag, p);
        } else if (!av_strcasecmp(tag, "Connection")) {
            if (!strcmp(p, "close"))
                s->willclose = 1;
        } else if (!av_strcasecmp(tag, "Server")) {
            if (!av_strcasecmp(p, "AkamaiGHost")) {
                s->is_akamai = 1;
            } else if (!av_strncasecmp(p, "MediaGateway", 12)) {
                s->is_mediagateway = 1;
            }
        } else if (!av_strcasecmp(tag, "Content-Type")) {
            av_free(s->mime_type);
            s->mime_type = av_strdup(p);
        } else if (!av_strcasecmp(tag, "Set-Cookie")) {
            if (http_parse_cookie(s, p, &s->cookie_dict))
                av_log(h, AV_LOG_WARNING, "Unable to parse '%s'\n", p);
        } else if (!av_strcasecmp(tag, "Icy-MetaInt")) {
            s->icy_metaint = strtoull(p, NULL, 10);
        } else if (!av_strncasecmp(tag, "Icy-", 4)) {
            if ((ret = http_parse_icy(s, tag, p)) < 0)
                return ret;
        } else if (!av_strcasecmp(tag, "Content-Encoding")) {
            if ((ret = http_parse_content_encoding(h, p)) < 0)
                return ret;
        }
    }
    
    return 1;
}

static int http_cookie_string(AVDictionary *dict, char **cookies)
{
    AVDictionaryEntry *e = NULL;
    int len = 1;

    // determine how much memory is needed for the cookies string
    while (e = av_dict_get(dict, "", e, AV_DICT_IGNORE_SUFFIX))
        len += strlen(e->key) + strlen(e->value) + 1;

    // reallocate the cookies
    e = NULL;
    if (*cookies) 
        av_free(*cookies);
    
    *cookies = av_malloc(len);
    if (!*cookies) 
        return AVERROR(ENOMEM);
    
    *cookies[0] = '\0';

    // write out the cookies
    while (e = av_dict_get(dict, "", e, AV_DICT_IGNORE_SUFFIX))
        av_strlcatf(*cookies, len, "%s%s\n", e->key, e->value);

    return 0;
}

static int http_read_header(URLContext *h, int *new_location)
{
    HTTPContext *s = h->pstPrivData;
    char line[MAX_URL_SIZE];
    int err = 0;

    s->chunksize = UINT64_MAX;

    for (;;) {
        if ((err = http_get_line(s, line, sizeof(line))) < 0)
            return err;

        av_log(h, AV_LOG_TRACE, "header='%s'\n", line);

        err = http_process_line(h, line, s->line_count, new_location);
        if (err < 0)
            return err;
        if (err == 0)
            break;
        s->line_count++;
    }

    if (s->seekable == -1 && s->is_mediagateway && s->filesize == 2000000000)
        h->is_streamed = 1; /* we can in fact _not_ seek */

    // add any new cookies into the existing cookie string
    http_cookie_string(s->cookie_dict, &s->cookies);
    av_dict_free(&s->cookie_dict);

    return err;
}

static int http_handshake(URLContext *c)
{
    int ret, err, new_location;
    HTTPContext *ch = c->pstPrivData;
    URLContext *cl = ch->hd;
    switch (ch->handshake_step) {
        case LOWER_PROTO:
            av_log(c, AV_LOG_TRACE, "Lower protocol\n");
            if ((ret = url_handshake(cl)) > 0)
                return 2 + ret;
            if (ret < 0)
                return ret;
            ch->handshake_step = READ_HEADERS;
            ch->is_connected_server = 1;
            return 2;
        case READ_HEADERS:
            av_log(c, AV_LOG_TRACE, "Read headers\n");
            if ((err = http_read_header(c, &new_location)) < 0) {
                http_handle_error_code(c, err);
                return err;
            }
            ch->handshake_step = WRITE_REPLY_HEADERS;
            return 1;
        case WRITE_REPLY_HEADERS:
            av_log(c, AV_LOG_TRACE, "Reply code: %d\n", ch->reply_code);
            if ((err = http_write_reply(c, ch->reply_code)) < 0)
                return err;
            ch->handshake_step = FINISH;
            return 1;
        case FINISH:
            return 0;
        default:
            return AVERROR(EINVAL);
    }
}

static int http_listen(URLContext *h, const char *uri, int flags, AVDictionary **options) {
    HTTPContext *s = h->pstPrivData;
    int ret;
    int port;
    char proto[10],hostname[1024];
    char lower_url[100];
    const char *lower_proto = "tcp";
    
    url_split(proto, sizeof(proto), NULL, 0, hostname, sizeof(hostname), &port, NULL, 0, uri);
    if (!strcmp(proto, "https"))
        lower_proto = "tls";
    
    url_join(lower_url, sizeof(lower_url), lower_proto, NULL, hostname, port, NULL);
    if ((ret = av_dict_set_int(options, "listen", s->listen, 0)) < 0)
        goto fail;

    ret = url_open_whitelist(&s->hd, lower_url, AVIO_FLAG_READ_WRITE, options, h);
    if (ret < 0)
        goto fail;
    
    s->handshake_step = LOWER_PROTO;
    if (s->listen == HTTP_SINGLE) { /* single client */
        s->reply_code = 200;
        do {
            ret = http_handshake(h);
        }while (ret > 0);
    }
    
fail:
    av_dict_free(&s->chained_options);
    return ret;
}

static int http_has_header(const char *str, const char *header)
{
    if (!str)
        return 0;
    
    /* header + 2 to skip over CRLF prefix. (make sure you have one!) */
    return av_stristart(str, header + 2, NULL) || av_stristr(str, header);
}

static int http_get_cookies(HTTPContext *s, char **cookies, const char *path, const char *domain)
{
    // cookie strings will look like Set-Cookie header field values.  Multiple
    // Set-Cookie fields will result in multiple values delimited by a newline
    int ret = 0;
    char *next, *cookie, *set_cookies = av_strdup(s->cookies), *cset_cookies = set_cookies;

    if (!set_cookies) return AVERROR(EINVAL);

    // destroy any cookies in the dictionary.
    av_dict_free(&s->cookie_dict);

    *cookies = NULL;
    while ((cookie = av_strtok(set_cookies, "\n", &next))) {
        int domain_offset = 0;
        char *param, *next_param, *cdomain = NULL, *cpath = NULL, *cvalue = NULL;
        set_cookies = NULL;

        // store the cookie in a dict in case it is updated in the response
        if (http_parse_cookie(s, cookie, &s->cookie_dict))
            av_log(s, AV_LOG_WARNING, "Unable to parse '%s'\n", cookie);

        while ((param = av_strtok(cookie, "; ", &next_param))) {
            if (cookie) {
                // first key-value pair is the actual cookie value
                cvalue = av_strdup(param);
                cookie = NULL;
            } else if (!av_strncasecmp("path=",   param, 5)) {
                av_free(cpath);
                cpath = av_strdup(&param[5]);
            } else if (!av_strncasecmp("domain=", param, 7)) {
                // if the cookie specifies a sub-domain, skip the leading dot thereby
                // supporting URLs that point to sub-domains and the master domain
                int leading_dot = (param[7] == '.');
                av_free(cdomain);
                cdomain = av_strdup(&param[7+leading_dot]);
            } else {
                // ignore unknown attributes
            }
        }
        if (!cdomain)
            cdomain = av_strdup(domain);

        // ensure all of the necessary values are valid
        if (!cdomain || !cpath || !cvalue) {
            av_log(s, AV_LOG_WARNING,
                   "Invalid cookie found, no value, path or domain specified\n");
            goto done_cookie;
        }

        // check if the request path matches the cookie path
        if (av_strncasecmp(path, cpath, strlen(cpath)))
            goto done_cookie;

        // the domain should be at least the size of our cookie domain
        domain_offset = strlen(domain) - strlen(cdomain);
        if (domain_offset < 0)
            goto done_cookie;

        // match the cookie domain
        if (av_strcasecmp(&domain[domain_offset], cdomain))
            goto done_cookie;

        // cookie parameters match, so copy the value
        if (!*cookies) {
            if (!(*cookies = av_strdup(cvalue))) {
                ret = AVERROR(ENOMEM);
                goto done_cookie;
            }
        } else {
            char *tmp = *cookies;
            size_t str_size = strlen(cvalue) + strlen(*cookies) + 3;
            if (!(*cookies = av_malloc(str_size))) {
                ret = AVERROR(ENOMEM);
                goto done_cookie;
            }
            snprintf(*cookies, str_size, "%s; %s", tmp, cvalue);
            av_free(tmp);
        }

        done_cookie:
        av_freep(&cdomain);
        av_freep(&cpath);
        av_freep(&cvalue);
        if (ret < 0) {
            if (*cookies) av_freep(cookies);
            av_free(cset_cookies);
            return ret;
        }
    }

    av_free(cset_cookies);

    return 0;
}

static int http_connect(URLContext *h, const char *path, const char *local_path, const char *hoststr, const char *auth, const char *proxyauth, int *new_location)
{
    HTTPContext *s = h->pstPrivData;
    int post, err;
    char headers[HTTP_HEADERS_SIZE] = "";
    char *authstr = NULL, *proxyauthstr = NULL;
    uint64_t off = s->off;
    uint64_t filesize = s->filesize;
    int len = 0;
    const char *method;
    int send_expect_100 = 0;
    int ret;

    /* send http header */
    post = h->flags & AVIO_FLAG_WRITE;

    if (s->post_data) {
        /* force POST method and disable chunked encoding when
         * custom HTTP post data is set */
        post            = 1;
        s->chunked_post = 0;
    }

    if (s->method)
        method = s->method;
    else
        method = post ? "POST" : "GET";

    //authstr      = http_auth_create_response(&s->auth_state, auth, local_path, method);
    //proxyauthstr = http_auth_create_response(&s->proxy_auth_state, proxyauth, local_path, method);
    
    if (post && !s->post_data) {
        send_expect_100 = s->send_expect_100;
        /* The user has supplied authentication but we don't know the auth type,
         * send Expect: 100-continue to get the 401 response including the
         * WWW-Authenticate header, or an 100 continue if no auth actually
         * is needed. */
        if (auth && *auth && s->auth_state.auth_type == HTTP_AUTH_NONE && s->http_code != 401)
            send_expect_100 = 1;
    }

#if 0 //FF_API_HTTP_USER_AGENT
    if (strcmp(s->user_agent_deprecated, DEFAULT_USER_AGENT)) {
        av_log(s, AV_LOG_WARNING, "the user-agent option is deprecated, please use user_agent option\n");
        s->user_agent = av_strdup(s->user_agent_deprecated);
    }
#endif

    /* set default headers if needed */
    if (!http_has_header(s->headers, "\r\nUser-Agent: "))
        len += av_strlcatf(headers + len, sizeof(headers) - len, "User-Agent: %s\r\n", s->user_agent);
    
    if (!http_has_header(s->headers, "\r\nAccept: "))
        len += av_strlcpy(headers + len, "Accept: */*\r\n", sizeof(headers) - len);
    
    // Note: we send this on purpose even when s->off is 0 when we're probing,
    // since it allows us to detect more reliably if a (non-conforming)
    // server supports seeking by analysing the reply headers.
    if (!http_has_header(s->headers, "\r\nRange: ") && !post && (s->off > 0 || s->end_off || s->seekable == -1)) {
        len += av_strlcatf(headers + len, sizeof(headers) - len, "Range: bytes=%"PRIu64"-", s->off);
        if (s->end_off)
            len += av_strlcatf(headers + len, sizeof(headers) - len, "%"PRId64, s->end_off - 1);
        len += av_strlcpy(headers + len, "\r\n", sizeof(headers) - len);
    }
    
    if (send_expect_100 && !http_has_header(s->headers, "\r\nExpect: "))
        len += av_strlcatf(headers + len, sizeof(headers) - len, "Expect: 100-continue\r\n");

    if (!http_has_header(s->headers, "\r\nConnection: ")) {
        if (s->multiple_requests)
            len += av_strlcpy(headers + len, "Connection: keep-alive\r\n", sizeof(headers) - len);
        else
            len += av_strlcpy(headers + len, "Connection: close\r\n", sizeof(headers) - len);
    }

    if (!http_has_header(s->headers, "\r\nHost: "))
        len += av_strlcatf(headers + len, sizeof(headers) - len, "Host: %s\r\n", hoststr);
    
    if (!http_has_header(s->headers, "\r\nContent-Length: ") && s->post_data)
        len += av_strlcatf(headers + len, sizeof(headers) - len, "Content-Length: %d\r\n", s->post_datalen);

    if (!http_has_header(s->headers, "\r\nContent-Type: ") && s->content_type)
        len += av_strlcatf(headers + len, sizeof(headers) - len, "Content-Type: %s\r\n", s->content_type);
    
    if (!http_has_header(s->headers, "\r\nCookie: ") && s->cookies) {
        char *cookies = NULL;
        if (!http_get_cookies(s, &cookies, path, hoststr) && cookies) {
            len += av_strlcatf(headers + len, sizeof(headers) - len, "Cookie: %s\r\n", cookies);
            av_free(cookies);
        }
    }
    
    if (!http_has_header(s->headers, "\r\nIcy-MetaData: ") && s->icy)
        len += av_strlcatf(headers + len, sizeof(headers) - len, "Icy-MetaData: %d\r\n", 1);

    /* now add in custom headers */
    if (s->headers)
        av_strlcpy(headers + len, s->headers, sizeof(headers) - len);

    ret = snprintf(s->buffer, sizeof(s->buffer),
             "%s %s HTTP/1.1\r\n"
             "%s"
             "%s"
             "%s"
             "%s%s"
             "\r\n",
             method,
             path,
             post && s->chunked_post ? "Transfer-Encoding: chunked\r\n" : "",
             headers,
             authstr ? authstr : "",
             proxyauthstr ? "Proxy-" : "", proxyauthstr ? proxyauthstr : "");

    av_log(h, AV_LOG_DEBUG, "request: %s\n", s->buffer);

    if (strlen(headers) + 1 == sizeof(headers) || ret >= sizeof(s->buffer)) {
        av_log(h, AV_LOG_ERROR, "overlong headers\n");
        err = AVERROR(EINVAL);
        goto done;
    }

    if ((err = url_write(s->hd, s->buffer, strlen(s->buffer))) < 0)
        goto done;

    if (s->post_data)
        if ((err = url_write(s->hd, s->post_data, s->post_datalen)) < 0)
            goto done;

    /* init input buffer */
    s->buf_ptr          = s->buffer;
    s->buf_end          = s->buffer;
    s->line_count       = 0;
    s->off              = 0;
    s->icy_data_read    = 0;
    s->filesize         = UINT64_MAX;
    s->willclose        = 0;
    s->end_chunked_post = 0;
    s->end_header       = 0;
    if (post && !s->post_data && !send_expect_100) {
        /* Pretend that it did work. We didn't read any header yet, since
         * we've still to send the POST data, but the code calling this
         * function will check http_code after we return. */
        s->http_code = 200;
        err = 0;
        goto done;
    }

    /* wait for header */
    err = http_read_header(h, new_location);
    if (err < 0)
        goto done;

    if (*new_location)
        s->off = off;

    /* Some buggy servers may missing 'Content-Range' header for range request */
    if (off > 0 && s->off <= 0 && (off + s->filesize == filesize)) {
        av_log(NULL, AV_LOG_WARNING, "try to fix missing 'Content-Range' at server side (%"PRId64",%"PRId64") => (%"PRId64",%"PRId64")",
               s->off, s->filesize, off, filesize);
        s->off = off;
        s->filesize = filesize;
    }

    err = (off == s->off) ? 0 : -1;
done:
    av_freep(&authstr);
    av_freep(&proxyauthstr);
    return err;
}

static int http_open_cnx_internal(URLContext *h, AVDictionary **options)
{
    const char *path, *proxy_path, *lower_proto = "tcp", *local_path;
    char hostname[1024], hoststr[1024], proto[10];
    char auth[1024], proxyauth[1024] = "";
    char path1[MAX_URL_SIZE];
    char buf[1024], urlbuf[MAX_URL_SIZE];
    int port, use_proxy, err, location_changed = 0;
    char prev_location[4096];
    HTTPContext *s = h->pstPrivData;
    lower_proto = s->tcp_hook;
    url_split(proto, sizeof(proto), auth, sizeof(auth), hostname, sizeof(hostname), &port, path1, sizeof(path1), s->location);
    url_join(hoststr, sizeof(hoststr), NULL, NULL, hostname, port, NULL);
    if (!strcmp(proto, "https")) {
        lower_proto = "tls";
        use_proxy   = 0;
        if (port < 0)
            port = 443;
    }
    if (port < 0)
        port = 80;
    if (path1[0] == '\0')
        path = "/";
    else
        path = path1;
    local_path = path;
    if (use_proxy) {
        /* Reassemble the request URL without auth string - we don't want to leak the auth to the proxy. */
        url_join(urlbuf, sizeof(urlbuf), proto, NULL, hostname, port, "%s", path1);
        path = urlbuf;
        url_split(NULL, 0, proxyauth, sizeof(proxyauth), hostname, sizeof(hostname), &port, NULL, 0, proxy_path);
    }
    
    url_join(buf, sizeof(buf), lower_proto, NULL, hostname, port, NULL);
    if (!s->hd) {
        err = url_open_whitelist(&s->hd, buf, AVIO_FLAG_READ_WRITE, options, h);
        if (err < 0)
            return err;
    }
    
    av_strlcpy(prev_location, s->location, sizeof(prev_location));
    err = http_connect(h, path, local_path, hoststr, auth, proxyauth, &location_changed);
    if (err < 0)
        return err;
    
    return location_changed;
}

static int http_open_cnx(URLContext *h, AVDictionary **options)
{
    HTTPAuthType cur_auth_type, cur_proxy_auth_type;
    HTTPContext *s = h->pstPrivData;
    int location_changed, attempts = 0, redirects = 0;
    
redo:
    av_dict_copy(options, s->chained_options, 0);

    cur_auth_type       = s->auth_state.auth_type;
    cur_proxy_auth_type = s->auth_state.auth_type;

    location_changed = http_open_cnx_internal(h, options);
    if (location_changed < 0)
        goto fail;

    attempts++;
    if (s->http_code == 401) {
        if ((cur_auth_type == HTTP_AUTH_NONE || s->auth_state.stale) && s->auth_state.auth_type != HTTP_AUTH_NONE && attempts < 4) 
        {
            url_closep(&s->hd);
            goto redo;
        } else {
            goto fail;
        }
    }
    
    if (s->http_code == 407) {
        if ((cur_proxy_auth_type == HTTP_AUTH_NONE || s->proxy_auth_state.stale) && s->proxy_auth_state.auth_type != HTTP_AUTH_NONE 
                                                                                 && attempts < 4) 
        {
            url_closep(&s->hd);
            goto redo;
        } else {
            goto fail;
        }
    }
    
    if ((s->http_code == 301 || s->http_code == 302 || s->http_code == 303 || s->http_code == 307) && location_changed == 1) {
        /* url moved, get next */
        url_closep(&s->hd);
        if (redirects++ >= MAX_REDIRECTS)
            return AVERROR(EIO);
        
        /* Restart the authentication process with the new target, which
         * might use a different auth mechanism. */
        memset(&s->auth_state, 0, sizeof(s->auth_state));
        attempts         = 0;
        location_changed = 0;
        goto redo;
    }
    
    return 0;

fail:
    if (s->hd)
        url_closep(&s->hd);
    
    if (location_changed < 0)
        return location_changed;
    
    return http_averror(s->http_code, AVERROR(EIO));
}

static int http_open(URLContext *h, const char *uri, int flags, AVDictionary **options)
{
    HTTPContext *s = h->pstPrivData;
    int ret;

    if( s->seekable == 1 )
        h->is_streamed = 0;
    else
        h->is_streamed = 1;

    s->filesize = UINT64_MAX;
    s->location = av_strdup(uri);
    if (!s->location)
        return AVERROR(ENOMEM);
    
    //if (options)
    //    dict_copy(&s->chained_options, *options, 0);

    if (s->headers) {
        int len = strlen(s->headers);
        if (len < 2 || strcmp("\r\n", s->headers + len - 2)) {
            av_log(h, AV_LOG_WARNING, "No trailing CRLF found in HTTP header.\n");
            ret = av_reallocp(&s->headers, len + 3);
            if (ret < 0)
                return ret;
            s->headers[len]     = '\r';
            s->headers[len + 1] = '\n';
            s->headers[len + 2] = '\0';
        }
    }

    if (s->listen) {
        return http_listen(h, uri, flags, options);
    }
    
    ret = http_open_cnx(h, options);
    if (ret < 0)
        av_dict_free(&s->chained_options);
    
    return ret;
}

static int http_buf_read(URLContext *h, uint8_t *buf, int size)
{
    HTTPContext *s = h->pstPrivData;
    int len;

    if (s->chunksize != UINT64_MAX) {
        if (!s->chunksize) {
            char line[32];
            int err;

            do {
                err = http_get_line(s, line, sizeof(line));
                if (err < 0)
                    return err;
            } while (!*line);    /* skip CR LF from last chunk */

            s->chunksize = strtoull(line, NULL, 16);

            av_log(h, AV_LOG_TRACE, "Chunked encoding data size: %"PRIu64"'\n", s->chunksize);

            if (!s->chunksize) {
                return 0;
            } else if (s->chunksize == UINT64_MAX) {
                av_log(h, AV_LOG_ERROR, "Invalid chunk size %"PRIu64"\n", s->chunksize);
                return AVERROR(EINVAL);
            }
        }
        
        size = FFMIN(size, s->chunksize);
    }

    /* read bytes from input buffer first */
    len = s->buf_end - s->buf_ptr;
    if (len > 0) {
        if (len > size)
            len = size;
        memcpy(buf, s->buf_ptr, len);
        s->buf_ptr += len;
    } else {
        uint64_t target_end = s->end_off ? s->end_off : s->filesize;
        if ((!s->willclose || s->chunksize == UINT64_MAX) && s->off >= target_end)
            return AVERROR_EOF;

        len = size;
        if (s->filesize > 0 && s->filesize != UINT64_MAX && s->filesize != 2147483647) {
            int64_t unread = s->filesize - s->off;
            if (len > unread)
                len = (int)unread;
        }
        
        if (len > 0)
            len = url_read(s->hd, buf, len);
        
        if (!len && (!s->willclose || s->chunksize == UINT64_MAX) && s->off < target_end) {
            av_log(h, AV_LOG_ERROR,
                   "Stream ends prematurely at %"PRIu64", should be %"PRIu64"\n",
                   s->off, target_end
                  );
            return AVERROR(EIO);
        }
    }
    
    if (len > 0) {
        s->off += len;
        if (s->chunksize > 0 && s->chunksize != UINT64_MAX) {
            av_assert0(s->chunksize >= len);
            s->chunksize -= len;
        }
    }
    
    return len;
}

static int64_t http_seek_internal(URLContext *h, int64_t off, int whence, int force_reconnect)
{
    HTTPContext *s = h->pstPrivData;
    URLContext *old_hd = s->hd;
    uint64_t old_off = s->off;
    uint8_t old_buf[BUFFER_SIZE];
    int old_buf_size, ret;
    AVDictionary *options = NULL;

    if (whence == AVSEEK_SIZE)
        return s->filesize;
    
    else if (!force_reconnect && ((whence == SEEK_CUR && off == 0) || (whence == SEEK_SET && off == s->off))) {
        return s->off;
    } else if ((s->filesize == UINT64_MAX && whence == SEEK_END)) {
        return AVERROR(ENOSYS);
    }

    if (whence == SEEK_CUR) {
        off += s->off;
    } else if (whence == SEEK_END) {
        off += s->filesize;
    } else if (whence != SEEK_SET) {
        return AVERROR(EINVAL);
    }
    
    if (off < 0)
        return AVERROR(EINVAL);
    
    s->off = off;

    if (s->off && h->is_streamed)
        return AVERROR(ENOSYS);

    /* we save the old context in case the seek fails */
    old_buf_size = s->buf_end - s->buf_ptr;
    memcpy(old_buf, s->buf_ptr, old_buf_size);
    s->hd = NULL;

    if ((ret = http_open_cnx(h, &options)) < 0) {
        av_dict_free(&options);
        memcpy(s->buffer, old_buf, old_buf_size);
        s->buf_ptr = s->buffer;
        s->buf_end = s->buffer + old_buf_size;
        s->hd      = old_hd;
        s->off     = old_off;
        return ret;
    }

    av_dict_free(&options);
    url_close(old_hd);
    return off;
}

static int http_store_icy(URLContext *h, int size)
{
#if 0
    HTTPContext *s = h->pstPrivData;
    /* until next metadata packet */
    uint64_t remaining;

    if (s->icy_metaint < s->icy_data_read)
        return AVERROR_INVALIDDATA;
    remaining = s->icy_metaint - s->icy_data_read;

    if (!remaining) {
        /* The metadata packet is variable sized. It has a 1 byte header
         * which sets the length of the packet (divided by 16). If it's 0,
         * the metadata doesn't change. After the packet, icy_metaint bytes
         * of normal data follows. */
        uint8_t ch;
        int len = http_read_stream_all(h, &ch, 1);
        if (len < 0)
            return len;
        if (ch > 0) {
            char data[255 * 16 + 1];
            int ret;
            len = ch * 16;
            ret = http_read_stream_all(h, data, len);
            if (ret < 0)
                return ret;
            data[len + 1] = 0;
            if ((ret = av_opt_set(s, "icy_metadata_packet", data, 0)) < 0)
                return ret;
            update_metadata(s, data);
        }
        s->icy_data_read = 0;
        remaining        = s->icy_metaint;
    }

    return FFMIN(size, remaining);
#endif
}

#if CONFIG_ZLIB
#define DECOMPRESS_BUF_SIZE (256 * 1024)
static int http_buf_read_compressed(URLContext *h, uint8_t *buf, int size)
{
    HTTPContext *s = h->pstPrivData;
    int ret;

    if (!s->inflate_buffer) {
        s->inflate_buffer = av_malloc(DECOMPRESS_BUF_SIZE);
        if (!s->inflate_buffer)
            return AVERROR(ENOMEM);
    }

    if (s->inflate_stream.avail_in == 0) {
        int read = http_buf_read(h, s->inflate_buffer, DECOMPRESS_BUF_SIZE);
        if (read <= 0)
            return read;
        s->inflate_stream.next_in  = s->inflate_buffer;
        s->inflate_stream.avail_in = read;
    }

    s->inflate_stream.avail_out = size;
    s->inflate_stream.next_out  = buf;

    ret = inflate(&s->inflate_stream, Z_SYNC_FLUSH);
    if (ret != Z_OK && ret != Z_STREAM_END)
        av_log(h, AV_LOG_WARNING, "inflate return value: %d, %s\n",
               ret, s->inflate_stream.msg);

    return size - s->inflate_stream.avail_out;
}
#endif /* CONFIG_ZLIB */

static int http_read_stream(URLContext *h, uint8_t *buf, int size)
{
    HTTPContext *s = h->pstPrivData;
    int err, new_location, read_ret;
    int64_t seek_ret;

    if (!s->hd)
        return AVERROR_EOF;

    if (s->end_chunked_post && !s->end_header) {
        err = http_read_header(h, &new_location);
        if (err < 0)
            return err;
    }

#if CONFIG_ZLIB
    if (s->compressed)
        return http_buf_read_compressed(h, buf, size);
#endif /* CONFIG_ZLIB */

    read_ret = http_buf_read(h, buf, size);
    if (   (read_ret  < 0 && s->reconnect        && (!h->is_streamed || s->reconnect_streamed) && s->filesize > 0 && s->off < s->filesize)
        || (read_ret == 0 && s->reconnect_at_eof && (!h->is_streamed || s->reconnect_streamed))) {
        uint64_t target = h->is_streamed ? 0 : s->off;

        if (s->reconnect_delay > s->reconnect_delay_max)
            return AVERROR(EIO);

        av_log(h, AV_LOG_INFO, "Will reconnect at %"PRIu64" error=%s.\n", s->off, av_err2str(read_ret));
        av_usleep(1000U*1000*s->reconnect_delay);
        s->reconnect_delay = 1 + 2*s->reconnect_delay;
        seek_ret = http_seek_internal(h, target, SEEK_SET, 1);
        if (seek_ret != target) {
            av_log(h, AV_LOG_ERROR, "Failed to reconnect at %"PRIu64".\n", target);
            return read_ret;
        }

        read_ret = http_buf_read(h, buf, size);
    } else
        s->reconnect_delay = 0;

    return read_ret;
}

static int http_read(URLContext *h, uint8_t *buf, int size)
{
    HTTPContext *s = h->pstPrivData;

    if (s->icy_metaint > 0) {
        size = http_store_icy(h, size);
        if (size < 0)
            return size;
    }

    size = http_read_stream(h, buf, size);
    if (size > 0)
        s->icy_data_read += size;
    return size;
}

static int http_shutdown(URLContext *h, int flags)
{
    int ret = 0;
    char footer[] = "0\r\n\r\n";
    HTTPContext *s = h->pstPrivData;

    /* signal end of chunked encoding if used */
    if (((flags & AVIO_FLAG_WRITE) && s->chunked_post) 
        || ((flags & AVIO_FLAG_READ) && s->chunked_post && s->listen)) 
    {
        ret = url_write(s->hd, footer, sizeof(footer) - 1);
        ret = ret > 0 ? 0 : ret;
        /* flush the receive buffer when it is write only mode */
        if (!(flags & AVIO_FLAG_READ)) {
            char buf[1024];
            int read_ret;
            s->hd->flags |= AVIO_FLAG_NONBLOCK;
            read_ret = url_read(s->hd, buf, sizeof(buf));
            s->hd->flags &= ~AVIO_FLAG_NONBLOCK;
            if (read_ret < 0 && read_ret != AVERROR(EAGAIN)) {
                av_log(h, AV_LOG_ERROR, "URL read error:  %d\n", read_ret);
                ret = read_ret;
            }
        }
        s->end_chunked_post = 1;
    }

    return ret;
}

static int http_close(URLContext *h)
{
    int ret = 0;
    HTTPContext *s = h->pstPrivData;

#if CONFIG_ZLIB
    inflateEnd(&s->inflate_stream);
    av_freep(&s->inflate_buffer);
#endif /* CONFIG_ZLIB */

    if (!s->end_chunked_post)
        /* Close the write direction by sending the end of chunked encoding. */
        ret = http_shutdown(h, h->flags);

    if (s->hd)
        url_closep(&s->hd);
    av_dict_free(&s->chained_options);
    return ret;
}

static int http_get_file_handle(URLContext *pstUrlCtx)
{
    HTTPContext *pstHttpCtx = pstUrlCtx->pstPrivData;
    return url_get_file_handle(pstHttpCtx->hd);
}

static int http_get_short_seek(URLContext *pstUrlCtx)
{
    HTTPContext *pstHttpCtx = pstUrlCtx->pstPrivData;
    return url_get_short_seek(pstHttpCtx->hd);
}

#define OFFSET(para) offsetof(HTTPContext, para)
#define D AV_OPT_FLAG_DECODING_PARAM
#define E AV_OPT_FLAG_ENCODING_PARAM
#define DEFAULT_USER_AGENT "Lavf/" AV_STRINGIFY(LIBAVFORMAT_VERSION)

static const AVOption http_options[] = {
    { "seekable", OFFSET(seekable), AV_OPT_TYPE_BOOL, { .i64 = -1 }, -1, 1, D },
    { "chunked_post", OFFSET(chunked_post), AV_OPT_TYPE_BOOL, { .i64 = 1 }, 0, 1, E },
    { "http_proxy", OFFSET(http_proxy), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, D | E },
    { "headers", OFFSET(headers), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, D | E },
    { "content_type", OFFSET(content_type), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, D | E },
    { "user_agent", OFFSET(user_agent), AV_OPT_TYPE_STRING, { .str = DEFAULT_USER_AGENT }, 0, 0, D },
#if 0//FF_API_HTTP_USER_AGENT
    { "user-agent", OFFSET(user_agent_deprecated), AV_OPT_TYPE_STRING, { .str = DEFAULT_USER_AGENT }, 0, 0, D|AV_OPT_FLAG_DEPRECATED },
#endif
    { "multiple_requests", OFFSET(multiple_requests), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, D | E },
    { "post_data", OFFSET(post_data), AV_OPT_TYPE_BINARY, .flags = D | E },
    { "mime_type", OFFSET(mime_type), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, AV_OPT_FLAG_EXPORT | AV_OPT_FLAG_READONLY },
    { "cookies", OFFSET(cookies), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, D },
    { "icy", OFFSET(icy), AV_OPT_TYPE_BOOL, { .i64 = 1 }, 0, 1, D },
    { "icy_metadata_headers", OFFSET(icy_metadata_headers), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, AV_OPT_FLAG_EXPORT },
    { "icy_metadata_packet", OFFSET(icy_metadata_packet), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, AV_OPT_FLAG_EXPORT },
    { "metadata", OFFSET(metadata), AV_OPT_TYPE_DICT, {0}, 0, 0, AV_OPT_FLAG_EXPORT },
    { "auth_type", OFFSET(auth_state.auth_type), AV_OPT_TYPE_INT, { .i64 = HTTP_AUTH_NONE }, HTTP_AUTH_NONE, HTTP_AUTH_BASIC, D | E, "auth_type"},
    { "none", 0, AV_OPT_TYPE_CONST, { .i64 = HTTP_AUTH_NONE }, 0, 0, D | E, "auth_type"},
    { "basic", 0, AV_OPT_TYPE_CONST, { .i64 = HTTP_AUTH_BASIC }, 0, 0, D | E, "auth_type"},
    { "send_expect_100", OFFSET(send_expect_100), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, E },
    { "location", OFFSET(location), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, D | E },
    { "offset", OFFSET(off), AV_OPT_TYPE_INT64, { .i64 = 0 }, 0, INT64_MAX, D },
    { "end_offset", OFFSET(end_off), AV_OPT_TYPE_INT64, { .i64 = 0 }, 0, INT64_MAX, D },
    { "method", OFFSET(method), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, D | E },
    { "reconnect", OFFSET(reconnect), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, D },
    { "reconnect_at_eof", OFFSET(reconnect_at_eof), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, D },
    { "reconnect_streamed", OFFSET(reconnect_streamed), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, D },
    { "reconnect_delay_max", OFFSET(reconnect_delay_max), AV_OPT_TYPE_INT, { .i64 = 120 }, 0, UINT_MAX/1000/1000, D },
    { "listen", OFFSET(listen), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, 2, D | E },
    { "resource", OFFSET(resource), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, E },
    { "reply_code", OFFSET(reply_code), AV_OPT_TYPE_INT, { .i64 = 200}, INT_MIN, 599, E},
    { NULL }
};

#define HTTP_CLASS(flavor)                          \
static const AVClass flavor ## _context_class = {   \
    .class_name = # flavor,                         \
    .item_name  = av_default_item_name,             \
    .option     = http_options,                     \
    .version    = LIBAVFORMAT_VERSION_INT,          \
}

HTTP_CLASS(http);
const URLProtocol http_protocol = {
    .url_name            = "http",
    .url_open2           = http_open,
    //.url_accept          = http_accept,
    //.url_handshake       = http_handshake,
    .url_read            = http_read,
    //.url_write           = http_write,
    //.url_seek            = http_seek,
    .url_close           = http_close,
    .url_get_file_handle = http_get_file_handle,
    .url_get_short_seek  = http_get_short_seek,
    .url_shutdown        = http_shutdown,
    .priv_data_size      = sizeof(HTTPContext),
    .pstPrivDataClass     = &http_context_class,
    .flags               = URL_PROTOCOL_FLAG_NETWORK,
    //.default_whitelist   = "http,https,tls,rtp,tcp,udp,crypto,httpproxy"
};

