/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-18
 * Description:
 
*********************************/



static void dump_metadata(void *ctx, AVDictionary *m, const char *indent)
{
    if (m && !(av_dict_count(m) == 1 && av_dict_get(m, "language", NULL, 0))) {
        AVDictionaryEntry *tag = NULL;

        av_log(ctx, AV_LOG_INFO, "%sMetadata:\n", indent);
        while ((tag = av_dict_get(m, "", tag, AV_DICT_IGNORE_SUFFIX)))
            if (strcmp("language", tag->key)) {
                const char *p = tag->value;
                av_log(ctx, AV_LOG_INFO, "%s  %-16s: ", indent, tag->key);
            
                while (*p) {
                    char tmp[256];
                    size_t len = strcspn(p, "\x8\xa\xb\xc\xd");
                    av_strlcpy(tmp, p, FFMIN(sizeof(tmp), len+1));
                    av_log(ctx, AV_LOG_INFO, "%s", tmp);
                    
                    p += len;
                    if (*p == 0xd) av_log(ctx, AV_LOG_INFO, " ");
                    if (*p == 0xa) av_log(ctx, AV_LOG_INFO, "\n%s  %-16s: ", indent, "");
                    if (*p) p++;
                }
                av_log(ctx, AV_LOG_INFO, "\n");
            }
    }
}

static void dump_stream_format(AVFormatContext *ic, int i, int index, int is_output)
{
    char buf[256];
    int flags = (is_output ? ic->oformat->flags : ic->iformat->flags);
    AVStream *st = ic->streams[i];
    AVDictionaryEntry *lang = av_dict_get(st->metadata, "language", NULL, 0);
    char *separator = ic->dump_separator;
    AVCodecContext *avctx;
    int ret;

    avctx = avcodec_alloc_context3(NULL);
    if (!avctx)
        return;

    ret = avcodec_parameters_to_context(avctx, st->codecpar);
    if (ret < 0) {
        avcodec_free_context(&avctx);
        return;
    }

    // Fields which are missing from AVCodecParameters need to be taken from the AVCodecContext
    avctx->properties = st->codec->properties;
    avctx->codec      = st->codec->codec;
    avctx->qmin       = st->codec->qmin;
    avctx->qmax       = st->codec->qmax;
    avctx->coded_width  = st->codec->coded_width;
    avctx->coded_height = st->codec->coded_height;

    if (separator)
        av_opt_set(avctx, "dump_separator", separator, 0);
    avcodec_string(buf, sizeof(buf), avctx, is_output);
    avcodec_free_context(&avctx);

    av_log(NULL, AV_LOG_INFO, "    Stream #%d:%d", index, i);

    /* the pid is an important information, so we display it */
    /* XXX: add a generic system */
    if (flags & AVFMT_SHOW_IDS)
        av_log(NULL, AV_LOG_INFO, "[0x%x]", st->id);
    if (lang)
        av_log(NULL, AV_LOG_INFO, "(%s)", lang->value);
    av_log(NULL, AV_LOG_DEBUG, ", %d, %d/%d", st->codec_info_nb_frames,
           st->time_base.num, st->time_base.den);
    av_log(NULL, AV_LOG_INFO, ": %s", buf);

    if (st->sample_aspect_ratio.num &&
        av_cmp_q(st->sample_aspect_ratio, st->codecpar->sample_aspect_ratio)) {
        AVRational display_aspect_ratio;
        av_reduce(&display_aspect_ratio.num, &display_aspect_ratio.den,
                  st->codecpar->width  * (int64_t)st->sample_aspect_ratio.num,
                  st->codecpar->height * (int64_t)st->sample_aspect_ratio.den,
                  1024 * 1024);
        av_log(NULL, AV_LOG_INFO, ", SAR %d:%d DAR %d:%d",
               st->sample_aspect_ratio.num, st->sample_aspect_ratio.den,
               display_aspect_ratio.num, display_aspect_ratio.den);
    }

    if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        int fps = st->avg_frame_rate.den && st->avg_frame_rate.num;
        int tbr = st->r_frame_rate.den && st->r_frame_rate.num;
        int tbn = st->time_base.den && st->time_base.num;
        int tbc = st->codec->time_base.den && st->codec->time_base.num;

        if (fps || tbr || tbn || tbc)
            av_log(NULL, AV_LOG_INFO, "%s", separator);

        if (fps)
            print_fps(av_q2d(st->avg_frame_rate), tbr || tbn || tbc ? "fps, " : "fps");
        if (tbr)
            print_fps(av_q2d(st->r_frame_rate), tbn || tbc ? "tbr, " : "tbr");
        if (tbn)
            print_fps(1 / av_q2d(st->time_base), tbc ? "tbn, " : "tbn");
        if (tbc)
            print_fps(1 / av_q2d(st->codec->time_base), "tbc");
    }

    if (st->disposition & AV_DISPOSITION_DEFAULT)
        av_log(NULL, AV_LOG_INFO, " (default)");
    if (st->disposition & AV_DISPOSITION_DUB)
        av_log(NULL, AV_LOG_INFO, " (dub)");
    if (st->disposition & AV_DISPOSITION_ORIGINAL)
        av_log(NULL, AV_LOG_INFO, " (original)");
    if (st->disposition & AV_DISPOSITION_COMMENT)
        av_log(NULL, AV_LOG_INFO, " (comment)");
    if (st->disposition & AV_DISPOSITION_LYRICS)
        av_log(NULL, AV_LOG_INFO, " (lyrics)");
    if (st->disposition & AV_DISPOSITION_KARAOKE)
        av_log(NULL, AV_LOG_INFO, " (karaoke)");
    if (st->disposition & AV_DISPOSITION_FORCED)
        av_log(NULL, AV_LOG_INFO, " (forced)");
    if (st->disposition & AV_DISPOSITION_HEARING_IMPAIRED)
        av_log(NULL, AV_LOG_INFO, " (hearing impaired)");
    if (st->disposition & AV_DISPOSITION_VISUAL_IMPAIRED)
        av_log(NULL, AV_LOG_INFO, " (visual impaired)");
    if (st->disposition & AV_DISPOSITION_CLEAN_EFFECTS)
        av_log(NULL, AV_LOG_INFO, " (clean effects)");
    if (st->disposition & AV_DISPOSITION_DESCRIPTIONS)
        av_log(NULL, AV_LOG_INFO, " (descriptions)");
    if (st->disposition & AV_DISPOSITION_DEPENDENT)
        av_log(NULL, AV_LOG_INFO, " (dependent)");
    if (st->disposition & AV_DISPOSITION_STILL_IMAGE)
        av_log(NULL, AV_LOG_INFO, " (still image)");
    av_log(NULL, AV_LOG_INFO, "\n");

    dump_metadata(NULL, st->metadata, "    ");

    dump_sidedata(NULL, st, "    ");
}

