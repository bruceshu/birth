/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/

#define DEFAULT_REORDERING_DELAY 100000


static int rtsp_probe(AVProbeData *p)
{
    if (
#if CONFIG_TLS_PROTOCOL
        av_strstart(p->filename, "rtsps:", NULL) ||
#endif
        av_strstart(p->filename, "rtsp:", NULL))
        return AVPROBE_SCORE_MAX;
    return 0;
}

static inline int read_line(AVFormatContext *fmtCtx, char *rbuf, const int rbufsize, int *rbuflen)
{
    RTSPState *rtspState = fmtCtx->priv_data;
    int idx       = 0;
    int ret       = 0;
    *rbuflen      = 0;

    do {
        ret = ffurl_read_complete(rtspState->rtsp_hd, rbuf + idx, 1);
        if (ret <= 0)
            return ret ? ret : AVERROR_EOF;
        if (rbuf[idx] == '\r') {
            /* Ignore */
        } else if (rbuf[idx] == '\n') {
            rbuf[idx] = '\0';
            *rbuflen  = idx;
            return 0;
        } else
            idx++;
    } while (idx < rbufsize);
    av_log(s, AV_LOG_ERROR, "Message too long\n");
    return AVERROR(EIO);
}

void ff_rtsp_skip_packet(AVFormatContext *s)
{
    RTSPState *rt = s->priv_data;
    int ret, len, len1;
    uint8_t buf[1024];

    ret = ffurl_read_complete(rt->rtsp_hd, buf, 3);
    if (ret != 3)
        return;
    len = AV_RB16(buf + 1);

    av_log(s, AV_LOG_TRACE, "skipping RTP packet len=%d\n", len);

    /* skip payload */
    while (len > 0) {
        len1 = len;
        if (len1 > sizeof(buf))
            len1 = sizeof(buf);
        ret = ffurl_read_complete(rt->rtsp_hd, buf, len1);
        if (ret != len1)
            return;
        len -= len1;
    }
}

void ff_http_init_auth_state(URLContext *dest, const URLContext *src)
{
    memcpy(&((HTTPContext *)dest->pstPrivData)->auth_state,
           &((HTTPContext *)src->pstPrivData)->auth_state,
           sizeof(HTTPAuthState));
    memcpy(&((HTTPContext *)dest->priv_data)->proxy_auth_state,
           &((HTTPContext *)src->priv_data)->proxy_auth_state,
           sizeof(HTTPAuthState));
}

static void get_word_until_chars(char *buf, int buf_size,
                                 const char *sep, const char **pp)
{
    const char *p;
    char *q;

    p = *pp;
    p += strspn(p, SPACE_CHARS);
    q = buf;
    while (!strchr(sep, *p) && *p != '\0') {
        if ((q - buf) < buf_size - 1)
            *q++ = *p;
        p++;
    }
    if (buf_size > 0)
        *q = '\0';
    *pp = p;
}

static void get_word_sep(char *buf, int buf_size, const char *sep, const char **pp)
{
    if (**pp == '/') (*pp)++;
    get_word_until_chars(buf, buf_size, sep, pp);
}

static void get_word(char *buf, int buf_size, const char **pp)
{
    get_word_until_chars(buf, buf_size, SPACE_CHARS, pp);
}

