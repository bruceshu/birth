/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


#ifndef INTERNAL_H
#define INTERNAL_H

#include "libavcodec/avcodec.h"

#define MAX_URL_SIZE 4096

/** size of probe buffer, for guessing file type from file contents */
#define PROBE_BUF_MIN 2048
#define PROBE_BUF_MAX (1 << 20)

#define MAX_PROBE_PACKETS 2500

#define SPACE_CHARS " \t\r\n"

#define FF_PACKETLIST_FLAG_REF_PACKET (1 << 0)


#ifdef __GNUC__
#define dynarray_add(tab, nb_ptr, elem)\
do {\
    __typeof__(tab) _tab = (tab);\
    __typeof__(elem) _elem = (elem);\
    (void)sizeof(**_tab == _elem); /* check that types are compatible */\
    av_dynarray_add(_tab, nb_ptr, _elem);\
} while(0)
#else
#define dynarray_add(tab, nb_ptr, elem)\
do {\
    av_dynarray_add((tab), nb_ptr, (elem));\
} while(0)
#endif

struct AVFormatInternal {
    /**
     * Number of streams relevant for interleaving.
     * Muxing only.
     */
    int nb_interleaved_streams;

    /**
     * This buffer is only needed when packets were already buffered but
     * not decoded, for example to get the codec parameters in MPEG
     * streams.
     */
    struct AVPacketList *packet_buffer;
    struct AVPacketList *packet_buffer_end;

    /* av_seek_frame() support */
    int64_t data_offset; /**< offset of the first packet */

    /**
     * Raw packets from the demuxer, prior to parsing and decoding.
     * This buffer is used for buffering packets until the codec can
     * be identified, as parsing cannot be done without knowing the
     * codec.
     */
    struct AVPacketList *raw_packet_buffer;
    struct AVPacketList *raw_packet_buffer_end;
    /**
     * Packets split by the parser get queued here.
     */
    struct AVPacketList *parse_queue;
    struct AVPacketList *parse_queue_end;
    /**
     * Remaining size available for raw_packet_buffer, in bytes.
     */
#define RAW_PACKET_BUFFER_SIZE 2500000
    int raw_packet_buffer_remaining_size;

    /**
     * Offset to remap timestamps to be non-negative.
     * Expressed in timebase units.
     * @see AVStream.mux_ts_offset
     */
    int64_t offset;

    /**
     * Timebase for the timestamp offset.
     */
    AVRational offset_timebase;

#if FF_API_COMPUTE_PKT_FIELDS2
    int missing_ts_warning;
#endif

    int inject_global_side_data;

    int avoid_negative_ts_use_pts;

    /**
     * Timestamp of the end of the shortest stream.
     */
    int64_t shortest_end;

    /**
     * Whether or not avformat_init_output has already been called
     */
    int initialized;

    /**
     * Whether or not avformat_init_output fully initialized streams
     */
    int streams_initialized;

    /**
     * ID3v2 tag useful for MP3 demuxing
     */
    AVDictionary *id3v2_meta;

    /*
     * Prefer the codec framerate for avg_frame_rate computation.
     */
    int prefer_codec_framerate;
};

struct AVStreamInternal {
    int reorder;
    AVBSFContext **bsfcs;
    int nb_bsfcs;
    int bitstream_checked;
    AVCodecContext *avctx;
    int avctx_inited;
    enum AVCodecID orig_codec_id;
    
    struct {
        AVBSFContext *bsf;
        AVPacket     *pkt;
        int inited;
    } extract_extradata;

    int need_context_update;

    FFFrac *priv_pts;
};


#endif
