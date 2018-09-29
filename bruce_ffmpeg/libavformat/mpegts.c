/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/





MpegTSContext *avpriv_mpegts_parse_open(AVFormatContext *s)
{
    MpegTSContext *ts;

    ts = av_mallocz(sizeof(MpegTSContext));
    if (!ts)
        return NULL;
    /* no stream case, currently used by RTP */
    ts->raw_packet_size = TS_PACKET_SIZE;
    ts->stream = s;
    ts->auto_guess = 1;
    mpegts_open_section_filter(ts, SDT_PID, sdt_cb, ts, 1);
    mpegts_open_section_filter(ts, PAT_PID, pat_cb, ts, 1);

    return ts;
}