void ff_rtsp_parse_line(AVFormatContext *s,
                        RTSPMessageHeader *reply, const char *buf,
                        RTSPState *rt, const char *method)
{
    const char *p;

    /* NOTE: we do case independent match for broken servers */
    p = buf;
    if (av_stristart(p, "Session:", &p)) {
        int t;
        get_word_sep(reply->session_id, sizeof(reply->session_id), ";", &p);
        if (av_stristart(p, ";timeout=", &p) &&
            (t = strtol(p, NULL, 10)) > 0) {
            reply->timeout = t;
        }
    } else if (av_stristart(p, "Content-Length:", &p)) {
        reply->content_length = strtol(p, NULL, 10);
    } else if (av_stristart(p, "Transport:", &p)) {
        rtsp_parse_transport(s, reply, p);
    } else if (av_stristart(p, "CSeq:", &p)) {
        reply->seq = strtol(p, NULL, 10);
    } else if (av_stristart(p, "Range:", &p)) {
        rtsp_parse_range_npt(p, &reply->range_start, &reply->range_end);
    } else if (av_stristart(p, "RealChallenge1:", &p)) {
        p += strspn(p, SPACE_CHARS);
        av_strlcpy(reply->real_challenge, p, sizeof(reply->real_challenge));
    } else if (av_stristart(p, "Server:", &p)) {
        p += strspn(p, SPACE_CHARS);
        av_strlcpy(reply->server, p, sizeof(reply->server));
    } else if (av_stristart(p, "Notice:", &p) ||
               av_stristart(p, "X-Notice:", &p)) {
        reply->notice = strtol(p, NULL, 10);
    } else if (av_stristart(p, "Location:", &p)) {
        p += strspn(p, SPACE_CHARS);
        av_strlcpy(reply->location, p , sizeof(reply->location));
    } else if (av_stristart(p, "WWW-Authenticate:", &p) && rt) {
        p += strspn(p, SPACE_CHARS);
        ff_http_auth_handle_header(&rt->auth_state, "WWW-Authenticate", p);
    } else if (av_stristart(p, "Authentication-Info:", &p) && rt) {
        p += strspn(p, SPACE_CHARS);
        ff_http_auth_handle_header(&rt->auth_state, "Authentication-Info", p);
    } else if (av_stristart(p, "Content-Base:", &p) && rt) {
        p += strspn(p, SPACE_CHARS);
        if (method && !strcmp(method, "DESCRIBE"))
            av_strlcpy(rt->control_uri, p , sizeof(rt->control_uri));
    } else if (av_stristart(p, "RTP-Info:", &p) && rt) {
        p += strspn(p, SPACE_CHARS);
        if (method && !strcmp(method, "PLAY"))
            rtsp_parse_rtp_info(rt, p);
    } else if (av_stristart(p, "Public:", &p) && rt) {
        if (strstr(p, "GET_PARAMETER") &&
            method && !strcmp(method, "OPTIONS"))
            rt->get_parameter_supported = 1;
    } else if (av_stristart(p, "x-Accept-Dynamic-Rate:", &p) && rt) {
        p += strspn(p, SPACE_CHARS);
        rt->accept_dynamic_rate = atoi(p);
    } else if (av_stristart(p, "Content-Type:", &p)) {
        p += strspn(p, SPACE_CHARS);
        av_strlcpy(reply->content_type, p, sizeof(reply->content_type));
    }
}


static int rtsp_send_cmd_with_content_async(AVFormatContext *s,
                                            const char *method, const char *url,
                                            const char *headers,
                                            const unsigned char *send_content,
                                            int send_content_length)
{
    RTSPState *rt = s->priv_data;
    char buf[4096], *out_buf;
    char base64buf[AV_BASE64_SIZE(sizeof(buf))];

    /* Add in RTSP headers */
    out_buf = buf;
    rt->seq++;
    snprintf(buf, sizeof(buf), "%s %s RTSP/1.0\r\n", method, url);
    if (headers)
        av_strlcat(buf, headers, sizeof(buf));
    
    av_strlcatf(buf, sizeof(buf), "CSeq: %d\r\n", rt->seq);
    av_strlcatf(buf, sizeof(buf), "User-Agent: %s\r\n",  rt->user_agent);
    
    if (rt->session_id[0] != '\0' && (!headers || !strstr(headers, "\nIf-Match:"))) {
        av_strlcatf(buf, sizeof(buf), "Session: %s\r\n", rt->session_id);
    }
    
    if (rt->auth[0]) {
        char *str = ff_http_auth_create_response(&rt->auth_state, rt->auth, url, method);
        if (str)
            av_strlcat(buf, str, sizeof(buf));
        av_free(str);
    }
    
    if (send_content_length > 0 && send_content)
        av_strlcatf(buf, sizeof(buf), "Content-Length: %d\r\n", send_content_length);
    
    av_strlcat(buf, "\r\n", sizeof(buf));

    /* base64 encode rtsp if tunneling */
    if (rt->control_transport == RTSP_MODE_TUNNEL) {
        av_base64_encode(base64buf, sizeof(base64buf), buf, strlen(buf));
        out_buf = base64buf;
    }
    av_log(s, AV_LOG_TRACE, "Sending:\n%s--\n", buf);

    ffurl_write(rt->rtsp_hd_out, out_buf, strlen(out_buf));
    if (send_content_length > 0 && send_content) {
        if (rt->control_transport == RTSP_MODE_TUNNEL) {
            //avpriv_report_missing_feature(s, "Tunneling of RTSP requests with content data");
            return AVERROR_PATCHWELCOME;
        }
        ffurl_write(rt->rtsp_hd_out, send_content, send_content_length);
    }
    rt->last_cmd_time = av_gettime_relative();

    return 0;
}

