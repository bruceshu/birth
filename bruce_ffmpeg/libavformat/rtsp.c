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

void ff_rtsp_close_streams(AVFormatContext *s)
{
    RTSPState *rt = s->priv_data;
    int i, j;
    RTSPStream *rtsp_st;

    ff_rtsp_undo_setup(s, 0);
    for (i = 0; i < rt->nb_rtsp_streams; i++) {
       rtsp_st = rt->rtsp_streams[i];
       if (rtsp_st) {
           if (rtsp_st->dynamic_handler && rtsp_st->dynamic_protocol_context) {
               if (rtsp_st->dynamic_handler->close)
                   rtsp_st->dynamic_handler->close(
                       rtsp_st->dynamic_protocol_context);
               av_free(rtsp_st->dynamic_protocol_context);
           }
           for (j = 0; j < rtsp_st->nb_include_source_addrs; j++)
               av_freep(&rtsp_st->include_source_addrs[j]);
           av_freep(&rtsp_st->include_source_addrs);
           for (j = 0; j < rtsp_st->nb_exclude_source_addrs; j++)
               av_freep(&rtsp_st->exclude_source_addrs[j]);
           av_freep(&rtsp_st->exclude_source_addrs);

           av_freep(&rtsp_st);
       }
    }
    av_freep(&rt->rtsp_streams);
    if (rt->asf_ctx) {
       avformat_close_input(&rt->asf_ctx);
    }
    if (CONFIG_RTPDEC && rt->ts)
       avpriv_mpegts_parse_close(rt->ts);
    av_freep(&rt->p);
    av_freep(&rt->recvbuf);
}

void ff_rtsp_close_connections(AVFormatContext *s)
{
    RTSPState *rt = s->priv_data;
    if (rt->rtsp_hd_out != rt->rtsp_hd) ffurl_close(rt->rtsp_hd_out);
    ffurl_close(rt->rtsp_hd);
    rt->rtsp_hd = rt->rtsp_hd_out = NULL;
}

