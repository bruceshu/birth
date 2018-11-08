/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-18
 * Description:
 
*********************************/


#include "libavutil/thread.h"
#include "libavutil/error.h"
#include "libavutil/frame.h"

#include "internal.h"

static AVMutex codec_mutex = AV_MUTEX_INITIALIZER;


int av_codec_is_decoder(const AVCodec *codec)
{
    return codec && (codec->decode || codec->receive_frame);
}

int avcodec_is_open(AVCodecContext *s)
{
    return !!s->internal;
}

static void ff_lock_avcodec(AVCodecContext *log_ctx, const AVCodec *codec)
{
    if (!(codec->caps_internal & FF_CODEC_CAP_INIT_THREADSAFE) && codec->init)
        ff_mutex_lock(&codec_mutex);
}

#if 0
int ff_set_dimensions(AVCodecContext *s, int width, int height)
{
    int ret = av_image_check_size2(width, height, s->max_pixels, AV_PIX_FMT_NONE, 0, s);

    if (ret < 0)
        width = height = 0;

    s->coded_width  = width;
    s->coded_height = height;
    s->width        = AV_CEIL_RSHIFT(width,  s->lowres);
    s->height       = AV_CEIL_RSHIFT(height, s->lowres);

    return ret;
}
#endif

int av_codec_is_encoder(const AVCodec *codec)
{   
    //codec->encode_sub || 
    return codec && (codec->encode2 || codec->send_frame);
}

int av_get_exact_bits_per_sample(enum AVCodecID codec_id)
{
    switch (codec_id) {
    case AV_CODEC_ID_8SVX_EXP:
    case AV_CODEC_ID_8SVX_FIB:
    case AV_CODEC_ID_ADPCM_CT:
    case AV_CODEC_ID_ADPCM_IMA_APC:
    case AV_CODEC_ID_ADPCM_IMA_EA_SEAD:
    case AV_CODEC_ID_ADPCM_IMA_OKI:
    case AV_CODEC_ID_ADPCM_IMA_WS:
    case AV_CODEC_ID_ADPCM_G722:
    case AV_CODEC_ID_ADPCM_YAMAHA:
    case AV_CODEC_ID_ADPCM_AICA:
        return 4;
    case AV_CODEC_ID_DSD_LSBF:
    case AV_CODEC_ID_DSD_MSBF:
    case AV_CODEC_ID_DSD_LSBF_PLANAR:
    case AV_CODEC_ID_DSD_MSBF_PLANAR:
    case AV_CODEC_ID_PCM_ALAW:
    case AV_CODEC_ID_PCM_MULAW:
    case AV_CODEC_ID_PCM_VIDC:
    case AV_CODEC_ID_PCM_S8:
    case AV_CODEC_ID_PCM_S8_PLANAR:
    case AV_CODEC_ID_PCM_U8:
    case AV_CODEC_ID_PCM_ZORK:
    case AV_CODEC_ID_SDX2_DPCM:
        return 8;
    case AV_CODEC_ID_PCM_S16BE:
    case AV_CODEC_ID_PCM_S16BE_PLANAR:
    case AV_CODEC_ID_PCM_S16LE:
    case AV_CODEC_ID_PCM_S16LE_PLANAR:
    case AV_CODEC_ID_PCM_U16BE:
    case AV_CODEC_ID_PCM_U16LE:
        return 16;
    case AV_CODEC_ID_PCM_S24DAUD:
    case AV_CODEC_ID_PCM_S24BE:
    case AV_CODEC_ID_PCM_S24LE:
    case AV_CODEC_ID_PCM_S24LE_PLANAR:
    case AV_CODEC_ID_PCM_U24BE:
    case AV_CODEC_ID_PCM_U24LE:
        return 24;
    case AV_CODEC_ID_PCM_S32BE:
    case AV_CODEC_ID_PCM_S32LE:
    case AV_CODEC_ID_PCM_S32LE_PLANAR:
    case AV_CODEC_ID_PCM_U32BE:
    case AV_CODEC_ID_PCM_U32LE:
    case AV_CODEC_ID_PCM_F32BE:
    case AV_CODEC_ID_PCM_F32LE:
    case AV_CODEC_ID_PCM_F24LE:
    case AV_CODEC_ID_PCM_F16LE:
        return 32;
    case AV_CODEC_ID_PCM_F64BE:
    case AV_CODEC_ID_PCM_F64LE:
    case AV_CODEC_ID_PCM_S64BE:
    case AV_CODEC_ID_PCM_S64LE:
        return 64;
    default:
        return 0;
    }
}