void av_dump_format(AVFormatContext *ic, int index, const char *url, int is_output)
{
    int i;
    uint8_t *printed = ic->nb_streams ? av_mallocz(ic->nb_streams) : NULL;
    if (ic->nb_streams && !printed)
        return;

    av_log(NULL, AV_LOG_INFO, "%s #%d, %s, %s '%s':\n",
           is_output ? "Output" : "Input",
           index,
           is_output ? ic->oformat->name : ic->iformat->name,
           is_output ? "to" : "from", url);
    
    dump_metadata(NULL, ic->metadata, "  ");

    if (!is_output) {
        av_log(NULL, AV_LOG_INFO, "  Duration: ");
        if (ic->duration != AV_NOPTS_VALUE) {
            int hours, mins, secs, us;
            int64_t duration = ic->duration + (ic->duration <= INT64_MAX - 5000 ? 5000 : 0);
            secs  = duration / AV_TIME_BASE;
            us    = duration % AV_TIME_BASE;
            mins  = secs / 60;
            secs %= 60;
            hours = mins / 60;
            mins %= 60;
            av_log(NULL, AV_LOG_INFO, "%02d:%02d:%02d.%02d", hours, mins, secs, (100 * us) / AV_TIME_BASE);
        } else {
            av_log(NULL, AV_LOG_INFO, "N/A");
        }
        
        if (ic->start_time != AV_NOPTS_VALUE) {
            int secs, us;
            av_log(NULL, AV_LOG_INFO, ", start: ");
            secs = llabs(ic->start_time / AV_TIME_BASE);
            us   = llabs(ic->start_time % AV_TIME_BASE);
            av_log(NULL, AV_LOG_INFO, "%s%d.%06d", ic->start_time >= 0 ? "" : "-", secs, (int)av_rescale(us, 1000000, AV_TIME_BASE));
        }
        
        av_log(NULL, AV_LOG_INFO, ", bitrate: ");
        if (ic->bit_rate) {
            av_log(NULL, AV_LOG_INFO, "%"PRId64" kb/s", ic->bit_rate / 1000);
        }
        else {
            av_log(NULL, AV_LOG_INFO, "N/A");
        }
        
        av_log(NULL, AV_LOG_INFO, "\n");
    }

    for (i = 0; i < ic->nb_chapters; i++) {
        AVChapter *ch = ic->chapters[i];
        av_log(NULL, AV_LOG_INFO, "    Chapter #%d:%d: ", index, i);
        av_log(NULL, AV_LOG_INFO, "start %f, ", ch->start * av_q2d(ch->time_base));
        av_log(NULL, AV_LOG_INFO, "end %f\n", ch->end * av_q2d(ch->time_base));

        dump_metadata(NULL, ch->metadata, "    ");
    }

    if (ic->nb_programs) {
        int j, k, total = 0;
        for (j = 0; j < ic->nb_programs; j++) {
            AVDictionaryEntry *name = av_dict_get(ic->programs[j]->metadata, "name", NULL, 0);
            
            av_log(NULL, AV_LOG_INFO, "  Program %d %s\n", ic->programs[j]->id, name ? name->value : "");
            dump_metadata(NULL, ic->programs[j]->metadata, "    ");
            
            for (k = 0; k < ic->programs[j]->nb_stream_indexes; k++) {
                dump_stream_format(ic, ic->programs[j]->stream_index[k],
                                   index, is_output);
                printed[ic->programs[j]->stream_index[k]] = 1;
            }
            total += ic->programs[j]->nb_stream_indexes;
        }
        if (total < ic->nb_streams)
            av_log(NULL, AV_LOG_INFO, "  No Program\n");
    }

    for (i = 0; i < ic->nb_streams; i++)
        if (!printed[i])
            dump_stream_format(ic, i, index, is_output);

    av_free(printed);
}