int ff_rtsp_read_reply(AVFormatContext *s, RTSPMessageHeader *reply,
                       unsigned char **content_ptr,
                       int return_on_interleaved_data, const char *method)
{
    RTSPState *rt = s->priv_data;
    char buf[4096], buf1[1024], *q;
    unsigned char ch;
    const char *p;
    int ret, content_length, line_count = 0, request = 0;
    unsigned char *content = NULL;

start:
    line_count = 0;
    request = 0;
    content = NULL;
    memset(reply, 0, sizeof(*reply));

    /* parse reply (XXX: use buffers) */
    rt->last_reply[0] = '\0';
    for (;;) {
        q = buf;
        for (;;) {
            ret = ffurl_read_complete(rt->rtsp_hd, &ch, 1);
            av_log(s, AV_LOG_TRACE, "ret=%d c=%02x [%c]\n", ret, ch, ch);
            if (ret != 1)
                return AVERROR_EOF;
            if (ch == '\n')
                break;
            if (ch == '$' && q == buf) {
                if (return_on_interleaved_data) {
                    return 1;
                } else
                    ff_rtsp_skip_packet(s);
            } else if (ch != '\r') {
                if ((q - buf) < sizeof(buf) - 1)
                    *q++ = ch;
            }
        }
        *q = '\0';

        av_log(s, AV_LOG_TRACE, "line='%s'\n", buf);

        /* test if last line */
        if (buf[0] == '\0')
            break;
        p = buf;
        if (line_count == 0) {
            /* get reply code */
            get_word(buf1, sizeof(buf1), &p);
            if (!strncmp(buf1, "RTSP/", 5)) {
                get_word(buf1, sizeof(buf1), &p);
                reply->status_code = atoi(buf1);
                av_strlcpy(reply->reason, p, sizeof(reply->reason));
            } else {
                av_strlcpy(reply->reason, buf1, sizeof(reply->reason)); // method
                get_word(buf1, sizeof(buf1), &p); // object
                request = 1;
            }
        } else {
            ff_rtsp_parse_line(s, reply, p, rt, method);
            av_strlcat(rt->last_reply, p,    sizeof(rt->last_reply));
            av_strlcat(rt->last_reply, "\n", sizeof(rt->last_reply));
        }
        line_count++;
    }

    if (rt->session_id[0] == '\0' && reply->session_id[0] != '\0' && !request)
        av_strlcpy(rt->session_id, reply->session_id, sizeof(rt->session_id));

    content_length = reply->content_length;
    if (content_length > 0) {
        /* leave some room for a trailing '\0' (useful for simple parsing) */
        content = av_malloc(content_length + 1);
        if (!content)
            return AVERROR(ENOMEM);
        ffurl_read_complete(rt->rtsp_hd, content, content_length);
        content[content_length] = '\0';
    }
    if (content_ptr)
        *content_ptr = content;
    else
        av_freep(&content);

    if (request) {
        char buf[1024];
        char base64buf[AV_BASE64_SIZE(sizeof(buf))];
        const char* ptr = buf;

        if (!strcmp(reply->reason, "OPTIONS")) {
            snprintf(buf, sizeof(buf), "RTSP/1.0 200 OK\r\n");
            if (reply->seq)
                av_strlcatf(buf, sizeof(buf), "CSeq: %d\r\n", reply->seq);
            if (reply->session_id[0])
                av_strlcatf(buf, sizeof(buf), "Session: %s\r\n",
                                              reply->session_id);
        } else {
            snprintf(buf, sizeof(buf), "RTSP/1.0 501 Not Implemented\r\n");
        }
        av_strlcat(buf, "\r\n", sizeof(buf));

        if (rt->control_transport == RTSP_MODE_TUNNEL) {
            av_base64_encode(base64buf, sizeof(base64buf), buf, strlen(buf));
            ptr = base64buf;
        }
        ffurl_write(rt->rtsp_hd_out, ptr, strlen(ptr));

        rt->last_cmd_time = av_gettime_relative();
        /* Even if the request from the server had data, it is not the data
         * that the caller wants or expects. The memory could also be leaked
         * if the actual following reply has content data. */
        if (content_ptr)
            av_freep(content_ptr);
        /* If method is set, this is called from ff_rtsp_send_cmd,
         * where a reply to exactly this request is awaited. For
         * callers from within packet receiving, we just want to
         * return to the caller and go back to receiving packets. */
        if (method)
            goto start;
        return 0;
    }

    if (rt->seq != reply->seq) {
        av_log(s, AV_LOG_WARNING, "CSeq %d expected, %d received.\n",
            rt->seq, reply->seq);
    }

    /* EOS */
    if (reply->notice == 2101 /* End-of-Stream Reached */      ||
        reply->notice == 2104 /* Start-of-Stream Reached */    ||
        reply->notice == 2306 /* Continuous Feed Terminated */) {
        rt->state = RTSP_STATE_IDLE;
    } else if (reply->notice >= 4400 && reply->notice < 5500) {
        return AVERROR(EIO); /* data or server error */
    } else if (reply->notice == 2401 /* Ticket Expired */ ||
             (reply->notice >= 5500 && reply->notice < 5600) /* end of term */ )
        return AVERROR(EPERM);

    return 0;
}


