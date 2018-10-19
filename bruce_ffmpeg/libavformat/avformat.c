/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-18
 * Description:
 
*********************************/

static void writeout(AVIOContext *s, const uint8_t *data, int len)
{
    if (!s->error) {
        int ret = 0;
        if (s->write_data_type)
            ret = s->write_data_type(s->opaque, (uint8_t *)data,
                                     len,
                                     s->current_type,
                                     s->last_time);
        else if (s->write_packet)
            ret = s->write_packet(s->opaque, (uint8_t *)data, len);
        if (ret < 0) {
            s->error = ret;
        } else {
            if (s->pos + len > s->written)
                s->written = s->pos + len;
        }
    }
    if (s->current_type == AVIO_DATA_MARKER_SYNC_POINT ||
        s->current_type == AVIO_DATA_MARKER_BOUNDARY_POINT) {
        s->current_type = AVIO_DATA_MARKER_UNKNOWN;
    }
    s->last_time = AV_NOPTS_VALUE;
    s->writeout_count ++;
    s->pos += len;
}

static void flush_buffer(AVIOContext *s)
{
    s->buf_ptr_max = FFMAX(s->buf_ptr, s->buf_ptr_max);
    if (s->write_flag && s->buf_ptr_max > s->buffer) {
        writeout(s, s->buffer, s->buf_ptr_max - s->buffer);
        if (s->update_checksum) {
            s->checksum     = s->update_checksum(s->checksum, s->checksum_ptr,
                                                 s->buf_ptr_max - s->checksum_ptr);
            s->checksum_ptr = s->buffer;
        }
    }
    s->buf_ptr = s->buf_ptr_max = s->buffer;
    if (!s->write_flag)
        s->buf_end = s->buffer;
}

static int io_open_default(AVFormatContext *s, AVIOContext **pb,
                           const char *url, int flags, AVDictionary **options)
{
    int loglevel;

    if (!strcmp(url, s->url))
    {
        loglevel = AV_LOG_DEBUG;
    } else
        loglevel = AV_LOG_INFO;

    av_log(s, loglevel, "Opening \'%s\' for %s\n", url, flags & AVIO_FLAG_WRITE ? "writing" : "reading");

    return avio_open_whitelist(pb, url, flags, &s->interrupt_callback, options);
}

static void io_close_default(AVFormatContext *s, AVIOContext *pb)
{
    avio_close(pb);
}

static const char* format_to_name(void* ptr)
{
    AVFormatContext* fc = (AVFormatContext*) ptr;
    
    if(fc->iformat) {
        return fc->iformat->name;
    } else if(fc->oformat) {
        return fc->oformat->name;
    }
    
    else return "NULL";
}

#define OFFSET(x) offsetof(AVFormatContext,x)
#define DEFAULT 0 //should be NAN but it does not work as it is not a constant in glibc as required by ANSI/ISO C
#define E AV_OPT_FLAG_ENCODING_PARAM
#define D AV_OPT_FLAG_DECODING_PARAM

