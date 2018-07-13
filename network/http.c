



#define CONFIG_ZLIB 0


static int has_header(const char *str, const char *header)
{
    /* header + 2 to skip over CRLF prefix. (make sure you have one!) */
    if (!str)
        return 0;
    
    return av_stristart(str, header + 2, NULL) || av_stristr(str, header);
}

#if 0
static int parse_set_cookie(const char *set_cookie, AVDictionary **dict)
{
    char *param, *next_param, *cstr, *back;

    if (!(cstr = av_strdup(set_cookie)))
        return AVERROR(EINVAL);

    // strip any trailing whitespace
    back = &cstr[strlen(cstr)-1];
    while (strchr(WHITESPACES, *back)) {
        *back='\0';
        back--;
    }

    next_param = cstr;
    while ((param = av_strtok(next_param, ";", &next_param))) {
        char *name, *value;
        param += strspn(param, WHITESPACES);
        if ((name = av_strtok(param, "=", &value))) {
            if (av_dict_set(dict, name, value, 0) < 0) {
                av_free(cstr);
                return -1;
            }
        }
    }

    av_free(cstr);
    return 0;
}

static int parse_cookie(HTTPContext *pstHttpCtx, const char *p, AVDictionary **cookies)
{
    AVDictionary *new_params = NULL;
    AVDictionaryEntry *e, *cookie_entry;
    char *eql, *name;

    // ensure the cookie is parsable
    if (parse_set_cookie(p, &new_params))
        return -1;

    // if there is no cookie value there is nothing to parse
    cookie_entry = av_dict_get(new_params, "", NULL, AV_DICT_IGNORE_SUFFIX);
    if (!cookie_entry || !cookie_entry->value) {
        av_dict_free(&new_params);
        return -1;
    }

    // ensure the cookie is not expired or older than an existing value
    if ((e = av_dict_get(new_params, "expires", NULL, 0)) && e->value) {
        struct tm new_tm = {0};
        if (!parse_set_cookie_expiry_time(e->value, &new_tm)) {
            AVDictionaryEntry *e2;

            // if the cookie has already expired ignore it
            if (av_timegm(&new_tm) < av_gettime() / 1000000) {
                av_dict_free(&new_params);
                return -1;
            }

            // only replace an older cookie with the same name
            e2 = av_dict_get(*cookies, cookie_entry->key, NULL, 0);
            if (e2 && e2->value) {
                AVDictionary *old_params = NULL;
                if (!parse_set_cookie(p, &old_params)) {
                    e2 = av_dict_get(old_params, "expires", NULL, 0);
                    if (e2 && e2->value) {
                        struct tm old_tm = {0};
                        if (!parse_set_cookie_expiry_time(e->value, &old_tm)) {
                            if (av_timegm(&new_tm) < av_timegm(&old_tm)) {
                                av_dict_free(&new_params);
                                av_dict_free(&old_params);
                                return -1;
                            }
                        }
                    }
                }
                av_dict_free(&old_params);
            }
        }
    }

    // duplicate the cookie name (dict will dupe the value)
    if (!(eql = strchr(p, '='))) return AVERROR(EINVAL);
    if (!(name = av_strndup(p, eql - p))) return AVERROR(ENOMEM);

    // add the cookie to the dictionary
    av_dict_set(cookies, name, eql, AV_DICT_DONT_STRDUP_KEY);

    return 0;
}