static int ff_rtsp_averror(enum RTSPStatusCode status_code, int default_averror)
{
    return ff_http_averror(status_code, default_averror);
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

static void init_rtp_handler(RTPDynamicProtocolHandler *handler, RTSPStream *rtsp_st, AVStream *st)
{
    AVCodecParameters *par = st ? st->codecpar : NULL;
    if (!handler)
        return;
    if (par)
        par->codec_id          = handler->codec_id;
    rtsp_st->dynamic_handler = handler;
    if (st)
        st->need_parsing = handler->need_parsing;
    if (handler->priv_data_size) {
        rtsp_st->dynamic_protocol_context = av_mallocz(handler->priv_data_size);
        if (!rtsp_st->dynamic_protocol_context)
            rtsp_st->dynamic_handler = NULL;
    }
}

static void finalize_rtp_handler_init(AVFormatContext *s, RTSPStream *rtsp_st, AVStream *st)
{
    if (rtsp_st->dynamic_handler && rtsp_st->dynamic_handler->init) {
        int ret = rtsp_st->dynamic_handler->init(s, st ? st->index : -1,
                                                 rtsp_st->dynamic_protocol_context);
        if (ret < 0) {
            if (rtsp_st->dynamic_protocol_context) {
                if (rtsp_st->dynamic_handler->close)
                    rtsp_st->dynamic_handler->close(
                        rtsp_st->dynamic_protocol_context);
                av_free(rtsp_st->dynamic_protocol_context);
            }
            rtsp_st->dynamic_protocol_context = NULL;
            rtsp_st->dynamic_handler = NULL;
        }
    }
}


static int get_sockaddr(AVFormatContext *s, const char *buf, struct sockaddr_storage *sock)
{
    struct addrinfo hints = { 0 }, *ai = NULL;
    int ret;

    hints.ai_flags = AI_NUMERICHOST;
    if ((ret = getaddrinfo(buf, NULL, &hints, &ai))) {
        av_log(s, AV_LOG_ERROR, "getaddrinfo(%s): %s\n", buf, gai_strerror(ret));
        return -1;
    }
    
    memcpy(sock, ai->ai_addr, FFMIN(sizeof(*sock), ai->ai_addrlen));
    freeaddrinfo(ai);
    
    return 0;
}

static void copy_default_source_addrs(RTSPSource **addrs, int count, RTSPSource ***dest, int *dest_count)
{
    RTSPSource *rtsp_src, *rtsp_src2;
    int i;
    for (i = 0; i < count; i++) {
        rtsp_src = addrs[i];
        rtsp_src2 = av_malloc(sizeof(*rtsp_src2));
        if (!rtsp_src2)
            continue;
        
        memcpy(rtsp_src2, rtsp_src, sizeof(*rtsp_src));
        dynarray_add(dest, dest_count, rtsp_src2);
    }
}

static void parse_fmtp(AVFormatContext *s, RTSPState *rt, int payload_type, const char *line)
{
    int i;

    for (i = 0; i < rt->nb_rtsp_streams; i++) {
        RTSPStream *rtsp_st = rt->rtsp_streams[i];
        if (rtsp_st->sdp_payload_type == payload_type &&
            rtsp_st->dynamic_handler &&
            rtsp_st->dynamic_handler->parse_sdp_a_line) {
            rtsp_st->dynamic_handler->parse_sdp_a_line(s, i, rtsp_st->dynamic_protocol_context, line);
        }
    }
}

static int sdp_parse_rtpmap(AVFormatContext *s, AVStream *st, RTSPStream *rtsp_st, int payload_type, const char *p)
{
    AVCodecParameters *par = st->codecpar;
    char buf[256];
    int i;
    const AVCodecDescriptor *desc;
    const char *c_name;

    /* See if we can handle this kind of payload.
     * The space should normally not be there but some Real streams or
     * particular servers ("RealServer Version 6.1.3.970", see issue 1658)
     * have a trailing space. */
    get_word_sep(buf, sizeof(buf), "/ ", &p);
    if (payload_type < RTP_PT_PRIVATE) {
        /* We are in a standard case
         * (from http://www.iana.org/assignments/rtp-parameters). */
        par->codec_id = ff_rtp_codec_id(buf, par->codec_type);
    }

    if (par->codec_id == AV_CODEC_ID_NONE) {
        RTPDynamicProtocolHandler *handler = ff_rtp_handler_find_by_name(buf, par->codec_type);
        init_rtp_handler(handler, rtsp_st, st);
        /* If no dynamic handler was found, check with the list of standard
         * allocated types, if such a stream for some reason happens to
         * use a private payload type. This isn't handled in rtpdec.c, since
         * the format name from the rtpmap line never is passed into rtpdec. */
        if (!rtsp_st->dynamic_handler)
            par->codec_id = ff_rtp_codec_id(buf, par->codec_type);
    }

    desc = avcodec_descriptor_get(par->codec_id);
    if (desc && desc->name)
        c_name = desc->name;
    else
        c_name = "(null)";

    get_word_sep(buf, sizeof(buf), "/", &p);
    i = atoi(buf);
    switch (par->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            av_log(s, AV_LOG_DEBUG, "audio codec set to: %s\n", c_name);
            par->sample_rate = RTSP_DEFAULT_AUDIO_SAMPLERATE;
            par->channels = RTSP_DEFAULT_NB_AUDIO_CHANNELS;
            if (i > 0) {
                par->sample_rate = i;
                avpriv_set_pts_info(st, 32, 1, par->sample_rate);
                get_word_sep(buf, sizeof(buf), "/", &p);
                i = atoi(buf);
                if (i > 0)
                    par->channels = i;
            }
            av_log(s, AV_LOG_DEBUG, "audio samplerate set to: %i\n", par->sample_rate);
            av_log(s, AV_LOG_DEBUG, "audio channels set to: %i\n", par->channels);
            break;
        case AVMEDIA_TYPE_VIDEO:
            av_log(s, AV_LOG_DEBUG, "video codec set to: %s\n", c_name);
            if (i > 0)
                avpriv_set_pts_info(st, 32, 1, i);
            break;
        default:
            break;
    }
    finalize_rtp_handler_init(s, rtsp_st, st);
    return 0;
}

int ff_wms_parse_sdp_a_line(AVFormatContext *s, const char *p)
{
    int ret = 0;
    if (av_strstart(p, "pgmpu:data:application/vnd.ms.wms-hdr.asfv1;base64,", &p)) {
        AVIOContext pb = { 0 };
        RTSPState *rt = s->priv_data;
        AVDictionary *opts = NULL;
        int len = strlen(p) * 6 / 8;
        char *buf = av_mallocz(len);
        AVInputFormat *iformat;

        if (!buf)
            return AVERROR(ENOMEM);
        av_base64_decode(buf, p, len);

        if (rtp_asf_fix_header(buf, len) < 0)
            av_log(s, AV_LOG_ERROR,
                   "Failed to fix invalid RTSP-MS/ASF min_pktsize\n");
        init_packetizer(&pb, buf, len);
        if (rt->asf_ctx) {
            avformat_close_input(&rt->asf_ctx);
        }

        if (!(iformat = av_find_input_format("asf")))
            return AVERROR_DEMUXER_NOT_FOUND;

        rt->asf_ctx = avformat_alloc_context();
        if (!rt->asf_ctx) {
            av_free(buf);
            return AVERROR(ENOMEM);
        }
        rt->asf_ctx->pb      = &pb;
        av_dict_set(&opts, "no_resync_search", "1", 0);

        if ((ret = ff_copy_whiteblacklists(rt->asf_ctx, s)) < 0) {
            av_dict_free(&opts);
            return ret;
        }

        ret = avformat_open_input(&rt->asf_ctx, "", iformat, &opts);
        av_dict_free(&opts);
        if (ret < 0) {
            av_free(buf);
            return ret;
        }
        av_dict_copy(&s->metadata, rt->asf_ctx->metadata, 0);
        rt->asf_pb_pos = avio_tell(&pb);
        av_free(buf);
        rt->asf_ctx->pb = NULL;
    }
    return ret;
}


static void rtsp_parse_range_npt(const char *p, int64_t *start, int64_t *end)
{
    char buf[256];

    p += strspn(p, SPACE_CHARS);
    if (!av_stristart(p, "npt=", &p))
        return;

    *start = AV_NOPTS_VALUE;
    *end = AV_NOPTS_VALUE;

    get_word_sep(buf, sizeof(buf), "-", &p);
    if (av_parse_time(start, buf, 1) < 0)
        return;
    if (*p == '-') {
        p++;
        get_word_sep(buf, sizeof(buf), "-", &p);
        if (av_parse_time(end, buf, 1) < 0)
            av_log(NULL, AV_LOG_DEBUG, "Failed to parse interval end specification '%s'\n", buf);
    }
}

static void sdp_parse_line(AVFormatContext *pstFmtCtx, SDPParseState *pstSdpState, int letter, const char *buf)
{
    RTSPState *pstRtspState = pstFmtCtx->priv_data;
    AVStream *pstStream;
    RTSPStream *pstRtspStream;
    RTSPSource *pstRtspSource;
    
    enum AVMediaType codec_type;
    char buf1[64], st_type[64];
    const char *p = buf;
    int payload_type;
    int ttl;
    struct sockaddr_storage sdp_ip;

    if (pstSdpState->skip_media && letter != 'm') {
        return;
    }

    switch (letter) {
        case 'c':
            get_word(buf1, sizeof(buf1), &p);
            if (strcmp(buf1, "IN") != 0) {
                return;
            }
            
            get_word(buf1, sizeof(buf1), &p);
            if (strcmp(buf1, "IP4") && strcmp(buf1, "IP6")) {
                return;
            }
            
            get_word_sep(buf1, sizeof(buf1), "/", &p);
            if (get_sockaddr(pstFmtCtx, buf1, &sdp_ip)) {
                return;
            }
            
            ttl = 16;
            if (*p == '/') {
                p++;
                get_word_sep(buf1, sizeof(buf1), "/", &p);
                ttl = atoi(buf1);
            }
            
            if (pstFmtCtx->nb_streams == 0) {
                pstSdpState->default_ip = sdp_ip;
                pstSdpState->default_ttl = ttl;
            } else {
                pstRtspStream = pstRtspState->rtsp_streams[pstRtspState->nb_rtsp_streams - 1];
                pstRtspStream->sdp_ip = sdp_ip;
                pstRtspStream->sdp_ttl = ttl;
            }
            
            break;
        case 's':
            av_dict_set(&pstFmtCtx->metadata, "title", p, 0);
            break;
        case 'i':
            if (pstFmtCtx->nb_streams == 0) {
                av_dict_set(&pstFmtCtx->metadata, "comment", p, 0);
                break;
            }
            break;
        case 'm':
            pstSdpState->skip_media  = 0;
            pstSdpState->seen_fmtp   = 0;
            pstSdpState->seen_rtpmap = 0;
            codec_type = AVMEDIA_TYPE_UNKNOWN;
            
            get_word(st_type, sizeof(st_type), &p);
            if (!strcmp(st_type, "audio")) {
                codec_type = AVMEDIA_TYPE_AUDIO;
            } else if (!strcmp(st_type, "video")) {
                codec_type = AVMEDIA_TYPE_VIDEO;
            } else if (!strcmp(st_type, "application")) {
                codec_type = AVMEDIA_TYPE_DATA;
            } else if (!strcmp(st_type, "text")) {
                codec_type = AVMEDIA_TYPE_SUBTITLE;
            }

            if (codec_type == AVMEDIA_TYPE_UNKNOWN || !(pstRtspState->media_type_mask & (1 << codec_type))) {
                pstSdpState->skip_media = 1;
                return;
            }
            
            pstRtspStream = av_mallocz(sizeof(RTSPStream));
            if (!pstRtspStream) {
                return;
            }
            memset(pstRtspStream, 0, sizeof(RTSPStream));
            
            pstRtspStream->stream_index = -1;
            dynarray_add(&pstRtspState->rtsp_streams, &pstRtspState->nb_rtsp_streams, pstRtspStream);

            pstRtspStream->sdp_ip = pstSdpState->default_ip;
            pstRtspStream->sdp_ttl = pstSdpState->default_ttl;

            copy_default_source_addrs(pstSdpState->default_include_source_addrs, pstSdpState->nb_default_include_source_addrs,
                                      &pstRtspStream->include_source_addrs, &pstRtspStream->nb_include_source_addrs);
            copy_default_source_addrs(pstSdpState->default_exclude_source_addrs, pstSdpState->nb_default_exclude_source_addrs,
                                      &pstRtspStream->exclude_source_addrs, &pstRtspStream->nb_exclude_source_addrs);

            get_word(buf1, sizeof(buf1), &p); /* port */
            pstRtspStream->sdp_port = atoi(buf1);

            get_word(buf1, sizeof(buf1), &p); /* protocol */
            if (!strcmp(buf1, "udp")) {
                pstRtspState->transport = RTSP_TRANSPORT_RAW;
            }
            else if (strstr(buf1, "/AVPF") || strstr(buf1, "/SAVPF")) {
                pstRtspStream->feedback = 1;
            }

            get_word(buf1, sizeof(buf1), &p); /* format list */
            pstRtspStream->sdp_payload_type = atoi(buf1);

            if (!strcmp(ff_rtp_enc_name(pstRtspStream->sdp_payload_type), "MP2T")) {
                if (pstRtspState->transport == RTSP_TRANSPORT_RAW) {
                    if (CONFIG_RTPDEC && !pstRtspState->ts) {
                        pstRtspState->ts = avpriv_mpegts_parse_open(s);
                    }
                } else {
                    RTPDynamicProtocolHandler *handler;
                    handler = ff_rtp_handler_find_by_id(pstRtspStream->sdp_payload_type, AVMEDIA_TYPE_DATA);
                    init_rtp_handler(handler, pstRtspStream, NULL);
                    finalize_rtp_handler_init(pstFmtCtx, pstRtspStream, NULL);
                }
            } else if (pstRtspState->server_type == RTSP_SERVER_WMS && codec_type == AVMEDIA_TYPE_DATA) {
                /* RTX stream, a stream that carries all the other actual audio/video streams. Don't expose this to the callers. */
            } else {
                pstStream = avformat_new_stream(pstFmtCtx, NULL);
                if (!pstStream) {
                    return;
                }
                
                pstStream->id = pstRtspState->nb_rtsp_streams - 1;
                pstRtspStream->stream_index = pstStream->index;
                pstStream->codecpar->codec_type = codec_type;
                if (pstRtspStream->sdp_payload_type < RTP_PT_PRIVATE) {
                    RTPDynamicProtocolHandler *handler;
                    ff_rtp_get_codec_info(pstStream->codecpar, pstRtspStream->sdp_payload_type);
                    if (pstStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && pstStream->codecpar->sample_rate > 0) {
                        avpriv_set_pts_info(pstStream, 32, 1, pstStream->codecpar->sample_rate);
                    }
                    
                    handler = ff_rtp_handler_find_by_id(pstRtspStream->sdp_payload_type, pstStream->codecpar->codec_type);
                    init_rtp_handler(handler, pstRtspStream, pstStream);
                    finalize_rtp_handler_init(pstFmtCtx, pstRtspStream, pstStream);
                }
                if (pstRtspState->default_lang[0])
                    av_dict_set(&pstStream->metadata, "language", pstRtspState->default_lang, 0);
            }

            av_strlcpy(pstRtspStream->control_url, pstRtspState->control_uri, sizeof(pstRtspStream->control_url));
            break;
        case 'a':
            if (av_strstart(p, "control:", &p)) {
                if (pstFmtCtx->nb_streams == 0) {
                    if (!strncmp(p, "rtsp://", 7)) {
                        av_strlcpy(pstRtspState->control_uri, p, sizeof(pstRtspState->control_uri));
                    }
                } else {
                    char proto[32];
                    pstRtspStream = pstRtspState->rtsp_streams[pstRtspState->nb_rtsp_streams - 1];

                    av_url_split(proto, sizeof(proto), NULL, 0, NULL, 0, NULL, NULL, 0, p);
                    if (proto[0] == '\0') {
                        if (pstRtspStream->control_url[strlen(pstRtspStream->control_url)-1]!='/') {
                            av_strlcat(pstRtspStream->control_url, "/", sizeof(pstRtspStream->control_url));
                        }
                        av_strlcat(pstRtspStream->control_url, p, sizeof(pstRtspStream->control_url));
                    } else {
                        av_strlcpy(pstRtspStream->control_url, p, sizeof(pstRtspStream->control_url));
                    }
                }
            } 
            else if (av_strstart(p, "rtpmap:", &p) && pstFmtCtx->nb_streams > 0) {
                /* NOTE: rtpmap is only supported AFTER the 'm=' tag */
                get_word(buf1, sizeof(buf1), &p);
                payload_type = atoi(buf1);
                pstRtspStream = pstRtspState->rtsp_streams[pstRtspState->nb_rtsp_streams - 1];
                if (pstRtspStream->stream_index >= 0) {
                    pstStream = pstFmtCtx->streams[pstRtspStream->stream_index];
                    sdp_parse_rtpmap(pstFmtCtx, pstStream, pstRtspStream, payload_type, p);
                }
                
                pstSdpState->seen_rtpmap = 1;
                if (pstSdpState->seen_fmtp) {
                    parse_fmtp(pstFmtCtx, pstRtspState, payload_type, pstSdpState->delayed_fmtp);
                }
            } else if (av_strstart(p, "fmtp:", &p) || av_strstart(p, "framesize:", &p)) {
                get_word(buf1, sizeof(buf1), &p);
                payload_type = atoi(buf1);
                if (pstSdpState->seen_rtpmap) {
                    parse_fmtp(pstFmtCtx, pstRtspState, payload_type, buf);
                } else {
                    pstSdpState->seen_fmtp = 1;
                    av_strlcpy(pstSdpState->delayed_fmtp, buf, sizeof(pstSdpState->delayed_fmtp));
                }
            } else if (av_strstart(p, "ssrc:", &p) && pstFmtCtx->nb_streams > 0) {
                pstRtspStream = pstRtspState->rtsp_streams[pstRtspState->nb_rtsp_streams - 1];
                get_word(buf1, sizeof(buf1), &p);
                pstRtspStream->ssrc = strtoll(buf1, NULL, 10);
            } else if (av_strstart(p, "range:", &p)) {
                int64_t start, end;

                rtsp_parse_range_npt(p, &start, &end);
                pstFmtCtx->start_time = start;
                /* AV_NOPTS_VALUE means live broadcast (and can't seek) */
                pstFmtCtx->duration   = (end == AV_NOPTS_VALUE) ? AV_NOPTS_VALUE : end - start;
            } else if (av_strstart(p, "lang:", &p)) {
                if (pstFmtCtx->nb_streams > 0) {
                    get_word(buf1, sizeof(buf1), &p);
                    pstRtspStream = pstRtspState->rtsp_streams[pstRtspState->nb_rtsp_streams - 1];
                    if (pstRtspStream->stream_index >= 0) {
                        pstStream = pstFmtCtx->streams[pstRtspStream->stream_index];
                        av_dict_set(&pstStream->metadata, "language", buf1, 0);
                    }
                } else
                    get_word(pstRtspState->default_lang, sizeof(pstRtspState->default_lang), &p);
            } else if (av_strstart(p, "IsRealDataType:integer;",&p)) {
                if (atoi(p) == 1) {
                    pstRtspState->transport = RTSP_TRANSPORT_RDT;
                }
            } else if (av_strstart(p, "SampleRate:integer;", &p) && pstFmtCtx->nb_streams > 0) {
                pstStream = pstFmtCtx->streams[pstFmtCtx->nb_streams - 1];
                pstStream->codecpar->sample_rate = atoi(p);
            } else if (av_strstart(p, "crypto:", &p) && pstFmtCtx->nb_streams > 0) {
                // RFC 4568
                pstRtspStream = pstRtspState->rtsp_streams[pstRtspState->nb_rtsp_streams - 1];
                get_word(buf1, sizeof(buf1), &p); // ignore tag
                get_word(pstRtspStream->crypto_suite, sizeof(pstRtspStream->crypto_suite), &p);
                p += strspn(p, SPACE_CHARS);
                
                if (av_strstart(p, "inline:", &p)) {
                    get_word(pstRtspStream->crypto_params, sizeof(pstRtspStream->crypto_params), &p);
                }
            } else if (av_strstart(p, "source-filter:", &p)) {
                int exclude = 0;
                get_word(buf1, sizeof(buf1), &p);
                if (strcmp(buf1, "incl") && strcmp(buf1, "excl")) {
                    return;
                }
                exclude = !strcmp(buf1, "excl");

                get_word(buf1, sizeof(buf1), &p);
                if (strcmp(buf1, "IN") != 0) {
                    return;
                }
                
                get_word(buf1, sizeof(buf1), &p);
                if (strcmp(buf1, "IP4") && strcmp(buf1, "IP6") && strcmp(buf1, "*")) {
                    return;
                }
                
                get_word(buf1, sizeof(buf1), &p);
                while (*p != '\0') {
                    pstRtspSource = av_mallocz(sizeof(RTSPSource));
                    if (!pstRtspSource)
                        return;
                    memset(pstRtspSource, 0, sizeof(RTSPSource));
                    
                    get_word(pstRtspSource->addr, sizeof(pstRtspSource->addr), &p);
                    if (exclude) {
                        if (pstFmtCtx->nb_streams == 0) {
                            dynarray_add(&pstSdpState->default_exclude_source_addrs, &pstSdpState->nb_default_exclude_source_addrs, pstRtspSource);
                        } else {
                            pstRtspStream = pstRtspState->rtsp_streams[pstRtspState->nb_rtsp_streams - 1];
                            dynarray_add(&pstRtspStream->exclude_source_addrs, &pstRtspStream->nb_exclude_source_addrs, pstRtspSource);
                        }
                    } else {
                        if (pstFmtCtx->nb_streams == 0) {
                            dynarray_add(&pstSdpState->default_include_source_addrs, &pstSdpState->nb_default_include_source_addrs, pstRtspSource);
                        } else {
                            pstRtspStream = pstRtspState->rtsp_streams[pstRtspState->nb_rtsp_streams - 1];
                            dynarray_add(&pstRtspStream->include_source_addrs, &pstRtspStream->nb_include_source_addrs, pstRtspSource);
                        }
                    }
                }
            }
            else {
                if (pstRtspState->server_type == RTSP_SERVER_WMS)
                    ff_wms_parse_sdp_a_line(s, p);
                if (s->nb_streams > 0) {
                    rtsp_st = rt->rtsp_streams[rt->nb_rtsp_streams - 1];

                    if (rt->server_type == RTSP_SERVER_REAL)
                        ff_real_parse_sdp_a_line(s, rtsp_st->stream_index, p);

                    if (rtsp_st->dynamic_handler &&
                        rtsp_st->dynamic_handler->parse_sdp_a_line)
                        rtsp_st->dynamic_handler->parse_sdp_a_line(s,
                            rtsp_st->stream_index,
                            rtsp_st->dynamic_protocol_context, buf);
                }
            }
            break;
    }
}


int ff_sdp_parse(AVFormatContext *pstFmtCtx, const char *content)
{
    RTSPState *pstRtspState = pstFmtCtx->priv_data;
    SDPParseState sdp_parse_state = { { 0 } }, *pstSdpState = &sdp_parse_state;
    char buf[16384], *q;
    const char *p = content;
    int letter, i;

    for (;;) {
        p += strspn(p, SPACE_CHARS);
        
        letter = *p;
        if (letter == '\0') {
            break;
        }
        
        p++;
        if (*p != '=') {
            goto next_line;
        }
        
        p++;
        q = buf;
        while (*p != '\n' && *p != '\r' && *p != '\0') {
            if ((q - buf) < sizeof(buf) - 1) {
                *q++ = *p;
            }
            
            p++;
        }
        
        *q = '\0';
        sdp_parse_line(pstSdpState, pstSdpState, letter, buf);
        
    next_line:
        while (*p != '\n' && *p != '\0') {
            p++;
        }
        
        if (*p == '\n') {
            p++;
        }
    }

    for (i = 0; i < s1->nb_default_include_source_addrs; i++)
        av_freep(&s1->default_include_source_addrs[i]);
    av_freep(&s1->default_include_source_addrs);
    
    for (i = 0; i < s1->nb_default_exclude_source_addrs; i++)
        av_freep(&s1->default_exclude_source_addrs[i]);
    av_freep(&s1->default_exclude_source_addrs);

    rt->p = av_malloc_array(rt->nb_rtsp_streams + 1, sizeof(struct pollfd) * 2);
    if (!rt->p) return AVERROR(ENOMEM);
    
    return 0;
}


int ff_rtsp_setup_input_streams(AVFormatContext *s, RTSPMessageHeader *reply)
{
    RTSPState *rt = s->priv_data;
    char cmd[1024];
    unsigned char *content = NULL;
    int ret;

    /* describe the stream */
    snprintf(cmd, sizeof(cmd), "Accept: application/sdp\r\n");
    
    if (rt->server_type == RTSP_SERVER_REAL) {
        /**
         * The Require: attribute is needed for proper streaming from Realmedia servers.
         */
        av_strlcat(cmd, "Require: com.real.retain-entity-for-setup\r\n", sizeof(cmd));
    }
    
    ff_rtsp_send_cmd(s, "DESCRIBE", rt->control_uri, cmd, reply, &content);
    if (reply->status_code != RTSP_STATUS_OK) {
        av_freep(&content);
        return ff_rtsp_averror(reply->status_code, AVERROR_INVALIDDATA);
    }
    
    if (!content) {
        return AVERROR_INVALIDDATA;
    }

    ret = ff_sdp_parse(s, (const char *)content);
    av_freep(&content);
    if (ret < 0)
        return ret;

    return 0;
}


int ff_rtsp_connect(AVFormatContext *pstFmtCtx)
{
    RTSPState *pstRtspState = pstFmtCtx->priv_data;
    RTSPMessageHeader reply1 = {0}, *reply = &reply1;
    char proto[128], host[1024], path[1024];
    char tcpname[1024], cmd[2048], auth[128];
    char real_challenge[64] = "";
    const char *lower_rtsp_proto = "tcp";
    int ret;
    int port, err, tcp_fd;
    int lower_transport_mask = 0;
    int default_port = RTSP_DEFAULT_PORT;
    struct sockaddr_storage peer;
    socklen_t peer_len = sizeof(peer);

    if (pstRtspState->rtp_port_max < pstRtspState->rtp_port_min) {
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
    pstRtspState->lower_transport_mask &= (1 << RTSP_LOWER_TRANSPORT_NB) - 1;

redirect:
    av_url_split(proto, sizeof(proto), auth, sizeof(auth), host, sizeof(host), &port, path, sizeof(path), pstFmtCtx->url);

    if (!strcmp(proto, "rtsps")) {
        lower_rtsp_proto = "tls";
        default_port = RTSPS_DEFAULT_PORT;
        pstRtspState->lower_transport_mask = 1 << RTSP_LOWER_TRANSPORT_TCP;
    }

    if (*auth) {
        av_strlcpy(pstRtspState->auth, auth, sizeof(pstRtspState->auth));
    }
    
    if (port < 0) {
        port = default_port;
    }

    lower_transport_mask = pstRtspState->lower_transport_mask;
    if (!lower_transport_mask) {
        lower_transport_mask = (1 << RTSP_LOWER_TRANSPORT_NB) - 1;
    }

    if (pstFmtCtx->oformat) {
        /* Only UDP or TCP - UDP multicast isn't supported. */
        lower_transport_mask &= (1 << RTSP_LOWER_TRANSPORT_UDP) | (1 << RTSP_LOWER_TRANSPORT_TCP);
        if (!lower_transport_mask || pstRtspState->control_transport == RTSP_MODE_TUNNEL) {
            err = AVERROR(EINVAL);
            goto fail;
        }
    }

    ff_url_join(pstRtspState->control_uri, sizeof(pstRtspState->control_uri), proto, NULL, host, port, "%s", path);

#if 0
    if (pstRtspState->control_transport == RTSP_MODE_TUNNEL) {
        char httpname[1024];
        char sessioncookie[17];
        char headers[1024];

        ff_url_join(httpname, sizeof(httpname), "http", auth, host, port, "%s", path);
        snprintf(sessioncookie, sizeof(sessioncookie), "%08x%08x", av_get_random_seed(), av_get_random_seed());

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
    else
#endif

    {
        /* open the tcp connection */
        ff_url_join(tcpname, sizeof(tcpname), lower_rtsp_proto, NULL, host, port, "?timeout=%d", pstRtspState->stimeout);
        if ((ret = ffurl_open_whitelist(&pstRtspState->rtsp_hd, tcpname, AVIO_FLAG_READ_WRITE,
             &pstFmtCtx->interrupt_callback, NULL, pstFmtCtx->protocol_whitelist, pstFmtCtx->protocol_blacklist, NULL)) < 0) {
            err = ret;
            goto fail;
        }
        pstRtspState->rtsp_hd_out = pstRtspState->rtsp_hd;
    }
    
    tcp_fd = ffurl_get_file_handle(pstRtspState->rtsp_hd);
    if (tcp_fd < 0) {
        err = tcp_fd;
        goto fail;
    }
    if (!getpeername(tcp_fd, (struct sockaddr*) &peer, &peer_len)) {
        getnameinfo((struct sockaddr*) &peer, peer_len, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
    }

    pstRtspState->seq = 0;
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
        ff_rtsp_send_cmd(pstFmtCtx, "OPTIONS", pstRtspState->control_uri, cmd, reply, NULL);
        if (reply->status_code != RTSP_STATUS_OK) {
            err = ff_rtsp_averror(reply->status_code, AVERROR_INVALIDDATA);
            goto fail;
        }

        /* detect server type if not standard-compliant RTP */
        if (pstRtspState->server_type != RTSP_SERVER_REAL && reply->real_challenge[0]) {
            pstRtspState->server_type = RTSP_SERVER_REAL;
            continue;
        } else if (!av_strncasecmp(reply->server, "WMServer/", 9)) {
            pstRtspState->server_type = RTSP_SERVER_WMS;
        } else if (pstRtspState->server_type == RTSP_SERVER_REAL) {
            strcpy(real_challenge, reply->real_challenge);
        }
        
        break;
    }

    if (CONFIG_RTSP_DEMUXER && pstFmtCtx->iformat)
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

static int rtsp_read_play(AVFormatContext *s)
{
    RTSPState *rt = s->priv_data;
    RTSPMessageHeader reply1, *reply = &reply1;
    int i;
    char cmd[1024];

    av_log(s, AV_LOG_DEBUG, "hello state=%d\n", rt->state);
    rt->nb_byes = 0;

    av_log(NULL, AV_LOG_INFO, "bruce >>> rt->lower_transport = %d\n", rt->lower_transport);
    if (rt->lower_transport == RTSP_LOWER_TRANSPORT_UDP) {
        for (i = 0; i < rt->nb_rtsp_streams; i++) {
            RTSPStream *rtsp_st = rt->rtsp_streams[i];
            /* Try to initialize the connection state in a
             * potential NAT router by sending dummy packets.
             * RTP/RTCP dummy packets are used for RDT, too.
             */
            if (rtsp_st->rtp_handle && !(rt->server_type == RTSP_SERVER_WMS && i > 1))
                ff_rtp_send_punch_packets(rtsp_st->rtp_handle);
        }
    }
    
    if (!(rt->server_type == RTSP_SERVER_REAL && rt->need_subscription)) {
        if (rt->transport == RTSP_TRANSPORT_RTP) {
            for (i = 0; i < rt->nb_rtsp_streams; i++) {
                RTSPStream *rtsp_st = rt->rtsp_streams[i];
                RTPDemuxContext *rtpctx = rtsp_st->transport_priv;
                if (!rtpctx)
                    continue;
                ff_rtp_reset_packet_queue(rtpctx);
                rtpctx->last_rtcp_ntp_time  = AV_NOPTS_VALUE;
                rtpctx->first_rtcp_ntp_time = AV_NOPTS_VALUE;
                rtpctx->base_timestamp      = 0;
                rtpctx->timestamp           = 0;
                rtpctx->unwrapped_timestamp = 0;
                rtpctx->rtcp_ts_offset      = 0;
            }
        }
        if (rt->state == RTSP_STATE_PAUSED) {
            cmd[0] = 0;
        } else {
            snprintf(cmd, sizeof(cmd),
                     "Range: npt=%"PRId64".%03"PRId64"-\r\n",
                     rt->seek_timestamp / AV_TIME_BASE,
                     rt->seek_timestamp / (AV_TIME_BASE / 1000) % 1000);
        }
        ff_rtsp_send_cmd(s, "PLAY", rt->control_uri, cmd, reply, NULL);
        if (reply->status_code != RTSP_STATUS_OK) {
            return ff_rtsp_averror(reply->status_code, -1);
        }

        av_log(NULL, AV_LOG_INFO, "bruce >>> reply->range_start = %"PRId64"\n", reply->range_start);
        if (rt->transport == RTSP_TRANSPORT_RTP && reply->range_start != AV_NOPTS_VALUE) {
            for (i = 0; i < rt->nb_rtsp_streams; i++) {
                RTSPStream *rtsp_st = rt->rtsp_streams[i];
                RTPDemuxContext *rtpctx = rtsp_st->transport_priv;
                AVStream *st = NULL;
                if (!rtpctx || rtsp_st->stream_index < 0)
                    continue;

                st = s->streams[rtsp_st->stream_index];
                rtpctx->range_start_offset = av_rescale_q(reply->range_start, AV_TIME_BASE_Q, st->time_base);
            }
        }
    }
    
    rt->state = RTSP_STATE_STREAMING;
    return 0;
}


static int rtsp_read_header(AVFormatContext *pstFmtCtx)
{
    RTSPState *pstRtspState = pstFmtCtx->priv_data;
    int ret;

    if (pstRtspState->initial_timeout > 0) {
        ret = rtsp_listen(pstFmtCtx);
        if (ret) {
            return ret;
        }
    } else {
        ret = ff_rtsp_connect(pstFmtCtx);
        if (ret) {
            return ret;
        }

        pstRtspState->real_setup_cache = !pstFmtCtx->nb_streams ? NULL : av_mallocz_array(pstFmtCtx->nb_streams, 2 * sizeof(*pstRtspState->real_setup_cache));
        if (!pstRtspState->real_setup_cache && pstFmtCtx->nb_streams) {
            return AVERROR(ENOMEM);
        }
        pstRtspState->real_setup = pstRtspState->real_setup_cache + pstFmtCtx->nb_streams;

        if (pstRtspState->initial_pause) {
            /* do not start immediately */
        } else {
            ret = rtsp_read_play(pstFmtCtx);
            if (ret < 0) {
                ff_rtsp_close_streams(pstFmtCtx);
                ff_rtsp_close_connections(pstFmtCtx);
                return ret;
            }
        }
    }

    return 0;
}

int ff_rtsp_fetch_packet(AVFormatContext *pstFmtCtx, AVPacket *pkt)
{
    RTSPState *pstRtspState = pstFmtCtx->priv_data;
    int ret, len;
    RTSPStream *rtsp_st, *first_queue_st = NULL;
    int64_t wait_end = 0;

    if (pstRtspState->nb_byes == pstRtspState->nb_rtsp_streams)
        return AVERROR_EOF;

    /* get next frames from the same RTP packet */
    if (pstRtspState->cur_transport_priv) {
#if 0
        if (pstRtspState->transport == RTSP_TRANSPORT_RDT) {
            ret = ff_rdt_parse_packet(pstRtspState->cur_transport_priv, pkt, NULL, 0);
        } else 
#endif
        if (pstRtspState->transport == RTSP_TRANSPORT_RTP) {
            ret = ff_rtp_parse_packet(pstRtspState->cur_transport_priv, pkt, NULL, 0);
        } 
#if 0
        else if (CONFIG_RTPDEC && pstRtspState->ts) {
            ret = avpriv_mpegts_parse_packet(rt->ts, pkt, rt->recvbuf + rt->recvbuf_pos, rt->recvbuf_len - rt->recvbuf_pos);
            if (ret >= 0) {
                rt->recvbuf_pos += ret;
                ret = rt->recvbuf_pos < rt->recvbuf_len;
            }
        }
#endif
        else {
            ret = -1;
        }
        
        if (ret == 0) {
            rt->cur_transport_priv = NULL;
            return 0;
        } else if (ret == 1) {
            return 0;
        } else
            rt->cur_transport_priv = NULL;
    }

redo:
    if (rt->transport == RTSP_TRANSPORT_RTP) {
        int i;
        int64_t first_queue_time = 0;
        for (i = 0; i < rt->nb_rtsp_streams; i++) {
            RTPDemuxContext *rtpctx = rt->rtsp_streams[i]->transport_priv;
            int64_t queue_time;
            if (!rtpctx)
                continue;
            queue_time = ff_rtp_queued_packet_time(rtpctx);
            if (queue_time && (queue_time - first_queue_time < 0 || !first_queue_time)) {
                first_queue_time = queue_time;
                first_queue_st   = rt->rtsp_streams[i];
            }
        }
        if (first_queue_time) {
            wait_end = first_queue_time + s->max_delay;
        } else {
            wait_end = 0;
            first_queue_st = NULL;
        }
    }

    /* read next RTP packet */
    if (!rt->recvbuf) {
        rt->recvbuf = av_malloc(RECVBUF_SIZE);
        if (!rt->recvbuf)
            return AVERROR(ENOMEM);
    }

    switch(rt->lower_transport) {
#if CONFIG_RTSP_DEMUXER
        case RTSP_LOWER_TRANSPORT_TCP:
            len = ff_rtsp_tcp_read_packet(s, &rtsp_st, rt->recvbuf, RECVBUF_SIZE);
            break;
#endif
        case RTSP_LOWER_TRANSPORT_UDP:
        case RTSP_LOWER_TRANSPORT_UDP_MULTICAST:
            len = udp_read_packet(s, &rtsp_st, rt->recvbuf, RECVBUF_SIZE, wait_end);
            if (len > 0 && rtsp_st->transport_priv && rt->transport == RTSP_TRANSPORT_RTP)
                ff_rtp_check_and_send_back_rr(rtsp_st->transport_priv, rtsp_st->rtp_handle, NULL, len);
            break;
        case RTSP_LOWER_TRANSPORT_CUSTOM:
            if (first_queue_st && rt->transport == RTSP_TRANSPORT_RTP &&
                wait_end && wait_end < av_gettime_relative())
                len = AVERROR(EAGAIN);
            else
                len = ffio_read_partial(s->pb, rt->recvbuf, RECVBUF_SIZE);
            len = pick_stream(s, &rtsp_st, rt->recvbuf, len);
            if (len > 0 && rtsp_st->transport_priv && rt->transport == RTSP_TRANSPORT_RTP)
                ff_rtp_check_and_send_back_rr(rtsp_st->transport_priv, NULL, s->pb, len);
            break;
            
       default:
            break;
    }
    
    if (len == AVERROR(EAGAIN) && first_queue_st && rt->transport == RTSP_TRANSPORT_RTP) {
        av_log(s, AV_LOG_WARNING, "max delay reached. need to consume packet\n");
        rtsp_st = first_queue_st;
        ret = ff_rtp_parse_packet(rtsp_st->transport_priv, pkt, NULL, 0);
        goto end;
    }
    
    if (len < 0)
        return len;
    if (len == 0)
        return AVERROR_EOF;
    
    if (rt->transport == RTSP_TRANSPORT_RDT) {
        ret = ff_rdt_parse_packet(rtsp_st->transport_priv, pkt, &rt->recvbuf, len);
    } else if (rt->transport == RTSP_TRANSPORT_RTP) {
        ret = ff_rtp_parse_packet(rtsp_st->transport_priv, pkt, &rt->recvbuf, len);
        if (rtsp_st->feedback) {
            AVIOContext *pb = NULL;
            if (rt->lower_transport == RTSP_LOWER_TRANSPORT_CUSTOM)
                pb = s->pb;
            ff_rtp_send_rtcp_feedback(rtsp_st->transport_priv, rtsp_st->rtp_handle, pb);
        }
        if (ret < 0) {
            /* Either bad packet, or a RTCP packet. Check if the
             * first_rtcp_ntp_time field was initialized. */
            RTPDemuxContext *rtpctx = rtsp_st->transport_priv;
            if (rtpctx->first_rtcp_ntp_time != AV_NOPTS_VALUE) {
                /* first_rtcp_ntp_time has been initialized for this stream,
                 * copy the same value to all other uninitialized streams,
                 * in order to map their timestamp origin to the same ntp time
                 * as this one. */
                int i;
                AVStream *st = NULL;
                if (rtsp_st->stream_index >= 0)
                    st = s->streams[rtsp_st->stream_index];
                for (i = 0; i < rt->nb_rtsp_streams; i++) {
                    RTPDemuxContext *rtpctx2 = rt->rtsp_streams[i]->transport_priv;
                    AVStream *st2 = NULL;
                    if (rt->rtsp_streams[i]->stream_index >= 0)
                        st2 = s->streams[rt->rtsp_streams[i]->stream_index];
                    if (rtpctx2 && st && st2 &&
                        rtpctx2->first_rtcp_ntp_time == AV_NOPTS_VALUE) {
                        rtpctx2->first_rtcp_ntp_time = rtpctx->first_rtcp_ntp_time;
                        rtpctx2->rtcp_ts_offset = av_rescale_q(
                            rtpctx->rtcp_ts_offset, st->time_base,
                            st2->time_base);
                    }
                }
                // Make real NTP start time available in AVFormatContext
                if (s->start_time_realtime == AV_NOPTS_VALUE) {
                    s->start_time_realtime = av_rescale (rtpctx->first_rtcp_ntp_time - (NTP_OFFSET << 32), 1000000, 1LL << 32);
                    if (rtpctx->st) {
                        s->start_time_realtime -=
                            av_rescale (rtpctx->rtcp_ts_offset,
                                        (uint64_t) rtpctx->st->time_base.num * 1000000,
                                                   rtpctx->st->time_base.den);
                    }
                }
            }
            
            if (ret == -RTCP_BYE) {
                rt->nb_byes++;

                av_log(s, AV_LOG_DEBUG, "Received BYE for stream %d (%d/%d)\n",
                       rtsp_st->stream_index, rt->nb_byes, rt->nb_rtsp_streams);

                if (rt->nb_byes == rt->nb_rtsp_streams)
                    return AVERROR_EOF;
            }
        }
    } 
    else if (CONFIG_RTPDEC && rt->ts) {
        ret = avpriv_mpegts_parse_packet(rt->ts, pkt, rt->recvbuf, len);
        if (ret >= 0) {
            if (ret < len) {
                rt->recvbuf_len = len;
                rt->recvbuf_pos = ret;
                rt->cur_transport_priv = rt->ts;
                return 1;
            } else {
                ret = 0;
            }
        }
    } else {
        return AVERROR_INVALIDDATA;
    }
end:
    if (ret < 0)
        goto redo;
    if (ret == 1)
        /* more packets may follow, so we save the RTP context */
        rt->cur_transport_priv = rtsp_st->transport_priv;

    return ret;
}

int ff_rtsp_send_cmd_async(AVFormatContext *s, const char *method, const char *url, const char *headers)
{
    return rtsp_send_cmd_with_content_async(s, method, url, headers, NULL, 0);
}

static int rtsp_read_pause(AVFormatContext *s)
{
    RTSPState *rt = s->priv_data;
    RTSPMessageHeader reply1, *reply = &reply1;

    if (rt->state != RTSP_STATE_STREAMING)
        return 0;
    else if (!(rt->server_type == RTSP_SERVER_REAL && rt->need_subscription)) {
        ff_rtsp_send_cmd(s, "PAUSE", rt->control_uri, NULL, reply, NULL);
        if (reply->status_code != RTSP_STATUS_OK) {
            return ff_rtsp_averror(reply->status_code, -1);
        }
    }
    
    rt->state = RTSP_STATE_PAUSED;
    return 0;
}

int ff_rtsp_make_setup_request(AVFormatContext *s, const char *host, int port, int lower_transport, const char *real_challenge)
{
    av_log(NULL, AV_LOG_INFO, "bruce >>> %s\n", __func__);
    RTSPState *rt = s->priv_data;
    int rtx = 0, j, i, err, interleave = 0, port_off;
    RTSPStream *rtsp_st;
    RTSPMessageHeader reply1, *reply = &reply1;
    char cmd[2048];
    const char *trans_pref;

    if (rt->transport == RTSP_TRANSPORT_RDT)
        trans_pref = "x-pn-tng";
    else if (rt->transport == RTSP_TRANSPORT_RAW)
        trans_pref = "RAW/RAW";
    else
        trans_pref = "RTP/AVP";

    /* default timeout: 1 minute */
    rt->timeout = 60;

    /* Choose a random starting offset within the first half of the
     * port range, to allow for a number of ports to try even if the offset
     * happens to be at the end of the random range. */
    port_off = av_get_random_seed() % ((rt->rtp_port_max - rt->rtp_port_min)/2);
    /* even random offset */
    port_off -= port_off & 0x01;

    for (j = rt->rtp_port_min + port_off, i = 0; i < rt->nb_rtsp_streams; ++i) {
        char transport[2048];

        /*
         * WMS serves all UDP data over a single connection, the RTX, which
         * isn't necessarily the first in the SDP but has to be the first
         * to be set up, else the second/third SETUP will fail with a 461.
         */

        av_log(NULL, AV_LOG_INFO, "bruce >>> lower_transport = %d\n", lower_transport);
        av_log(NULL, AV_LOG_INFO, "bruce >>> rt->server_type = %d\n", rt->server_type);
    
        if (lower_transport == RTSP_LOWER_TRANSPORT_UDP && rt->server_type == RTSP_SERVER_WMS) {
            if (i == 0) {
                /* rtx first */
                for (rtx = 0; rtx < rt->nb_rtsp_streams; rtx++) {
                    int len = strlen(rt->rtsp_streams[rtx]->control_url);
                    if (len >= 4 && !strcmp(rt->rtsp_streams[rtx]->control_url + len - 4, "/rtx"))
                        break;
                }
                if (rtx == rt->nb_rtsp_streams)
                    return -1; /* no RTX found */
                rtsp_st = rt->rtsp_streams[rtx];
            } else
                rtsp_st = rt->rtsp_streams[i > rtx ? i : i - 1];
        } else
            rtsp_st = rt->rtsp_streams[i];

        /* RTP/UDP */
        if (lower_transport == RTSP_LOWER_TRANSPORT_UDP) {
            char buf[256];

            if (rt->server_type == RTSP_SERVER_WMS && i > 1) {
                port = reply->transports[0].client_port_min;
                goto have_port;
            }

            /* first try in specified port range */
            while (j <= rt->rtp_port_max) {
                AVDictionary *opts = map_to_opts(rt);

                ff_url_join(buf, sizeof(buf), "rtp", NULL, host, -1, "?localport=%d", j);
                /* we will use two ports per rtp stream (rtp and rtcp) */
                j += 2;
                err = ffurl_open_whitelist(&rtsp_st->rtp_handle, buf, AVIO_FLAG_READ_WRITE,
                                 &s->interrupt_callback, &opts, s->protocol_whitelist, s->protocol_blacklist, NULL);

                av_dict_free(&opts);

                if (!err)
                    goto rtp_opened;
            }
            av_log(s, AV_LOG_ERROR, "Unable to open an input RTP port\n");
            err = AVERROR(EIO);
            goto fail;

        rtp_opened:
            port = ff_rtp_get_local_rtp_port(rtsp_st->rtp_handle);
        have_port:
            snprintf(transport, sizeof(transport) - 1, "%s/UDP;", trans_pref);
            if (rt->server_type != RTSP_SERVER_REAL)
                av_strlcat(transport, "unicast;", sizeof(transport));
            av_strlcatf(transport, sizeof(transport), "client_port=%d", port);
            if (rt->transport == RTSP_TRANSPORT_RTP &&
                !(rt->server_type == RTSP_SERVER_WMS && i > 0))
                av_strlcatf(transport, sizeof(transport), "-%d", port + 1);
        }

        /* RTP/TCP */
        else if (lower_transport == RTSP_LOWER_TRANSPORT_TCP) {
            /* For WMS streams, the application streams are only used for
             * UDP. When trying to set it up for TCP streams, the server
             * will return an error. Therefore, we skip those streams. */
            if (rt->server_type == RTSP_SERVER_WMS &&
                (rtsp_st->stream_index < 0 || s->streams[rtsp_st->stream_index]->codecpar->codec_type == AVMEDIA_TYPE_DATA))
                continue;
            snprintf(transport, sizeof(transport) - 1, "%s/TCP;", trans_pref);
            if (rt->transport != RTSP_TRANSPORT_RDT)
                av_strlcat(transport, "unicast;", sizeof(transport));
            av_strlcatf(transport, sizeof(transport), "interleaved=%d-%d", interleave, interleave + 1);
            interleave += 2;
        }

        else if (lower_transport == RTSP_LOWER_TRANSPORT_UDP_MULTICAST) {
            snprintf(transport, sizeof(transport) - 1, "%s/UDP;multicast", trans_pref);
        }
        
        if (s->oformat) {
            av_strlcat(transport, ";mode=record", sizeof(transport));
        } else if (rt->server_type == RTSP_SERVER_REAL || rt->server_type == RTSP_SERVER_WMS)
            av_strlcat(transport, ";mode=play", sizeof(transport));
        
        snprintf(cmd, sizeof(cmd), "Transport: %s\r\n", transport);
        
        if (rt->accept_dynamic_rate)
            av_strlcat(cmd, "x-Dynamic-Rate: 0\r\n", sizeof(cmd));
        
        if (CONFIG_RTPDEC && i == 0 && rt->server_type == RTSP_SERVER_REAL) {
            char real_res[41], real_csum[9];
            ff_rdt_calc_response_and_checksum(real_res, real_csum, real_challenge);
            av_strlcatf(cmd, sizeof(cmd),
                        "If-Match: %s\r\n"
                        "RealChallenge2: %s, sd=%s\r\n",
                        rt->session_id, real_res, real_csum);
        }
        
        ff_rtsp_send_cmd(s, "SETUP", rtsp_st->control_url, cmd, reply, NULL);
        if (reply->status_code == 461 /* Unsupported protocol */ && i == 0) {
            err = 1;
            goto fail;
        } else if (reply->status_code != RTSP_STATUS_OK || reply->nb_transports != 1) {
            err = ff_rtsp_averror(reply->status_code, AVERROR_INVALIDDATA);
            goto fail;
        }

        /* XXX: same protocol for all streams is required */
        av_log(NULL, AV_LOG_INFO, "bruce >>> reply->transports[0].lower_transport = %d\n", reply->transports[0].lower_transport);
        av_log(NULL, AV_LOG_INFO, "bruce >>> reply->transports[0].transport = %d\n", reply->transports[0].transport);
        av_log(NULL, AV_LOG_INFO, "bruce >>> rt->lower_transport = %d\n", rt->lower_transport);
        av_log(NULL, AV_LOG_INFO, "bruce >>> rt->transport = %d\n", rt->transport);
        av_log(NULL, AV_LOG_INFO, "bruce >>> i = %d\n", i);
                
        if (i > 0) {
            if (reply->transports[0].lower_transport != rt->lower_transport ||
                reply->transports[0].transport != rt->transport) {
                err = AVERROR_INVALIDDATA;
                goto fail;
            }
        } else {
            rt->lower_transport = reply->transports[0].lower_transport;
            rt->transport = reply->transports[0].transport;
        }

        /* Fail if the server responded with another lower transport mode
         * than what we requested. */
        if (reply->transports[0].lower_transport != lower_transport) {
            av_log(s, AV_LOG_ERROR, "Nonmatching transport in server reply\n");
            err = AVERROR_INVALIDDATA;
            goto fail;
        }

        switch(reply->transports[0].lower_transport) {
            case RTSP_LOWER_TRANSPORT_TCP:
                rtsp_st->interleaved_min = reply->transports[0].interleaved_min;
                rtsp_st->interleaved_max = reply->transports[0].interleaved_max;
                break;

            case RTSP_LOWER_TRANSPORT_UDP: {
                char url[1024], options[30] = "";
                const char *peer = host;

                if (rt->rtsp_flags & RTSP_FLAG_FILTER_SRC)
                    av_strlcpy(options, "?connect=1", sizeof(options));
                /* Use source address if specified */
                if (reply->transports[0].source[0])
                    peer = reply->transports[0].source;
                
                ff_url_join(url, sizeof(url), "rtp", NULL, peer, reply->transports[0].server_port_min, "%s", options);
                
                if (!(rt->server_type == RTSP_SERVER_WMS && i > 1) && ff_rtp_set_remote_url(rtsp_st->rtp_handle, url) < 0) {
                    err = AVERROR_INVALIDDATA;
                    goto fail;
                }
                break;
            }
                
            case RTSP_LOWER_TRANSPORT_UDP_MULTICAST: {
                char url[1024], namebuf[50], optbuf[20] = "";
                struct sockaddr_storage addr;
                int port, ttl;

                if (reply->transports[0].destination.ss_family) {
                    addr      = reply->transports[0].destination;
                    port      = reply->transports[0].port_min;
                    ttl       = reply->transports[0].ttl;
                } else {
                    addr      = rtsp_st->sdp_ip;
                    port      = rtsp_st->sdp_port;
                    ttl       = rtsp_st->sdp_ttl;
                }
                
                if (ttl > 0)
                    snprintf(optbuf, sizeof(optbuf), "?ttl=%d", ttl);
                
                getnameinfo((struct sockaddr*) &addr, sizeof(addr), namebuf, sizeof(namebuf), NULL, 0, NI_NUMERICHOST);
                
                ff_url_join(url, sizeof(url), "rtp", NULL, namebuf, port, "%s", optbuf);
                
                if (ffurl_open_whitelist(&rtsp_st->rtp_handle, url, AVIO_FLAG_READ_WRITE,
                               &s->interrupt_callback, NULL, s->protocol_whitelist, s->protocol_blacklist, NULL) < 0) {
                    err = AVERROR_INVALIDDATA;
                    goto fail;
                }
                break;
            }
        }

        if ((err = ff_rtsp_open_transport_ctx(s, rtsp_st)))
            goto fail;
    }

    if (rt->nb_rtsp_streams && reply->timeout > 0)
        rt->timeout = reply->timeout;

    if (rt->server_type == RTSP_SERVER_REAL)
        rt->need_subscription = 1;

    return 0;

fail:
    ff_rtsp_undo_setup(s, 0);
    return err;
}


void ff_rtsp_undo_setup(AVFormatContext *s, int send_packets)
{
    RTSPState *rt = s->priv_data;
    int i;

    for (i = 0; i < rt->nb_rtsp_streams; i++) {
        RTSPStream *rtsp_st = rt->rtsp_streams[i];
        if (!rtsp_st)
            continue;
        if (rtsp_st->transport_priv) {
            if (s->oformat) {
                AVFormatContext *rtpctx = rtsp_st->transport_priv;
                av_write_trailer(rtpctx);
                if (rt->lower_transport == RTSP_LOWER_TRANSPORT_TCP) {
                    if (CONFIG_RTSP_MUXER && rtpctx->pb && send_packets)
                        ff_rtsp_tcp_write_packet(s, rtsp_st);
                    ffio_free_dyn_buf(&rtpctx->pb);
                } else {
                    avio_closep(&rtpctx->pb);
                }
                avformat_free_context(rtpctx);
            } else if (CONFIG_RTPDEC && rt->transport == RTSP_TRANSPORT_RDT)
                ff_rdt_parse_close(rtsp_st->transport_priv);
            else if (CONFIG_RTPDEC && rt->transport == RTSP_TRANSPORT_RTP)
                ff_rtp_parse_close(rtsp_st->transport_priv);
        }
        rtsp_st->transport_priv = NULL;
        if (rtsp_st->rtp_handle)
            ffurl_close(rtsp_st->rtp_handle);
        rtsp_st->rtp_handle = NULL;
    }
}

static int resetup_tcp(AVFormatContext *s)
{
    RTSPState *rt = s->priv_data;
    char host[1024];
    int port;

    av_url_split(NULL, 0, NULL, 0, host, sizeof(host), &port, NULL, 0, s->filename);
    ff_rtsp_undo_setup(s, 0);
    return ff_rtsp_make_setup_request(s, host, port, RTSP_LOWER_TRANSPORT_TCP, rt->real_challenge);
}

static int rtsp_read_packet(AVFormatContext *pstFmtCtx, AVPacket *pkt)
{
    RTSPState *pstRtspState = pstFmtCtx->priv_data;
    int ret;
    RTSPMessageHeader reply1, *reply = &reply1;
    char cmd[1024];

retry:
    #if 0
    if (pstRtspState->server_type == RTSP_SERVER_REAL) {
        int i;

        for (i = 0; i < pstFmtCtx->nb_streams; i++)
            pstRtspState->real_setup[i] = pstFmtCtx->streams[i]->discard;

        if (!pstRtspState->need_subscription) {
            if (memcmp (rt->real_setup, rt->real_setup_cache,
                        sizeof(enum AVDiscard) * s->nb_streams)) {
                snprintf(cmd, sizeof(cmd),
                         "Unsubscribe: %s\r\n",
                         rt->last_subscription);
                ff_rtsp_send_cmd(s, "SET_PARAMETER", rt->control_uri,
                                 cmd, reply, NULL);
                if (reply->status_code != RTSP_STATUS_OK)
                    return ff_rtsp_averror(reply->status_code, AVERROR_INVALIDDATA);
                rt->need_subscription = 1;
            }
        }

        if (rt->need_subscription) {
            int r, rule_nr, first = 1;

            memcpy(rt->real_setup_cache, rt->real_setup,
                   sizeof(enum AVDiscard) * s->nb_streams);
            rt->last_subscription[0] = 0;

            snprintf(cmd, sizeof(cmd),
                     "Subscribe: ");
            for (i = 0; i < rt->nb_rtsp_streams; i++) {
                rule_nr = 0;
                for (r = 0; r < s->nb_streams; r++) {
                    if (s->streams[r]->id == i) {
                        if (s->streams[r]->discard != AVDISCARD_ALL) {
                            if (!first)
                                av_strlcat(rt->last_subscription, ",",
                                           sizeof(rt->last_subscription));
                            ff_rdt_subscribe_rule(
                                rt->last_subscription,
                                sizeof(rt->last_subscription), i, rule_nr);
                            first = 0;
                        }
                        rule_nr++;
                    }
                }
            }
            av_strlcatf(cmd, sizeof(cmd), "%s\r\n", rt->last_subscription);
            ff_rtsp_send_cmd(s, "SET_PARAMETER", rt->control_uri,
                             cmd, reply, NULL);
            if (reply->status_code != RTSP_STATUS_OK)
                return ff_rtsp_averror(reply->status_code, AVERROR_INVALIDDATA);
            rt->need_subscription = 0;

            if (rt->state == RTSP_STATE_STREAMING)
                rtsp_read_play (s);
        }
    }
    #endif

    ret = ff_rtsp_fetch_packet(pstFmtCtx, pkt);
    if (ret < 0) {
        if (ret == AVERROR(ETIMEDOUT) && !pstRtspState->packets) {
            if (pstRtspState->lower_transport == RTSP_LOWER_TRANSPORT_UDP &&
                pstRtspState->lower_transport_mask & (1 << RTSP_LOWER_TRANSPORT_TCP)) {
                RTSPMessageHeader reply1, *reply = &reply1;
                if (rtsp_read_pause(pstFmtCtx) != 0)
                    return -1;
                // TEARDOWN is required on Real-RTSP, but might make other servers close the connection.
                if (pstRtspState->server_type == RTSP_SERVER_REAL)
                    ff_rtsp_send_cmd(pstFmtCtx, "TEARDOWN", pstRtspState->control_uri, NULL, reply, NULL);
                pstRtspState->session_id[0] = '\0';
                if (resetup_tcp(pstFmtCtx) == 0) {
                    rt->state = RTSP_STATE_IDLE;
                    rt->need_subscription = 1;
                    if (rtsp_read_play(s) != 0)
                        return -1;
                    goto retry;
                }
            }
        }
        return ret;
    }
    
    pstRtspState->packets++;

    if (!(pstRtspState->rtsp_flags & RTSP_FLAG_LISTEN)) {
        /* send dummy request to keep TCP connection alive */
        if ((av_gettime_relative() - pstRtspState->last_cmd_time) / 1000000 >= pstRtspState->timeout / 2 
                || pstRtspState->auth_state.stale) {
            if (pstRtspState->server_type == RTSP_SERVER_WMS 
                || (pstRtspState->server_type != RTSP_SERVER_REAL && pstRtspState->get_parameter_supported)) {
                ff_rtsp_send_cmd_async(pstFmtCtx, "GET_PARAMETER", pstRtspState->control_uri, NULL);
            } else {
                ff_rtsp_send_cmd_async(pstFmtCtx, "OPTIONS", pstRtspState->control_uri, NULL);
            }
            
            pstRtspState->auth_state.stale = 0;
        }
    }

    return 0;
}





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

