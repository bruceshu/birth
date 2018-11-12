/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-18
 * Description:
 
*********************************/


#include <string.h>
#include <inttypes.h>

#include "libavutil/mem.h"
#include "libavutil/error.h"
#include "libavutil/opt.h"

#include "libavcodec/avpacket.h"
#include "libavcodec/utils.h"

#include "libavutil/frame.h"
#include "libavutil/mathematics.h"

#include "utils.h"

int packet_list_put(AVPacketList **packet_buffer, AVPacketList **plast_pktl, AVPacket *pkt, int flags)
{
    AVPacketList *pktl = av_mallocz(sizeof(AVPacketList));
    int ret;

    if (!pktl)
        return AVERROR(ENOMEM);

    if (flags & FF_PACKETLIST_FLAG_REF_PACKET) {
        if ((ret = av_packet_ref(&pktl->pkt, pkt)) < 0) {
            av_free(pktl);
            return ret;
        }
    } else {
        // TODO: Adapt callers in this file so the line below can use
        //       av_packet_move_ref() to effectively move the reference
        //       to the list.
        pktl->pkt = *pkt;
    }

    if (*packet_buffer)
        (*plast_pktl)->next = pktl;
    else
        *packet_buffer = pktl;

    /* Add the packet in the buffered packet list. */
    *plast_pktl = pktl;
    return 0;
}

static const AVCodec *find_decoder(AVFormatContext *s, const AVStream *st, enum AVCodecID codec_id)
{
#if 0//FF_API_LAVF_AVCTX
FF_DISABLE_DEPRECATION_WARNINGS
    if (st->codec->codec)
        return st->codec->codec;
FF_ENABLE_DEPRECATION_WARNINGS
#endif

    switch (st->codecpar->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        if (s->video_codec)    return s->video_codec;
        break;
    case AVMEDIA_TYPE_AUDIO:
        if (s->audio_codec)    return s->audio_codec;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        if (s->subtitle_codec) return s->subtitle_codec;
        break;
    }

    return avcodec_find_decoder(codec_id);
}

static const AVCodec *find_probe_decoder(AVFormatContext *s, const AVStream *st, enum AVCodecID codec_id)
{
    const AVCodec *codec;

#if CONFIG_H264_DECODER
    /* Other parts of the code assume this decoder to be used for h264,
     * so force it if possible. */
    if (codec_id == AV_CODEC_ID_H264)
        return avcodec_find_decoder_by_name("h264");
#endif

    codec = find_decoder(s, st, codec_id);
    if (!codec)
        return NULL;

    if (codec->capabilities & AV_CODEC_CAP_AVOID_PROBING) {
        const AVCodec *probe_codec = NULL;
        while (probe_codec = av_codec_next(probe_codec)) {
            if (probe_codec->id == codec_id && av_codec_is_decoder(probe_codec) 
                                            && !(probe_codec->capabilities & (AV_CODEC_CAP_AVOID_PROBING | AV_CODEC_CAP_EXPERIMENTAL))) {
                return probe_codec;
            }
        }
    }

    return codec;
}

static int determinable_frame_size(AVCodecContext *avctx)
{
    switch(avctx->codec_id) {
    case AV_CODEC_ID_MP1:
    case AV_CODEC_ID_MP2:
    case AV_CODEC_ID_MP3:
    case AV_CODEC_ID_CODEC2:
        return 1;
    }

    return 0;
}

static int has_codec_parameters(AVStream *st, const char **errmsg_ptr)
{
    AVCodecContext *avctx = st->internal->avctx;

#define FAIL(errmsg) do {                                         \
        if (errmsg_ptr)                                           \
            *errmsg_ptr = errmsg;                                 \
        return 0;                                                 \
    } while (0)

    if (avctx->codec_id == AV_CODEC_ID_NONE && avctx->codec_type != AVMEDIA_TYPE_DATA)
        FAIL("unknown codec");
    
    switch (avctx->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        if (!avctx->frame_size && determinable_frame_size(avctx))
            FAIL("unspecified frame size");
        if (st->info->found_decoder >= 0 &&
            avctx->sample_fmt == AV_SAMPLE_FMT_NONE)
            FAIL("unspecified sample format");
        if (!avctx->sample_rate)
            FAIL("unspecified sample rate");
        if (!avctx->channels)
            FAIL("unspecified number of channels");
        if (st->info->found_decoder >= 0 && !st->nb_decoded_frames && avctx->codec_id == AV_CODEC_ID_DTS)
            FAIL("no decodable DTS frames");
        break;
    case AVMEDIA_TYPE_VIDEO:
        if (!avctx->width)
            FAIL("unspecified size");
        if (st->info->found_decoder >= 0 && avctx->pix_fmt == AV_PIX_FMT_NONE)
            FAIL("unspecified pixel format");
        if (st->codecpar->codec_id == AV_CODEC_ID_RV30 || st->codecpar->codec_id == AV_CODEC_ID_RV40)
            if (!st->sample_aspect_ratio.num && !st->codecpar->sample_aspect_ratio.num && !st->codec_info_nb_frames)
                FAIL("no frame in rv30/40 and no sar");
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        if (avctx->codec_id == AV_CODEC_ID_HDMV_PGS_SUBTITLE && !avctx->width)
            FAIL("unspecified size");
        break;
    case AVMEDIA_TYPE_DATA:
        if (avctx->codec_id == AV_CODEC_ID_NONE) return 1;
    }

    return 1;
}

static int has_decode_delay_been_guessed(AVStream *st)
{
    if (st->codecpar->codec_id != AV_CODEC_ID_H264) return 1;
    if (!st->info) // if we have left find_stream_info then nb_decoded_frames won't increase anymore for stream copy
        return 1;
#if 0//CONFIG_H264_DECODER
    if (st->internal->avctx->has_b_frames &&
       avpriv_h264_has_num_reorder_frames(st->internal->avctx) == st->internal->avctx->has_b_frames)
        return 1;
#endif
    if (st->internal->avctx->has_b_frames<3)
        return st->nb_decoded_frames >= 7;
    else if (st->internal->avctx->has_b_frames<4)
        return st->nb_decoded_frames >= 18;
    else
        return st->nb_decoded_frames >= 20;
}

static int try_decode_frame(AVFormatContext *s, AVStream *st, AVPacket *avpkt, AVDictionary **options)
{
    AVCodecContext *avctx = st->internal->avctx;
    const AVCodec *codec;
    int got_picture = 1, ret = 0;
    AVFrame *frame = av_frame_alloc();
    //AVSubtitle subtitle;
    AVPacket pkt = *avpkt;
    int do_skip_frame = 0;
    enum AVDiscard skip_frame;

    if (!frame)
        return AVERROR(ENOMEM);

    if (!avcodec_is_open(avctx) && st->info->found_decoder <= 0 
                                && (st->codecpar->codec_id != -st->info->found_decoder || !st->codecpar->codec_id)) {
        AVDictionary *thread_opt = NULL;

        codec = find_probe_decoder(s, st, st->codecpar->codec_id);

        if (!codec) {
            st->info->found_decoder = -st->codecpar->codec_id;
            ret                     = -1;
            goto fail;
        }

        /* Force thread count to 1 since the H.264 decoder will not extract
         * SPS and PPS to extradata during multi-threaded decoding. */
        av_dict_set(options ? options : &thread_opt, "threads", "1", 0);
        if (s->codec_whitelist)
            av_dict_set(options ? options : &thread_opt, "codec_whitelist", s->codec_whitelist, 0);
        ret = avcodec_open2(avctx, codec, options ? options : &thread_opt);
        if (!options)
            av_dict_free(&thread_opt);
        if (ret < 0) {
            st->info->found_decoder = -avctx->codec_id;
            goto fail;
        }
        st->info->found_decoder = 1;
    } else if (!st->info->found_decoder)
        st->info->found_decoder = 1;

    if (st->info->found_decoder < 0) {
        ret = -1;
        goto fail;
    }

    if (avpriv_codec_get_cap_skip_frame_fill_param(avctx->codec)) {
        do_skip_frame = 1;
        skip_frame = avctx->skip_frame;
        avctx->skip_frame = AVDISCARD_ALL;
    }

    while ((pkt.size > 0 || (!pkt.data && got_picture)) &&
           ret >= 0 &&
           (!has_codec_parameters(st, NULL) || !has_decode_delay_been_guessed(st) ||
            (!st->codec_info_nb_frames &&
             (avctx->codec->capabilities & AV_CODEC_CAP_CHANNEL_CONF)))) {
        got_picture = 0;
        
        /*if (avctx->codec_type == AVMEDIA_TYPE_VIDEO ||
            avctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            ret = avcodec_send_packet(avctx, &pkt);
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
                break;
            if (ret >= 0)
                pkt.size = 0;
            ret = avcodec_receive_frame(avctx, frame);
            if (ret >= 0)
                got_picture = 1;
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                ret = 0;
        } else if (avctx->codec_type == AVMEDIA_TYPE_SUBTITLE) {
            ret = avcodec_decode_subtitle2(avctx, &subtitle,
                                           &got_picture, &pkt);
            if (ret >= 0)
                pkt.size = 0;
        }
        if (ret >= 0) {
            if (got_picture)
                st->nb_decoded_frames++;
            ret       = got_picture;
        }*/
    }

    if (!pkt.data && !got_picture)
        ret = -1;

fail:
    if (do_skip_frame) {
        avctx->skip_frame = skip_frame;
    }

    av_frame_free(&frame);
    return ret;
}