int ff_rtsp_send_cmd(AVFormatContext *s, const char *method, const char *url,
                     const char *headers, RTSPMessageHeader *reply,
                     unsigned char **content_ptr)
{
    return ff_rtsp_send_cmd_with_content(s, method, url, headers, reply,
                                         content_ptr, NULL, 0);
}

 int ff_rtsp_send_cmd_with_content(AVFormatContext *s,
                                   const char *method, const char *url,
                                   const char *header,
                                   RTSPMessageHeader *reply,
                                   unsigned char **content_ptr,
                                   const unsigned char *send_content,
                                   int send_content_length)
 {
     RTSPState *rt = s->priv_data;
     HTTPAuthType cur_auth_type;
     int ret, attempts = 0;
 
 retry:
     cur_auth_type = rt->auth_state.auth_type;
     if ((ret = rtsp_send_cmd_with_content_async(s, method, url, header,
                                                 send_content,
                                                 send_content_length)))
         return ret;
 
     if ((ret = ff_rtsp_read_reply(s, reply, content_ptr, 0, method) ) < 0)
         return ret;
     attempts++;
 
     if (reply->status_code == 401 &&
         (cur_auth_type == HTTP_AUTH_NONE || rt->auth_state.stale) &&
         rt->auth_state.auth_type != HTTP_AUTH_NONE && attempts < 2)
         goto retry;
 
     if (reply->status_code > 400){
         av_log(s, AV_LOG_ERROR, "method %s failed: %d%s\n",
                method,
                reply->status_code,
                reply->reason);
         av_log(s, AV_LOG_DEBUG, "%s\n", rt->last_reply);
     }
 
     return 0;
 }


