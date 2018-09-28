/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/



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

