/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


#define RTP_SEQ_MOD (1 << 16)
#define RTP_NOTS_VALUE ((uint32_t)-1)



static const struct {
    int pt;
    const char enc_name[6];
    enum AVMediaType codec_type;
    enum AVCodecID codec_id;
    int clock_rate;
    int audio_channels;
} rtp_payload_types[] = {
  {0, "PCMU",        AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_PCM_MULAW, 8000, 1},
  {3, "GSM",         AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_NONE, 8000, 1},
  {4, "G723",        AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_G723_1, 8000, 1},
  {5, "DVI4",        AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_NONE, 8000, 1},
  {6, "DVI4",        AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_NONE, 16000, 1},
  {7, "LPC",         AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_NONE, 8000, 1},
  {8, "PCMA",        AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_PCM_ALAW, 8000, 1},
  {9, "G722",        AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_ADPCM_G722, 8000, 1},
  {10, "L16",        AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_PCM_S16BE, 44100, 2},
  {11, "L16",        AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_PCM_S16BE, 44100, 1},
  {12, "QCELP",      AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_QCELP, 8000, 1},
  {13, "CN",         AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_NONE, 8000, 1},
  {14, "MPA",        AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_MP2, -1, -1},
  {14, "MPA",        AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_MP3, -1, -1},
  {15, "G728",       AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_NONE, 8000, 1},
  {16, "DVI4",       AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_NONE, 11025, 1},
  {17, "DVI4",       AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_NONE, 22050, 1},
  {18, "G729",       AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_NONE, 8000, 1},
  {25, "CelB",       AVMEDIA_TYPE_VIDEO,   AV_CODEC_ID_NONE, 90000, -1},
  {26, "JPEG",       AVMEDIA_TYPE_VIDEO,   AV_CODEC_ID_MJPEG, 90000, -1},
  {28, "nv",         AVMEDIA_TYPE_VIDEO,   AV_CODEC_ID_NONE, 90000, -1},
  {31, "H261",       AVMEDIA_TYPE_VIDEO,   AV_CODEC_ID_H261, 90000, -1},
  {32, "MPV",        AVMEDIA_TYPE_VIDEO,   AV_CODEC_ID_MPEG1VIDEO, 90000, -1},
  {32, "MPV",        AVMEDIA_TYPE_VIDEO,   AV_CODEC_ID_MPEG2VIDEO, 90000, -1},
  {33, "MP2T",       AVMEDIA_TYPE_DATA,    AV_CODEC_ID_MPEG2TS, 90000, -1},
  {34, "H263",       AVMEDIA_TYPE_VIDEO,   AV_CODEC_ID_H263, 90000, -1},
  {-1, "",           AVMEDIA_TYPE_UNKNOWN, AV_CODEC_ID_NONE, -1, -1}
};

int ff_rtp_get_codec_info(AVCodecParameters *par, int payload_type)
{
    int i = 0;

    for (i = 0; rtp_payload_types[i].pt >= 0; i++)
        if (rtp_payload_types[i].pt == payload_type) {
            if (rtp_payload_types[i].codec_id != AV_CODEC_ID_NONE) {
                par->codec_type = rtp_payload_types[i].codec_type;
                par->codec_id = rtp_payload_types[i].codec_id;
            
                if (rtp_payload_types[i].audio_channels > 0) {
                    par->channels = rtp_payload_types[i].audio_channels;
                }

                if (rtp_payload_types[i].clock_rate > 0) {
                    par->sample_rate = rtp_payload_types[i].clock_rate;
                }
                
                return 0;
            }
        }
        
    return -1;
}


const char *ff_rtp_enc_name(int payload_type)
{
    int i;

    for (i = 0; rtp_payload_types[i].pt >= 0; i++)
        if (rtp_payload_types[i].pt == payload_type)
            return rtp_payload_types[i].enc_name;

    return "";
}

RTPDynamicProtocolHandler *ff_rtp_handler_find_by_id(int id, enum AVMediaType codec_type)
{
    RTPDynamicProtocolHandler *handler;
    for (handler = rtp_first_dynamic_payload_handler; handler; handler = handler->next)
        if (handler->static_payload_id && handler->static_payload_id == id && codec_type == handler->codec_type)
            return handler;
    return NULL;
}

static int has_next_packet(RTPDemuxContext *s)
{
    return s->queue && s->queue->seq == (uint16_t) (s->seq + 1);
}

static void rtp_init_sequence(RTPStatistics *s, uint16_t seq)
{
    s->max_seq        = seq;
    s->cycles         = 0;
    s->base_seq       = seq - 1;
    s->bad_seq        = RTP_SEQ_MOD + 1;
    s->received       = 0;
    s->expected_prior = 0;
    s->received_prior = 0;
    s->jitter         = 0;
    s->transit        = 0;
}

