/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-17
 * Description:
 
*********************************/



#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "libavutil/rational.h"
#include "libavutil/log.h"
#include "libavutil/dict.h"
#include "libavutil/mem.h"
#include "libavutil/error.h"
#include "libavutil/assert.h"

#include "libavcodec/avcodec.h"
#include "libavcodec/utils.h"

#include "libavformat/avformat.h"
#include "libavformat/utils.h"
#include "libavformat/dump.h"

#include "fftools/cmdutil.h"

#define SECTION_MAX_NB_LEVELS 10
#define MAX_REGISTERED_WRITERS_NB 64

#if 0
typedef struct WriterContext {
    const AVClass *class;           ///< class of the writer
    const Writer *writer;           ///< the Writer of which this is an instance
    char *name;                     ///< name of this writer instance
    void *priv;                     ///< private data for use by the filter

    const struct section *sections; ///< array containing all sections
    int nb_sections;                ///< number of sections

    int level;                      ///< current level, starting from 0

    /** number of the item printed in the given section, starting from 0 */
    unsigned int nb_item[SECTION_MAX_NB_LEVELS];

    /** section per each level */
    const struct section *section[SECTION_MAX_NB_LEVELS];
    //AVBPrint section_pbuf[SECTION_MAX_NB_LEVELS]; ///< generic print buffer dedicated to each section,
                                                  ///  used by various writers

    unsigned int nb_section_packet; ///< number of the packet section in case we are in "packets_and_frames" section
    unsigned int nb_section_frame;  ///< number of the frame  section in case we are in "packets_and_frames" section
    unsigned int nb_section_packet_frame; ///< nb_section_packet or nb_section_frame according if is_packets_and_frames

    int string_validation;
    char *string_validation_replacement;
    unsigned int string_validation_utf8_flags;
} WriterContext;

typedef struct Writer {
    const AVClass *priv_class;      ///< private class of the writer, if any
    int priv_size;                  ///< private size for the writer context
    const char *name;

    int  (*init)  (WriterContext *wctx);
    void (*uninit)(WriterContext *wctx);

    void (*print_section_header)(WriterContext *wctx);
    void (*print_section_footer)(WriterContext *wctx);
    void (*print_integer)       (WriterContext *wctx, const char *, long long int);
    void (*print_rational)      (WriterContext *wctx, AVRational *q, char *sep);
    void (*print_string)        (WriterContext *wctx, const char *, const char *);
    int flags;                  ///< a combination or WRITER_FLAG_*
} Writer;
#endif

typedef struct DefaultContext {
    const AVClass *class;
    int nokey;
    int noprint_wrappers;
    int nested_section[SECTION_MAX_NB_LEVELS];
} DefaultContext;

typedef struct InputStream {
    AVStream *st;
    AVCodecContext *dec_ctx;
} InputStream;

typedef struct InputFile {
    AVFormatContext *fmt_ctx;
    InputStream *streams;
    int       nb_streams;
} InputFile;

#if HAVE_THREADS
pthread_mutex_t log_mutex;
#endif

const char program_name[] = "ffprobe";
const int program_birth_year = 2007;

static int do_show_error   = 0;
static int hide_banner = 1;
static int find_stream_info  = 1;


static const char *input_filename = NULL;

//static const Writer *registered_writers[MAX_REGISTERED_WRITERS_NB + 1];

static void (*program_exit)(int ret);

static void ffprobe_cleanup(int ret)
{
    //int i;
    //for (i = 0; i < FF_ARRAY_ELEMS(sections); i++)
    //    av_dict_free(&(sections[i].entries_to_show));

#if HAVE_THREADS
    //pthread_mutex_destroy(&log_mutex);
#endif
}

void register_exit(void (*cb)(int ret))
{
    program_exit = cb;
}

void exit_program(int ret)
{
    if (program_exit)
        program_exit(ret);

    exit(ret);
}