static const AVOption avformat_options[] = {
{"avioflags", OFFSET(avio_flags), AV_OPT_TYPE_FLAGS, {.i64 = DEFAULT }, INT_MIN, INT_MAX, D|E, "avioflags"},
{"direct", 0, AV_OPT_TYPE_CONST, {.i64 = AVIO_FLAG_DIRECT }, INT_MIN, INT_MAX, D|E, "avioflags"},
{"probesize", OFFSET(probesize), AV_OPT_TYPE_INT64, {.i64 = 5000000 }, 32, INT64_MAX, D},
{"formatprobesize", OFFSET(format_probesize), AV_OPT_TYPE_INT, {.i64 = PROBE_BUF_MAX}, 0, INT_MAX-1, D},
{"packetsize", OFFSET(packet_size), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, 0, INT_MAX, E},
{"fflags", OFFSET(flags), AV_OPT_TYPE_FLAGS, {.i64 = AVFMT_FLAG_AUTO_BSF }, INT_MIN, INT_MAX, D|E, "fflags"},
{"flush_packets", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_FLUSH_PACKETS }, INT_MIN, INT_MAX, E, "fflags"},
{"ignidx", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_IGNIDX }, INT_MIN, INT_MAX, D, "fflags"},
{"genpts", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_GENPTS }, INT_MIN, INT_MAX, D, "fflags"},
{"nofillin", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_NOFILLIN }, INT_MIN, INT_MAX, D, "fflags"},
{"noparse", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_NOPARSE }, INT_MIN, INT_MAX, D, "fflags"},
{"igndts", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_IGNDTS }, INT_MIN, INT_MAX, D, "fflags"},
{"discardcorrupt", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_DISCARD_CORRUPT }, INT_MIN, INT_MAX, D, "fflags"},
{"sortdts", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_SORT_DTS }, INT_MIN, INT_MAX, D, "fflags"},
#if FF_API_LAVF_KEEPSIDE_FLAG
{"keepside", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_KEEP_SIDE_DATA }, INT_MIN, INT_MAX, D, "fflags"},
#endif
{"fastseek", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_FAST_SEEK }, INT_MIN, INT_MAX, D, "fflags"},
#if 0 //FF_API_LAVF_MP4A_LATM
{"latm", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_MP4A_LATM }, INT_MIN, INT_MAX, E, "fflags"},
#endif
{"nobuffer", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_NOBUFFER }, 0, INT_MAX, D, "fflags"},
{"bitexact", 0, AV_OPT_TYPE_CONST, { .i64 = AVFMT_FLAG_BITEXACT }, 0, 0, E, "fflags" },
{"shortest", 0, AV_OPT_TYPE_CONST, { .i64 = AVFMT_FLAG_SHORTEST }, 0, 0, E, "fflags" },
{"autobsf", 0, AV_OPT_TYPE_CONST, { .i64 = AVFMT_FLAG_AUTO_BSF }, 0, 0, E, "fflags" },
{"seek2any", OFFSET(seek2any), AV_OPT_TYPE_BOOL, {.i64 = 0 }, 0, 1, D},
{"analyzeduration", OFFSET(max_analyze_duration), AV_OPT_TYPE_INT64, {.i64 = 0 }, 0, INT64_MAX, D},
{"cryptokey", OFFSET(key), AV_OPT_TYPE_BINARY, {.dbl = 0}, 0, 0, D},
{"indexmem", OFFSET(max_index_size), AV_OPT_TYPE_INT, {.i64 = 1<<20 }, 0, INT_MAX, D},
{"rtbufsize", OFFSET(max_picture_buffer), AV_OPT_TYPE_INT, {.i64 = 3041280 }, 0, INT_MAX, D}, /* defaults to 1s of 15fps 352x288 YUYV422 video */
{"fdebug", OFFSET(debug), AV_OPT_TYPE_FLAGS, {.i64 = DEFAULT }, 0, INT_MAX, E|D, "fdebug"},
{"ts", 0, AV_OPT_TYPE_CONST, {.i64 = FF_FDEBUG_TS }, INT_MIN, INT_MAX, E|D, "fdebug"},
{"max_delay", OFFSET(max_delay), AV_OPT_TYPE_INT, {.i64 = -1 }, -1, INT_MAX, E|D},
{"start_time_realtime", OFFSET(start_time_realtime), AV_OPT_TYPE_INT64, {.i64 = AV_NOPTS_VALUE}, INT64_MIN, INT64_MAX, E},
{"fpsprobesize", OFFSET(fps_probe_size), AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX-1, D},
{"audio_preload", OFFSET(audio_preload), AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX-1, E},
{"chunk_duration", OFFSET(max_chunk_duration), AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX-1, E},
{"chunk_size", OFFSET(max_chunk_size), AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX-1, E},
/* this is a crutch for avconv, since it cannot deal with identically named options in different contexts. to be removed when avconv is fixed */
{"f_err_detect", OFFSET(error_recognition), AV_OPT_TYPE_FLAGS, {.i64 = AV_EF_CRCCHECK }, INT_MIN, INT_MAX, D, "err_detect"},
{"err_detect", OFFSET(error_recognition), AV_OPT_TYPE_FLAGS, {.i64 = AV_EF_CRCCHECK }, INT_MIN, INT_MAX, D, "err_detect"},
{"crccheck", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_CRCCHECK }, INT_MIN, INT_MAX, D, "err_detect"},
{"bitstream", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_BITSTREAM }, INT_MIN, INT_MAX, D, "err_detect"},
{"buffer", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_BUFFER }, INT_MIN, INT_MAX, D, "err_detect"},
{"explode", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_EXPLODE }, INT_MIN, INT_MAX, D, "err_detect"},
{"ignore_err", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_IGNORE_ERR }, INT_MIN, INT_MAX, D, "err_detect"},
{"careful", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_CAREFUL }, INT_MIN, INT_MAX, D, "err_detect"},
{"compliant",  0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_COMPLIANT }, INT_MIN, INT_MAX, D, "err_detect"},
{"aggressive", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_AGGRESSIVE }, INT_MIN, INT_MAX, D, "err_detect"},
{"use_wallclock_as_timestamps", OFFSET(use_wallclock_as_timestamps), AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, D},
{"skip_initial_bytes", OFFSET(skip_initial_bytes), AV_OPT_TYPE_INT64, {.i64 = 0}, 0, INT64_MAX-1, D},
{"correct_ts_overflow", OFFSET(correct_ts_overflow), AV_OPT_TYPE_BOOL, {.i64 = 1}, 0, 1, D},
{"flush_packets", OFFSET(flush_packets), AV_OPT_TYPE_INT, {.i64 = -1}, -1, 1, E},
{"metadata_header_padding", OFFSET(metadata_header_padding), AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX, E},
{"output_ts_offset", OFFSET(output_ts_offset), AV_OPT_TYPE_DURATION, {.i64 = 0}, -INT64_MAX, INT64_MAX, E},
{"max_interleave_delta", OFFSET(max_interleave_delta), AV_OPT_TYPE_INT64, { .i64 = 10000000 }, 0, INT64_MAX, E },
{"f_strict", OFFSET(strict_std_compliance), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, D|E, "strict"},
{"strict", OFFSET(strict_std_compliance), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, D|E, "strict"},
{"very", 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_VERY_STRICT }, INT_MIN, INT_MAX, D|E, "strict"},
{"strict", 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_STRICT }, INT_MIN, INT_MAX, D|E, "strict"},
{"normal", 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_NORMAL }, INT_MIN, INT_MAX, D|E, "strict"},
{"unofficial", 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_UNOFFICIAL }, INT_MIN, INT_MAX, D|E, "strict"},
{"experimental", 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_EXPERIMENTAL }, INT_MIN, INT_MAX, D|E, "strict"},
{"max_ts_probe", OFFSET(max_ts_probe), AV_OPT_TYPE_INT, { .i64 = 50 }, 0, INT_MAX, D },
{"avoid_negative_ts", OFFSET(avoid_negative_ts), AV_OPT_TYPE_INT, {.i64 = -1}, -1, 2, E, "avoid_negative_ts"},
{"auto", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_AVOID_NEG_TS_AUTO },              INT_MIN, INT_MAX, E, "avoid_negative_ts"},
{"disabled", 0, AV_OPT_TYPE_CONST, {.i64 = 0 },                                    INT_MIN, INT_MAX, E, "avoid_negative_ts"},
{"make_non_negative", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_AVOID_NEG_TS_MAKE_NON_NEGATIVE }, INT_MIN, INT_MAX, E, "avoid_negative_ts"},
{"make_zero", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_AVOID_NEG_TS_MAKE_ZERO },         INT_MIN, INT_MAX, E, "avoid_negative_ts"},
{"dump_separator", OFFSET(dump_separator), AV_OPT_TYPE_STRING, {.str = ", "}, CHAR_MIN, CHAR_MAX, D|E},
{"codec_whitelist", OFFSET(codec_whitelist), AV_OPT_TYPE_STRING, { .str = NULL },  CHAR_MIN, CHAR_MAX, D },
{"format_whitelist", OFFSET(format_whitelist), AV_OPT_TYPE_STRING, { .str = NULL },  CHAR_MIN, CHAR_MAX, D },
{"protocol_whitelist", OFFSET(protocol_whitelist), AV_OPT_TYPE_STRING, { .str = NULL },  CHAR_MIN, CHAR_MAX, D },
{"protocol_blacklist", OFFSET(protocol_blacklist), AV_OPT_TYPE_STRING, { .str = NULL },  CHAR_MIN, CHAR_MAX, D },
{"max_streams", OFFSET(max_streams), AV_OPT_TYPE_INT, { .i64 = 1000 }, 0, INT_MAX, D },
{"skip_estimate_duration_from_pts", OFFSET(skip_estimate_duration_from_pts), AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, D},
{NULL},
};