static AVPacketList *get_next_pkt(AVFormatContext *s, AVStream *st, AVPacketList *pktl)
{
    if (pktl->next)
        return pktl->next;
    if (pktl == s->internal->packet_buffer_end)
        return s->internal->parse_queue;
    return NULL;
}

static int64_t select_from_pts_buffer(AVStream *st, int64_t *pts_buffer, int64_t dts) {
    int onein_oneout = st->codecpar->codec_id != AV_CODEC_ID_H264 && st->codecpar->codec_id != AV_CODEC_ID_HEVC;

    if(!onein_oneout) {
        int delay = st->internal->avctx->has_b_frames;
        int i;

        if (dts == AV_NOPTS_VALUE) {
            int64_t best_score = INT64_MAX;
            for (i = 0; i<delay; i++) {
                if (st->pts_reorder_error_count[i]) {
                    int64_t score = st->pts_reorder_error[i] / st->pts_reorder_error_count[i];
                    if (score < best_score) {
                        best_score = score;
                        dts = pts_buffer[i];
                    }
                }
            }
        } else {
            for (i = 0; i<delay; i++) {
                if (pts_buffer[i] != AV_NOPTS_VALUE) {
                    int64_t diff =  FFABS(pts_buffer[i] - dts) + (uint64_t)st->pts_reorder_error[i];
                    diff = FFMAX(diff, st->pts_reorder_error[i]);
                    st->pts_reorder_error[i] = diff;
                    st->pts_reorder_error_count[i]++;
                    if (st->pts_reorder_error_count[i] > 250) {
                        st->pts_reorder_error[i] >>= 1;
                        st->pts_reorder_error_count[i] >>= 1;
                    }
                }
            }
        }
    }

    if (dts == AV_NOPTS_VALUE)
        dts = pts_buffer[0];

    return dts;
}

static void update_dts_from_pts(AVFormatContext *s, int stream_index, AVPacketList *pkt_buffer)
{
    AVStream *st = s->streams[stream_index];
    int delay = st->internal->avctx->has_b_frames;
    int i;

    int64_t pts_buffer[MAX_REORDER_DELAY+1];

    for (i = 0; i<MAX_REORDER_DELAY+1; i++)
        pts_buffer[i] = AV_NOPTS_VALUE;

    for (; pkt_buffer; pkt_buffer = get_next_pkt(s, st, pkt_buffer)) {
        if (pkt_buffer->pkt.stream_index != stream_index)
            continue;

        if (pkt_buffer->pkt.pts != AV_NOPTS_VALUE && delay <= MAX_REORDER_DELAY) {
            pts_buffer[0] = pkt_buffer->pkt.pts;
            for (i = 0; i<delay && pts_buffer[i] > pts_buffer[i + 1]; i++)
                FFSWAP(int64_t, pts_buffer[i], pts_buffer[i + 1]);

            pkt_buffer->pkt.dts = select_from_pts_buffer(st, pts_buffer, pkt_buffer->pkt.dts);
        }
    }
}