int av_get_bits_per_sample(enum AVCodecID codec_id)
{
    switch (codec_id) {
    case AV_CODEC_ID_ADPCM_SBPRO_2:
        return 2;
    case AV_CODEC_ID_ADPCM_SBPRO_3:
        return 3;
    case AV_CODEC_ID_ADPCM_SBPRO_4:
    case AV_CODEC_ID_ADPCM_IMA_WAV:
    case AV_CODEC_ID_ADPCM_IMA_QT:
    case AV_CODEC_ID_ADPCM_SWF:
    case AV_CODEC_ID_ADPCM_MS:
        return 4;
    default:
        return av_get_exact_bits_per_sample(codec_id);
    }
}

static int64_t get_bit_rate(AVCodecContext *ctx)
{
    int64_t bit_rate;
    int bits_per_sample;

    switch (ctx->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
    case AVMEDIA_TYPE_DATA:
    case AVMEDIA_TYPE_SUBTITLE:
    case AVMEDIA_TYPE_ATTACHMENT:
        bit_rate = ctx->bit_rate;
        break;
    case AVMEDIA_TYPE_AUDIO:
        bits_per_sample = av_get_bits_per_sample(ctx->codec_id);
        bit_rate = bits_per_sample ? ctx->sample_rate * (int64_t)ctx->channels * bits_per_sample : ctx->bit_rate;
        break;
    default:
        bit_rate = 0;
        break;
    }
    return bit_rate;
}

static void ff_unlock_avcodec(const AVCodec *codec)
{
    if (!(codec->caps_internal & FF_CODEC_CAP_INIT_THREADSAFE) && codec->init)
        ff_mutex_unlock(&codec_mutex);
}