static void *format_child_next(void *obj, void *prev)
{
    AVFormatContext *s = obj;
    if (!prev && s->priv_data &&
        ((s->iformat && s->iformat->priv_class) || s->oformat && s->oformat->priv_class))
        return s->priv_data;
    
    if (s->pb && s->pb->av_class && prev != s->pb)
        return s->pb;
    
    return NULL;
}

#include "muxer_list.c"
#include "demuxer_list.c"

static const AVInputFormat * const *indev_list = NULL;
static const AVOutputFormat * const *outdev_list = NULL;

const AVInputFormat *av_demuxer_iterate(uint32_t *opaque)
{
    static const uint32_t size = sizeof(demuxer_list)/sizeof(demuxer_list[0]) - 1;
    uint32_t i = *opaque;
    const AVInputFormat *f = NULL;

    if (i < size) {
        f = demuxer_list[i];
    }

    if (f) {
        *opaque = i + 1;
    }
    
    return f;
}

#if 0 //后续需要时再放开
const AVOutputFormat *av_muxer_iterate(void **opaque)
{
    static const uintptr_t size = sizeof(muxer_list)/sizeof(muxer_list[0]) - 1;
    uintptr_t i = (uintptr_t)*opaque;
    const AVOutputFormat *f = NULL;

    if (i < size) {
        f = muxer_list[i];
    } else if (indev_list) {
        f = outdev_list[i - size];
    }

    if (f)
        *opaque = (void*)(i + 1);
    
    return f;
}
#endif