int locate_option(int argc, char **argv, const OptionDef *options, const char *optname)
{
    const OptionDef *po;
    int i;

    for (i = 1; i < argc; i++) {
        const char *cur_opt = argv[i];

        if (*cur_opt++ != '-')
            continue;

        po = find_option(options, cur_opt);
        if (!po->name && cur_opt[0] == 'n' && cur_opt[1] == 'o')
            po = find_option(options, cur_opt + 2);

        if ((!po->name && !strcmp(cur_opt, optname)) || (po->name && !strcmp(optname, po->name)))
            return i;

        if (!po->name || po->flags & HAS_ARG)
            i++;
    }
    
    return 0;
}

static void check_options(const OptionDef *po)
{
    while (po->name) {
        if (po->flags & OPT_PERFILE)
            av_assert0(po->flags & (OPT_INPUT | OPT_OUTPUT));
        po++;
    }
}

#if 0
void parse_loglevel(int argc, char **argv, const OptionDef *options)
{
    int idx = locate_option(argc, argv, options, "loglevel");
    const char *env;

    check_options(options);

    if (!idx)
        idx = locate_option(argc, argv, options, "v");
    if (idx && argv[idx + 1])
        opt_loglevel(NULL, "loglevel", argv[idx + 1]);
    idx = locate_option(argc, argv, options, "report");
    if ((env = getenv("FFREPORT")) || idx) {
        init_report(env);
        if (report_file) {
            int i;
            fprintf(report_file, "Command line:\n");
            for (i = 0; i < argc; i++) {
                dump_argument(argv[i]);
                fputc(i < argc - 1 ? ' ' : '\n', report_file);
            }
            fflush(report_file);
        }
    }
    idx = locate_option(argc, argv, options, "hide_banner");
    if (idx)
        hide_banner = 1;
}

int avformat_network_init(void)
{
#if CONFIG_NETWORK
    int ret;
    if ((ret = ff_network_init()) < 0)
        return ret;
    if ((ret = ff_tls_init()) < 0)
        return ret;
#endif
    return 0;
}

int avformat_network_deinit(void)
{
#if CONFIG_NETWORK
    ff_network_close();
    ff_tls_deinit();
#endif
    return 0;
}
#endif



void init_opts(void)
{
    //av_dict_set(&sws_dict, "flags", "bicubic", 0);
}

void show_banner(int argc, char **argv, const OptionDef *options)
{
    int idx = locate_option(argc, argv, options, "version");
    if (hide_banner || idx)
        return;

    print_program_info (INDENT|SHOW_COPYRIGHT, AV_LOG_INFO);
    //print_all_libs_info(INDENT|SHOW_CONFIG,  AV_LOG_INFO);
    print_all_libs_info(INDENT|SHOW_VERSION, AV_LOG_INFO);
}

void parse_options(void *optctx, int argc, char **argv, const OptionDef *options, void (*parse_arg_function)(void *, const char*))
{
    const char *opt;
    int optindex, handleoptions = 1, ret;

    /* perform system-dependent conversions for arguments list */
    //prepare_app_arguments(&argc, &argv);

    /* parse options */
    optindex = 1;
    while (optindex < argc) {
        opt = argv[optindex++];

        if (handleoptions && opt[0] == '-' && opt[1] != '\0') {
            if (opt[1] == '-' && opt[2] == '\0') {
                handleoptions = 0;
                continue;
            }
            opt++;

            if ((ret = parse_option(optctx, opt, argv[optindex], options)) < 0)
                exit_program(1);
            
            optindex += ret;
        } else {
            if (parse_arg_function)
                parse_arg_function(optctx, opt);
        }
    }
}

static void opt_input_file(void *optctx, const char *arg)
{
    if (input_filename) {
        av_log(NULL, AV_LOG_ERROR, "Argument '%s' provided as input filename, but '%s' was already specified.\n", arg, input_filename);
        exit_program(1);
    }
    
    if (!strcmp(arg, "-"))
        arg = "pipe:";
    
    input_filename = arg;
}