#if 0
static int extract_extradata(AVStream *st, AVPacket *pkt)
{
    AVStreamInternal *sti = st->internal;
    AVPacket *pkt_ref;
    int ret;

    if (!sti->extract_extradata.inited) {
        /*ret = extract_extradata_init(st);
        if (ret < 0)
            return ret;*/
    }

    if (sti->extract_extradata.inited && !sti->extract_extradata.bsf)
        return 0;

    pkt_ref = sti->extract_extradata.pkt;
    ret = av_packet_ref(pkt_ref, pkt);
    if (ret < 0)
        return ret;

    ret = av_bsf_send_packet(sti->extract_extradata.bsf, pkt_ref);
    if (ret < 0) {
        av_packet_unref(pkt_ref);
        return ret;
    }

    while (ret >= 0 && !sti->avctx->extradata) {
        int extradata_size;
        uint8_t *extradata;

        ret = av_bsf_receive_packet(sti->extract_extradata.bsf, pkt_ref);
        if (ret < 0) {
            if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
                return ret;
            continue;
        }

        extradata = av_packet_get_side_data(pkt_ref, AV_PKT_DATA_NEW_EXTRADATA,
                                            &extradata_size);

        if (extradata) {
            av_assert0(!sti->avctx->extradata);
            if ((unsigned)extradata_size < FF_MAX_EXTRADATA_SIZE)
                sti->avctx->extradata = av_mallocz(extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
            if (!sti->avctx->extradata) {
                av_packet_unref(pkt_ref);
                return AVERROR(ENOMEM);
            }
            memcpy(sti->avctx->extradata, extradata, extradata_size);
            sti->avctx->extradata_size = extradata_size;
        }
        av_packet_unref(pkt_ref);
    }

    return 0;
}
#endif

static int probe_codec(AVFormatContext *s, AVStream *st, const AVPacket *pkt)
{
    if (st->request_probe>0) {
        AVProbeData *pd = &st->probe_data;
        int end;
        av_log(s, AV_LOG_DEBUG, "probing stream %d pp:%d\n", st->index, st->probe_packets);
        --st->probe_packets;

        if (pkt) {
            uint8_t *new_buf = av_realloc(pd->buf, pd->buf_size+pkt->size+AVPROBE_PADDING_SIZE);
            if (!new_buf) {
                av_log(s, AV_LOG_WARNING, "Failed to reallocate probe buffer for stream %d\n", st->index);
                goto no_packet;
            }
            pd->buf = new_buf;
            memcpy(pd->buf + pd->buf_size, pkt->data, pkt->size);
            pd->buf_size += pkt->size;
            memset(pd->buf + pd->buf_size, 0, AVPROBE_PADDING_SIZE);
        } else {
no_packet:
            st->probe_packets = 0;
            if (!pd->buf_size) {
                av_log(s, AV_LOG_WARNING, "nothing to probe for stream %d\n", st->index);
            }
        }

        end= s->internal->raw_packet_buffer_remaining_size <= 0 || st->probe_packets<= 0;

        /*if (end || av_log2(pd->buf_size) != av_log2(pd->buf_size - pkt->size)) {
            int score = set_codec_from_probe_data(s, st, pd);
            if ((st->codecpar->codec_id != AV_CODEC_ID_NONE && score > AVPROBE_SCORE_STREAM_RETRY) || end) 
            {
                pd->buf_size = 0;
                av_freep(&pd->buf);
                st->request_probe = -1;
                if (st->codecpar->codec_id != AV_CODEC_ID_NONE) {
                    av_log(s, AV_LOG_DEBUG, "probed stream %d\n", st->index);
                } else
                    av_log(s, AV_LOG_WARNING, "probed stream %d failed\n", st->index);
            }
            force_codec_ids(s, st);
        }*/
    }
    return 0;
}

static int update_wrap_reference(AVFormatContext *s, AVStream *st, int stream_index, AVPacket *pkt)
{
    int64_t ref = pkt->dts;
    int i, pts_wrap_behavior;
    int64_t pts_wrap_reference;
    AVProgram *first_program;

    if (ref == AV_NOPTS_VALUE)
        ref = pkt->pts;
    if (st->pts_wrap_reference != AV_NOPTS_VALUE || st->pts_wrap_bits >= 63 || ref == AV_NOPTS_VALUE || !s->correct_ts_overflow)
        return 0;
    ref &= (1LL << st->pts_wrap_bits)-1;

    // reference time stamp should be 60 s before first time stamp
    pts_wrap_reference = ref - av_rescale(60, st->time_base.den, st->time_base.num);
    // if first time stamp is not more than 1/8 and 60s before the wrap point, subtract rather than add wrap offset
    pts_wrap_behavior = (ref < (1LL << st->pts_wrap_bits) - (1LL << st->pts_wrap_bits-3)) ||
        (ref < (1LL << st->pts_wrap_bits) - av_rescale(60, st->time_base.den, st->time_base.num)) ?
        AV_PTS_WRAP_ADD_OFFSET : AV_PTS_WRAP_SUB_OFFSET;

    first_program = av_find_program_from_stream(s, NULL, stream_index);

    if (!first_program) {
        int default_stream_index = av_find_default_stream_index(s);
        if (s->streams[default_stream_index]->pts_wrap_reference == AV_NOPTS_VALUE) {
            for (i = 0; i < s->nb_streams; i++) {
                if (av_find_program_from_stream(s, NULL, i))
                    continue;
                s->streams[i]->pts_wrap_reference = pts_wrap_reference;
                s->streams[i]->pts_wrap_behavior = pts_wrap_behavior;
            }
        }
        else {
            st->pts_wrap_reference = s->streams[default_stream_index]->pts_wrap_reference;
            st->pts_wrap_behavior = s->streams[default_stream_index]->pts_wrap_behavior;
        }
    }
    else {
        AVProgram *program = first_program;
        while (program) {
            if (program->pts_wrap_reference != AV_NOPTS_VALUE) {
                pts_wrap_reference = program->pts_wrap_reference;
                pts_wrap_behavior = program->pts_wrap_behavior;
                break;
            }
            program = av_find_program_from_stream(s, program, stream_index);
        }

        // update every program with differing pts_wrap_reference
        program = first_program;
        while (program) {
            if (program->pts_wrap_reference != pts_wrap_reference) {
                for (i = 0; i<program->nb_stream_indexes; i++) {
                    s->streams[program->stream_index[i]]->pts_wrap_reference = pts_wrap_reference;
                    s->streams[program->stream_index[i]]->pts_wrap_behavior = pts_wrap_behavior;
                }

                program->pts_wrap_reference = pts_wrap_reference;
                program->pts_wrap_behavior = pts_wrap_behavior;
            }
            program = av_find_program_from_stream(s, program, stream_index);
        }
    }
    return 1;
}

static int is_relative(int64_t ts) {
    return ts > (RELATIVE_TS_BASE - (1LL<<48));
}

static int64_t wrap_timestamp(const AVStream *st, int64_t timestamp)
{
    if (st->pts_wrap_behavior != AV_PTS_WRAP_IGNORE && st->pts_wrap_reference != AV_NOPTS_VALUE && timestamp != AV_NOPTS_VALUE) {
        if (st->pts_wrap_behavior == AV_PTS_WRAP_ADD_OFFSET && timestamp < st->pts_wrap_reference)
            return timestamp + (1ULL << st->pts_wrap_bits);
        else if (st->pts_wrap_behavior == AV_PTS_WRAP_SUB_OFFSET && timestamp >= st->pts_wrap_reference)
            return timestamp - (1ULL << st->pts_wrap_bits);
    }
    
    return timestamp;
}

static void force_codec_ids(AVFormatContext *s, AVStream *st)
{
    switch (st->codecpar->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        if (s->video_codec_id)
            st->codecpar->codec_id = s->video_codec_id;
        break;
    case AVMEDIA_TYPE_AUDIO:
        if (s->audio_codec_id)
            st->codecpar->codec_id = s->audio_codec_id;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        if (s->subtitle_codec_id)
            st->codecpar->codec_id = s->subtitle_codec_id;
        break;
    case AVMEDIA_TYPE_DATA:
        if (s->data_codec_id)
            st->codecpar->codec_id = s->data_codec_id;
        break;
    }
}

int ff_packet_list_put(AVPacketList **packet_buffer, AVPacketList **plast_pktl, AVPacket *pkt, int flags)
{
    AVPacketList *pktl = av_mallocz(sizeof(AVPacketList));
    int ret;

    if (!pktl)
        return AVERROR(ENOMEM);

    if (flags & FF_PACKETLIST_FLAG_REF_PACKET) {
        if ((ret = av_packet_ref(&pktl->pkt, pkt)) < 0) {
            av_free(pktl);
            return ret;
        }
    } else {
        // TODO: Adapt callers in this file so the line below can use
        //       av_packet_move_ref() to effectively move the reference
        //       to the list.
        pktl->pkt = *pkt;
    }

    if (*packet_buffer)
        (*plast_pktl)->next = pktl;
    else
        *packet_buffer = pktl;

    /* Add the packet in the buffered packet list. */
    *plast_pktl = pktl;
    return 0;
}

int ff_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    int ret, i, err;
    AVStream *st;

    for (;;) {
        AVPacketList *pktl = s->internal->raw_packet_buffer;

        if (pktl) {
            *pkt = pktl->pkt;
            st   = s->streams[pkt->stream_index];
            if (s->internal->raw_packet_buffer_remaining_size <= 0)
                if ((err = probe_codec(s, st, NULL)) < 0)
                    return err;
            if (st->request_probe <= 0) {
                s->internal->raw_packet_buffer                 = pktl->next;
                s->internal->raw_packet_buffer_remaining_size += pkt->size;
                av_free(pktl);
                return 0;
            }
        }

        pkt->data = NULL;
        pkt->size = 0;
        av_init_packet(pkt);
        ret = s->iformat->read_packet(s, pkt);
        if (ret < 0) {
            /* Some demuxers return FFERROR_REDO when they consume
               data and discard it (ignored streams, junk, extradata).
               We must re-call the demuxer to get the real packet. */
            if (ret == FFERROR_REDO)
                continue;
            if (!pktl || ret == AVERROR(EAGAIN))
                return ret;
            for (i = 0; i < s->nb_streams; i++) {
                st = s->streams[i];
                if (st->probe_packets || st->request_probe > 0)
                    if ((err = probe_codec(s, st, NULL)) < 0)
                        return err;
                av_assert0(st->request_probe <= 0);
            }
            continue;
        }

        err = av_packet_make_refcounted(pkt);
        if (err < 0)
            return err;

        if ((s->flags & AVFMT_FLAG_DISCARD_CORRUPT) &&
            (pkt->flags & AV_PKT_FLAG_CORRUPT)) {
            av_log(s, AV_LOG_WARNING, "Dropped corrupted packet (stream = %d)\n", pkt->stream_index);
            av_packet_unref(pkt);
            continue;
        }

        if (pkt->stream_index >= (unsigned)s->nb_streams) {
            av_log(s, AV_LOG_ERROR, "Invalid stream index %d\n", pkt->stream_index);
            continue;
        }

        st = s->streams[pkt->stream_index];

        if (update_wrap_reference(s, st, pkt->stream_index, pkt) && st->pts_wrap_behavior == AV_PTS_WRAP_SUB_OFFSET) {
            // correct first time stamps to negative values
            if (!is_relative(st->first_dts))
                st->first_dts = wrap_timestamp(st, st->first_dts);
            if (!is_relative(st->start_time))
                st->start_time = wrap_timestamp(st, st->start_time);
            if (!is_relative(st->cur_dts))
                st->cur_dts = wrap_timestamp(st, st->cur_dts);
        }

        pkt->dts = wrap_timestamp(st, pkt->dts);
        pkt->pts = wrap_timestamp(st, pkt->pts);

        force_codec_ids(s, st);

        /* TODO: audio: time filter; video: frame reordering (pts != dts) */
        if (s->use_wallclock_as_timestamps)
            pkt->dts = pkt->pts = av_rescale_q(av_gettime(), AV_TIME_BASE_Q, st->time_base);

        if (!pktl && st->request_probe <= 0)
            return ret;

        err = ff_packet_list_put(&s->internal->raw_packet_buffer,
                                 &s->internal->raw_packet_buffer_end,
                                 pkt, 0);
        if (err)
            return err;
        s->internal->raw_packet_buffer_remaining_size -= pkt->size;

        if ((err = probe_codec(s, st, pkt)) < 0)
            return err;
    }
}

