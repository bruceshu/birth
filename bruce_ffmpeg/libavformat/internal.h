/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


#ifndef AVFORMAT_INTERNAL_H
#define AVFORMAT_INTERNAL_H

#include <stdint.h>

#include "libavutil/rational.h"
#include "libavutil/buffer.h"

#include "libavcodec/avcodec.h"
#include "libavcodec/internal.h"

#include "version.h"

#define MAX_URL_SIZE 4096

#define PROBE_BUF_MIN 2048
#define PROBE_BUF_MAX (1 << 20)

#define MAX_PROBE_PACKETS 2500

#define SPACE_CHARS " \t\r\n"

#define FF_PACKETLIST_FLAG_REF_PACKET (1 << 0)

#define FFERROR_REDO FFERRTAG('R','E','D','O')


#if 0
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
#endif

struct AVFormatInternal {
    int nb_interleaved_streams;
    struct AVPacketList *packet_buffer;
    struct AVPacketList *packet_buffer_end;
    int64_t data_offset; /**< offset of the first packet */
    
    struct AVPacketList *raw_packet_buffer;
    struct AVPacketList *raw_packet_buffer_end;
    struct AVPacketList *parse_queue;
    struct AVPacketList *parse_queue_end;

#define RAW_PACKET_BUFFER_SIZE 2500000
    int raw_packet_buffer_remaining_size;
    int64_t offset;
    AVRational offset_timebase;
#if FF_API_COMPUTE_PKT_FIELDS2
    int missing_ts_warning;
#endif
    int inject_global_side_data;
    int avoid_negative_ts_use_pts;
    int64_t shortest_end;
    int initialized;
    int streams_initialized;
    AVDictionary *id3v2_meta;
    int prefer_codec_framerate;
};

struct AVStreamInternal {
    int reorder;
    //AVBSFContext **bsfcs;
    int nb_bsfcs;
    int bitstream_checked;
    AVCodecContext *avctx;
    int avctx_inited;
    enum AVCodecID orig_codec_id;
    struct {
        //AVBSFContext *bsf;
        AVPacket     *pkt;
        int inited;
    } extract_extradata;
    int need_context_update;
    //FFFrac *priv_pts;
};

#endif