#if 0
static int writer_register(const Writer *writer)
{
    static int next_registered_writer_idx = 0;

    if (next_registered_writer_idx == MAX_REGISTERED_WRITERS_NB)
        return AVERROR(ENOMEM);

    registered_writers[next_registered_writer_idx++] = writer;
    return 0;
}
#endif

#if 0
#undef OFFSET
#define OFFSET(x) offsetof(DefaultContext, x)
static const AVOption default_options[] = {
    { "noprint_wrappers", "do not print headers and footers", OFFSET(noprint_wrappers), AV_OPT_TYPE_BOOL, {.i64=0}, 0, 1 },
    { "nw",               "do not print headers and footers", OFFSET(noprint_wrappers), AV_OPT_TYPE_BOOL, {.i64=0}, 0, 1 },
    { "nokey",          "force no key printing",     OFFSET(nokey),          AV_OPT_TYPE_BOOL, {.i64=0}, 0, 1 },
    { "nk",             "force no key printing",     OFFSET(nokey),          AV_OPT_TYPE_BOOL, {.i64=0}, 0, 1 },
    {NULL},
};

static const Writer default_writer = {
    .name                  = "default",
    .priv_size             = sizeof(DefaultContext),
    .print_section_header  = default_print_section_header,
    .print_section_footer  = default_print_section_footer,
    .print_integer         = default_print_int,
    .print_string          = default_print_str,
    .flags = WRITER_FLAG_DISPLAY_OPTIONAL_FIELDS,
    .priv_class            = &default_class,
};

static const Writer compact_writer = {
    .name                 = "compact",
    .priv_size            = sizeof(CompactContext),
    .init                 = compact_init,
    .print_section_header = compact_print_section_header,
    .print_section_footer = compact_print_section_footer,
    .print_integer        = compact_print_int,
    .print_string         = compact_print_str,
    .flags = WRITER_FLAG_DISPLAY_OPTIONAL_FIELDS,
    .priv_class           = &compact_class,
};

static void writer_register_all(void)
{
    static int initialized;

    if (initialized)
        return;
    initialized = 1;

    writer_register(&default_writer);
    writer_register(&compact_writer);
    writer_register(&csv_writer);
    writer_register(&flat_writer);
    writer_register(&ini_writer);
    writer_register(&json_writer);
    writer_register(&xml_writer);
}
#endif

static void show_usage(void)
{
    av_log(NULL, AV_LOG_INFO, "Simple multimedia streams analyzer\n");
    av_log(NULL, AV_LOG_INFO, "usage: %s [OPTIONS] [INPUT_FILE]\n", program_name);
    av_log(NULL, AV_LOG_INFO, "\n");
}

#if 0
static void show_error(WriterContext *w, int err)
{
    char errbuf[128];
    const char *errbuf_ptr = errbuf;

    if (av_strerror(err, errbuf, sizeof(errbuf)) < 0)
        errbuf_ptr = strerror(AVUNERROR(err));

    //writer_print_section_header(w, SECTION_ID_ERROR);
    //print_int("code", err);
    //print_str("string", errbuf_ptr);
    //writer_print_section_footer(w);
}
#endif

