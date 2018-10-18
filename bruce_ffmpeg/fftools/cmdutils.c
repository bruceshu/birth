/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-17
 * Description:
 
*********************************/



#define INDENT        1
#define SHOW_VERSION  2
#define SHOW_CONFIG   4
#define SHOW_COPYRIGHT 8

#define PRINT_LIB_INFO(libname, LIBNAME, flags, level)                  \
    if (CONFIG_##LIBNAME) {                                             \
        const char *indent = flags & INDENT? "  " : "";                 \
        if (flags & SHOW_VERSION) {                                     \
            unsigned int version = libname##_version();                 \
            av_log(NULL, level,                                         \
                   "%slib%-11s %2d.%3d.%3d / %2d.%3d.%3d\n",            \
                   indent, #libname,                                    \
                   LIB##LIBNAME##_VERSION_MAJOR,                        \
                   LIB##LIBNAME##_VERSION_MINOR,                        \
                   LIB##LIBNAME##_VERSION_MICRO,                        \
                   AV_VERSION_MAJOR(version), AV_VERSION_MINOR(version),\
                   AV_VERSION_MICRO(version));                          \
        }                                                               \

#if 0 //后续需要再放开
        if (flags & SHOW_CONFIG) {                                      \
            const char *cfg = libname##_configuration();                \
            if (strcmp(FFMPEG_CONFIGURATION, cfg)) {                    \
                if (!warned_cfg) {                                      \
                    av_log(NULL, level,                                 \
                            "%sWARNING: library configuration mismatch\n", \
                            indent);                                    \
                    warned_cfg = 1;                                     \
                }                                                       \
                av_log(NULL, level, "%s%-11s configuration: %s\n",      \
                        indent, #libname, cfg);                         \
            }                                                           \
        }                                                               
#endif
    }


void print_program_info(int flags, int level)
{
    const char *indent = flags & INDENT? "  " : "";

    av_log(NULL, level, "%s version " FFMPEG_VERSION, program_name);
    if (flags & SHOW_COPYRIGHT) {
        av_log(NULL, level, " Copyright (c) %d-%d the FFmpeg developers", program_birth_year, CONFIG_THIS_YEAR);
    }
    
    av_log(NULL, level, "\n");
    av_log(NULL, level, "%sbuilt with %s\n", indent, CC_IDENT);
    av_log(NULL, level, "%sconfiguration: " FFMPEG_CONFIGURATION "\n", indent);
}


void print_all_libs_info(int flags, int level)
{
    PRINT_LIB_INFO(avutil,     AVUTIL,     flags, level);
    PRINT_LIB_INFO(avcodec,    AVCODEC,    flags, level);
    PRINT_LIB_INFO(avformat,   AVFORMAT,   flags, level);
    PRINT_LIB_INFO(avdevice,   AVDEVICE,   flags, level);
    PRINT_LIB_INFO(avfilter,   AVFILTER,   flags, level);
    PRINT_LIB_INFO(avresample, AVRESAMPLE, flags, level);
    PRINT_LIB_INFO(swscale,    SWSCALE,    flags, level);
    PRINT_LIB_INFO(swresample, SWRESAMPLE, flags, level);
    PRINT_LIB_INFO(postproc,   POSTPROC,   flags, level);
}

double parse_number_or_die(const char *context, const char *numstr, int type, double min, double max)
{
    char *tail;
    const char *error;
    double d = av_strtod(numstr, &tail);
    if (*tail)
        error = "Expected number for %s but found: %s\n";
    else if (d < min || d > max)
        error = "The value for %s was %s which is not within %f - %f\n";
    else if (type == OPT_INT64 && (int64_t)d != d)
        error = "Expected int64 for %s but found %s\n";
    else if (type == OPT_INT && (int)d != d)
        error = "Expected int for %s but found %s\n";
    else
        return d;
    
    av_log(NULL, AV_LOG_FATAL, error, context, numstr, min, max);
    exit_program(1);
    
    return 0;
}

static int write_option(void *optctx, const OptionDef *po, const char *opt, const char *arg)
{
    /* new-style options contain an offset into optctx, old-style address of
     * a global var*/
    void *dst = po->flags & (OPT_OFFSET | OPT_SPEC) ? (uint8_t *)optctx + po->u.off : po->u.dst_ptr;
    int *dstcount;

    if (po->flags & OPT_SPEC) {
        SpecifierOpt **so = dst;
        char *p = strchr(opt, ':');
        char *str;

        dstcount = (int *)(so + 1);
        *so = grow_array(*so, sizeof(**so), dstcount, *dstcount + 1);
        str = av_strdup(p ? p + 1 : "");
        if (!str)
            return AVERROR(ENOMEM);
        (*so)[*dstcount - 1].specifier = str;
        dst = &(*so)[*dstcount - 1].u;
    }

    if (po->flags & OPT_STRING) {
        char *str;
        str = av_strdup(arg);
        av_freep(dst);
        if (!str)
            return AVERROR(ENOMEM);
        *(char **)dst = str;
    } else if (po->flags & OPT_BOOL || po->flags & OPT_INT) {
        *(int *)dst = parse_number_or_die(opt, arg, OPT_INT64, INT_MIN, INT_MAX);
    } else if (po->flags & OPT_INT64) {
        *(int64_t *)dst = parse_number_or_die(opt, arg, OPT_INT64, INT64_MIN, INT64_MAX);
    } else if (po->flags & OPT_TIME) {
        *(int64_t *)dst = parse_time_or_die(opt, arg, 1);
    } else if (po->flags & OPT_FLOAT) {
        *(float *)dst = parse_number_or_die(opt, arg, OPT_FLOAT, -INFINITY, INFINITY);
    } else if (po->flags & OPT_DOUBLE) {
        *(double *)dst = parse_number_or_die(opt, arg, OPT_DOUBLE, -INFINITY, INFINITY);
    } else if (po->u.func_arg) {
        int ret = po->u.func_arg(optctx, opt, arg);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Failed to set value '%s' for option '%s': %s\n", arg, opt, av_err2str(ret));
            return ret;
        }
    }
    if (po->flags & OPT_EXIT)
        exit_program(0);

    return 0;
}