static AVMutex avpriv_register_devices_mutex = AV_MUTEX_INITIALIZER;
static AVOnce av_format_next_init = AV_ONCE_INIT;

static void av_format_init_next(void)
{
    AVOutputFormat *prevout = NULL, *out;
    AVInputFormat *previn = NULL, *in;

    ff_mutex_lock(&avpriv_register_devices_mutex);

    for (int i = 0; (out = (AVOutputFormat*)muxer_list[i]); i++) {
        if (prevout)
            prevout->next = out;
        prevout = out;
    }

    if (outdev_list) {
        for (int i = 0; (out = (AVOutputFormat*)outdev_list[i]); i++) {
            if (prevout)
                prevout->next = out;
            prevout = out;
        }
    }

    for (int i = 0; (in = (AVInputFormat*)demuxer_list[i]); i++) {
        if (previn)
            previn->next = in;
        previn = in;
    }

    if (indev_list) {
        for (int i = 0; (in = (AVInputFormat*)indev_list[i]); i++) {
            if (previn)
                previn->next = in;
            previn = in;
        }
    }

    ff_mutex_unlock(&avpriv_register_devices_mutex);
}

AVInputFormat *av_iformat_next(const AVInputFormat *f)
{
    ff_thread_once(&av_format_next_init, av_format_init_next);

    if (f) {
        return f->next;
    } else {
        uint32_t opaque = 0;
        return (AVInputFormat *)av_demuxer_iterate(&opaque);
    }
}

AVOutputFormat *av_oformat_next(const AVOutputFormat *f)
{
    ff_thread_once(&av_format_next_init, av_format_init_next);

    if (f)
        return f->next;
    else {
        void *opaque = NULL;
        return (AVOutputFormat *)av_muxer_iterate(&opaque);
    }
}

static const AVClass *format_child_class_next(const AVClass *prev)
{
    AVInputFormat  *ifmt = NULL;
    AVOutputFormat *ofmt = NULL;

    if (!prev)
        return &avio_class;

    while ((ifmt = av_iformat_next(ifmt)))
        if (ifmt->priv_class == prev)
            break;

    if (!ifmt)
        while ((ofmt = av_oformat_next(ofmt)))
            if (ofmt->priv_class == prev)
                break;
    if (!ofmt)
        while (ifmt = av_iformat_next(ifmt))
            if (ifmt->priv_class)
                return ifmt->priv_class;

    while (ofmt = av_oformat_next(ofmt))
        if (ofmt->priv_class)
            return ofmt->priv_class;

    return NULL;
}

static AVClassCategory get_category(void *ptr)
{
    AVFormatContext* s = ptr;
    
    if(s->iformat) 
        return AV_CLASS_CATEGORY_DEMUXER;
    else           
        return AV_CLASS_CATEGORY_MUXER;
}

static const AVClass av_format_context_class = {
    .class_name     = "AVFormatContext",
    .item_name      = format_to_name,
    .option         = avformat_options,
    .version        = LIBAVUTIL_VERSION_INT,
    .child_next     = format_child_next,
    .child_class_next = format_child_class_next,
    .category       = AV_CLASS_CATEGORY_MUXER,
    .get_category   = get_category,
};

static void avformat_get_context_defaults(AVFormatContext *s)
{
    memset(s, 0, sizeof(AVFormatContext));

    s->av_class = &av_format_context_class;
    s->io_open  = io_open_default;
    s->io_close = io_close_default;

    av_opt_set_defaults(s);
}