static int open_input_file(InputFile *ifile, const char *filename)
{
    int err, i;
    AVFormatContext *fmt_ctx = NULL;
    AVDictionaryEntry *t;
    int scan_all_pmts_set = 0;

    fmt_ctx = avformat_alloc_context();
    if (!fmt_ctx) {
        print_error(filename, AVERROR(ENOMEM));
        exit_program(1);
    }

    /*if (!av_dict_get(format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE)) {
        av_dict_set(&format_opts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
        scan_all_pmts_set = 1;
    }*/
    
    if ((err = avformat_open_input(&fmt_ctx, filename, NULL, NULL)) < 0) {
        print_error(filename, err);
        return err;
    }
    
    ifile->fmt_ctx = fmt_ctx;
    /*if (scan_all_pmts_set) {
        av_dict_set(&format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE);
    }*/
    
    /*if ((t = av_dict_get(format_opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
        av_log(NULL, AV_LOG_ERROR, "Option %s not found.\n", t->key);
        return AVERROR_OPTION_NOT_FOUND;
    }*/

    if (find_stream_info) {
        AVDictionary **opts = NULL;//setup_find_stream_info_opts(fmt_ctx, NULL);
        int orig_nb_streams = fmt_ctx->nb_streams;

        err = avformat_find_stream_info(fmt_ctx, opts);

        for (i = 0; i < orig_nb_streams; i++)
            av_dict_free(&opts[i]);
        
        av_freep(&opts);
        
        if (err < 0) {
            print_error(filename, err);
            return err;
        }
    }

    av_dump_format(fmt_ctx, 0, filename, 0);

    ifile->streams = av_mallocz_array(fmt_ctx->nb_streams, sizeof(*ifile->streams));
    if (!ifile->streams) {
        exit(1);
    }
    
    ifile->nb_streams = fmt_ctx->nb_streams;

    /* bind a decoder to each input stream */
    for (i = 0; i < fmt_ctx->nb_streams; i++) {
        InputStream *ist = &ifile->streams[i];
        AVStream *stream = fmt_ctx->streams[i];
        AVCodec *codec;

        ist->st = stream;

        if (stream->codecpar->codec_id == AV_CODEC_ID_PROBE) {
            av_log(NULL, AV_LOG_WARNING, "Failed to probe codec for input stream %d\n", stream->index);
            continue;
        }

        codec = avcodec_find_decoder(stream->codecpar->codec_id);
        if (!codec) {
            av_log(NULL, AV_LOG_WARNING, "Unsupported codec with id %d for input stream %d\n", stream->codecpar->codec_id, stream->index);
            continue;
        }
        
        //AVDictionary *opts = filter_codec_opts(NULL, stream->codecpar->codec_id, fmt_ctx, stream, codec);

        ist->dec_ctx = avcodec_alloc_context3(codec);
        if (!ist->dec_ctx)
            exit(1);

        err = avcodec_parameters_to_context(ist->dec_ctx, stream->codecpar);
        if (err < 0)
            exit(1);

        /*if (do_show_log) {
            // For loging it is needed to disable at least frame threads as otherwise
            // the log information would need to be reordered and matches up to contexts and frames
            // That is in fact possible but not trivial
            av_dict_set(&codec_opts, "threads", "1", 0);
        }*/

        ist->dec_ctx->pkt_timebase = stream->time_base;
        ist->dec_ctx->framerate = stream->avg_frame_rate;
        
#if 0 //FF_API_LAVF_AVCTX
        ist->dec_ctx->coded_width = stream->codec->coded_width;
        ist->dec_ctx->coded_height = stream->codec->coded_height;
#endif

        /*if (avcodec_open2(ist->dec_ctx, codec, &opts) < 0) {
            av_log(NULL, AV_LOG_WARNING, "Could not open codec for input stream %d\n", stream->index);
            exit(1);
        }*/

        /*if ((t = av_dict_get(opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
            av_log(NULL, AV_LOG_ERROR, "Option %s for input stream %d not found\n", t->key, stream->index);
            return AVERROR_OPTION_NOT_FOUND;
        }
        */
    }

    ifile->fmt_ctx = fmt_ctx;
    return 0;
}

static void close_input_file(InputFile *ifile)
{
    int i;

    /* close decoder for each stream */
    for (i = 0; i < ifile->nb_streams; i++)
        if (ifile->streams[i].st->codecpar->codec_id != AV_CODEC_ID_NONE)
            avcodec_free_context(&ifile->streams[i].dec_ctx);

    av_freep(&ifile->streams);
    ifile->nb_streams = 0;

    avformat_close_input(&ifile->fmt_ctx);
}