static int get_cookies(HTTPContext *pstHttpCtx, char **cookies, const char *path, const char *domain)
{
    // cookie strings will look like Set-Cookie header field values.  Multiple
    // Set-Cookie fields will result in multiple values delimited by a newline
    int ret = 0;
    char *cookie, *set_cookies = av_strdup(s->cookies), *next = set_cookies;

    if (!set_cookies) return -1;

    // destroy any cookies in the dictionary.
    av_dict_free(&pstHttpCtx->cookie_dict);

    *cookies = NULL;
    while ((cookie = av_strtok(next, "\n", &next))) {
        AVDictionary *cookie_params = NULL;
        AVDictionaryEntry *cookie_entry, *e;

        // store the cookie in a dict in case it is updated in the response
        if (parse_cookie(s, cookie, &s->cookie_dict))
            //av_log(s, AV_LOG_WARNING, "Unable to parse '%s'\n", cookie);

        // continue on to the next cookie if this one cannot be parsed
        if (parse_set_cookie(cookie, &cookie_params))
            continue;

        // if the cookie has no value, skip it
        cookie_entry = av_dict_get(cookie_params, "", NULL, AV_DICT_IGNORE_SUFFIX);
        if (!cookie_entry || !cookie_entry->value) {
            av_dict_free(&cookie_params);
            continue;
        }

        // if the cookie has expired, don't add it
        if ((e = av_dict_get(cookie_params, "expires", NULL, 0)) && e->value) {
            struct tm tm_buf = {0};
            if (!parse_set_cookie_expiry_time(e->value, &tm_buf)) {
                if (av_timegm(&tm_buf) < av_gettime() / 1000000) {
                    av_dict_free(&cookie_params);
                    continue;
                }
            }
        }

        // if no domain in the cookie assume it appied to this request
        if ((e = av_dict_get(cookie_params, "domain", NULL, 0)) && e->value) {
            // find the offset comparison is on the min domain (b.com, not a.b.com)
            int domain_offset = strlen(domain) - strlen(e->value);
            if (domain_offset < 0) {
                av_dict_free(&cookie_params);
                continue;
            }

            // match the cookie domain
            if (av_strcasecmp(&domain[domain_offset], e->value)) {
                av_dict_free(&cookie_params);
                continue;
            }
        }

        // ensure this cookie matches the path
        e = av_dict_get(cookie_params, "path", NULL, 0);
        if (!e || av_strncasecmp(path, e->value, strlen(e->value))) {
            av_dict_free(&cookie_params);
            continue;
        }

        // cookie parameters match, so copy the value
        if (!*cookies) {
            if (!(*cookies = av_asprintf("%s=%s", cookie_entry->key, cookie_entry->value))) {
                ret = AVERROR(ENOMEM);
                break;
            }
        } else {
            char *tmp = *cookies;
            size_t str_size = strlen(cookie_entry->key) + strlen(cookie_entry->value) + strlen(*cookies) + 4;
            if (!(*cookies = av_malloc(str_size))) {
                ret = AVERROR(ENOMEM);
                av_free(tmp);
                break;
            }
            snprintf(*cookies, str_size, "%s; %s=%s", tmp, cookie_entry->key, cookie_entry->value);
            av_free(tmp);
        }
    }

    av_free(set_cookies);

    return ret;
}
#endif

static int http_getc(HTTPContext *pstHttpCtx)
{
    int len;
    if (pstHttpCtx->buf_ptr >= pstHttpCtx->buf_end) {
        len = url_read(pstHttpCtx->pstSubUrlCtx, pstHttpCtx->buffer, BUFFER_SIZE);
        if (len < 0) {
            return len;
        } else if (len == 0) {
            return AVERROR_EOF;
        } else {
            pstHttpCtx->buf_ptr = pstHttpCtx->buffer;
            pstHttpCtx->buf_end = pstHttpCtx->buffer + len;
        }
    }
    return *pstHttpCtx->buf_ptr++;
}