int avcodec_open2(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options)
{
    int ret = 0;
    AVDictionary *tmp = NULL;
    const AVPixFmtDescriptor *pixdesc;

    if (avcodec_is_open(avctx))
        return 0;

    if ((!codec && !avctx->codec)) {
        av_log(avctx, AV_LOG_ERROR, "No codec provided to avcodec_open2()\n");
        return AVERROR(EINVAL);
    }
    if ((codec && avctx->codec && codec != avctx->codec)) {
        av_log(avctx, AV_LOG_ERROR, "This AVCodecContext was allocated for %s, "
                                    "but %s passed to avcodec_open2()\n", avctx->codec->name, codec->name);
        return AVERROR(EINVAL);
    }
    if (!codec)
        codec = avctx->codec;

    if (avctx->extradata_size < 0 || avctx->extradata_size >= FF_MAX_EXTRADATA_SIZE)
        return AVERROR(EINVAL);

    if (options)
        av_dict_copy(&tmp, *options, 0);

    ff_lock_avcodec(avctx, codec);

    avctx->internal = av_mallocz(sizeof(*avctx->internal));
    if (!avctx->internal) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    avctx->internal->pool = av_mallocz(sizeof(*avctx->internal->pool));
    if (!avctx->internal->pool) {
        ret = AVERROR(ENOMEM);
        goto free_and_end;
    }

    avctx->internal->to_free = av_frame_alloc();
    if (!avctx->internal->to_free) {
        ret = AVERROR(ENOMEM);
        goto free_and_end;
    }

    avctx->internal->compat_decode_frame = av_frame_alloc();
    if (!avctx->internal->compat_decode_frame) {
        ret = AVERROR(ENOMEM);
        goto free_and_end;
    }

    avctx->internal->buffer_frame = av_frame_alloc();
    if (!avctx->internal->buffer_frame) {
        ret = AVERROR(ENOMEM);
        goto free_and_end;
    }

    avctx->internal->buffer_pkt = av_packet_alloc();
    if (!avctx->internal->buffer_pkt) {
        ret = AVERROR(ENOMEM);
        goto free_and_end;
    }

    avctx->internal->ds.in_pkt = av_packet_alloc();
    if (!avctx->internal->ds.in_pkt) {
        ret = AVERROR(ENOMEM);
        goto free_and_end;
    }

    avctx->internal->last_pkt_props = av_packet_alloc();
    if (!avctx->internal->last_pkt_props) {
        ret = AVERROR(ENOMEM);
        goto free_and_end;
    }

    avctx->internal->skip_samples_multiplier = 1;

    if (codec->priv_data_size > 0) {
        if (!avctx->priv_data) {
            avctx->priv_data = av_mallocz(codec->priv_data_size);
            if (!avctx->priv_data) {
                ret = AVERROR(ENOMEM);
                goto end;
            }
            if (codec->priv_class) {
                *(const AVClass **)avctx->priv_data = codec->priv_class;
                av_opt_set_defaults(avctx->priv_data);
            }
        }
        if (codec->priv_class && (ret = av_opt_set_dict(avctx->priv_data, &tmp)) < 0)
            goto free_and_end;
    } else {
        avctx->priv_data = NULL;
    }
    if ((ret = av_opt_set_dict(avctx, &tmp)) < 0)
        goto free_and_end;

    if (avctx->codec_whitelist && av_match_list(codec->name, avctx->codec_whitelist, ',') <= 0) {
        av_log(avctx, AV_LOG_ERROR, "Codec (%s) not on whitelist \'%s\'\n", codec->name, avctx->codec_whitelist);
        ret = AVERROR(EINVAL);
        goto free_and_end;
    }

    // only call ff_set_dimensions() for non H.264/VP6F/DXV codecs so as not to overwrite previously setup dimensions
    if (!(avctx->coded_width && avctx->coded_height && avctx->width && avctx->height 
        && (avctx->codec_id == AV_CODEC_ID_H264 || avctx->codec_id == AV_CODEC_ID_VP6F || avctx->codec_id == AV_CODEC_ID_DXV))) 
    {
        /*if (avctx->coded_width && avctx->coded_height)
            ret = ff_set_dimensions(avctx, avctx->coded_width, avctx->coded_height);
        else if (avctx->width && avctx->height)
            ret = ff_set_dimensions(avctx, avctx->width, avctx->height);
        if (ret < 0)
            goto free_and_end;*/
    }

    /*if ((avctx->coded_width || avctx->coded_height || avctx->width || avctx->height)
        && (  av_image_check_size2(avctx->coded_width, avctx->coded_height, avctx->max_pixels, AV_PIX_FMT_NONE, 0, avctx) < 0
           || av_image_check_size2(avctx->width,       avctx->height,       avctx->max_pixels, AV_PIX_FMT_NONE, 0, avctx) < 0)) {
        av_log(avctx, AV_LOG_WARNING, "Ignoring invalid width/height values\n");
        ff_set_dimensions(avctx, 0, 0);
    }*/

    /*if (avctx->width > 0 && avctx->height > 0) {
        if (av_image_check_sar(avctx->width, avctx->height,
                               avctx->sample_aspect_ratio) < 0) {
            av_log(avctx, AV_LOG_WARNING, "ignoring invalid SAR: %u/%u\n",
                   avctx->sample_aspect_ratio.num,
                   avctx->sample_aspect_ratio.den);
            avctx->sample_aspect_ratio = (AVRational){ 0, 1 };
        }
    }*/

    /* if the decoder init function was already called previously,
     * free the already allocated subtitle_header before overwriting it */
    if (av_codec_is_decoder(codec))
        av_freep(&avctx->subtitle_header);

    if (avctx->channels > FF_SANE_NB_CHANNELS) {
        av_log(avctx, AV_LOG_ERROR, "Too many channels: %d\n", avctx->channels);
        ret = AVERROR(EINVAL);
        goto free_and_end;
    }

    avctx->codec = codec;
    if ((avctx->codec_type == AVMEDIA_TYPE_UNKNOWN || avctx->codec_type == codec->type) &&
        avctx->codec_id == AV_CODEC_ID_NONE) {
        avctx->codec_type = codec->type;
        avctx->codec_id   = codec->id;
    }
    if (avctx->codec_id != codec->id || (avctx->codec_type != codec->type
                                         && avctx->codec_type != AVMEDIA_TYPE_ATTACHMENT)) {
        av_log(avctx, AV_LOG_ERROR, "Codec type or id mismatches\n");
        ret = AVERROR(EINVAL);
        goto free_and_end;
    }
    avctx->frame_number = 0;
    //avctx->codec_descriptor = avcodec_descriptor_get(avctx->codec_id);

    if ((avctx->codec->capabilities & AV_CODEC_CAP_EXPERIMENTAL) &&
        avctx->strict_std_compliance > FF_COMPLIANCE_EXPERIMENTAL) {
        const char *codec_string = av_codec_is_encoder(codec) ? "encoder" : "decoder";
        AVCodec *codec2;
        av_log(avctx, AV_LOG_ERROR,
               "The %s '%s' is experimental but experimental codecs are not enabled, "
               "add '-strict %d' if you want to use it.\n",
               codec_string, codec->name, FF_COMPLIANCE_EXPERIMENTAL);
        codec2 = av_codec_is_encoder(codec) ? avcodec_find_encoder(codec->id) : avcodec_find_decoder(codec->id);
        if (!(codec2->capabilities & AV_CODEC_CAP_EXPERIMENTAL))
            av_log(avctx, AV_LOG_ERROR, "Alternatively use the non experimental %s '%s'.\n",
                codec_string, codec2->name);
        ret = AVERROR_EXPERIMENTAL;
        goto free_and_end;
    }

    if (avctx->codec_type == AVMEDIA_TYPE_AUDIO &&
        (!avctx->time_base.num || !avctx->time_base.den)) {
        avctx->time_base.num = 1;
        avctx->time_base.den = avctx->sample_rate;
    }

    if (!HAVE_THREADS)
        av_log(avctx, AV_LOG_WARNING, "Warning: not compiled with thread support, using thread emulation\n");

    /*if (CONFIG_FRAME_THREAD_ENCODER && av_codec_is_encoder(avctx->codec)) {
        ff_unlock_avcodec(codec); //we will instantiate a few encoders thus kick the counter to prevent false detection of a problem
        ret = ff_frame_thread_encoder_init(avctx, options ? *options : NULL);
        ff_lock_avcodec(avctx, codec);
        if (ret < 0)
            goto free_and_end;
    }*/

    /*if (av_codec_is_decoder(avctx->codec)) {
        ret = ff_decode_bsfs_init(avctx);
        if (ret < 0)
            goto free_and_end;
    }*/

    /*if (HAVE_THREADS && !(avctx->internal->frame_thread_encoder 
                     && (avctx->active_thread_type&FF_THREAD_FRAME))) {
        ret = ff_thread_init(avctx);
        if (ret < 0) {
            goto free_and_end;
        }
    }*/
    
    if (!HAVE_THREADS && !(codec->capabilities & AV_CODEC_CAP_AUTO_THREADS))
        avctx->thread_count = 1;

    if (avctx->codec->max_lowres < avctx->lowres || avctx->lowres < 0) {
        av_log(avctx, AV_LOG_WARNING, "The maximum value for lowres supported by the decoder is %d\n",
               avctx->codec->max_lowres);
        avctx->lowres = avctx->codec->max_lowres;
    }

    if (av_codec_is_encoder(avctx->codec)) {
        int i;
#if 0//FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
        avctx->coded_frame = av_frame_alloc();
        if (!avctx->coded_frame) {
            ret = AVERROR(ENOMEM);
            goto free_and_end;
        }
FF_ENABLE_DEPRECATION_WARNINGS
#endif

        if (avctx->time_base.num <= 0 || avctx->time_base.den <= 0) {
            av_log(avctx, AV_LOG_ERROR, "The encoder timebase is not set.\n");
            ret = AVERROR(EINVAL);
            goto free_and_end;
        }

        if (avctx->codec->sample_fmts) {
            for (i = 0; avctx->codec->sample_fmts[i] != AV_SAMPLE_FMT_NONE; i++) {
                if (avctx->sample_fmt == avctx->codec->sample_fmts[i])
                    break;
                if (avctx->channels == 1 &&
                    av_get_planar_sample_fmt(avctx->sample_fmt) ==
                    av_get_planar_sample_fmt(avctx->codec->sample_fmts[i])) {
                    avctx->sample_fmt = avctx->codec->sample_fmts[i];
                    break;
                }
            }
            if (avctx->codec->sample_fmts[i] == AV_SAMPLE_FMT_NONE) {
                char buf[128];
                snprintf(buf, sizeof(buf), "%d", avctx->sample_fmt);
                av_log(avctx, AV_LOG_ERROR, "Specified sample format %s is invalid or not supported\n",
                       (char *)av_x_if_null(av_get_sample_fmt_name(avctx->sample_fmt), buf));
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
        }
        if (avctx->codec->pix_fmts) {
            for (i = 0; avctx->codec->pix_fmts[i] != AV_PIX_FMT_NONE; i++)
                if (avctx->pix_fmt == avctx->codec->pix_fmts[i])
                    break;
            if (avctx->codec->pix_fmts[i] == AV_PIX_FMT_NONE
                && !((avctx->codec_id == AV_CODEC_ID_MJPEG || avctx->codec_id == AV_CODEC_ID_LJPEG)
                     && avctx->strict_std_compliance <= FF_COMPLIANCE_UNOFFICIAL)) {
                char buf[128];
                snprintf(buf, sizeof(buf), "%d", avctx->pix_fmt);
                av_log(avctx, AV_LOG_ERROR, "Specified pixel format %s is invalid or not supported\n",
                       (char *)av_x_if_null(av_get_pix_fmt_name(avctx->pix_fmt), buf));
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
            if (avctx->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ420P ||
                avctx->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ411P ||
                avctx->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ422P ||
                avctx->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ440P ||
                avctx->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ444P)
                avctx->color_range = AVCOL_RANGE_JPEG;
        }
        if (avctx->codec->supported_samplerates) {
            for (i = 0; avctx->codec->supported_samplerates[i] != 0; i++)
                if (avctx->sample_rate == avctx->codec->supported_samplerates[i])
                    break;
            if (avctx->codec->supported_samplerates[i] == 0) {
                av_log(avctx, AV_LOG_ERROR, "Specified sample rate %d is not supported\n",
                       avctx->sample_rate);
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
        }
        if (avctx->sample_rate < 0) {
            av_log(avctx, AV_LOG_ERROR, "Specified sample rate %d is not supported\n",
                    avctx->sample_rate);
            ret = AVERROR(EINVAL);
            goto free_and_end;
        }
        
        /*if (avctx->codec->channel_layouts) {
            if (!avctx->channel_layout) {
                av_log(avctx, AV_LOG_WARNING, "Channel layout not specified\n");
            } else {
                for (i = 0; avctx->codec->channel_layouts[i] != 0; i++)
                    if (avctx->channel_layout == avctx->codec->channel_layouts[i])
                        break;
                if (avctx->codec->channel_layouts[i] == 0) {
                    char buf[512];
                    av_get_channel_layout_string(buf, sizeof(buf), -1, avctx->channel_layout);
                    av_log(avctx, AV_LOG_ERROR, "Specified channel layout '%s' is not supported\n", buf);
                    ret = AVERROR(EINVAL);
                    goto free_and_end;
                }
            }
        }*/
        
        /*if (avctx->channel_layout && avctx->channels) {
            int channels = av_get_channel_layout_nb_channels(avctx->channel_layout);
            if (channels != avctx->channels) {
                char buf[512];
                av_get_channel_layout_string(buf, sizeof(buf), -1, avctx->channel_layout);
                av_log(avctx, AV_LOG_ERROR,
                       "Channel layout '%s' with %d channels does not match number of specified channels %d\n",
                       buf, channels, avctx->channels);
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
        } else if (avctx->channel_layout) {
            avctx->channels = av_get_channel_layout_nb_channels(avctx->channel_layout);
        }*/
        
        if (avctx->channels < 0) {
            av_log(avctx, AV_LOG_ERROR, "Specified number of channels %d is not supported\n",
                    avctx->channels);
            ret = AVERROR(EINVAL);
            goto free_and_end;
        }
        
        if(avctx->codec_type == AVMEDIA_TYPE_VIDEO) {
            pixdesc = av_pix_fmt_desc_get(avctx->pix_fmt);
            if (    avctx->bits_per_raw_sample < 0
                || (avctx->bits_per_raw_sample > 8 && pixdesc->comp[0].depth <= 8)) {
                av_log(avctx, AV_LOG_WARNING, "Specified bit depth %d not possible with the specified pixel formats depth %d\n",
                    avctx->bits_per_raw_sample, pixdesc->comp[0].depth);
                avctx->bits_per_raw_sample = pixdesc->comp[0].depth;
            }
            if (avctx->width <= 0 || avctx->height <= 0) {
                av_log(avctx, AV_LOG_ERROR, "dimensions not set\n");
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
        }
        if (   (avctx->codec_type == AVMEDIA_TYPE_VIDEO || avctx->codec_type == AVMEDIA_TYPE_AUDIO)
            && avctx->bit_rate>0 && avctx->bit_rate<1000) {
            av_log(avctx, AV_LOG_WARNING, "Bitrate %"PRId64" is extremely low, maybe you mean %"PRId64"k\n", avctx->bit_rate, avctx->bit_rate);
        }

        if (!avctx->rc_initial_buffer_occupancy)
            avctx->rc_initial_buffer_occupancy = avctx->rc_buffer_size * 3LL / 4;

        if (avctx->ticks_per_frame && avctx->time_base.num &&
            avctx->ticks_per_frame > INT_MAX / avctx->time_base.num) {
            av_log(avctx, AV_LOG_ERROR,
                   "ticks_per_frame %d too large for the timebase %d/%d.",
                   avctx->ticks_per_frame,
                   avctx->time_base.num,
                   avctx->time_base.den);
            goto free_and_end;
        }

        if (avctx->hw_frames_ctx) {
            AVHWFramesContext *frames_ctx = (AVHWFramesContext*)avctx->hw_frames_ctx->data;
            if (frames_ctx->format != avctx->pix_fmt) {
                av_log(avctx, AV_LOG_ERROR,
                       "Mismatching AVCodecContext.pix_fmt and AVHWFramesContext.format\n");
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
            if (avctx->sw_pix_fmt != AV_PIX_FMT_NONE &&
                avctx->sw_pix_fmt != frames_ctx->sw_format) {
                av_log(avctx, AV_LOG_ERROR,
                       "Mismatching AVCodecContext.sw_pix_fmt (%s) "
                       "and AVHWFramesContext.sw_format (%s)\n",
                       av_get_pix_fmt_name(avctx->sw_pix_fmt),
                       av_get_pix_fmt_name(frames_ctx->sw_format));
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
            avctx->sw_pix_fmt = frames_ctx->sw_format;
        }
    }

    avctx->pts_correction_num_faulty_pts =
    avctx->pts_correction_num_faulty_dts = 0;
    avctx->pts_correction_last_pts =
    avctx->pts_correction_last_dts = INT64_MIN;

    /*if (!CONFIG_GRAY && avctx->flags & AV_CODEC_FLAG_GRAY  && avctx->codec_descriptor->type == AVMEDIA_TYPE_VIDEO)
        av_log(avctx, AV_LOG_WARNING, "gray decoding requested but not enabled at configuration time\n");
    */

    if (avctx->codec->init && (!(avctx->active_thread_type&FF_THREAD_FRAME) || avctx->internal->frame_thread_encoder)) {
        ret = avctx->codec->init(avctx);
        if (ret < 0) {
            goto free_and_end;
        }
    }

    ret=0;

    if (av_codec_is_decoder(avctx->codec)) {
        if (!avctx->bit_rate)
            avctx->bit_rate = get_bit_rate(avctx);
        /* validate channel layout from the decoder */
        /*if (avctx->channel_layout) {
            int channels = av_get_channel_layout_nb_channels(avctx->channel_layout);
            if (!avctx->channels)
                avctx->channels = channels;
            else if (channels != avctx->channels) {
                char buf[512];
                av_get_channel_layout_string(buf, sizeof(buf), -1, avctx->channel_layout);
                av_log(avctx, AV_LOG_WARNING,
                       "Channel layout '%s' with %d channels does not match specified number of channels %d: "
                       "ignoring specified channel layout\n",
                       buf, channels, avctx->channels);
                avctx->channel_layout = 0;
            }
        }*/
        
        if (avctx->channels && avctx->channels < 0 ||
            avctx->channels > FF_SANE_NB_CHANNELS) {
            ret = AVERROR(EINVAL);
            goto free_and_end;
        }

#if 0
        if (avctx->sub_charenc) {
            if (avctx->codec_type != AVMEDIA_TYPE_SUBTITLE) {
                av_log(avctx, AV_LOG_ERROR, "Character encoding is only "
                       "supported with subtitles codecs\n");
                ret = AVERROR(EINVAL);
                goto free_and_end;
            } else if (avctx->codec_descriptor->props & AV_CODEC_PROP_BITMAP_SUB) {
                av_log(avctx, AV_LOG_WARNING, "Codec '%s' is bitmap-based, "
                       "subtitles character encoding will be ignored\n",
                       avctx->codec_descriptor->name);
                avctx->sub_charenc_mode = FF_SUB_CHARENC_MODE_DO_NOTHING;
            } else {
                /* input character encoding is set for a text based subtitle
                 * codec at this point */
                if (avctx->sub_charenc_mode == FF_SUB_CHARENC_MODE_AUTOMATIC)
                    avctx->sub_charenc_mode = FF_SUB_CHARENC_MODE_PRE_DECODER;

                if (avctx->sub_charenc_mode == FF_SUB_CHARENC_MODE_PRE_DECODER) {
#if CONFIG_ICONV
                    iconv_t cd = iconv_open("UTF-8", avctx->sub_charenc);
                    if (cd == (iconv_t)-1) {
                        ret = AVERROR(errno);
                        av_log(avctx, AV_LOG_ERROR, "Unable to open iconv context "
                               "with input character encoding \"%s\"\n", avctx->sub_charenc);
                        goto free_and_end;
                    }
                    iconv_close(cd);
#else
                    av_log(avctx, AV_LOG_ERROR, "Character encoding subtitles "
                           "conversion needs a libavcodec built with iconv support "
                           "for this codec\n");
                    ret = AVERROR(ENOSYS);
                    goto free_and_end;
#endif
                }
            }
        }
#endif

#if 0//FF_API_AVCTX_TIMEBASE
        if (avctx->framerate.num > 0 && avctx->framerate.den > 0)
            avctx->time_base = av_inv_q(av_mul_q(avctx->framerate, (AVRational){avctx->ticks_per_frame, 1}));
#endif
    }
    
    if (codec->priv_data_size > 0 && avctx->priv_data && codec->priv_class) {
        av_assert0(*(const AVClass **)avctx->priv_data == codec->priv_class);
    }

end:
    ff_unlock_avcodec(codec);
    if (options) {
        av_dict_free(options);
        *options = tmp;
    }

    return ret;
free_and_end:
    if (avctx->codec && (avctx->codec->caps_internal & FF_CODEC_CAP_INIT_CLEANUP))
        avctx->codec->close(avctx);

    if (codec->priv_class && codec->priv_data_size)
        av_opt_free(avctx->priv_data);
    
    av_opt_free(avctx);

#if 0//FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
    av_frame_free(&avctx->coded_frame);
FF_ENABLE_DEPRECATION_WARNINGS
#endif

    av_dict_free(&tmp);
    av_freep(&avctx->priv_data);
    if (avctx->internal) {
        av_frame_free(&avctx->internal->to_free);
        av_frame_free(&avctx->internal->compat_decode_frame);
        av_frame_free(&avctx->internal->buffer_frame);
        av_packet_free(&avctx->internal->buffer_pkt);
        av_packet_free(&avctx->internal->last_pkt_props);

        av_packet_free(&avctx->internal->ds.in_pkt);
        //ff_decode_bsfs_uninit(avctx);

        av_freep(&avctx->internal->pool);
    }
    av_freep(&avctx->internal);
    avctx->codec = NULL;
    goto end;
}


int avcodec_parameters_to_context(AVCodecContext *codec, const AVCodecParameters *par)
{
    codec->codec_type = par->codec_type;
    codec->codec_id   = par->codec_id;
    codec->codec_tag  = par->codec_tag;

    codec->bit_rate              = par->bit_rate;
    codec->bits_per_coded_sample = par->bits_per_coded_sample;
    codec->bits_per_raw_sample   = par->bits_per_raw_sample;
    codec->profile               = par->profile;
    codec->level                 = par->level;

    switch (par->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        codec->pix_fmt                = par->format;
        codec->width                  = par->width;
        codec->height                 = par->height;
        codec->field_order            = par->field_order;
        codec->color_range            = par->color_range;
        codec->color_primaries        = par->color_primaries;
        codec->color_trc              = par->color_trc;
        codec->colorspace             = par->color_space;
        codec->chroma_sample_location = par->chroma_location;
        codec->sample_aspect_ratio    = par->sample_aspect_ratio;
        codec->has_b_frames           = par->video_delay;
        break;
    case AVMEDIA_TYPE_AUDIO:
        codec->sample_fmt       = par->format;
        codec->channel_layout   = par->channel_layout;
        codec->channels         = par->channels;
        codec->sample_rate      = par->sample_rate;
        codec->block_align      = par->block_align;
        codec->frame_size       = par->frame_size;
        codec->delay            =
        codec->initial_padding  = par->initial_padding;
        codec->trailing_padding = par->trailing_padding;
        codec->seek_preroll     = par->seek_preroll;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        codec->width  = par->width;
        codec->height = par->height;
        break;
    }

    if (par->extradata) {
        av_freep(&codec->extradata);
        codec->extradata = av_mallocz(par->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        if (!codec->extradata)
            return AVERROR(ENOMEM);
        
        memcpy(codec->extradata, par->extradata, par->extradata_size);
        codec->extradata_size = par->extradata_size;
    }

    return 0;
}

AVCodec *avcodec_find_encoder(enum AVCodecID id)
{
    return find_codec(id, av_codec_is_encoder);
}

static void codec_parameters_reset(AVCodecParameters *par)
{
    av_freep(&par->extradata);

    memset(par, 0, sizeof(*par));

    par->codec_type          = AVMEDIA_TYPE_UNKNOWN;
    par->codec_id            = AV_CODEC_ID_NONE;
    par->format              = -1;
    par->field_order         = AV_FIELD_UNKNOWN;
    par->color_range         = AVCOL_RANGE_UNSPECIFIED;
    par->color_primaries     = AVCOL_PRI_UNSPECIFIED;
    par->color_trc           = AVCOL_TRC_UNSPECIFIED;
    par->color_space         = AVCOL_SPC_UNSPECIFIED;
    par->chroma_location     = AVCHROMA_LOC_UNSPECIFIED;
    par->sample_aspect_ratio = (AVRational){ 0, 1 };
    par->profile             = FF_PROFILE_UNKNOWN;
    par->level               = FF_LEVEL_UNKNOWN;
}

void avcodec_parameters_free(AVCodecParameters **ppar)
{
    AVCodecParameters *par = *ppar;

    if (!par)
        return;
    codec_parameters_reset(par);

    av_freep(ppar);
}

int avpriv_codec_get_cap_skip_frame_fill_param(const AVCodec *codec){
    return !!(codec->caps_internal & FF_CODEC_CAP_SKIP_FRAME_FILL_PARAM);
}