static int rtp_valid_packet_in_sequence(RTPStatistics *s, uint16_t seq)
{
    uint16_t udelta = seq - s->max_seq;
    const int MAX_DROPOUT    = 3000;
    const int MAX_MISORDER   = 100;
    const int MIN_SEQUENTIAL = 2;

    /* source not valid until MIN_SEQUENTIAL packets with sequence
     * seq. numbers have been received */
    if (s->probation) {
        if (seq == s->max_seq + 1) {
            s->probation--;
            s->max_seq = seq;
            if (s->probation == 0) {
                rtp_init_sequence(s, seq);
                s->received++;
                return 1;
            }
        } else {
            s->probation = MIN_SEQUENTIAL - 1;
            s->max_seq   = seq;
        }
    } else if (udelta < MAX_DROPOUT) {
        // in order, with permissible gap
        if (seq < s->max_seq) {
            // sequence number wrapped; count another 64k cycles
            s->cycles += RTP_SEQ_MOD;
        }
        s->max_seq = seq;
    } else if (udelta <= RTP_SEQ_MOD - MAX_MISORDER) {
        // sequence made a large jump...
        if (seq == s->bad_seq) {
            /* two sequential packets -- assume that the other side restarted without telling us; just resync. */
            rtp_init_sequence(s, seq);
        } else {
            s->bad_seq = (seq + 1) & (RTP_SEQ_MOD - 1);
            return 0;
        }
    } else {
        // duplicate or reordered packet...
    }
    
    s->received++;
    return 1;
}

static int rtp_parse_packet_internal(RTPDemuxContext *s, AVPacket *pkt, const uint8_t *buf, int len)
{
    unsigned int ssrc;
    int payload_type, seq, flags = 0;
    int ext, csrc;
    AVStream *st;
    uint32_t timestamp;
    int rv = 0;

    csrc         = buf[0] & 0x0f;
    ext          = buf[0] & 0x10;
    payload_type = buf[1] & 0x7f;
    
    if (buf[1] & 0x80)
        flags |= RTP_FLAG_MARKER;
    
    seq       = AV_RB16(buf + 2);
    timestamp = AV_RB32(buf + 4);
    ssrc      = AV_RB32(buf + 8);

    s->ssrc = ssrc;

    if (s->payload_type != payload_type)
        return -1;

    st = s->st;
    // only do something with this if all the rtp checks pass...
    if (!rtp_valid_packet_in_sequence(&s->statistics, seq)) {
        av_log(s->ic, AV_LOG_ERROR,
               "RTP: PT=%02x: bad cseq %04x expected=%04x\n",
               payload_type, seq, ((s->seq + 1) & 0xffff));
        return -1;
    }

    if (buf[0] & 0x20) {
        int padding = buf[len - 1];
        if (len >= 12 + padding)
            len -= padding;
    }

    s->seq = seq;
    len   -= 12;
    buf   += 12;

    len   -= 4 * csrc;
    buf   += 4 * csrc;
    if (len < 0)
        return AVERROR_INVALIDDATA;

    /* RFC 3550 Section 5.3.1 RTP Header Extension handling */
    if (ext) {
        if (len < 4)
            return -1;
        /* calculate the header extension length (stored as number
         * of 32-bit words) */
        ext = (AV_RB16(buf + 2) + 1) << 2;

        if (len < ext)
            return -1;
        // skip past RTP header extension
        len -= ext;
        buf += ext;
    }

    if (s->handler && s->handler->parse_packet) {
        rv = s->handler->parse_packet(s->ic, s->dynamic_protocol_context,
                                      s->st, pkt, &timestamp, buf, len, seq,
                                      flags);
    } else if (st) {
        if ((rv = av_new_packet(pkt, len)) < 0)
            return rv;
        memcpy(pkt->data, buf, len);
        pkt->stream_index = st->index;
    } else {
        return AVERROR(EINVAL);
    }

    // now perform timestamp things....
    finalize_packet(s, pkt, timestamp);

    return rv;
}

static int rtp_parse_queued_packet(RTPDemuxContext *s, AVPacket *pkt)
{
    int rv;
    RTPPacket *next;

    if (s->queue_len <= 0)
        return -1;

    if (!has_next_packet(s))
        av_log(s->ic, AV_LOG_WARNING, "RTP: missed %d packets\n", s->queue->seq - s->seq - 1);

    /* Parse the first packet in the queue, and dequeue it */
    rv   = rtp_parse_packet_internal(s, pkt, s->queue->buf, s->queue->len);
    next = s->queue->next;
    av_freep(&s->queue->buf);
    av_freep(&s->queue);
    s->queue = next;
    s->queue_len--;
    return rv;
}