void ff_compute_frame_duration(AVFormatContext *s, int *pnum, int *pden, AVStream *st, AVCodecParserContext *pc, AVPacket *pkt)
{
    AVRational codec_framerate = s->iformat ? st->internal->avctx->framerate : av_mul_q(av_inv_q(st->internal->avctx->time_base), (AVRational){1, st->internal->avctx->ticks_per_frame});
    int frame_size, sample_rate;

#if FF_API_LAVF_AVCTX
FF_DISABLE_DEPRECATION_WARNINGS
    if ((!codec_framerate.den || !codec_framerate.num) && st->codec->time_base.den && st->codec->time_base.num)
        codec_framerate = av_mul_q(av_inv_q(st->codec->time_base), (AVRational){1, st->codec->ticks_per_frame});
FF_ENABLE_DEPRECATION_WARNINGS
#endif

    *pnum = 0;
    *pden = 0;
    switch (st->codecpar->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        if (st->r_frame_rate.num && !pc && s->iformat) {
            *pnum = st->r_frame_rate.den;
            *pden = st->r_frame_rate.num;
        } else if (st->time_base.num * 1000LL > st->time_base.den) {
            *pnum = st->time_base.num;
            *pden = st->time_base.den;
        } else if (codec_framerate.den * 1000LL > codec_framerate.num) {
            av_assert0(st->internal->avctx->ticks_per_frame);
            av_reduce(pnum, pden,
                      codec_framerate.den,
                      codec_framerate.num * (int64_t)st->internal->avctx->ticks_per_frame,
                      INT_MAX);

            if (pc && pc->repeat_pict) {
                av_assert0(s->iformat); // this may be wrong for interlaced encoding but its not used for that case
                av_reduce(pnum, pden,
                          (*pnum) * (1LL + pc->repeat_pict),
                          (*pden),
                          INT_MAX);
            }
            /* If this codec can be interlaced or progressive then we need
             * a parser to compute duration of a packet. Thus if we have
             * no parser in such case leave duration undefined. */
            if (st->internal->avctx->ticks_per_frame > 1 && !pc)
                *pnum = *pden = 0;
        }
        break;
    case AVMEDIA_TYPE_AUDIO:
        if (st->internal->avctx_inited) {
            frame_size = av_get_audio_frame_duration(st->internal->avctx, pkt->size);
            sample_rate = st->internal->avctx->sample_rate;
        } else {
            frame_size = av_get_audio_frame_duration2(st->codecpar, pkt->size);
            sample_rate = st->codecpar->sample_rate;
        }
        if (frame_size <= 0 || sample_rate <= 0)
            break;
        *pnum = frame_size;
        *pden = sample_rate;
        break;
    default:
        break;
    }
}

AVProgram *av_find_program_from_stream(AVFormatContext *ic, AVProgram *last, int s)
{
    int i, j;

    for (i = 0; i < ic->nb_programs; i++) {
        if (ic->programs[i] == last) {
            last = NULL;
        } else {
            if (!last)
                for (j = 0; j < ic->programs[i]->nb_stream_indexes; j++)
                    if (ic->programs[i]->stream_index[j] == s)
                        return ic->programs[i];
        }
    }
    return NULL;
}