void avformat_free_context(AVFormatContext *s)
{
    int i;

    if (!s)
        return;

    av_opt_free(s);
    if (s->iformat && s->iformat->priv_class && s->priv_data)
        av_opt_free(s->priv_data);
    if (s->oformat && s->oformat->priv_class && s->priv_data)
        av_opt_free(s->priv_data);

    for (i = s->nb_streams - 1; i >= 0; i--)
        av_free_stream(s, s->streams[i]);


    for (i = s->nb_programs - 1; i >= 0; i--) {
        av_dict_free(&s->programs[i]->metadata);
        av_freep(&s->programs[i]->stream_index);
        av_freep(&s->programs[i]);
    }
    av_freep(&s->programs);
    av_freep(&s->priv_data);
    while (s->nb_chapters--) {
        av_dict_free(&s->chapters[s->nb_chapters]->metadata);
        av_freep(&s->chapters[s->nb_chapters]);
    }
    av_freep(&s->chapters);
    av_dict_free(&s->metadata);
    av_dict_free(&s->internal->id3v2_meta);
    av_freep(&s->streams);
    flush_packet_queue(s);
    av_freep(&s->internal);
    av_freep(&s->url);
    av_free(s);
}

int av_probe_input_buffer2(AVIOContext *pb, AVInputFormat **fmt, const char *filename, void *logctx, unsigned int offset, unsigned int max_probe_size)
{
    AVProbeData pd = { filename ? filename : "" };
    uint8_t *buf = NULL;
    int ret = 0, probe_size, buf_offset = 0;
    int score = 0;
    int ret2;

    if (!max_probe_size)
        max_probe_size = PROBE_BUF_MAX;
    else if (max_probe_size < PROBE_BUF_MIN) {
        av_log(logctx, AV_LOG_ERROR, "Specified probe size value %u cannot be < %u\n", max_probe_size, PROBE_BUF_MIN);
        return AVERROR(EINVAL);
    }

    if (offset >= max_probe_size)
        return AVERROR(EINVAL);

    if (pb->av_class) {
        uint8_t *mime_type_opt = NULL;
        char *semi;
        av_opt_get(pb, "mime_type", AV_OPT_SEARCH_CHILDREN, &mime_type_opt);
        pd.mime_type = (const char *)mime_type_opt;
        semi = pd.mime_type ? strchr(pd.mime_type, ';') : NULL;
        if (semi) {
            *semi = '\0';
        }
    }

    for (probe_size = PROBE_BUF_MIN; probe_size <= max_probe_size && !*fmt;
         probe_size = FFMIN(probe_size << 1, FFMAX(max_probe_size, probe_size + 1))) {
        score = probe_size < max_probe_size ? AVPROBE_SCORE_RETRY : 0;

        /* Read probe data. */
        if ((ret = av_reallocp(&buf, probe_size + AVPROBE_PADDING_SIZE)) < 0)
            goto fail;
        
        if ((ret = avio_read(pb, buf + buf_offset, probe_size - buf_offset)) < 0) {
            /* Fail if error was not end of file, otherwise, lower score. */
            if (ret != AVERROR_EOF)
                goto fail;

            score = 0;
            ret   = 0;          /* error was end of file, nothing read */
        }
        
        buf_offset += ret;
        if (buf_offset < offset)
            continue;
        
        pd.buf_size = buf_offset - offset;
        pd.buf = &buf[offset];

        memset(pd.buf + pd.buf_size, 0, AVPROBE_PADDING_SIZE);

        /* Guess file format. */
        *fmt = av_probe_input_format2(&pd, 1, &score);
        if (*fmt) {
            /* This can only be true in the last iteration. */
            if (score <= AVPROBE_SCORE_RETRY) {
                av_log(logctx, AV_LOG_WARNING,
                       "Format %s detected only with low score of %d, "
                       "misdetection possible!\n", (*fmt)->name, score);
            } else
                av_log(logctx, AV_LOG_DEBUG,
                       "Format %s probed with size=%d and score=%d\n",
                       (*fmt)->name, probe_size, score);
#if 0
            FILE *f = fopen("probestat.tmp", "ab");
            fprintf(f, "probe_size:%d format:%s score:%d filename:%s\n", probe_size, (*fmt)->name, score, filename);
            fclose(f);
#endif
        }
    }

    if (!*fmt)
        ret = AVERROR_INVALIDDATA;

fail:
    /* Rewind. Reuse probe buffer to avoid seeking. */
    ret2 = avio_rewind_with_probe_data(pb, &buf, buf_offset);
    if (ret >= 0)
        ret = ret2;

    av_freep(&pd.mime_type);
    return ret < 0 ? ret : score;
}