static int probe_file(const char *filename)
{
    InputFile ifile = { 0 };
    int ret, i;
    int section_id;
    int nb_streams;

    //do_read_frames = do_show_frames || do_count_frames;
    //do_read_packets = do_show_packets || do_count_packets;

    ret = open_input_file(&ifile, filename);
    if (ret < 0) {
        goto end;
    }
#if 0
#define CHECK_END if (ret < 0) goto end

    nb_streams = ifile.fmt_ctx->nb_streams;
    REALLOCZ_ARRAY_STREAM(nb_streams_frames,0,ifile.fmt_ctx->nb_streams);
    REALLOCZ_ARRAY_STREAM(nb_streams_packets,0,ifile.fmt_ctx->nb_streams);
    REALLOCZ_ARRAY_STREAM(selected_streams,0,ifile.fmt_ctx->nb_streams);

    for (i = 0; i < ifile.fmt_ctx->nb_streams; i++) {
        if (stream_specifier) {
            ret = avformat_match_stream_specifier(ifile.fmt_ctx,
                                                  ifile.fmt_ctx->streams[i],
                                                  stream_specifier);
            CHECK_END;
            else
                selected_streams[i] = ret;
            ret = 0;
        } else {
            selected_streams[i] = 1;
        }
        if (!selected_streams[i])
            ifile.fmt_ctx->streams[i]->discard = AVDISCARD_ALL;
    }
    
    if (do_read_frames || do_read_packets) {
        if (do_show_frames && do_show_packets &&
            wctx->writer->flags & WRITER_FLAG_PUT_PACKETS_AND_FRAMES_IN_SAME_CHAPTER)
            section_id = SECTION_ID_PACKETS_AND_FRAMES;
        else if (do_show_packets && !do_show_frames)
            section_id = SECTION_ID_PACKETS;
        else // (!do_show_packets && do_show_frames)
            section_id = SECTION_ID_FRAMES;
        if (do_show_frames || do_show_packets)
            writer_print_section_header(wctx, section_id);
        ret = read_packets(wctx, &ifile);
        if (do_show_frames || do_show_packets)
            writer_print_section_footer(wctx);
        CHECK_END;
    }

    if (do_show_programs) {
        ret = show_programs(wctx, &ifile);
        CHECK_END;
    }

    if (do_show_streams) {
        ret = show_streams(wctx, &ifile);
        CHECK_END;
    }
    if (do_show_chapters) {
        ret = show_chapters(wctx, &ifile);
        CHECK_END;
    }
    if (do_show_format) {
        ret = show_format(wctx, &ifile);
        CHECK_END;
    }
#endif

end:
    if (ifile.fmt_ctx) {
        close_input_file(&ifile);
    }
    //av_freep(&nb_streams_frames);
    //av_freep(&nb_streams_packets);
    //av_freep(&selected_streams);

    return ret;
}