static int http_get_line(HTTPContext *pstHttpCtx, char *line, int line_size)
{
    int ch;
    char *q;

    q = line;
    for (;;) {
        ch = http_getc(pstHttpCtx);
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

static int process_line(URLContext *pstUrlCtx, char *line, int line_count, int *new_location)
{
    HTTPContext *pstHttpCtx = pstUrlCtx->priv_data;
    const char *auto_method =  pstUrlCtx->flags & AVIO_FLAG_READ ? "POST" : "GET";
    char *tag, *p, *end, *method, *resource, *version;
    int ret;

    /* end of header */
    if (line[0] == '\0') {
        pstHttpCtx->end_header = 1;
        return 0;
    }

    p = line;
    if (line_count == 0) {
        if (pstHttpCtx->is_connected_server) {
            // HTTP method
            method = p;
            while (*p && !av_isspace(*p))
                p++;
            
            *(p++) = '\0';
            //av_log(h, AV_LOG_TRACE, "Received method: %s\n", method);
            
            if (pstHttpCtx->method) {
                if (av_strcasecmp(s->method, method)) {
                    //av_log(h, AV_LOG_ERROR, "Received and expected HTTP method do not match. (%s expected, %s received)\n", s->method, method);
                    return ff_http_averror(400, AVERROR(EIO));
                }
            } else {
                // use autodetected HTTP method to expect
                av_log(h, AV_LOG_TRACE, "Autodetected %s HTTP method\n", auto_method);
                if (av_strcasecmp(auto_method, method)) {
                    av_log(h, AV_LOG_ERROR, "Received and autodetected HTTP method did not match "
                           "(%s autodetected %s received)\n", auto_method, method);
                    return ff_http_averror(400, AVERROR(EIO));
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
                return ff_http_averror(400, AVERROR(EIO));
            }
            av_log(h, AV_LOG_TRACE, "HTTP version string: %s\n", version);
        } else {
            while (!av_isspace(*p) && *p != '\0')
                p++;
            while (av_isspace(*p))
                p++;
            s->http_code = strtol(p, &end, 10);

            av_log(h, AV_LOG_TRACE, "http_code=%d\n", s->http_code);

            if ((ret = check_http_code(h, s->http_code, end)) < 0)
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
            if ((ret = parse_location(s, p)) < 0)
                return ret;
            *new_location = 1;
        } else if (!av_strcasecmp(tag, "Content-Length") &&
                   s->filesize == UINT64_MAX) {
            s->filesize = strtoull(p, NULL, 10);
        } else if (!av_strcasecmp(tag, "Content-Range")) {
            parse_content_range(h, p);
        } else if (!av_strcasecmp(tag, "Accept-Ranges") &&
                   !strncmp(p, "bytes", 5) &&
                   s->seekable == -1) {
            h->is_streamed = 0;
        } else if (!av_strcasecmp(tag, "Transfer-Encoding") &&
                   !av_strncasecmp(p, "chunked", 7)) {
            s->filesize  = UINT64_MAX;
            s->chunksize = 0;
        } else if (!av_strcasecmp(tag, "WWW-Authenticate")) {
            ff_http_auth_handle_header(&s->auth_state, tag, p);
        } else if (!av_strcasecmp(tag, "Authentication-Info")) {
            ff_http_auth_handle_header(&s->auth_state, tag, p);
        } else if (!av_strcasecmp(tag, "Proxy-Authenticate")) {
            ff_http_auth_handle_header(&s->proxy_auth_state, tag, p);
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
            if (parse_cookie(s, p, &s->cookie_dict))
                av_log(h, AV_LOG_WARNING, "Unable to parse '%s'\n", p);
        } else if (!av_strcasecmp(tag, "Icy-MetaInt")) {
            s->icy_metaint = strtoull(p, NULL, 10);
        } else if (!av_strncasecmp(tag, "Icy-", 4)) {
            if ((ret = parse_icy(s, tag, p)) < 0)
                return ret;
        } else if (!av_strcasecmp(tag, "Content-Encoding")) {
            if ((ret = parse_content_encoding(h, p)) < 0)
                return ret;
        }
    }
    return 1;
}

static int cookie_string(AVDictionary *dict, char **cookies)
{
    AVDictionaryEntry *e = NULL;
    int len = 1;

    // determine how much memory is needed for the cookies string
    while (e = av_dict_get(dict, "", e, AV_DICT_IGNORE_SUFFIX))
        len += strlen(e->key) + strlen(e->value) + 1;

    // reallocate the cookies
    e = NULL;
    if (*cookies) av_free(*cookies);
    
    *cookies = av_malloc(len);
    if (!*cookies) 
        return -1;//AVERROR(ENOMEM);
        
    *cookies[0] = '\0';
    // write out the cookies
    while (e = av_dict_get(dict, "", e, AV_DICT_IGNORE_SUFFIX))
        av_strlcatf(*cookies, len, "%s%s\n", e->key, e->value);

    return 0;
}


static int http_read_header(URLContext *pstUrlCtx, int *new_location)
{
    HTTPContext *pstHttpCtx = pstUrlCtx->priv_data;
    char line[MAX_URL_SIZE];
    int err = 0;

    pstHttpCtx->chunksize = UINT64_MAX;

    for (;;) {
        if ((err = http_get_line(pstHttpCtx, line, sizeof(line))) < 0)
            return err;

        //av_log(h, AV_LOG_TRACE, "header='%s'\n", line);

        err = process_line(pstUrlCtx, line, pstHttpCtx->line_count, new_location);
        if (err < 0)
            return err;
        if (err == 0)
            break;
        pstHttpCtx->line_count++;
    }

    if (pstHttpCtx->seekable == -1 && pstHttpCtx->is_mediagateway && pstHttpCtx->filesize == 2000000000)
        pstUrlCtx->is_streamed = 1; /* we can in fact _not_ seek */

    // add any new cookies into the existing cookie string
    cookie_string(pstHttpCtx->cookie_dict, &pstHttpCtx->cookies);
    av_dict_free(&pstHttpCtx->cookie_dict);

    return err;
}


static int http_handshake(URLContext *pstUrlCtx)
{
    int ret, err, new_location;
    HTTPContext *pstHttpCtx = pstUrlCtx->priv_data;
    URLContext *pstSubUrlCtx = pstHttpCtx->pstSubUrlCtx;
    switch (pstHttpCtx->handshake_step) {
    case LOWER_PROTO:
        //av_log(c, AV_LOG_TRACE, "Lower protocol\n");
        if ((ret = ffurl_handshake(pstSubUrlCtx)) > 0)
            return 2 + ret;
        if (ret < 0)
            return ret;
        pstHttpCtx->handshake_step = READ_HEADERS;
        pstHttpCtx->is_connected_server = 1;
        return 2;
    case READ_HEADERS:
        //av_log(c, AV_LOG_TRACE, "Read headers\n");
        if ((err = http_read_header(c, &new_location)) < 0) {
            handle_http_errors(c, err);
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
    }
    
    return -1;
}

static int http_listen(URLContext *pstUrlCtx, const char *uri, int flags, AVDictionary **options) {
    HTTPContext *pstHttpCtx = pstUrlCtx->priv_data;
    int ret;
    
    char proto[10];
    char hostname[1024];
    char lower_url[100];
    const char *lower_proto = "tcp";
    int port;
    
    url_split(proto, sizeof(proto), NULL, 0, hostname, sizeof(hostname), &port, NULL, 0, uri);
    if (!strcmp(proto, "https")) {
        lower_proto = "tls";
    }
    
    url_join(lower_url, sizeof(lower_url), lower_proto, NULL, hostname, port, NULL);
    /*if ((ret = av_dict_set_int(options, "listen", s->listen, 0)) < 0)
        goto fail;*/

    ret = url_open_whitelist(&pstHttpCtx->pstSubUrlCtx, lower_url, AVIO_FLAG_READ_WRITE, options, pstUrlCtx);
    if (ret < 0)
        goto fail;
    
    pstHttpCtx->handshake_step = LOWER_PROTO;
    if (pstHttpCtx->listen == HTTP_SINGLE) { /* single client */
        pstHttpCtx->reply_code = 200;
        while ((ret = http_handshake(pstUrlCtx)) > 0);
    }
fail:
    av_dict_free(&s->chained_options);
    return ret;
}

static int http_connect(URLContext *pstUrlCtx, const char *path, const char *local_path,
                        const char *hoststr, const char *auth,
                        const char *proxyauth, int *new_location)
{
    HTTPContext *pstHttpCtx = pstUrlCtx->priv_data;
    int post, err;
    char headers[HTTP_HEADERS_SIZE] = "";
    char *authstr = NULL, *proxyauthstr = NULL;
    uint64_t off = pstHttpCtx->off;
    uint64_t filesize = pstHttpCtx->filesize;
    int len = 0;
    const char *method;
    int send_expect_100 = 0;
    int ret;

    /* send http header */
    post = pstUrlCtx->flags & AVIO_FLAG_WRITE;

    if (pstHttpCtx->post_data) {
        /* force POST method and disable chunked encoding when custom HTTP post data is set */
        post = 1;
        pstHttpCtx->chunked_post = 0;
    }

    if (pstHttpCtx->method)
        method = pstHttpCtx->method;
    else
        method = post ? "POST" : "GET";

    //authstr      = ff_http_auth_create_response(&s->auth_state, auth, local_path, method);
    //proxyauthstr = ff_http_auth_create_response(&s->proxy_auth_state, proxyauth, local_path, method);
    if (post && !pstHttpCtx->post_data) {
        send_expect_100 = pstHttpCtx->send_expect_100;
        /* The user has supplied authentication but we don't know the auth type,
         * send Expect: 100-continue to get the 401 response including the
         * WWW-Authenticate header, or an 100 continue if no auth actually
         * is needed. */
        if (auth && *auth && pstHttpCtx->auth_state.auth_type == HTTP_AUTH_NONE && pstHttpCtx->http_code != 401)
            send_expect_100 = 1;
    }

    if (strcmp(pstHttpCtx->user_agent_deprecated, DEFAULT_USER_AGENT)) {
        //av_log(s, AV_LOG_WARNING, "the user-agent option is deprecated, please use user_agent option\n");
        pstHttpCtx->user_agent = av_strdup(pstHttpCtx->user_agent_deprecated);
    }
    
    /* set default headers if needed */
    if (!has_header(pstHttpCtx->headers, "\r\nUser-Agent: "))
        len += av_strlcatf(headers + len, sizeof(headers) - len, "User-Agent: %s\r\n", pstHttpCtx->user_agent);

    if (!has_header(pstHttpCtx->headers, "\r\nAccept: "))
        len += av_strlcpy(headers + len, "Accept: */*\r\n", sizeof(headers) - len);
    
    // Note: we send this on purpose even when s->off is 0 when we're probing,
    // since it allows us to detect more reliably if a (non-conforming)
    // server supports seeking by analysing the reply headers.
    if (!has_header(pstHttpCtx->headers, "\r\nRange: ") && !post && (pstHttpCtx->off > 0 || pstHttpCtx->end_off || pstHttpCtx->seekable == -1)) {
        len += av_strlcatf(headers + len, sizeof(headers) - len, "Range: bytes=%"PRIu64"-", pstHttpCtx->off);

        if (pstHttpCtx->end_off)
            len += av_strlcatf(headers + len, sizeof(headers) - len, "%"PRId64, pstHttpCtx->end_off - 1);

        len += av_strlcpy(headers + len, "\r\n", sizeof(headers) - len);
    }
    
    if (send_expect_100 && !has_header(pstHttpCtx->headers, "\r\nExpect: "))
        len += av_strlcatf(headers + len, sizeof(headers) - len, "Expect: 100-continue\r\n");

    if (!has_header(pstHttpCtx->headers, "\r\nConnection: ")) {
        if (pstHttpCtx->multiple_requests)
            len += av_strlcpy(headers + len, "Connection: keep-alive\r\n", sizeof(headers) - len);
        else
            len += av_strlcpy(headers + len, "Connection: close\r\n", sizeof(headers) - len);
    }

    if (!has_header(pstHttpCtx->headers, "\r\nHost: "))
        len += av_strlcatf(headers + len, sizeof(headers) - len, "Host: %s\r\n", hoststr);
    
    if (!has_header(pstHttpCtx->headers, "\r\nContent-Length: ") && pstHttpCtx->post_data)
        len += av_strlcatf(headers + len, sizeof(headers) - len, "Content-Length: %d\r\n", pstHttpCtx->post_datalen);

    if (!has_header(pstHttpCtx->headers, "\r\nContent-Type: ") && pstHttpCtx->content_type)
        len += av_strlcatf(headers + len, sizeof(headers) - len, "Content-Type: %s\r\n", pstHttpCtx->content_type);

    if (!has_header(pstHttpCtx->headers, "\r\nCookie: ") && pstHttpCtx->cookies) {
        char *cookies = NULL;
        /*
        if (!get_cookies(s, &cookies, path, hoststr) && cookies) {
            len += av_strlcatf(headers + len, sizeof(headers) - len, "Cookie: %s\r\n", cookies);
            av_free(cookies);
        }
        */
    }
    if (!has_header(pstHttpCtx->headers, "\r\nIcy-MetaData: ") && pstHttpCtx->icy)
        len += av_strlcatf(headers + len, sizeof(headers) - len, "Icy-MetaData: %d\r\n", 1);

    /* now add in custom headers */
    if (pstHttpCtx->headers)
        av_strlcpy(headers + len, pstHttpCtx->headers, sizeof(headers) - len);

    ret = snprintf(pstHttpCtx->buffer, sizeof(pstHttpCtx->buffer),
             "%s %s HTTP/1.1\r\n"
             "%s"
             "%s"
             "%s"
             "%s%s"
             "\r\n",
             method,
             path,
             post && pstHttpCtx->chunked_post ? "Transfer-Encoding: chunked\r\n" : "",
             headers,
             authstr ? authstr : "",
             proxyauthstr ? "Proxy-" : "", 
             proxyauthstr ? proxyauthstr : "");

    //av_log(h, AV_LOG_DEBUG, "request: %s\n", s->buffer);
    printf("request: %s\n", pstHttpCtx->buffer);

    if (strlen(headers) + 1 == sizeof(headers) || ret >= sizeof(pstHttpCtx->buffer)) {
        //av_log(h, AV_LOG_ERROR, "overlong headers\n");
        err = -1;
        goto done;
    }


    if ((err = url_write(pstHttpCtx->pstSubUrlCtx, pstHttpCtx->buffer, strlen(pstHttpCtx->buffer))) < 0)
        goto done;

    if (pstHttpCtx->post_data)
        if ((err = url_write(pstHttpCtx->pstSubUrlCtx, pstHttpCtx->post_data, pstHttpCtx->post_datalen)) < 0)
            goto done;

    /* init input buffer */
    pstHttpCtx->buf_ptr          = pstHttpCtx->buffer;
    pstHttpCtx->buf_end          = pstHttpCtx->buffer;
    pstHttpCtx->line_count       = 0;
    pstHttpCtx->off              = 0;
    pstHttpCtx->icy_data_read    = 0;
    pstHttpCtx->filesize         = UINT64_MAX;
    pstHttpCtx->willclose        = 0;
    pstHttpCtx->end_chunked_post = 0;
    pstHttpCtx->end_header       = 0;
    
#if CONFIG_ZLIB
    pstHttpCtx->compressed       = 0;
#endif

    if (post && !pstHttpCtx->post_data && !send_expect_100) {
        /* Pretend that it did work. We didn't read any header yet, since
         * we've still to send the POST data, but the code calling this
         * function will check http_code after we return. */
        pstHttpCtx->http_code = 200;
        err = 0;
        goto done;
    }

    /* wait for header */
    err = http_read_header(pstUrlCtx, new_location);
    if (err < 0)
        goto done;

    if (*new_location)
        pstHttpCtx->off = off;

    /* Some buggy servers may missing 'Content-Range' header for range request */
    if (off > 0 && pstHttpCtx->off <= 0 && (off + pstHttpCtx->filesize == filesize)) {
        /*av_log(NULL, AV_LOG_WARNING, "try to fix missing 'Content-Range' at server side (%"PRId64",%"PRId64") => (%"PRId64",%"PRId64")",
               s->off, s->filesize, off, filesize);*/
        pstHttpCtx->off = off;
        pstHttpCtx->filesize = filesize;
    }

    err = (off == pstHttpCtx->off) ? 0 : -1;
    
done:
    av_freep(&authstr);
    av_freep(&proxyauthstr);
    return err;
}


static int http_open_cnx_internal(URLContext *pstUrlCtx, AVDictionary **options)
{
    const char *path;
    const char *proxy_path;
    const char *lower_proto = "tcp";
    const char *local_path;
    
    char hostname[1024];
    char hoststr[1024];
    char proto[10];
    
    char auth[1024];
    char proxyauth[1024] = "";
    char tmp_path[MAX_URL_SIZE];
    char buf[1024];
    char urlbuf[MAX_URL_SIZE];
    
    int port;
    int use_proxy;
    int err;
    int location_changed = 0;
    HTTPContext *pstHttpCtx = pstUrlCtx->priv_data;

    lower_proto = pstHttpCtx->tcp_hook;

    url_split(proto, sizeof(proto), auth, sizeof(auth), hostname, sizeof(hostname), &port, tmp_path, sizeof(tmp_path), pstHttpCtx->location);
    url_join(hoststr, sizeof(hoststr), NULL, NULL, hostname, port, NULL);

    proxy_path = pstHttpCtx->http_proxy ? pstHttpCtx->http_proxy : getenv("http_proxy");
    use_proxy  = !ff_http_match_no_proxy(getenv("no_proxy"), hostname) && proxy_path && av_strstart(proxy_path, "http://", NULL);

    if (!strcmp(proto, "https")) {
        //av_dict_set_int(options, "fastopen", 0, 0);
        lower_proto = "tls";
        use_proxy   = 0;
    
        if (port < 0)
            port = 443;
    }
    
    if (port < 0)
        port = 80;

    if (tmp_path[0] == '\0')
        path = "/";
    else
        path = tmp_path;
    
    local_path = path;
    if (use_proxy) {
        url_join(urlbuf, sizeof(urlbuf), proto, NULL, hostname, port, "%s", tmp_path);
        
        path = urlbuf;
        url_split(NULL, 0, proxyauth, sizeof(proxyauth), hostname, sizeof(hostname), &port, NULL, 0, proxy_path);
    }

    url_join(buf, sizeof(buf), lower_proto, NULL, hostname, port, NULL);

    if (!pstHttpCtx->pstSubUrlCtx) {
        //av_dict_set_int(options, "ijkapplication", (int64_t)(intptr_t)s->app_ctx, 0);
        err = url_open_whitelist(&pstHttpCtx->pstSubUrlCtx, buf, AVIO_FLAG_READ_WRITE, &pstUrlCtx->interrupt_callback, options, pstUrlCtx);
        if (err < 0)
            return err;
    }

    err = http_connect(pstUrlCtx, path, local_path, hoststr, auth, proxyauth, &location_changed);
    if (err < 0)
        return err;

    return location_changed;
}

int http_averror(int status_code, int default_averror)
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

static int http_open_cnx(URLContext *pstUrlCtx, AVDictionary **options)
{
    HTTPAuthType cur_auth_type, cur_proxy_auth_type;
    HTTPContext *pstHttpCtx = pstUrlCtx->priv_data;
    int location_changed, attempts = 0, redirects = 0;
redo:
    //av_dict_copy(options, s->chained_options, 0);

    cur_auth_type       = pstHttpCtx->auth_state.auth_type;
    cur_proxy_auth_type = pstHttpCtx->auth_state.auth_type;

    location_changed = http_open_cnx_internal(pstUrlCtx, options);
    if (location_changed < 0)
        goto fail;

    attempts++;
    if (pstHttpCtx->http_code == 401) {
        if ((cur_auth_type == HTTP_AUTH_NONE || pstHttpCtx->auth_state.stale) &&
            pstHttpCtx->auth_state.auth_type != HTTP_AUTH_NONE && attempts < 4) {
            url_closep(&pstHttpCtx->pstSubUrlCtx);
            goto redo;
        } else
            goto fail;
    }
    
    if (pstHttpCtx->http_code == 407) {
        if ((cur_proxy_auth_type == HTTP_AUTH_NONE || pstHttpCtx->proxy_auth_state.stale) 
            && pstHttpCtx->proxy_auth_state.auth_type != HTTP_AUTH_NONE 
            && attempts < 4) {
            url_closep(&pstHttpCtx->pstSubUrlCtx);
            goto redo;
        } else
            goto fail;
    }
    
    if ((pstHttpCtx->http_code == 301 || pstHttpCtx->http_code == 302 
          || pstHttpCtx->http_code == 303 || pstHttpCtx->http_code == 307) 
        && location_changed == 1) {
        /* url moved, get next */
        url_closep(&pstHttpCtx->pstSubUrlCtx);
        if (redirects++ >= MAX_REDIRECTS)
            return -1;//AVERROR(EIO);
            
        /* Restart the authentication process with the new target, which
         * might use a different auth mechanism. */
        memset(&pstHttpCtx->auth_state, 0, sizeof(pstHttpCtx->auth_state));
        attempts         = 0;
        location_changed = 0;
        goto redo;
    }
    return 0;

fail:
    if (pstHttpCtx->pstSubUrlCtx)
        url_closep(&pstHttpCtx->pstSubUrlCtx);
    
    if (location_changed < 0)
        return location_changed;
    
    return http_averror(pstHttpCtx->http_code, -1);//AVERROR(EIO));
}

static int http_open(URLContext *pstUrlCtx, const char *uri, int flags, AVDictionary **options)
{
    HTTPContext *pstHttpCtx = pstUrlCtx->priv_data;
    int ret;

    pstHttpCtx->app_ctx = (ApplicationContext *)(intptr_t)pstHttpCtx->app_ctx_intptr;

    if(pstHttpCtx->seekable == 1)
        pstUrlCtx->is_streamed = 0;
    else
        pstUrlCtx->is_streamed = 1;

    pstHttpCtx->filesize = UINT64_MAX;
    pstHttpCtx->location = strdup(uri);
    if (!pstHttpCtx->location)
        return -1;
    
    if (options) {
        //av_dict_copy(&s->chained_options, *options, 0);
    }

    if (pstHttpCtx->headers) {
        int len = strlen(pstHttpCtx->headers);
        if (len < 2 || strcmp("\r\n", pstHttpCtx->headers + len - 2)) {
            //av_log(h, AV_LOG_WARNING, "No trailing CRLF found in HTTP header.\n");
            ret = ff_reallocp(&pstHttpCtx->headers, len + 3);
            if (ret < 0)
                return ret;
            pstHttpCtx->headers[len]     = '\r';
            pstHttpCtx->headers[len + 1] = '\n';
            pstHttpCtx->headers[len + 2] = '\0';
        }
    }

    if (pstHttpCtx->listen) {
        return http_listen(pstUrlCtx, uri, flags, options);
    }

    //av_application_will_http_open(s->app_ctx, (void*)h, uri);
    ret = http_open_cnx(pstUrlCtx, options);
    //av_application_did_http_open(s->app_ctx, (void*)h, uri, ret, s->http_code, s->filesize);
    if (ret < 0) {
        //av_dict_free(&s->chained_options);
    }
    
    return ret;
}

static int http_read()
{
}


#define OFFSET(x) offsetof(HTTPContext, x)
#define D AV_OPT_FLAG_DECODING_PARAM
#define E AV_OPT_FLAG_ENCODING_PARAM
#define DEFAULT_USER_AGENT "Lavf/"


static const AVOption options[] = {
    {"seekable", OFFSET(seekable), AV_OPT_TYPE_BOOL, {.i64 = -1}, -1, 1, D},
    {"chunked_post", OFFSET(chunked_post), AV_OPT_TYPE_BOOL, {.i64 = 1}, 0, 1, E},
    {"http_proxy", OFFSET(http_proxy), AV_OPT_TYPE_STRING, {.str = NULL}, 0, 0, D|E},
    {"headers", OFFSET(headers), AV_OPT_TYPE_STRING, {.str = NULL}, 0, 0, D|E},
    {"content_type", OFFSET(content_type), AV_OPT_TYPE_STRING,{.str = NULL}, 0, 0, D|E},
    {"user_agent", OFFSET(user_agent), AV_OPT_TYPE_STRING, {.str = DEFAULT_USER_AGENT}, 0, 0, D},
    {"user-agent", OFFSET(user_agent_deprecated), AV_OPT_TYPE_STRING, {.str = DEFAULT_USER_AGENT}, 0, 0, D},

    {"multiple_requests", OFFSET(multiple_requests), AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, D|E},
    {"post_data", OFFSET(post_data), AV_OPT_TYPE_BINARY, .flags = D | E},
    {"mime_type", OFFSET(mime_type), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, AV_OPT_FLAG_EXPORT | AV_OPT_FLAG_READONLY},
    {"cookies", OFFSET(cookies), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, D },
    {"icy", OFFSET(icy), AV_OPT_TYPE_BOOL, { .i64 = 1 }, 0, 1, D },
    {"icy_metadata_headers", OFFSET(icy_metadata_headers), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, AV_OPT_FLAG_EXPORT},
    {"icy_metadata_packet", OFFSET(icy_metadata_packet), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, AV_OPT_FLAG_EXPORT},
    {"metadata", OFFSET(metadata), AV_OPT_TYPE_DICT, {0}, 0, 0, AV_OPT_FLAG_EXPORT },
    {"auth_type", OFFSET(auth_state.auth_type), AV_OPT_TYPE_INT, { .i64 = HTTP_AUTH_NONE }, HTTP_AUTH_NONE, HTTP_AUTH_BASIC, D | E, "auth_type"},
    {"none", 0, AV_OPT_TYPE_CONST, { .i64 = HTTP_AUTH_NONE }, 0, 0, D | E, "auth_type"},
    {"basic", 0, AV_OPT_TYPE_CONST, { .i64 = HTTP_AUTH_BASIC }, 0, 0, D | E, "auth_type"},
    //{"send_expect_100", OFFSET(send_expect_100), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, E },
    {"location", OFFSET(location), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, D | E },
    {"offset", OFFSET(off), AV_OPT_TYPE_INT64, { .i64 = 0 }, 0, INT64_MAX, D },
    {"end_offset", OFFSET(end_off), AV_OPT_TYPE_INT64, { .i64 = 0 }, 0, INT64_MAX, D },
    {"method", OFFSET(method), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, D | E },
    //{"reconnect", OFFSET(reconnect), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, D },
    //{"reconnect_at_eof", OFFSET(reconnect_at_eof), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, D },
    //{"reconnect_streamed", OFFSET(reconnect_streamed), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, D },
    //{"reconnect_delay_max", OFFSET(reconnect_delay_max), AV_OPT_TYPE_INT, { .i64 = 120 }, 0, UINT_MAX/1000/1000, D },
    {"listen", OFFSET(listen), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, 2, D | E },
    {"resource", OFFSET(resource), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, E },
    {"reply_code", OFFSET(reply_code), AV_OPT_TYPE_INT, { .i64 = 200}, INT_MIN, 599, E},
    {"http-tcp-hook", OFFSET(tcp_hook), AV_OPT_TYPE_STRING, { .str = "tcp" }, 0, 0, D | E },
    {"ijkapplication", OFFSET(app_ctx_intptr), AV_OPT_TYPE_INT64, { .i64 = 0 }, INT64_MIN, INT64_MAX, .flags = D },
    { NULL }
};

static const AVClass http_class = {
    .class_name = "http",
    .item_name  = default_item_name,
    .option     = options,
}

const URLProtocol http_protocol = {
    .url_name           = "http",
    .url_open2          = http_open,
    .url_handshake      = http_handshake,
    .url_read           = http_read,
    .url_seek           = http_seek,
    .url_close          = http_close,
    .priv_data_size     = sizeof(HTTPContext),
    .flags              = URL_PROTOCOL_FLAG_NETWORK,
    .priv_data_class    = &http_class
};