int av_match_ext(const char *filename, const char *extensions)
{
    const char *ext;

    if (!filename)
        return 0;

    ext = strrchr(filename, '.');
    if (ext)
        return av_match_name(ext + 1, extensions);
    
    return 0;
}

AVInputFormat *av_probe_input_format3(AVProbeData *pd, int is_opened, int *score_ret)
{
    AVProbeData lpd = *pd;
    const AVInputFormat *fmt1 = NULL;
    AVInputFormat *fmt = NULL;
    int score, score_max = 0;
    uint32_t i = 0;
    const static uint8_t zerobuffer[AVPROBE_PADDING_SIZE];
    enum nodat {
        NO_ID3,
        ID3_ALMOST_GREATER_PROBE,
        ID3_GREATER_PROBE,
        ID3_GREATER_MAX_PROBE,
    } nodat = NO_ID3;

    if (!lpd.buf)
        lpd.buf = (unsigned char *) zerobuffer;

    if (lpd.buf_size > 10 && ff_id3v2_match(lpd.buf, ID3v2_DEFAULT_MAGIC)) {
        int id3len = ff_id3v2_tag_len(lpd.buf);
        if (lpd.buf_size > id3len + 16) {
            if (lpd.buf_size < 2LL*id3len + 16)
                nodat = ID3_ALMOST_GREATER_PROBE;
            lpd.buf      += id3len;
            lpd.buf_size -= id3len;
        } else if (id3len >= PROBE_BUF_MAX) {
            nodat = ID3_GREATER_MAX_PROBE;
        } else
            nodat = ID3_GREATER_PROBE;
    }

    while ((fmt1 = av_demuxer_iterate(&i))) {
        if (is_opened == (fmt1->flags & AVFMT_NOFILE) && strcmp(fmt1->name, "image2"))
            continue;
        
        score = 0;
        if (fmt1->read_probe) {
            score = fmt1->read_probe(&lpd);
            if (score)
                av_log(NULL, AV_LOG_TRACE, "Probing %s score:%d size:%d\n", fmt1->name, score, lpd.buf_size);
            
            if (fmt1->extensions && av_match_ext(lpd.filename, fmt1->extensions)) {
                switch (nodat) {
                case NO_ID3:
                    score = FFMAX(score, 1);
                    break;
                case ID3_GREATER_PROBE:
                case ID3_ALMOST_GREATER_PROBE:
                    score = FFMAX(score, AVPROBE_SCORE_EXTENSION / 2 - 1);
                    break;
                case ID3_GREATER_MAX_PROBE:
                    score = FFMAX(score, AVPROBE_SCORE_EXTENSION);
                    break;
                }
            }
        } else if (fmt1->extensions) {
            if (av_match_ext(lpd.filename, fmt1->extensions))
                score = AVPROBE_SCORE_EXTENSION;
        }
        
        if (av_match_name(lpd.mime_type, fmt1->mime_type)) {
            if (AVPROBE_SCORE_MIME > score) {
                av_log(NULL, AV_LOG_DEBUG, "Probing %s score:%d increased to %d due to MIME type\n", fmt1->name, score, AVPROBE_SCORE_MIME);
                score = AVPROBE_SCORE_MIME;
            }
        }
        
        if (score > score_max) {
            score_max = score;
            fmt       = (AVInputFormat*)fmt1;
        } else if (score == score_max) {
            fmt = NULL;
        }
    }
    
    if (nodat == ID3_GREATER_PROBE) {
        score_max = FFMIN(AVPROBE_SCORE_EXTENSION / 2 - 1, score_max);
    }
    
    *score_ret = score_max;

    return fmt;
}

AVInputFormat *av_probe_input_format2(AVProbeData *pd, int is_opened, int *score_max)
{
    int score_ret;
    AVInputFormat *fmt = av_probe_input_format3(pd, is_opened, &score_ret);
    if (score_ret > *score_max) {
        *score_max = score_ret;
        return fmt;
    } else
        return NULL;
}