static void finalize_packet(RTPDemuxContext *s, AVPacket *pkt, uint32_t timestamp)
{
    if (pkt->pts != AV_NOPTS_VALUE || pkt->dts != AV_NOPTS_VALUE)
        return; /* Timestamp already set by depacketizer */
    
    if (timestamp == RTP_NOTS_VALUE)
        return;

    if (s->last_rtcp_ntp_time != AV_NOPTS_VALUE && s->ic->nb_streams > 1) {
        int64_t addend;
        int delta_timestamp;

        /* compute pts from timestamp with received ntp_time */
        delta_timestamp = timestamp - s->last_rtcp_timestamp;
        
        /* convert to the PTS timebase */
        addend = av_rescale(s->last_rtcp_ntp_time - s->first_rtcp_ntp_time, s->st->time_base.den, (uint64_t) s->st->time_base.num << 32);
        pkt->pts = s->range_start_offset + s->rtcp_ts_offset + addend + delta_timestamp;

        return;
    }

    if (!s->base_timestamp)
        s->base_timestamp = timestamp;
    /* assume that the difference is INT32_MIN < x < INT32_MAX, but allow the first timestamp to exceed INT32_MAX */
    if (!s->timestamp)
        s->unwrapped_timestamp += timestamp;
    else
        s->unwrapped_timestamp += (int32_t)(timestamp - s->timestamp);
    
    s->timestamp = timestamp;
    pkt->pts = s->unwrapped_timestamp + s->range_start_offset - s->base_timestamp;
}

static int rtp_parse_one_packet(RTPDemuxContext *s, AVPacket *pkt, uint8_t **bufptr, int len)
{
    uint8_t *buf = bufptr ? *bufptr : NULL;
    int flags = 0;
    uint32_t timestamp;
    int rv = 0;

    if (!buf) {
        if (s->prev_ret <= 0)
            return rtp_parse_queued_packet(s, pkt);
        /* return the next packets, if any */
        if (s->handler && s->handler->parse_packet) {
            /* timestamp should be overwritten by parse_packet, if not,
             * the packet is left with pts == AV_NOPTS_VALUE */
            timestamp = RTP_NOTS_VALUE;
            rv = s->handler->parse_packet(s->ic, s->dynamic_protocol_context, s->st, pkt, &timestamp, NULL, 0, 0, flags);
            finalize_packet(s, pkt, timestamp);
            return rv;
        }
    }

    if (len < 12)
        return -1;

    if ((buf[0] & 0xc0) != (RTP_VERSION << 6))
        return -1;
    if (RTP_PT_IS_RTCP(buf[1])) {
        return rtcp_parse_packet(s, buf, len);
    }

    if (s->st) {
        int64_t received = av_gettime_relative();
        uint32_t arrival_ts = av_rescale_q(received, AV_TIME_BASE_Q,
                                           s->st->time_base);
        timestamp = AV_RB32(buf + 4);
        // Calculate the jitter immediately, before queueing the packet
        // into the reordering queue.
        rtcp_update_jitter(&s->statistics, timestamp, arrival_ts);
    }

    if ((s->seq == 0 && !s->queue) || s->queue_size <= 1) {
        /* First packet, or no reordering */
        return rtp_parse_packet_internal(s, pkt, buf, len);
    } else {
        uint16_t seq = AV_RB16(buf + 2);
        int16_t diff = seq - s->seq;
        if (diff < 0) {
            /* Packet older than the previously emitted one, drop */
            av_log(s->ic, AV_LOG_WARNING,
                   "RTP: dropping old packet received too late\n");
            return -1;
        } else if (diff <= 1) {
            /* Correct packet */
            rv = rtp_parse_packet_internal(s, pkt, buf, len);
            return rv;
        } else {
            /* Still missing some packet, enqueue this one. */
            rv = enqueue_packet(s, buf, len);
            if (rv < 0)
                return rv;
            *bufptr = NULL;
            /* Return the first enqueued packet if the queue is full,
             * even if we're missing something */
            if (s->queue_len >= s->queue_size) {
                av_log(s->ic, AV_LOG_WARNING, "jitter buffer full\n");
                return rtp_parse_queued_packet(s, pkt);
            }
            return -1;
        }
    }
}

int ff_rtp_parse_packet(RTPDemuxContext *s, AVPacket *pkt, uint8_t **bufptr, int len)
{
    int rv;
    if (s->srtp_enabled && bufptr && ff_srtp_decrypt(&s->srtp, *bufptr, &len) < 0)
        return -1;
    rv = rtp_parse_one_packet(s, pkt, bufptr, len);
    s->prev_ret = rv;
    while (rv < 0 && has_next_packet(s))
        rv = rtp_parse_queued_packet(s, pkt);
    return rv ? rv : has_next_packet(s);
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