static void update_stream_timings(AVFormatContext *ic)
{
    int64_t start_time, start_time1, start_time_text, end_time, end_time1, end_time_text;
    int64_t duration, duration1, duration_text, filesize;
    int i;
    AVProgram *p;

    start_time = INT64_MAX;
    start_time_text = INT64_MAX;
    end_time   = INT64_MIN;
    end_time_text   = INT64_MIN;
    duration   = INT64_MIN;
    duration_text = INT64_MIN;

    for (i = 0; i < ic->nb_streams; i++) {
        AVStream *st = ic->streams[i];
        int is_text = st->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE || st->codecpar->codec_type == AVMEDIA_TYPE_DATA;
        if (st->start_time != AV_NOPTS_VALUE && st->time_base.den) {
            start_time1 = av_rescale_q(st->start_time, st->time_base, AV_TIME_BASE_Q);
            if (is_text)
                start_time_text = FFMIN(start_time_text, start_time1);
            else
                start_time = FFMIN(start_time, start_time1);

            end_time1 = av_rescale_q_rnd(st->duration, st->time_base, AV_TIME_BASE_Q, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
            if (end_time1 != AV_NOPTS_VALUE && (end_time1 > 0 ? start_time1 <= INT64_MAX - end_time1 : start_time1 >= INT64_MIN - end_time1)) {
                end_time1 += start_time1;
                if (is_text)
                    end_time_text = FFMAX(end_time_text, end_time1);
                else
                    end_time = FFMAX(end_time, end_time1);
            }

            for (p = NULL; (p = av_find_program_from_stream(ic, p, i)); ) {
                if (p->start_time == AV_NOPTS_VALUE || p->start_time > start_time1)
                    p->start_time = start_time1;
                if (p->end_time < end_time1)
                    p->end_time = end_time1;
            }
        }
        if (st->duration != AV_NOPTS_VALUE) {
            duration1 = av_rescale_q(st->duration, st->time_base, AV_TIME_BASE_Q);
            if (is_text)
                duration_text = FFMAX(duration_text, duration1);
            else
                duration = FFMAX(duration, duration1);
        }
    }
    if (start_time == INT64_MAX || (start_time > start_time_text && start_time - (uint64_t)start_time_text < AV_TIME_BASE))
        start_time = start_time_text;
    else if (start_time > start_time_text)
        av_log(ic, AV_LOG_VERBOSE, "Ignoring outlier non primary stream starttime %f\n", start_time_text / (float)AV_TIME_BASE);

    if (end_time == INT64_MIN || (end_time < end_time_text && end_time_text - (uint64_t)end_time < AV_TIME_BASE))
        end_time = end_time_text;
    else if (end_time < end_time_text)
        av_log(ic, AV_LOG_VERBOSE, "Ignoring outlier non primary stream endtime %f\n", end_time_text / (float)AV_TIME_BASE);

     if (duration == INT64_MIN || (duration < duration_text && duration_text - duration < AV_TIME_BASE))
         duration = duration_text;
     else if (duration < duration_text)
         av_log(ic, AV_LOG_VERBOSE, "Ignoring outlier non primary stream duration %f\n", duration_text / (float)AV_TIME_BASE);

    if (start_time != INT64_MAX) {
        ic->start_time = start_time;
        if (end_time != INT64_MIN) {
            if (ic->nb_programs > 1) {
                for (i = 0; i < ic->nb_programs; i++) {
                    p = ic->programs[i];
                    if (p->start_time != AV_NOPTS_VALUE && p->end_time > p->start_time && p->end_time - (uint64_t)p->start_time <= INT64_MAX)
                        duration = FFMAX(duration, p->end_time - p->start_time);
                }
            } else if (end_time >= start_time && end_time - (uint64_t)start_time <= INT64_MAX) {
                duration = FFMAX(duration, end_time - start_time);
            }
        }
    }
    if (duration != INT64_MIN && duration > 0 && ic->duration == AV_NOPTS_VALUE) {
        ic->duration = duration;
    }
    if (ic->pb && (filesize = avio_size(ic->pb)) > 0 && ic->duration > 0) {
        /* compute the bitrate */
        double bitrate = (double) filesize * 8.0 * AV_TIME_BASE / (double) ic->duration;
        if (bitrate >= 0 && bitrate <= INT64_MAX)
            ic->bit_rate = bitrate;
    }
}

static void fill_all_stream_timings(AVFormatContext *ic)
{
    int i;
    AVStream *st;

    update_stream_timings(ic);
    for (i = 0; i < ic->nb_streams; i++) {
        st = ic->streams[i];
        if (st->start_time == AV_NOPTS_VALUE) {
            if (ic->start_time != AV_NOPTS_VALUE)
                st->start_time = av_rescale_q(ic->start_time, AV_TIME_BASE_Q,
                                              st->time_base);
            if (ic->duration != AV_NOPTS_VALUE)
                st->duration = av_rescale_q(ic->duration, AV_TIME_BASE_Q,
                                            st->time_base);
        }
    }
}

static void estimate_timings_from_pts(AVFormatContext *ic, int64_t old_offset)
{
    AVPacket pkt1, *pkt = &pkt1;
    AVStream *st;
    int num, den, read_size, i, ret;
    int found_duration = 0;
    int is_end;
    int64_t filesize, offset, duration;
    int retry = 0;

    /* flush packet queue */
    flush_packet_queue(ic);

    for (i = 0; i < ic->nb_streams; i++) {
        st = ic->streams[i];
        if (st->start_time == AV_NOPTS_VALUE && st->first_dts == AV_NOPTS_VALUE 
                                             && st->codecpar->codec_type != AVMEDIA_TYPE_UNKNOWN)
            av_log(ic, AV_LOG_WARNING, "start time for stream %d is not set in estimate_timings_from_pts\n", i);

        /*if (st->parser) {
            av_parser_close(st->parser);
            st->parser = NULL;
        }*/
    }

    if (ic->skip_estimate_duration_from_pts) {
        av_log(ic, AV_LOG_INFO, "Skipping duration calculation in estimate_timings_from_pts\n");
        goto skip_duration_calc;
    }

    av_opt_set(ic, "skip_changes", "1", AV_OPT_SEARCH_CHILDREN);
    /* estimate the end time (duration) */
    /* XXX: may need to support wrapping */
    filesize = ic->pb ? avio_size(ic->pb) : 0;
    do {
        is_end = found_duration;
        offset = filesize - (DURATION_MAX_READ_SIZE << retry);
        if (offset < 0)
            offset = 0;

        avio_seek(ic->pb, offset, SEEK_SET);
        read_size = 0;
        for (;;) {
            if (read_size >= DURATION_MAX_READ_SIZE << (FFMAX(retry - 1, 0)))
                break;

            do {
                ret = ff_read_packet(ic, pkt);
            } while (ret == AVERROR(EAGAIN));
            if (ret != 0)
                break;
            read_size += pkt->size;
            st         = ic->streams[pkt->stream_index];
            if (pkt->pts != AV_NOPTS_VALUE &&
                (st->start_time != AV_NOPTS_VALUE ||
                 st->first_dts  != AV_NOPTS_VALUE)) {
                if (pkt->duration == 0) {
                    ff_compute_frame_duration(ic, &num, &den, st, st->parser, pkt);
                    if (den && num) {
                        pkt->duration = av_rescale_rnd(1,
                                           num * (int64_t) st->time_base.den,
                                           den * (int64_t) st->time_base.num,
                                           AV_ROUND_DOWN);
                    }
                }
                duration = pkt->pts + pkt->duration;
                found_duration = 1;
                if (st->start_time != AV_NOPTS_VALUE)
                    duration -= st->start_time;
                else
                    duration -= st->first_dts;
                if (duration > 0) {
                    if (st->duration == AV_NOPTS_VALUE || st->info->last_duration<= 0 ||
                        (st->duration < duration && FFABS(duration - st->info->last_duration) < 60LL*st->time_base.den / st->time_base.num))
                        st->duration = duration;
                    st->info->last_duration = duration;
                }
            }
            av_packet_unref(pkt);
        }

        /* check if all audio/video streams have valid duration */
        if (!is_end) {
            is_end = 1;
            for (i = 0; i < ic->nb_streams; i++) {
                st = ic->streams[i];
                switch (st->codecpar->codec_type) {
                    case AVMEDIA_TYPE_VIDEO:
                    case AVMEDIA_TYPE_AUDIO:
                        if (st->duration == AV_NOPTS_VALUE)
                            is_end = 0;
                }
            }
        }
    } while (!is_end && offset && ++retry <= DURATION_MAX_RETRY);

    av_opt_set(ic, "skip_changes", "0", AV_OPT_SEARCH_CHILDREN);

    /* warn about audio/video streams which duration could not be estimated */
    for (i = 0; i < ic->nb_streams; i++) {
        st = ic->streams[i];
        if (st->duration == AV_NOPTS_VALUE) {
            switch (st->codecpar->codec_type) {
            case AVMEDIA_TYPE_VIDEO:
            case AVMEDIA_TYPE_AUDIO:
                if (st->start_time != AV_NOPTS_VALUE || st->first_dts  != AV_NOPTS_VALUE) {
                    av_log(ic, AV_LOG_DEBUG, "stream %d : no PTS found at end of file, duration not set\n", i);
                } else
                    av_log(ic, AV_LOG_DEBUG, "stream %d : no TS found at start of file, duration not set\n", i);
            }
        }
    }
skip_duration_calc:
    fill_all_stream_timings(ic);

    avio_seek(ic->pb, old_offset, SEEK_SET);
    for (i = 0; i < ic->nb_streams; i++) {
        int j;

        st              = ic->streams[i];
        st->cur_dts     = st->first_dts;
        st->last_IP_pts = AV_NOPTS_VALUE;
        st->last_dts_for_order_check = AV_NOPTS_VALUE;
        for (j = 0; j < MAX_REORDER_DELAY + 1; j++)
            st->pts_buffer[j] = AV_NOPTS_VALUE;
    }
}

static int has_duration(AVFormatContext *ic)
{
    int i;
    AVStream *st;

    for (i = 0; i < ic->nb_streams; i++) {
        st = ic->streams[i];
        if (st->duration != AV_NOPTS_VALUE)
            return 1;
    }
    
    if (ic->duration != AV_NOPTS_VALUE)
        return 1;
    
    return 0;
}

static void estimate_timings(AVFormatContext *ic, int64_t old_offset)
{
    int64_t file_size;

    /* get the file size, if possible */
    if (ic->iformat->flags & AVFMT_NOFILE) {
        file_size = 0;
    } else {
        file_size = avio_size(ic->pb);
        file_size = FFMAX(0, file_size);
    }

    if ((!strcmp(ic->iformat->name, "mpeg") || !strcmp(ic->iformat->name, "mpegts")) &&
        file_size && (ic->pb->seekable & AVIO_SEEKABLE_NORMAL)) {
        /* get accurate estimate from the PTSes */
        estimate_timings_from_pts(ic, old_offset);
        ic->duration_estimation_method = AVFMT_DURATION_FROM_PTS;
    } else if (has_duration(ic)) {
        /* at least one component has timings - we use them for all
         * the components */
        fill_all_stream_timings(ic);
        ic->duration_estimation_method = AVFMT_DURATION_FROM_STREAM;
    } else {
        /* less precise: use bitrate info */
        //estimate_timings_from_bit_rate(ic);
        ic->duration_estimation_method = AVFMT_DURATION_FROM_BITRATE;
    }
    update_stream_timings(ic);

    {
        int i;
        AVStream *st;
        for (i = 0; i < ic->nb_streams; i++) {
            st = ic->streams[i];
            av_log(ic, AV_LOG_TRACE, "stream %d: start_time: %0.3f duration: %0.3f\n", i,
                   (double) st->start_time * av_q2d(st->time_base),
                   (double) st->duration   * av_q2d(st->time_base));
        }
        
        av_log(ic, AV_LOG_TRACE, "format: start_time: %0.3f duration: %0.3f bitrate=%"PRId64" kb/s\n",
                (double) ic->start_time / AV_TIME_BASE,
                (double) ic->duration   / AV_TIME_BASE,
                (int64_t)ic->bit_rate / 1000);
    }
}

static void compute_chapters_end(AVFormatContext *s)
{
    unsigned int i, j;
    int64_t max_time = 0;

    if (s->duration > 0 && s->start_time < INT64_MAX - s->duration)
        max_time = s->duration + ((s->start_time == AV_NOPTS_VALUE) ? 0 : s->start_time);

    for (i = 0; i < s->nb_chapters; i++)
        if (s->chapters[i]->end == AV_NOPTS_VALUE) {
            AVChapter *ch = s->chapters[i];
            int64_t end = max_time ? av_rescale_q(max_time, AV_TIME_BASE_Q, ch->time_base) : INT64_MAX;

            for (j = 0; j < s->nb_chapters; j++) {
                AVChapter *ch1     = s->chapters[j];
                int64_t next_start = av_rescale_q(ch1->start, ch1->time_base, ch->time_base);
                if (j != i && next_start > ch->start && next_start < end)
                    end = next_start;
            }
            ch->end = (end == INT64_MAX || end < ch->start) ? ch->start : end;
        }
}

int avformat_find_stream_info(AVFormatContext *ic, AVDictionary **options)
{
    int i, count = 0, ret = 0, j;
    int64_t read_size;
    AVStream *st;
    AVCodecContext *avctx;
    AVPacket pkt1, *pkt;
    int64_t old_offset  = avio_tell(ic->pb);
    // new streams might appear, no options for those
    int orig_nb_streams = ic->nb_streams;
    int flush_codecs;
    int64_t max_analyze_duration = ic->max_analyze_duration;
    int64_t max_stream_analyze_duration;
    int64_t max_subtitle_analyze_duration;
    int64_t probesize = ic->probesize;
    int eof_reached = 0;
    int *missing_streams = av_opt_ptr(ic->iformat->priv_class, ic->priv_data, "missing_streams");

    flush_codecs = probesize > 0;

    av_opt_set(ic, "skip_clear", "1", AV_OPT_SEARCH_CHILDREN);

    max_stream_analyze_duration = max_analyze_duration;
    max_subtitle_analyze_duration = max_analyze_duration;
    if (!max_analyze_duration) {
        max_stream_analyze_duration =
        max_analyze_duration        = 5*AV_TIME_BASE;
        max_subtitle_analyze_duration = 30*AV_TIME_BASE;
        if (!strcmp(ic->iformat->name, "flv"))
            max_stream_analyze_duration = 90*AV_TIME_BASE;
        if (!strcmp(ic->iformat->name, "mpeg") || !strcmp(ic->iformat->name, "mpegts"))
            max_stream_analyze_duration = 7*AV_TIME_BASE;
    }

    if (ic->pb) {
        av_log(ic, AV_LOG_DEBUG, "Before avformat_find_stream_info() pos: %"PRId64" bytes read:%"PRId64" seeks:%d nb_streams:%d\n",
            avio_tell(ic->pb), ic->pb->bytes_read, ic->pb->seek_count, ic->nb_streams);
    }

    for (i = 0; i < ic->nb_streams; i++) {
        const AVCodec *codec;
        AVDictionary *thread_opt = NULL;
        st = ic->streams[i];
        avctx = st->internal->avctx;

        if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO || st->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
/*            if (!st->time_base.num)
                st->time_base = */
            if (!avctx->time_base.num)
                avctx->time_base = st->time_base;
        }

        /* check if the caller has overridden the codec id */
#if 0 //FF_API_LAVF_AVCTX
FF_DISABLE_DEPRECATION_WARNINGS
        if (st->codec->codec_id != st->internal->orig_codec_id) {
            st->codecpar->codec_id   = st->codec->codec_id;
            st->codecpar->codec_type = st->codec->codec_type;
            st->internal->orig_codec_id = st->codec->codec_id;
        }
FF_ENABLE_DEPRECATION_WARNINGS
#endif
        // only for the split stuff
        /*
        if (!st->parser && !(ic->flags & AVFMT_FLAG_NOPARSE) && st->request_probe <= 0) {
            st->parser = av_parser_init(st->codecpar->codec_id);
            if (st->parser) {
                if (st->need_parsing == AVSTREAM_PARSE_HEADERS) {
                    st->parser->flags |= PARSER_FLAG_COMPLETE_FRAMES;
                } else if (st->need_parsing == AVSTREAM_PARSE_FULL_RAW) {
                    st->parser->flags |= PARSER_FLAG_USE_CODEC_TS;
                }
            } else if (st->need_parsing) {
                av_log(ic, AV_LOG_VERBOSE, "parser not found for codec "
                       "%s, packets or times may be invalid.\n",
                       avcodec_get_name(st->codecpar->codec_id));
            }
        }
        */

        if (st->codecpar->codec_id != st->internal->orig_codec_id)
            st->internal->orig_codec_id = st->codecpar->codec_id;

        ret = avcodec_parameters_to_context(avctx, st->codecpar);
        if (ret < 0)
            goto find_stream_info_err;
        
        if (st->request_probe <= 0)
            st->internal->avctx_inited = 1;

        //codec = find_probe_decoder(ic, st, st->codecpar->codec_id);

        /* Force thread count to 1 since the H.264 decoder will not extract
         * SPS and PPS to extradata during multi-threaded decoding. */
        av_dict_set(options ? &options[i] : &thread_opt, "threads", "1", 0);

        if (ic->codec_whitelist)
            av_dict_set(options ? &options[i] : &thread_opt, "codec_whitelist", ic->codec_whitelist, 0);

        /* Ensure that subtitle_header is properly set. */
        /*if (st->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE
            && codec && !avctx->codec) {
            if (avcodec_open2(avctx, codec, options ? &options[i] : &thread_opt) < 0)
                av_log(ic, AV_LOG_WARNING,
                       "Failed to open codec in %s\n",__FUNCTION__);
        }*/

        // Try to just open decoders, in case this is enough to get parameters.
        /*if (!has_codec_parameters(st, NULL) && st->request_probe <= 0) {
            if (codec && !avctx->codec)
                if (avcodec_open2(avctx, codec, options ? &options[i] : &thread_opt) < 0)
                    av_log(ic, AV_LOG_WARNING,
                           "Failed to open codec in %s\n",__FUNCTION__);
        }*/
        if (!options)
            av_dict_free(&thread_opt);
    }

    for (i = 0; i < ic->nb_streams; i++) {
#if FF_API_R_FRAME_RATE
        ic->streams[i]->info->last_dts = AV_NOPTS_VALUE;
#endif
        ic->streams[i]->info->fps_first_dts = AV_NOPTS_VALUE;
        ic->streams[i]->info->fps_last_dts  = AV_NOPTS_VALUE;
    }

    read_size = 0;
    for (;;) {
        int analyzed_all_streams;
        /*if (ff_check_interrupt(&ic->interrupt_callback)) {
            ret = AVERROR_EXIT;
            av_log(ic, AV_LOG_DEBUG, "interrupted\n");
            break;
        }*/

        /* check if one codec still needs to be handled */
        for (i = 0; i < ic->nb_streams; i++) {
            int fps_analyze_framecount = 20;
            int count;

            st = ic->streams[i];
            //if (!has_codec_parameters(st, NULL))
            //    break;
            
            /* If the timebase is coarse (like the usual millisecond precision
             * of mkv), we need to analyze more frames to reliably arrive at
             * the correct fps. */
            if (av_q2d(st->time_base) > 0.0005)
                fps_analyze_framecount *= 2;
            
            /*if (!tb_unreliable(st->internal->avctx))
                fps_analyze_framecount = 0;*/
            
            if (ic->fps_probe_size >= 0)
                fps_analyze_framecount = ic->fps_probe_size;
            
            if (st->disposition & AV_DISPOSITION_ATTACHED_PIC)
                fps_analyze_framecount = 0;
            
            /* variable fps and no guess at the real fps */
            count = (ic->iformat->flags & AVFMT_NOTIMESTAMPS) ? st->info->codec_info_duration_fields/2 : st->info->duration_count;
            if (!(st->r_frame_rate.num && st->avg_frame_rate.num) && st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                if (count < fps_analyze_framecount)
                    break;
            }
            
            // Look at the first 3 frames if there is evidence of frame delay
            // but the decoder delay is not set.
            /*if (st->info->frame_delay_evidence && count < 2 && st->internal->avctx->has_b_frames == 0)
                break;
                */
            
            /*if (!st->internal->avctx->extradata && (!st->internal->extract_extradata.inited || st->internal->extract_extradata.bsf) 
                                                && extract_extradata_check(st))
                break;*/
            
            if (st->first_dts == AV_NOPTS_VALUE && !(ic->iformat->flags & AVFMT_NOTIMESTAMPS) 
                                                && st->codec_info_nb_frames < ((st->disposition & AV_DISPOSITION_ATTACHED_PIC) ? 1 : ic->max_ts_probe) 
                                                && (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO || st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO))
                break;
        }
        analyzed_all_streams = 0;
        if (!missing_streams || !*missing_streams)
            if (i == ic->nb_streams) {
                analyzed_all_streams = 1;
                /* NOTE: If the format has no header, then we need to read some
                 * packets to get most of the streams, so we cannot stop here. */
                //if (!(ic->ctx_flags & AVFMTCTX_NOHEADER)) {
                    /* If we found the info for all the codecs, we can stop. */
                /*    ret = count;
                    av_log(ic, AV_LOG_DEBUG, "All info found\n");
                    flush_codecs = 0;
                    break;
                }*/
            }
        /* We did not get all the codec info, but we read too much data. */
        if (read_size >= probesize) {
            ret = count;
            av_log(ic, AV_LOG_DEBUG, "Probe buffer size limit of %"PRId64" bytes reached\n", probesize);
            for (i = 0; i < ic->nb_streams; i++)
                if (!ic->streams[i]->r_frame_rate.num && ic->streams[i]->info->duration_count <= 1 
                                                      && ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO 
                                                      && strcmp(ic->iformat->name, "image2"))
                    av_log(ic, AV_LOG_WARNING, "Stream #%d: not enough frames to estimate rate;consider increasing probesize\n", i);
            break;
        }

        /* NOTE: A new stream can be added there if no header in file
         * (AVFMTCTX_NOHEADER). */
        /*ret = read_frame_internal(ic, &pkt1);
        if (ret == AVERROR(EAGAIN))
            continue;*/

        if (ret < 0) {
            /* EOF or error*/
            eof_reached = 1;
            break;
        }

        pkt = &pkt1;

        /*if (!(ic->flags & AVFMT_FLAG_NOBUFFER)) {
            ret = ff_packet_list_put(&ic->internal->packet_buffer, &ic->internal->packet_buffer_end, pkt, 0);
            if (ret < 0)
                goto find_stream_info_err;
        }*/

        st = ic->streams[pkt->stream_index];
        if (!(st->disposition & AV_DISPOSITION_ATTACHED_PIC))
            read_size += pkt->size;

        avctx = st->internal->avctx;
        if (!st->internal->avctx_inited) {
            ret = avcodec_parameters_to_context(avctx, st->codecpar);
            if (ret < 0)
                goto find_stream_info_err;
            st->internal->avctx_inited = 1;
        }

        if (pkt->dts != AV_NOPTS_VALUE && st->codec_info_nb_frames > 1) {
            /* check for non-increasing dts */
            if (st->info->fps_last_dts != AV_NOPTS_VALUE &&
                st->info->fps_last_dts >= pkt->dts) {
                av_log(ic, AV_LOG_DEBUG, "Non-increasing DTS in stream %d: packet %d with DTS %"PRId64", packet %d with DTS %"PRId64"\n",
                       st->index, st->info->fps_last_dts_idx,
                       st->info->fps_last_dts, st->codec_info_nb_frames,
                       pkt->dts);
                st->info->fps_first_dts =
                st->info->fps_last_dts  = AV_NOPTS_VALUE;
            }
            /* Check for a discontinuity in dts. If the difference in dts
             * is more than 1000 times the average packet duration in the
             * sequence, we treat it as a discontinuity. */
            if (st->info->fps_last_dts != AV_NOPTS_VALUE &&
                st->info->fps_last_dts_idx > st->info->fps_first_dts_idx &&
                (pkt->dts - st->info->fps_last_dts) / 1000 >
                (st->info->fps_last_dts     - (uint64_t)st->info->fps_first_dts) /
                (st->info->fps_last_dts_idx - st->info->fps_first_dts_idx)) {
                av_log(ic, AV_LOG_WARNING,
                       "DTS discontinuity in stream %d: packet %d with DTS "
                       "%"PRId64", packet %d with DTS %"PRId64"\n",
                       st->index, st->info->fps_last_dts_idx,
                       st->info->fps_last_dts, st->codec_info_nb_frames,
                       pkt->dts);
                st->info->fps_first_dts =
                st->info->fps_last_dts  = AV_NOPTS_VALUE;
            }

            /* update stored dts values */
            if (st->info->fps_first_dts == AV_NOPTS_VALUE) {
                st->info->fps_first_dts     = pkt->dts;
                st->info->fps_first_dts_idx = st->codec_info_nb_frames;
            }
            st->info->fps_last_dts     = pkt->dts;
            st->info->fps_last_dts_idx = st->codec_info_nb_frames;
        }
        if (st->codec_info_nb_frames>1) {
            int64_t t = 0;
            int64_t limit;

            if (st->time_base.den > 0)
                t = av_rescale_q(st->info->codec_info_duration, st->time_base, AV_TIME_BASE_Q);

            /*if (st->avg_frame_rate.num > 0)
                t = FFMAX(t, av_rescale_q(st->codec_info_nb_frames, av_inv_q(st->avg_frame_rate), AV_TIME_BASE_Q));
            */

            if (t == 0 && st->codec_info_nb_frames>30
                       && st->info->fps_first_dts != AV_NOPTS_VALUE
                       && st->info->fps_last_dts  != AV_NOPTS_VALUE)
                t = FFMAX(t, av_rescale_q(st->info->fps_last_dts - st->info->fps_first_dts, st->time_base, AV_TIME_BASE_Q));

            if (analyzed_all_streams) {
                limit = max_analyze_duration;
            } else if (avctx->codec_type == AVMEDIA_TYPE_SUBTITLE) {
                limit = max_subtitle_analyze_duration;
            } else {
                limit = max_stream_analyze_duration;
            }

            if (t >= limit) {
                av_log(ic, AV_LOG_VERBOSE, "max_analyze_duration %"PRId64" reached at %"PRId64" microseconds st:%d\n",
                       limit,  t, pkt->stream_index);
                
                if (ic->flags & AVFMT_FLAG_NOBUFFER)
                    av_packet_unref(pkt);
                
                break;
            }
            if (pkt->duration) {
                if (avctx->codec_type == AVMEDIA_TYPE_SUBTITLE && pkt->pts != AV_NOPTS_VALUE && st->start_time != AV_NOPTS_VALUE && pkt->pts >= st->start_time) {
                    st->info->codec_info_duration = FFMIN(pkt->pts - st->start_time, st->info->codec_info_duration + pkt->duration);
                } else
                    st->info->codec_info_duration += pkt->duration;
                //st->info->codec_info_duration_fields += st->parser && st->need_parsing && avctx->ticks_per_frame ==2 ? st->parser->repeat_pict + 1 : 2;
            }
        }

        if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
#if 0//FF_API_R_FRAME_RATE
            ff_rfps_add_frame(ic, st, pkt->dts);
#endif
            /*if (pkt->dts != pkt->pts && pkt->dts != AV_NOPTS_VALUE && pkt->pts != AV_NOPTS_VALUE)
                st->info->frame_delay_evidence = 1;*/
        }
        
        /*if (!st->internal->avctx->extradata) {
            ret = extract_extradata(st, pkt);
            if (ret < 0)
                goto find_stream_info_err;
        }*/

        /* If still no information, we try to open the codec and to
         * decompress the frame. We try to avoid that in most cases as
         * it takes longer and uses more memory. For MPEG-4, we need to
         * decompress for QuickTime.
         *
         * If AV_CODEC_CAP_CHANNEL_CONF is set this will force decoding of at
         * least one frame of codec data, this makes sure the codec initializes
         * the channel configuration and does not only trust the values from
         * the container. */
        try_decode_frame(ic, st, pkt, (options && i < orig_nb_streams) ? &options[i] : NULL);

        if (ic->flags & AVFMT_FLAG_NOBUFFER)
            av_packet_unref(pkt);

        st->codec_info_nb_frames++;
        count++;
    }

    if (eof_reached) {
        int stream_index;
        for (stream_index = 0; stream_index < ic->nb_streams; stream_index++) {
            st = ic->streams[stream_index];
            avctx = st->internal->avctx;
            if (!has_codec_parameters(st, NULL)) {
                const AVCodec *codec = find_probe_decoder(ic, st, st->codecpar->codec_id);
                if (codec && !avctx->codec) {
                    AVDictionary *opts = NULL;
                    if (ic->codec_whitelist)
                        av_dict_set(&opts, "codec_whitelist", ic->codec_whitelist, 0);
                    if (avcodec_open2(avctx, codec, (options && stream_index < orig_nb_streams) ? &options[stream_index] : &opts) < 0)
                        av_log(ic, AV_LOG_WARNING, "Failed to open codec in %s\n",__FUNCTION__);
                    av_dict_free(&opts);
                }
            }

            // EOF already reached while reading the stream above.
            // So continue with reoordering DTS with whatever delay we have.
            if (ic->internal->packet_buffer && !has_decode_delay_been_guessed(st)) {
                update_dts_from_pts(ic, stream_index, ic->internal->packet_buffer);
            }
        }
    }

    if (flush_codecs) {
        AVPacket empty_pkt = { 0 };
        int err = 0;
        av_init_packet(&empty_pkt);

        for (i = 0; i < ic->nb_streams; i++) {

            st = ic->streams[i];

            /* flush the decoders */
            if (st->info->found_decoder == 1) {
                do {
                    err = try_decode_frame(ic, st, &empty_pkt, (options && i < orig_nb_streams) ? &options[i] : NULL);
                } while (err > 0 && !has_codec_parameters(st, NULL));

                if (err < 0) {
                    av_log(ic, AV_LOG_INFO, "decoding for stream %d failed\n", st->index);
                }
            }
        }
    }

    //ff_rfps_calculate(ic);

    for (i = 0; i < ic->nb_streams; i++) {
        st = ic->streams[i];
        avctx = st->internal->avctx;
        if (avctx->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (avctx->codec_id == AV_CODEC_ID_RAWVIDEO && !avctx->codec_tag && !avctx->bits_per_coded_sample) {
                //uint32_t tag= avcodec_pix_fmt_to_codec_tag(avctx->pix_fmt);
                //if (avpriv_find_pix_fmt(avpriv_get_raw_pix_fmt_tags(), tag) == avctx->pix_fmt)
                //    avctx->codec_tag= tag;
            }

            /* estimate average framerate if not set by demuxer */
            if (st->info->codec_info_duration_fields &&
                !st->avg_frame_rate.num &&
                st->info->codec_info_duration) {
                int best_fps      = 0;
                double best_error = 0.01;
                AVRational codec_frame_rate = avctx->framerate;

                if (st->info->codec_info_duration        >= INT64_MAX / st->time_base.num / 2||
                    st->info->codec_info_duration_fields >= INT64_MAX / st->time_base.den ||
                    st->info->codec_info_duration        < 0)
                    continue;
                av_reduce(&st->avg_frame_rate.num, &st->avg_frame_rate.den,
                          st->info->codec_info_duration_fields * (int64_t) st->time_base.den,
                          st->info->codec_info_duration * 2 * (int64_t) st->time_base.num, 60000);

                /* Round guessed framerate to a "standard" framerate if it's within 1% of the original estimate. */
                /*for (j = 0; j < MAX_STD_TIMEBASES; j++) {
                    AVRational std_fps = { get_std_framerate(j), 12 * 1001 };
                    double error       = fabs(av_q2d(st->avg_frame_rate) / av_q2d(std_fps) - 1);

                    if (error < best_error) {
                        best_error = error;
                        best_fps   = std_fps.num;
                    }

                    if (ic->internal->prefer_codec_framerate && codec_frame_rate.num > 0 && codec_frame_rate.den > 0) {
                        error       = fabs(av_q2d(codec_frame_rate) / av_q2d(std_fps) - 1);
                        if (error < best_error) {
                            best_error = error;
                            best_fps   = std_fps.num;
                        }
                    }
                }*/
                if (best_fps)
                    av_reduce(&st->avg_frame_rate.num, &st->avg_frame_rate.den, best_fps, 12 * 1001, INT_MAX);
            }

            if (!st->r_frame_rate.num) {
                if (avctx->time_base.den * (int64_t) st->time_base.num <= avctx->time_base.num * avctx->ticks_per_frame * (int64_t) st->time_base.den) {
                    av_reduce(&st->r_frame_rate.num, &st->r_frame_rate.den, avctx->time_base.den, (int64_t)avctx->time_base.num * avctx->ticks_per_frame, INT_MAX);
                } else {
                    st->r_frame_rate.num = st->time_base.den;
                    st->r_frame_rate.den = st->time_base.num;
                }
            }
            if (st->display_aspect_ratio.num && st->display_aspect_ratio.den) {
                AVRational hw_ratio = { avctx->height, avctx->width };
                st->sample_aspect_ratio = av_mul_q(st->display_aspect_ratio, hw_ratio);
            }
        } else if (avctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (!avctx->bits_per_coded_sample)
                avctx->bits_per_coded_sample =
                    av_get_bits_per_sample(avctx->codec_id);
            // set stream disposition based on audio service type
            switch (avctx->audio_service_type) {
            case AV_AUDIO_SERVICE_TYPE_EFFECTS:
                st->disposition = AV_DISPOSITION_CLEAN_EFFECTS;
                break;
            case AV_AUDIO_SERVICE_TYPE_VISUALLY_IMPAIRED:
                st->disposition = AV_DISPOSITION_VISUAL_IMPAIRED;
                break;
            case AV_AUDIO_SERVICE_TYPE_HEARING_IMPAIRED:
                st->disposition = AV_DISPOSITION_HEARING_IMPAIRED;
                break;
            case AV_AUDIO_SERVICE_TYPE_COMMENTARY:
                st->disposition = AV_DISPOSITION_COMMENT;
                break;
            case AV_AUDIO_SERVICE_TYPE_KARAOKE:
                st->disposition = AV_DISPOSITION_KARAOKE;
                break;
            }
        }
    }

    if (probesize)
        estimate_timings(ic, old_offset);

    av_opt_set(ic, "skip_clear", "0", AV_OPT_SEARCH_CHILDREN);

    if (ret >= 0 && ic->nb_streams)
        /* We could not have all the codec parameters before EOF. */
        ret = -1;
    for (i = 0; i < ic->nb_streams; i++) {
        const char *errmsg;
        st = ic->streams[i];

        /* if no packet was ever seen, update context now for has_codec_parameters */
        if (!st->internal->avctx_inited) {
            if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO &&
                st->codecpar->format == AV_SAMPLE_FMT_NONE)
                st->codecpar->format = st->internal->avctx->sample_fmt;
            ret = avcodec_parameters_to_context(st->internal->avctx, st->codecpar);
            if (ret < 0)
                goto find_stream_info_err;
        }
        if (!has_codec_parameters(st, &errmsg)) {
            char buf[256];
            //avcodec_string(buf, sizeof(buf), st->internal->avctx, 0);
            av_log(ic, AV_LOG_WARNING,
                   "Could not find codec parameters for stream %d (%s): %s\n"
                   "Consider increasing the value for the 'analyzeduration' and 'probesize' options\n",
                   i, buf, errmsg);
        } else {
            ret = 0;
        }
    }

    compute_chapters_end(ic);

    /* update the stream parameters from the internal codec contexts */
    for (i = 0; i < ic->nb_streams; i++) {
        st = ic->streams[i];

        if (st->internal->avctx_inited) {
            int orig_w = st->codecpar->width;
            int orig_h = st->codecpar->height;
            ret = avcodec_parameters_from_context(st->codecpar, st->internal->avctx);
            if (ret < 0)
                goto find_stream_info_err;
#if FF_API_LOWRES
            // The decoder might reduce the video size by the lowres factor.
            if (st->internal->avctx->lowres && orig_w) {
                st->codecpar->width = orig_w;
                st->codecpar->height = orig_h;
            }
#endif
        }

#if FF_API_LAVF_AVCTX
FF_DISABLE_DEPRECATION_WARNINGS
        ret = avcodec_parameters_to_context(st->codec, st->codecpar);
        if (ret < 0)
            goto find_stream_info_err;

#if FF_API_LOWRES
        // The old API (AVStream.codec) "requires" the resolution to be adjusted
        // by the lowres factor.
        if (st->internal->avctx->lowres && st->internal->avctx->width) {
            st->codec->lowres = st->internal->avctx->lowres;
            st->codec->width = st->internal->avctx->width;
            st->codec->height = st->internal->avctx->height;
        }
#endif

        if (st->codec->codec_tag != MKTAG('t','m','c','d')) {
            st->codec->time_base = st->internal->avctx->time_base;
            st->codec->ticks_per_frame = st->internal->avctx->ticks_per_frame;
        }
        st->codec->framerate = st->avg_frame_rate;

        if (st->internal->avctx->subtitle_header) {
            st->codec->subtitle_header = av_malloc(st->internal->avctx->subtitle_header_size);
            if (!st->codec->subtitle_header)
                goto find_stream_info_err;
            st->codec->subtitle_header_size = st->internal->avctx->subtitle_header_size;
            memcpy(st->codec->subtitle_header, st->internal->avctx->subtitle_header,
                   st->codec->subtitle_header_size);
        }

        // Fields unavailable in AVCodecParameters
        st->codec->coded_width = st->internal->avctx->coded_width;
        st->codec->coded_height = st->internal->avctx->coded_height;
        st->codec->properties = st->internal->avctx->properties;
FF_ENABLE_DEPRECATION_WARNINGS
#endif

        st->internal->avctx_inited = 0;
    }