static int init_input(AVFormatContext *pstFmtCtx, const char *filename, AVDictionary **options)
{
    int ret;
    AVProbeData pd = { filename, NULL, 0 };
    int score = AVPROBE_SCORE_RETRY;

    if (pstFmtCtx->pb) {
        pstFmtCtx->flags |= AVFMT_FLAG_CUSTOM_IO;
        if (!pstFmtCtx->iformat) {
            return av_probe_input_buffer2(pstFmtCtx->pb, &pstFmtCtx->iformat, filename, pstFmtCtx, 0, pstFmtCtx->format_probesize);
        }
        else if (pstFmtCtx->iformat->flags & AVFMT_NOFILE) {
            av_log(pstFmtCtx, AV_LOG_WARNING, "Custom AVIOContext makes no sense and will be ignored with AVFMT_NOFILE format.\n");
        }
        
        return 0;
    }

    if ((pstFmtCtx->iformat && pstFmtCtx->iformat->flags & AVFMT_NOFILE) ||
        (!pstFmtCtx->iformat && (pstFmtCtx->iformat = av_probe_input_format2(&pd, 0, &score))))
        return score;

    if ((ret = pstFmtCtx->io_open(pstFmtCtx, &pstFmtCtx->pb, filename, AVIO_FLAG_READ | pstFmtCtx->avio_flags, options)) < 0)
        return ret;

    if (pstFmtCtx->iformat)
        return 0;
    
    return av_probe_input_buffer2(pstFmtCtx->pb, &pstFmtCtx->iformat, filename, pstFmtCtx, 0, pstFmtCtx->format_probesize);
}

int avformat_queue_attached_pictures(AVFormatContext *s)
{
    int i, ret;
    for (i = 0; i < s->nb_streams; i++)
        if (s->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC && s->streams[i]->discard < AVDISCARD_ALL) {
            if (s->streams[i]->attached_pic.size <= 0) {
                av_log(s, AV_LOG_WARNING, "Attached picture on stream %d has invalid size, ignoring\n", i);
                continue;
            }

            ret = packet_list_put(&s->internal->raw_packet_buffer,
                                     &s->internal->raw_packet_buffer_end,
                                     &s->streams[i]->attached_pic,
                                     FF_PACKETLIST_FLAG_REF_PACKET);
            if (ret < 0)
                return ret;
        }
    return 0;
}