int main(int argc, char **argv)
{
    //const Writer *w;
    //WriterContext *wctx;
    //char *buf;
    //char *w_name = NULL, *w_args = NULL;
    int ret;

    //init_dynload();

#if 0 //HAVE_THREADS
    ret = pthread_mutex_init(&log_mutex, NULL);
    if (ret != 0) {
        goto end;
    }
#endif

    //av_log_set_flags(AV_LOG_SKIP_REPEATED);
    //register_exit(ffprobe_cleanup);

    //options = real_options;
    //parse_loglevel(argc, argv, NULL);
    //avformat_network_init();
    //init_opts();
#if 0 //CONFIG_AVDEVICE
    avdevice_register_all();
#endif

    //show_banner(argc, argv, options);
    parse_options(NULL, argc, argv, NULL, opt_input_file);

    //if (do_show_log)
    //    av_log_set_callback(log_callback);

#if 0
    /* mark things to show, based on -show_entries */
    SET_DO_SHOW(CHAPTERS, chapters);
    SET_DO_SHOW(ERROR, error);
    SET_DO_SHOW(FORMAT, format);
    SET_DO_SHOW(FRAMES, frames);
    SET_DO_SHOW(LIBRARY_VERSIONS, library_versions);
    SET_DO_SHOW(PACKETS, packets);
    SET_DO_SHOW(PIXEL_FORMATS, pixel_formats);
    SET_DO_SHOW(PIXEL_FORMAT_FLAGS, pixel_format_flags);
    SET_DO_SHOW(PIXEL_FORMAT_COMPONENTS, pixel_format_components);
    SET_DO_SHOW(PROGRAM_VERSION, program_version);
    SET_DO_SHOW(PROGRAMS, programs);
    SET_DO_SHOW(STREAMS, streams);
    SET_DO_SHOW(STREAM_DISPOSITION, stream_disposition);
    SET_DO_SHOW(PROGRAM_STREAM_DISPOSITION, stream_disposition);

    SET_DO_SHOW(CHAPTER_TAGS, chapter_tags);
    SET_DO_SHOW(FORMAT_TAGS, format_tags);
    SET_DO_SHOW(FRAME_TAGS, frame_tags);
    SET_DO_SHOW(PROGRAM_TAGS, program_tags);
    SET_DO_SHOW(STREAM_TAGS, stream_tags);
    SET_DO_SHOW(PROGRAM_STREAM_TAGS, stream_tags);
    SET_DO_SHOW(PACKET_TAGS, packet_tags);
#endif

    //writer_register_all();

#if 0
    if (!print_format)
        print_format = av_strdup("default");
    if (!print_format) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    w_name = av_strtok(print_format, "=", &buf);
    if (!w_name) {
        av_log(NULL, AV_LOG_ERROR,
               "No name specified for the output format\n");
        ret = AVERROR(EINVAL);
        goto end;
    }
    w_args = buf;

    if (show_data_hash) {
        if ((ret = av_hash_alloc(&hash, show_data_hash)) < 0) {
            if (ret == AVERROR(EINVAL)) {
                const char *n;
                av_log(NULL, AV_LOG_ERROR,
                       "Unknown hash algorithm '%s'\nKnown algorithms:",
                       show_data_hash);
                for (i = 0; (n = av_hash_names(i)); i++)
                    av_log(NULL, AV_LOG_ERROR, " %s", n);
                av_log(NULL, AV_LOG_ERROR, "\n");
            }
            goto end;
        }
    }

    w = writer_get_by_name(w_name);
    if (!w) {
        av_log(NULL, AV_LOG_ERROR, "Unknown output format with name '%s'\n", w_name);
        ret = AVERROR(EINVAL);
        goto end;
    }

    if ((ret = writer_open(&wctx, w, w_args, sections, FF_ARRAY_ELEMS(sections))) >= 0) {
        if (w == &xml_writer)
            wctx->string_validation_utf8_flags |= AV_UTF8_FLAG_EXCLUDE_XML_INVALID_CONTROL_CODES;

        writer_print_section_header(wctx, SECTION_ID_ROOT);

        if (do_show_program_version)
            ffprobe_show_program_version(wctx);
        if (do_show_library_versions)
            ffprobe_show_library_versions(wctx);
        if (do_show_pixel_formats)
            ffprobe_show_pixel_formats(wctx);

        //writer_print_section_footer(wctx);
        //writer_close(&wctx);
    }
#endif

    if (!input_filename) {
        show_usage();
        av_log(NULL, AV_LOG_ERROR, "You have to specify one input file.\n");
        av_log(NULL, AV_LOG_ERROR, "Use -h to get full help or, even better, run 'man %s'.\n", program_name);
        ret = AVERROR(EINVAL);
    } else if (input_filename) {
        probe_file(input_filename);
        //if (ret < 0 && do_show_error)
        //    show_error(NULL, ret);
    }

//end:
    //av_freep(&print_format);
    //av_freep(&read_intervals);
    //av_hash_freep(&hash);

    //uninit_opts();
    //for (i = 0; i < FF_ARRAY_ELEMS(sections); i++)
    //    av_dict_free(&(sections[i].entries_to_show));

    //avformat_network_deinit();

    return ret < 0;
}