find_stream_info_err:
    for (i = 0; i < ic->nb_streams; i++) {
        st = ic->streams[i];
        if (st->info)
            av_freep(&st->info->duration_error);
        avcodec_close(ic->streams[i]->internal->avctx);
        av_freep(&ic->streams[i]->info);
        av_bsf_free(&ic->streams[i]->internal->extract_extradata.bsf);
        av_packet_free(&ic->streams[i]->internal->extract_extradata.pkt);
    }
    if (ic->pb)
        av_log(ic, AV_LOG_DEBUG, "After avformat_find_stream_info() pos: %"PRId64" bytes read:%"PRId64" seeks:%d frames:%d\n",
               avio_tell(ic->pb), ic->pb->bytes_read, ic->pb->seek_count, count);
    return ret;
}

int update_stream_avctx(AVFormatContext *s)
{
    int i, ret;
    for (i = 0; i < s->nb_streams; i++) {
        AVStream *st = s->streams[i];

        if (!st->internal->need_context_update)
            continue;

        /* close parser, because it depends on the codec */
        /*if (st->parser && st->internal->avctx->codec_id != st->codecpar->codec_id) {
            av_parser_close(st->parser);
            st->parser = NULL;
        }*/

        /* update internal codec context, for the parser */
        ret = avcodec_parameters_to_context(st->internal->avctx, st->codecpar);
        if (ret < 0)
            return ret;

        st->internal->need_context_update = 0;
    }
    return 0;
}

void packet_list_free(AVPacketList **pkt_buf, AVPacketList **pkt_buf_end)
{
    AVPacketList *tmp = *pkt_buf;

    while (tmp) {
        AVPacketList *pktl = tmp;
        tmp = pktl->next;
        av_packet_unref(&pktl->pkt);
        av_freep(&pktl);
    }
    *pkt_buf     = NULL;
    *pkt_buf_end = NULL;
}

void flush_packet_queue(AVFormatContext *s)
{
    if (!s->internal)
        return;
    packet_list_free(&s->internal->parse_queue,       &s->internal->parse_queue_end);
    packet_list_free(&s->internal->packet_buffer,     &s->internal->packet_buffer_end);
    packet_list_free(&s->internal->raw_packet_buffer, &s->internal->raw_packet_buffer_end);

    s->internal->raw_packet_buffer_remaining_size = RAW_PACKET_BUFFER_SIZE;
}