static int rtsp_listen(AVFormatContext *s)
{
    RTSPState *rt = s->priv_data;
    char proto[128], host[128], path[512], auth[128];
    char uri[500];
    int port;
    int default_port = RTSP_DEFAULT_PORT;
    char tcpname[500];
    const char *lower_proto = "tcp";
    unsigned char rbuf[4096];
    unsigned char method[10];
    int rbuflen = 0;
    int ret;
    enum RTSPMethod methodcode;

    av_url_split(proto, sizeof(proto), auth, sizeof(auth), host, sizeof(host), &port, path, sizeof(path), s->filename);

    ff_url_join(rt->control_uri, sizeof(rt->control_uri), proto, NULL, host, port, "%s", path);

    if (!strcmp(proto, "rtsps")) {
        lower_proto  = "tls";
        default_port = RTSPS_DEFAULT_PORT;
    }

    if (port < 0)
        port = default_port;

    /* Create TCP connection */
    ff_url_join(tcpname, sizeof(tcpname), lower_proto, NULL, host, port, "?listen&listen_timeout=%d", rt->initial_timeout * 1000);

    if (ret = ffurl_open_whitelist(&rt->rtsp_hd, tcpname, AVIO_FLAG_READ_WRITE,
                                   &s->interrupt_callback, NULL,
                                   s->protocol_whitelist, s->protocol_blacklist, NULL)) {
        av_log(s, AV_LOG_ERROR, "Unable to open RTSP for listening\n");
        return ret;
    }
    rt->state       = RTSP_STATE_IDLE;
    rt->rtsp_hd_out = rt->rtsp_hd;
    for (;;) { /* Wait for incoming RTSP messages */
        ret = read_line(s, rbuf, sizeof(rbuf), &rbuflen);
        if (ret < 0)
            return ret;
        ret = parse_command_line(s, rbuf, rbuflen, uri, sizeof(uri), method,
                                 sizeof(method), &methodcode);
        if (ret) {
            av_log(s, AV_LOG_ERROR, "RTSP: Unexpected Command\n");
            return ret;
        }

        if (methodcode == ANNOUNCE) {
            ret       = rtsp_read_announce(s);
            rt->state = RTSP_STATE_PAUSED;
        } else if (methodcode == OPTIONS) {
            ret = rtsp_read_options(s);
        } else if (methodcode == RECORD) {
            ret = rtsp_read_record(s);
            if (!ret)
                return 0; // We are ready for streaming
        } else if (methodcode == SETUP)
            ret = rtsp_read_setup(s, host, uri);
        if (ret) {
            ffurl_close(rt->rtsp_hd);
            return AVERROR_INVALIDDATA;
        }
    }
}

