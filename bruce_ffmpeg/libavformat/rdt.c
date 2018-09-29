/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


int ff_rdt_parse_header(const uint8_t *buf, int len,
                    int *pset_id, int *pseq_no, int *pstream_id,
                    int *pis_keyframe, uint32_t *ptimestamp)
{
    GetBitContext gb;
    int consumed = 0, set_id, seq_no, stream_id, is_keyframe, len_included, need_reliable;
    uint32_t timestamp;

    /* skip status packets */
    while (len >= 5 && buf[1] == 0xFF) {
        int pkt_len;

        if (!(buf[0] & 0x80)) {
            return -1;
        }

        pkt_len = AV_RB16(buf+3);
        buf += pkt_len;
        len -= pkt_len;
        consumed += pkt_len;
    }
    if (len < 16)
        return -1;
    
    init_get_bits(&gb, buf, len << 3);
    len_included  = get_bits1(&gb);
    need_reliable = get_bits1(&gb);
    set_id        = get_bits(&gb, 5);
    skip_bits(&gb, 1);
    seq_no        = get_bits(&gb, 16);
    if (len_included)
        skip_bits(&gb, 16);
    skip_bits(&gb, 2);
    stream_id     = get_bits(&gb, 5);
    is_keyframe   = !get_bits1(&gb);
    timestamp     = get_bits_long(&gb, 32);
    if (set_id == 0x1f)
        set_id    = get_bits(&gb, 16);
    if (need_reliable)
        skip_bits(&gb, 16);
    if (stream_id == 0x1f)
        stream_id = get_bits(&gb, 16);

    if (pset_id)      *pset_id      = set_id;
    if (pseq_no)      *pseq_no      = seq_no;
    if (pstream_id)   *pstream_id   = stream_id;
    if (pis_keyframe) *pis_keyframe = is_keyframe;
    if (ptimestamp)   *ptimestamp   = timestamp;

    return consumed + (get_bits_count(&gb) >> 3);
}

int ff_rdt_parse_packet(RDTDemuxContext *pstRdtDemuxCtx, AVPacket *pkt, uint8_t **bufptr, int len)
{
    uint8_t *buf = bufptr ? *bufptr : NULL;
    int seq_no, flags = 0, stream_id, set_id, is_keyframe;
    uint32_t timestamp;
    int rv= 0;

    if (!pstRdtDemuxCtx->parse_packet)
        return -1;

    if (!buf && pstRdtDemuxCtx->prev_stream_id != -1) {
        timestamp= 0; ///< Should not be used if buf is NULL, but should be set to the timestamp of the packet returned....
        rv= pstRdtDemuxCtx->parse_packet(pstRdtDemuxCtx->ic, pstRdtDemuxCtx->dynamic_protocol_context,
                            pstRdtDemuxCtx->streams[pstRdtDemuxCtx->prev_stream_id],
                            pkt, &timestamp, NULL, 0, 0, flags);
        return rv;
    }

    if (len < 12)
        return -1;
    rv = ff_rdt_parse_header(buf, len, &set_id, &seq_no, &stream_id, &is_keyframe, &timestamp);
    if (rv < 0)
        return rv;
    if (is_keyframe &&
        (set_id != s->prev_set_id || timestamp != s->prev_timestamp ||
         stream_id != s->prev_stream_id)) {
        flags |= RTP_FLAG_KEY;
        s->prev_set_id    = set_id;
        s->prev_timestamp = timestamp;
    }
    s->prev_stream_id = stream_id;
    buf += rv;
    len -= rv;

     if (s->prev_stream_id >= s->n_streams) {
         s->prev_stream_id = -1;
         return -1;
     }

    rv = s->parse_packet(s->ic, s->dynamic_protocol_context,
                         s->streams[s->prev_stream_id],
                         pkt, &timestamp, buf, len, 0, flags);

    return rv;
}