int parse_option(void *optctx, const char *opt, const char *arg, const OptionDef *options)
{
    const OptionDef *po;
    int ret;

    po = find_option(options, opt);
    if (!po->name && opt[0] == 'n' && opt[1] == 'o') {
        /* handle 'no' bool option */
        po = find_option(options, opt + 2);
        if ((po->name && (po->flags & OPT_BOOL)))
            arg = "0";
    } else if (po->flags & OPT_BOOL)
        arg = "1";

    if (!po->name)
        po = find_option(options, "default");
    if (!po->name) {
        av_log(NULL, AV_LOG_ERROR, "Unrecognized option '%s'\n", opt);
        return AVERROR(EINVAL);
    }
    if (po->flags & HAS_ARG && !arg) {
        av_log(NULL, AV_LOG_ERROR, "Missing argument for option '%s'\n", opt);
        return AVERROR(EINVAL);
    }

    ret = write_option(optctx, po, opt, arg);
    if (ret < 0)
        return ret;

    return !!(po->flags & HAS_ARG);
}

void print_error(const char *filename, int err)
{
    char errbuf[128];
    const char *errbuf_ptr = errbuf;

    if (av_strerror(err, errbuf, sizeof(errbuf)) < 0)
        errbuf_ptr = strerror(AVUNERROR(err));
    av_log(NULL, AV_LOG_ERROR, "%s: %s\n", filename, errbuf_ptr);
}

AVDictionary **setup_find_stream_info_opts(AVFormatContext *s, AVDictionary *codec_opts)
{
    int i;
    AVDictionary **opts;

    if (!s->nb_streams)
        return NULL;
    
    opts = av_mallocz_array(s->nb_streams, sizeof(*opts));
    if (!opts) {
        av_log(NULL, AV_LOG_ERROR,
               "Could not alloc memory for stream options.\n");
        return NULL;
    }
    
    for (i = 0; i < s->nb_streams; i++)
        opts[i] = filter_codec_opts(codec_opts, s->streams[i]->codecpar->codec_id, s, s->streams[i], NULL);
    
    return opts;
}

AVDictionary *filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id, AVFormatContext *s, AVStream *st, AVCodec *codec)
{
    AVDictionary    *ret = NULL;
    AVDictionaryEntry *t = NULL;
    int flags = s->oformat ? AV_OPT_FLAG_ENCODING_PARAM : AV_OPT_FLAG_DECODING_PARAM;
    char prefix = 0;
    const AVClass *cc = avcodec_get_class();

    if (!codec)
        codec = s->oformat ? avcodec_find_encoder(codec_id) : avcodec_find_decoder(codec_id);

    switch (st->codecpar->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        prefix  = 'v';
        flags  |= AV_OPT_FLAG_VIDEO_PARAM;
        break;
    case AVMEDIA_TYPE_AUDIO:
        prefix  = 'a';
        flags  |= AV_OPT_FLAG_AUDIO_PARAM;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        prefix  = 's';
        flags  |= AV_OPT_FLAG_SUBTITLE_PARAM;
        break;
    }

    while (t = av_dict_get(opts, "", t, AV_DICT_IGNORE_SUFFIX)) {
        char *p = strchr(t->key, ':');

        /* check stream specification in opt name */
        if (p)
            switch (check_stream_specifier(s, st, p + 1)) {
            case  1: *p = 0; break;
            case  0:         continue;
            default:         exit_program(1);
            }

        if (av_opt_find(&cc, t->key, NULL, flags, AV_OPT_SEARCH_FAKE_OBJ) ||
            !codec ||
            (codec->priv_class &&
             av_opt_find(&codec->priv_class, t->key, NULL, flags,
                         AV_OPT_SEARCH_FAKE_OBJ)))
            av_dict_set(&ret, t->key, t->value, 0);
        else if (t->key[0] == prefix &&
                 av_opt_find(&cc, t->key + 1, NULL, flags,
                             AV_OPT_SEARCH_FAKE_OBJ))
            av_dict_set(&ret, t->key + 1, t->value, 0);

        if (p)
            *p = ':';
    }
    return ret;
}