int ff_rtsp_connect(AVFormatContext *pstFmtCtx)
{
    RTSPState *pstRtspState = pstFmtCtx->priv_data;
    char proto[128], host[1024], path[1024];
    char tcpname[1024], cmd[2048], auth[128];
    const char *lower_rtsp_proto = "tcp";
    int port, err, tcp_fd;
    RTSPMessageHeader reply1 = {0}, *reply = &reply1;
    int lower_transport_mask = 0;
    int default_port = RTSP_DEFAULT_PORT;
    char real_challenge[64] = "";
    struct sockaddr_storage peer;
    socklen_t peer_len = sizeof(peer);

    if (pstRtspState->rtp_port_max < pstRtspState->rtp_port_min) {
        //av_log(s, AV_LOG_ERROR, "Invalid UDP port range, max port %d less than min port %d\n",
        //    rt->rtp_port_max, rt->rtp_port_min);
        return AVERROR(EINVAL);
    }

    if (!ff_network_init())
        return AVERROR(EIO);

    if (pstFmtCtx->max_delay < 0) /* Not set by the caller */
        pstFmtCtx->max_delay = pstFmtCtx->iformat ? DEFAULT_REORDERING_DELAY : 0;

    pstRtspState->control_transport = RTSP_MODE_PLAIN;
    if (pstRtspState->lower_transport_mask & (1 << RTSP_LOWER_TRANSPORT_HTTP)) {
        pstRtspState->lower_transport_mask = 1 << RTSP_LOWER_TRANSPORT_TCP;
        pstRtspState->control_transport = RTSP_MODE_TUNNEL;
    }
    /* Only pass through valid flags from here */
    pstRtspState->lower_transport_mask &= (1 << RTSP_LOWER_TRANSPORT_NB) - 1;

redirect:
    /* extract hostname and port */
    av_url_split(proto, sizeof(proto), auth, sizeof(auth), host, sizeof(host), &port, path, sizeof(path), 
        pstFmtCtx->url);

    if (!strcmp(proto, "rtsps")) {
        lower_rtsp_proto         = "tls";
        default_port             = RTSPS_DEFAULT_PORT;
        pstRtspState->lower_transport_mask = 1 << RTSP_LOWER_TRANSPORT_TCP;
    }

    if (*auth) {
        av_strlcpy(pstRtspState->auth, auth, sizeof(pstRtspState->auth));
    }
    if (port < 0)
        port = default_port;

    lower_transport_mask = pstRtspState->lower_transport_mask;

    if (!lower_transport_mask)
        lower_transport_mask = (1 << RTSP_LOWER_TRANSPORT_NB) - 1;

    if (pstFmtCtx->oformat) {
        /* Only UDP or TCP - UDP multicast isn't supported. */
        lower_transport_mask &= (1 << RTSP_LOWER_TRANSPORT_UDP) | (1 << RTSP_LOWER_TRANSPORT_TCP);
        if (!lower_transport_mask || pstRtspState->control_transport == RTSP_MODE_TUNNEL) {
            //av_log(s, AV_LOG_ERROR, "Unsupported lower transport method, "
            //                       "only UDP and TCP are supported for output.\n");
            err = AVERROR(EINVAL);
            goto fail;
        }
    }

    /* Construct the URI used in request; this is similar to s->url,
     * but with authentication credentials removed and RTSP specific options
     * stripped out. */
    ff_url_join(pstRtspState->control_uri, sizeof(pstRtspState->control_uri), proto, NULL,
                host, port, "%s", path);

    if (pstRtspState->control_transport == RTSP_MODE_TUNNEL) {
        /* set up initial handshake for tunneling */
        char httpname[1024];
        char sessioncookie[17];
        char headers[1024];

        ff_url_join(httpname, sizeof(httpname), "http", auth, host, port, "%s", path);
        //snprintf(sessioncookie, sizeof(sessioncookie), "%08x%08x",
        //         av_get_random_seed(), av_get_random_seed());

        /* GET requests */
        if (ffurl_alloc(&pstRtspState->rtsp_hd, httpname, AVIO_FLAG_READ, &pstFmtCtx->interrupt_callback) < 0) {
            err = AVERROR(EIO);
            goto fail;
        }

        /* generate GET headers */
        snprintf(headers, sizeof(headers),
                 "x-sessioncookie: %s\r\n"
                 "Accept: application/x-rtsp-tunnelled\r\n"
                 "Pragma: no-cache\r\n"
                 "Cache-Control: no-cache\r\n",
                 sessioncookie);
        av_opt_set(pstRtspState->rtsp_hd->pstPrivData, "headers", headers, 0);

        if (!pstRtspState->rtsp_hd->protocol_whitelist && pstFmtCtx->protocol_whitelist) {
            pstRtspState->rtsp_hd->protocol_whitelist = av_strdup(pstFmtCtx->protocol_whitelist);
            if (!pstRtspState->rtsp_hd->protocol_whitelist) {
                err = AVERROR(ENOMEM);
                goto fail;
            }
        }

        /* complete the connection */
        if (ffurl_connect(pstRtspState->rtsp_hd, NULL)) {
            err = AVERROR(EIO);
            goto fail;
        }

        /* POST requests */
        if (ffurl_alloc(&pstRtspState->rtsp_hd_out, httpname, AVIO_FLAG_WRITE, &pstFmtCtx->interrupt_callback) < 0 ) {
            err = AVERROR(EIO);
            goto fail;
        }

        /* generate POST headers */
        snprintf(headers, sizeof(headers),
                 "x-sessioncookie: %s\r\n"
                 "Content-Type: application/x-rtsp-tunnelled\r\n"
                 "Pragma: no-cache\r\n"
                 "Cache-Control: no-cache\r\n"
                 "Content-Length: 32767\r\n"
                 "Expires: Sun, 9 Jan 1972 00:00:00 GMT\r\n",
                 sessioncookie);
        av_opt_set(pstRtspState->rtsp_hd_out->pstPrivData, "headers", headers, 0);
        av_opt_set(pstRtspState->rtsp_hd_out->pstPrivData, "chunked_post", "0", 0);

        /* Initialize the authentication state for the POST session. The HTTP
         * protocol implementation doesn't properly handle multi-pass
         * authentication for POST requests, since it would require one of
         * the following:
         * - implementing Expect: 100-continue, which many HTTP servers
         *   don't support anyway, even less the RTSP servers that do HTTP
         *   tunneling
         * - sending the whole POST data until getting a 401 reply specifying
         *   what authentication method to use, then resending all that data
         * - waiting for potential 401 replies directly after sending the
         *   POST header (waiting for some unspecified time)
         * Therefore, we copy the full auth state, which works for both basic
         * and digest. (For digest, we would have to synchronize the nonce
         * count variable between the two sessions, if we'd do more requests
         * with the original session, though.)
         */
        ff_http_init_auth_state(pstRtspState->rtsp_hd_out, pstRtspState->rtsp_hd);

        /* complete the connection */
        if (ffurl_connect(pstRtspState->rtsp_hd_out, NULL)) {
            err = AVERROR(EIO);
            goto fail;
        }
    } 
    else {
        int ret;
        /* open the tcp connection */
        ff_url_join(tcpname, sizeof(tcpname), lower_rtsp_proto, NULL, host, port, "?timeout=%d", pstRtspState->stimeout);
        if ((ret = ffurl_open_whitelist(&pstRtspState->rtsp_hd, tcpname, AVIO_FLAG_READ_WRITE,
             &pstFmtCtx->interrupt_callback, NULL, pstFmtCtx->protocol_whitelist, pstFmtCtx->protocol_blacklist, NULL)) < 0) {
            err = ret;
            goto fail;
        }
        pstRtspState->rtsp_hd_out = pstRtspState->rtsp_hd;
    }
    
    pstRtspState->seq = 0;

    tcp_fd = ffurl_get_file_handle(pstRtspState->rtsp_hd);
    if (tcp_fd < 0) {
        err = tcp_fd;
        goto fail;
    }
    if (!getpeername(tcp_fd, (struct sockaddr*) &peer, &peer_len)) {
        getnameinfo((struct sockaddr*) &peer, peer_len, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
    }

    /* request options supported by the server; this also detects server type */
    for (pstRtspState->server_type = RTSP_SERVER_RTP;;) {
        cmd[0] = 0;
        if (pstRtspState->server_type == RTSP_SERVER_REAL)
            av_strlcat(cmd,
                       /*
                        * The following entries are required for proper
                        * streaming from a Realmedia server. They are
                        * interdependent in some way although we currently
                        * don't quite understand how. Values were copied
                        * from mplayer SVN r23589.
                        *   ClientChallenge is a 16-byte ID in hex
                        *   CompanyID is a 16-byte ID in base64
                        */
                       "ClientChallenge: 9e26d33f2984236010ef6253fb1887f7\r\n"
                       "PlayerStarttime: [28/03/2003:22:50:23 00:00]\r\n"
                       "CompanyID: KnKV4M4I/B2FjJ1TToLycw==\r\n"
                       "GUID: 00000000-0000-0000-0000-000000000000\r\n",
                       sizeof(cmd));
        ff_rtsp_send_cmd(s, "OPTIONS", rt->control_uri, cmd, reply, NULL);
        if (reply->status_code != RTSP_STATUS_OK) {
            err = ff_rtsp_averror(reply->status_code, AVERROR_INVALIDDATA);
            goto fail;
        }

        /* detect server type if not standard-compliant RTP */
        if (rt->server_type != RTSP_SERVER_REAL && reply->real_challenge[0]) {
            rt->server_type = RTSP_SERVER_REAL;
            continue;
        } else if (!av_strncasecmp(reply->server, "WMServer/", 9)) {
            rt->server_type = RTSP_SERVER_WMS;
        } else if (rt->server_type == RTSP_SERVER_REAL)
            strcpy(real_challenge, reply->real_challenge);
        break;
    }

    if (CONFIG_RTSP_DEMUXER && s->iformat)
        err = ff_rtsp_setup_input_streams(s, reply);
    else if (CONFIG_RTSP_MUXER)
        err = ff_rtsp_setup_output_streams(s, host);
    else
        av_assert0(0);
    if (err)
        goto fail;

    do {
        int lower_transport = ff_log2_tab[lower_transport_mask &
                                  ~(lower_transport_mask - 1)];

        if ((lower_transport_mask & (1 << RTSP_LOWER_TRANSPORT_TCP))
                && (rt->rtsp_flags & RTSP_FLAG_PREFER_TCP))
            lower_transport = RTSP_LOWER_TRANSPORT_TCP;

        err = ff_rtsp_make_setup_request(s, host, port, lower_transport,
                                 rt->server_type == RTSP_SERVER_REAL ?
                                     real_challenge : NULL);
        if (err < 0)
            goto fail;
        lower_transport_mask &= ~(1 << lower_transport);
        if (lower_transport_mask == 0 && err == 1) {
            err = AVERROR(EPROTONOSUPPORT);
            goto fail;
        }
    } while (err);

    rt->lower_transport_mask = lower_transport_mask;
    av_strlcpy(rt->real_challenge, real_challenge, sizeof(rt->real_challenge));
    rt->state = RTSP_STATE_IDLE;
    rt->seek_timestamp = 0; /* default is to start stream at position zero */
    return 0;
 fail:
    ff_rtsp_close_streams(s);
    ff_rtsp_close_connections(s);
    if (reply->status_code >=300 && reply->status_code < 400 && s->iformat) {
        char *new_url = av_strdup(reply->location);
        if (!new_url) {
            err = AVERROR(ENOMEM);
            goto fail2;
        }
        ff_format_set_url(s, new_url);
        rt->session_id[0] = '\0';
        av_log(s, AV_LOG_INFO, "Status %d: Redirecting to %s\n",
               reply->status_code,
               s->url);
        goto redirect;
    }
 fail2:
    ff_network_close();
    return err;
}

static int rtsp_read_header(AVFormatContext *s)
{
    RTSPState *rt = s->priv_data;
    int ret;

    av_log(NULL, AV_LOG_INFO, "bruce >>> rt->initial_timeout = %d\n", rt->initial_timeout);
    if (rt->initial_timeout > 0)
        rt->rtsp_flags |= RTSP_FLAG_LISTEN;

    av_log(NULL, AV_LOG_INFO, "bruce >>> rt->rtsp_flags = %d\n", rt->rtsp_flags);
    av_log(NULL, AV_LOG_INFO, "bruce >>> before s->nb_streams = %d\n", s->nb_streams);
    if (rt->rtsp_flags & RTSP_FLAG_LISTEN) {
        ret = rtsp_listen(s);
        if (ret)
            return ret;
    } else {
        ret = ff_rtsp_connect(s);
        if (ret)
            return ret;

        av_log(NULL, AV_LOG_INFO, "bruce >>> after s->nb_streams = %d\n", s->nb_streams);
        rt->real_setup_cache = !s->nb_streams ? NULL :
            av_mallocz_array(s->nb_streams, 2 * sizeof(*rt->real_setup_cache));
        if (!rt->real_setup_cache && s->nb_streams)
            return AVERROR(ENOMEM);
        rt->real_setup = rt->real_setup_cache + s->nb_streams;

        av_log(NULL, AV_LOG_INFO, "bruce >>> rt->initial_pause = %d\n", rt->initial_pause);
        if (rt->initial_pause) {
            /* do not start immediately */
        } else {
            if ((ret = rtsp_read_play(s)) < 0) {
                ff_rtsp_close_streams(s);
                ff_rtsp_close_connections(s);
                return ret;
            }
        }
    }

    return 0;
}





AVInputFormat ff_rtp_demuxer = {
    .name           = "rtp",
    .long_name      = "RTP input",
    .priv_data_size = sizeof(RTSPState),
    .read_probe     = rtp_probe,
    .read_header    = rtp_read_header,
    .read_packet    = ff_rtsp_fetch_packet,
    .read_close     = sdp_read_close,
    .flags          = AVFMT_NOFILE,
    .priv_class     = &rtp_demuxer_class,
};

AVInputFormat ff_rtsp_demuxer = {
    .name           = "rtsp",
    .long_name      = "RTSP input",
    .priv_data_size = sizeof(RTSPState),
    .read_probe     = rtsp_probe,
    .read_header    = rtsp_read_header,
    .read_packet    = rtsp_read_packet,
    .read_close     = rtsp_read_close,
    .read_seek      = rtsp_read_seek,
    .flags          = AVFMT_NOFILE,
    .read_play      = rtsp_read_play,
    .read_pause     = rtsp_read_pause,
    .priv_class     = &rtsp_demuxer_class,
};