int avformat_open_input(AVFormatContext **ppstFmtCtx, const char *filename, AVInputFormat *fmt, AVDictionary **options)
{
    AVFormatContext *pstFmtCtx = *ppstFmtCtx;
    int i, ret = 0;
    AVDictionary *tmp = NULL;
    ID3v2ExtraMeta *id3v2_extra_meta = NULL;

    if (!pstFmtCtx && !(pstFmtCtx = avformat_alloc_context()))
        return AVERROR(ENOMEM);
    
    if (!pstFmtCtx->av_class) {
        av_log(NULL, AV_LOG_ERROR, "Input context has not been properly allocated by avformat_alloc_context() and is not NULL either\n");
        return AVERROR(EINVAL);
    }
    
    if (fmt)
        pstFmtCtx->iformat = fmt;

    if (options)
        av_dict_copy(&tmp, *options, 0);

    if (pstFmtCtx->pb) {
        pstFmtCtx->flags |= AVFMT_FLAG_CUSTOM_IO;
    }

    if ((ret = av_opt_set_dict(pstFmtCtx, &tmp)) < 0)
        goto fail;

    if (!(pstFmtCtx->url = av_strdup(filename ? filename : ""))) {
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    if ((ret = init_input(pstFmtCtx, filename, &tmp)) < 0)
        goto fail;
    
    pstFmtCtx->probe_score = ret;

    avio_skip(pstFmtCtx->pb, pstFmtCtx->skip_initial_bytes);

    /* Check filename in case an image number is expected. */
    /*if (s->iformat->flags & AVFMT_NEEDNUMBER) {
        if (!av_filename_number_test(filename)) {
            ret = AVERROR(EINVAL);
            goto fail;
        }
    }*/

    pstFmtCtx->duration = pstFmtCtx->start_time = AV_NOPTS_VALUE;

    /* Allocate private data. */
    if (pstFmtCtx->iformat->priv_data_size > 0) {
        if (!(pstFmtCtx->priv_data = av_mallocz(pstFmtCtx->iformat->priv_data_size))) {
            ret = AVERROR(ENOMEM);
            goto fail;
        }
        
        if (pstFmtCtx->iformat->priv_class) {
            *(const AVClass **) pstFmtCtx->priv_data = pstFmtCtx->iformat->priv_class;
            av_opt_set_defaults(pstFmtCtx->priv_data);
        
            if ((ret = av_opt_set_dict(pstFmtCtx->priv_data, &tmp)) < 0)
                goto fail;
        }
    }


    /* e.g. AVFMT_NOFILE formats will not have a AVIOContext */
    /*if (s->pb) {
        id3v2_read_dict(s->pb, &s->internal->id3v2_meta, ID3v2_DEFAULT_MAGIC, &id3v2_extra_meta);
    }*/

    if (!(pstFmtCtx->flags & AVFMT_FLAG_PRIV_OPT) && pstFmtCtx->iformat->read_header)
        if ((ret = pstFmtCtx->iformat->read_header(pstFmtCtx)) < 0)
            goto fail;

#if 0
    if (!s->metadata) {
        s->metadata = s->internal->id3v2_meta;
        s->internal->id3v2_meta = NULL;
    } else if (s->internal->id3v2_meta) {
        int level = AV_LOG_WARNING;
        if (s->error_recognition & AV_EF_COMPLIANT)
            level = AV_LOG_ERROR;
        av_log(s, level, "Discarding ID3 tags because more suitable tags were found.\n");
        av_dict_free(&s->internal->id3v2_meta);
        if (s->error_recognition & AV_EF_EXPLODE)
            return AVERROR_INVALIDDATA;
    }

    if (id3v2_extra_meta) {
        if (!strcmp(s->iformat->name, "mp3") || !strcmp(s->iformat->name, "aac") ||
            !strcmp(s->iformat->name, "tta")) {
            if ((ret = id3v2_parse_apic(s, &id3v2_extra_meta)) < 0)
                goto fail;
            if ((ret = id3v2_parse_chapters(s, &id3v2_extra_meta)) < 0)
                goto fail;
            if ((ret = id3v2_parse_priv(s, &id3v2_extra_meta)) < 0)
                goto fail;
        } else
            av_log(s, AV_LOG_DEBUG, "demuxer does not support additional id3 data, skipping\n");
    }
    id3v2_free_extra_meta(&id3v2_extra_meta);
#endif

    if ((ret = avformat_queue_attached_pictures(pstFmtCtx)) < 0)
        goto fail;

    if (!(pstFmtCtx->flags & AVFMT_FLAG_PRIV_OPT) && pstFmtCtx->pb && !pstFmtCtx->internal->data_offset)
        pstFmtCtx->internal->data_offset = avio_tell(pstFmtCtx->pb);

    pstFmtCtx->internal->raw_packet_buffer_remaining_size = RAW_PACKET_BUFFER_SIZE;

    update_stream_avctx(pstFmtCtx);

    for (i = 0; i < pstFmtCtx->nb_streams; i++)
        pstFmtCtx->streams[i]->internal->orig_codec_id = pstFmtCtx->streams[i]->codecpar->codec_id;

    if (options) {
        av_dict_free(options);
        *options = tmp;
    }
    
    *ppstFmtCtx = pstFmtCtx;
    return 0;

fail:
    //ff_id3v2_free_extra_meta(&id3v2_extra_meta);
    av_dict_free(&tmp);
    if (pstFmtCtx->pb && !(pstFmtCtx->flags & AVFMT_FLAG_CUSTOM_IO)) {
        avio_closep(&pstFmtCtx->pb);
    }
    
    avformat_free_context(pstFmtCtx);
    *ppstFmtCtx = NULL;
    return ret;
}

void avformat_close_input(AVFormatContext **ps)
{
    AVFormatContext *s;
    AVIOContext *pb;

    if (!ps || !*ps)
        return;

    s  = *ps;
    pb = s->pb;

    if ((s->iformat && strcmp(s->iformat->name, "image2") && s->iformat->flags & AVFMT_NOFILE) ||
        (s->flags & AVFMT_FLAG_CUSTOM_IO))
        pb = NULL;

    flush_packet_queue(s);

    if (s->iformat)
        if (s->iformat->read_close)
            s->iformat->read_close(s);

    avformat_free_context(s);

    *ps = NULL;

    avio_close(pb);
}

AVFormatContext *avformat_alloc_context(void)
{
    AVFormatContext *ic;
    
    ic = av_malloc(sizeof(AVFormatContext));
    if (!ic) {
        return NULL;
    }
    
    avformat_get_context_defaults(ic);

    ic->internal = av_mallocz(sizeof(*ic->internal));
    if (!ic->internal) {
        avformat_free_context(ic);
        return NULL;
    }
    
    ic->internal->offset = AV_NOPTS_VALUE;
    ic->internal->raw_packet_buffer_remaining_size = RAW_PACKET_BUFFER_SIZE;
    ic->internal->shortest_end = AV_NOPTS_VALUE;

    return ic;
}

