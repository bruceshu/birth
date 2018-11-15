/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-18
 * Description:
 
*********************************/


#include <string.h>

#include "libavutil/error.h"
#include "libavutil/mem.h"
#include "libavutil/assert.h"

#include "utils.h"
#include "avcodec.h"
#include "avcodec_table.h"

extern AVCodec ff_h264_decoder;

#if CONFIG_OSSFUZZ
AVCodec * codec_list[] = {
    NULL,
    NULL
};
#else
#include "libavcodec/codec_list.c"
#endif

static enum AVCodecID remap_deprecated_codec_id(enum AVCodecID id)
{
    switch(id){
        //This is for future deprecatec codec ids, its empty since
        //last major bump but will fill up again over time, please don't remove it
        default: 
            return id;
    }
}

static AVOnce av_codec_static_init = AV_ONCE_INIT;
static void av_codec_init_static(void)
{
    for (int i = 0; codec_list[i]; i++) {
        if (codec_list[i]->init_static_data)
            codec_list[i]->init_static_data((AVCodec*)codec_list[i]);
    }
}

const AVCodec *av_codec_iterate(void **opaque)
{
    uintptr_t i = (uintptr_t)*opaque;
    const AVCodec *c = codec_list[i];

    ff_thread_once(&av_codec_static_init, av_codec_init_static);

    if (c)
        *opaque = (void*)(i + 1);

    return c;
}

static AVCodec *find_codec(enum AVCodecID id, int (*x)(const AVCodec *))
{
    const AVCodec *p, *experimental = NULL;
    void *i = 0;

    id = remap_deprecated_codec_id(id);

    while ((p = av_codec_iterate(&i))) {
        if (!x(p))
            continue;
        if (p->id == id) {
            if (p->capabilities & AV_CODEC_CAP_EXPERIMENTAL && !experimental) {
                experimental = p;
            } else
                return (AVCodec*)p;
        }
    }

    return (AVCodec*)experimental;
}

AVCodec *avcodec_find_decoder(enum AVCodecID id)
{
    return find_codec(id, av_codec_is_decoder);
}

static const char* context_to_name(void* ptr) {
    AVCodecContext *avc= ptr;

    if(avc && avc->codec && avc->codec->name)
        return avc->codec->name;
    else
        return "NULL";
}

static void *codec_child_next(void *obj, void *prev)
{
    AVCodecContext *s = obj;
    if (!prev && s->codec && s->codec->priv_class && s->priv_data)
        return s->priv_data;
    return NULL;
}

static const AVClass *codec_child_class_next(const AVClass *prev)
{
    AVCodec *c = NULL;

    /* find the codec that corresponds to prev */
    while (prev && (c = av_codec_next(c)))
        if (c->priv_class == prev)
            break;

    /* find next codec with priv options */
    while (c = av_codec_next(c))
        if (c->priv_class)
            return c->priv_class;
        
    return NULL;
}

static AVClassCategory get_category(void *ptr)
{
    AVCodecContext* avctx = ptr;
    
    if(avctx->codec && avctx->codec->decode) {
        return AV_CLASS_CATEGORY_DECODER;
    }
    else {
        return AV_CLASS_CATEGORY_ENCODER;
    }
}

static const AVClass av_codec_context_class = {
    .class_name              = "AVCodecContext",
    .item_name               = context_to_name,
    .option                  = avcodec_options,
    .version                 = LIBAVUTIL_VERSION_INT,
    .log_level_offset_offset = offsetof(AVCodecContext, log_level_offset),
    .child_next              = codec_child_next,
    .child_class_next        = codec_child_class_next,
    .category                = AV_CLASS_CATEGORY_ENCODER,
    .get_category            = get_category,
};

static int init_context_defaults(AVCodecContext *s, const AVCodec *codec)
{
    int flags=0;
    memset(s, 0, sizeof(AVCodecContext));

    s->av_class = &av_codec_context_class;

    s->codec_type = codec ? codec->type : AVMEDIA_TYPE_UNKNOWN;
    if (codec) {
        s->codec = codec;
        s->codec_id = codec->id;
    }

    if(s->codec_type == AVMEDIA_TYPE_AUDIO)
        flags= AV_OPT_FLAG_AUDIO_PARAM;
    else if(s->codec_type == AVMEDIA_TYPE_VIDEO)
        flags= AV_OPT_FLAG_VIDEO_PARAM;
    else if(s->codec_type == AVMEDIA_TYPE_SUBTITLE)
        flags= AV_OPT_FLAG_SUBTITLE_PARAM;
    
    av_opt_set_defaults2(s, flags, flags);

    s->time_base           = (AVRational){0,1};
    s->framerate           = (AVRational){ 0, 1 };
    s->pkt_timebase        = (AVRational){ 0, 1 };
    s->get_buffer2         = avcodec_default_get_buffer2;
    //s->get_format          = avcodec_default_get_format;
    //s->execute             = avcodec_default_execute;
    //s->execute2            = avcodec_default_execute2;
    s->sample_aspect_ratio = (AVRational){0,1};
    s->pix_fmt             = AV_PIX_FMT_NONE;
    s->sw_pix_fmt          = AV_PIX_FMT_NONE;
    s->sample_fmt          = AV_SAMPLE_FMT_NONE;

    s->reordered_opaque    = AV_NOPTS_VALUE;
    if(codec && codec->priv_data_size){
        if(!s->priv_data){
            s->priv_data= av_mallocz(codec->priv_data_size);
            if (!s->priv_data) {
                return AVERROR(ENOMEM);
            }
        }
        if(codec->priv_class){
            *(const AVClass**)s->priv_data = codec->priv_class;
            av_opt_set_defaults(s->priv_data);
        }
    }
    if (codec && codec->defaults) {
        int ret;
        const AVCodecDefault *d = codec->defaults;
        while (d->key) {
            ret = av_opt_set(s, d->key, d->value, 0);
            av_assert0(ret >= 0);
            d++;
        }
    }
    return 0;
}

#if 0
static int update_frame_pool(AVCodecContext *avctx, AVFrame *frame)
{
    FramePool *pool = avctx->internal->pool;
    int i, ret;

    switch (avctx->codec_type) {
    case AVMEDIA_TYPE_VIDEO: {
        uint8_t *data[4];
        int linesize[4];
        int size[4] = { 0 };
        int w = frame->width;
        int h = frame->height;
        int tmpsize, unaligned;

        if (pool->format == frame->format &&
            pool->width == frame->width && pool->height == frame->height)
            return 0;

        avcodec_align_dimensions2(avctx, &w, &h, pool->stride_align);

        do {
            // NOTE: do not align linesizes individually, this breaks e.g. assumptions
            // that linesize[0] == 2*linesize[1] in the MPEG-encoder for 4:2:2
            ret = av_image_fill_linesizes(linesize, avctx->pix_fmt, w);
            if (ret < 0)
                return ret;
            // increase alignment of w for next try (rhs gives the lowest bit set in w)
            w += w & ~(w - 1);

            unaligned = 0;
            for (i = 0; i < 4; i++)
                unaligned |= linesize[i] % pool->stride_align[i];
        } while (unaligned);

        tmpsize = av_image_fill_pointers(data, avctx->pix_fmt, h,
                                         NULL, linesize);
        if (tmpsize < 0)
            return tmpsize;

        for (i = 0; i < 3 && data[i + 1]; i++)
            size[i] = data[i + 1] - data[i];
        size[i] = tmpsize - (data[i] - data[0]);

        for (i = 0; i < 4; i++) {
            av_buffer_pool_uninit(&pool->pools[i]);
            pool->linesize[i] = linesize[i];
            if (size[i]) {
                pool->pools[i] = av_buffer_pool_init(size[i] + 16 + STRIDE_ALIGN - 1,
                                                     CONFIG_MEMORY_POISONING ?
                                                        NULL :
                                                        av_buffer_allocz);
                if (!pool->pools[i]) {
                    ret = AVERROR(ENOMEM);
                    goto fail;
                }
            }
        }
        pool->format = frame->format;
        pool->width  = frame->width;
        pool->height = frame->height;

        break;
        }
    case AVMEDIA_TYPE_AUDIO: {
        int ch     = frame->channels; //av_get_channel_layout_nb_channels(frame->channel_layout);
        int planar = av_sample_fmt_is_planar(frame->format);
        int planes = planar ? ch : 1;

        if (pool->format == frame->format && pool->planes == planes &&
            pool->channels == ch && frame->nb_samples == pool->samples)
            return 0;

        av_buffer_pool_uninit(&pool->pools[0]);
        ret = av_samples_get_buffer_size(&pool->linesize[0], ch,
                                         frame->nb_samples, frame->format, 0);
        if (ret < 0)
            goto fail;

        pool->pools[0] = av_buffer_pool_init(pool->linesize[0], NULL);
        if (!pool->pools[0]) {
            ret = AVERROR(ENOMEM);
            goto fail;
        }

        pool->format     = frame->format;
        pool->planes     = planes;
        pool->channels   = ch;
        pool->samples = frame->nb_samples;
        break;
        }
    default: av_assert0(0);
    }
    return 0;
fail:
    for (i = 0; i < 4; i++)
        av_buffer_pool_uninit(&pool->pools[i]);
    pool->format = -1;
    pool->planes = pool->channels = pool->samples = 0;
    pool->width  = pool->height = 0;
    return ret;
}
#endif

int avcodec_default_get_buffer2(AVCodecContext *avctx, AVFrame *frame, int flags)
{
    int ret;

    if (avctx->hw_frames_ctx) {
        //ret = av_hwframe_get_buffer(avctx->hw_frames_ctx, frame, 0);
        frame->width  = avctx->coded_width;
        frame->height = avctx->coded_height;
        return ret;
    }

#if 0
    if ((ret = update_frame_pool(avctx, frame)) < 0)
        return ret;

    switch (avctx->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        return video_get_buffer(avctx, frame);
    case AVMEDIA_TYPE_AUDIO:
        return audio_get_buffer(avctx, frame);
    default:
        return -1;
    }
#endif

}

AVCodecContext *avcodec_alloc_context3(const AVCodec *codec)
{
    AVCodecContext *avctx= av_malloc(sizeof(AVCodecContext));

    if (!avctx)
        return NULL;

    if (init_context_defaults(avctx, codec) < 0) {
        av_free(avctx);
        return NULL;
    }

    return avctx;
}

int avcodec_close(AVCodecContext *avctx)
{
    int i;

    if (!avctx)
        return 0;
#if 0
    if (avcodec_is_open(avctx)) {
        FramePool *pool = avctx->internal->pool;
        if (CONFIG_FRAME_THREAD_ENCODER &&
            avctx->internal->frame_thread_encoder && avctx->thread_count > 1) {
            ff_frame_thread_encoder_free(avctx);
        }
        if (HAVE_THREADS && avctx->internal->thread_ctx)
            ff_thread_free(avctx);
        if (avctx->codec && avctx->codec->close)
            avctx->codec->close(avctx);
        avctx->internal->byte_buffer_size = 0;
        av_freep(&avctx->internal->byte_buffer);
        av_frame_free(&avctx->internal->to_free);
        av_frame_free(&avctx->internal->compat_decode_frame);
        av_frame_free(&avctx->internal->buffer_frame);
        av_packet_free(&avctx->internal->buffer_pkt);
        av_packet_free(&avctx->internal->last_pkt_props);

        av_packet_free(&avctx->internal->ds.in_pkt);

        for (i = 0; i < FF_ARRAY_ELEMS(pool->pools); i++)
            av_buffer_pool_uninit(&pool->pools[i]);
        av_freep(&avctx->internal->pool);

        if (avctx->hwaccel && avctx->hwaccel->uninit)
            avctx->hwaccel->uninit(avctx);
        av_freep(&avctx->internal->hwaccel_priv_data);

        ff_decode_bsfs_uninit(avctx);

        av_freep(&avctx->internal);
    }

    for (i = 0; i < avctx->nb_coded_side_data; i++)
        av_freep(&avctx->coded_side_data[i].data);
    av_freep(&avctx->coded_side_data);
    avctx->nb_coded_side_data = 0;

    av_buffer_unref(&avctx->hw_frames_ctx);
    av_buffer_unref(&avctx->hw_device_ctx);

    if (avctx->priv_data && avctx->codec && avctx->codec->priv_class)
        av_opt_free(avctx->priv_data);
    av_opt_free(avctx);
    av_freep(&avctx->priv_data);
    if (av_codec_is_encoder(avctx->codec)) {
        av_freep(&avctx->extradata);
#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
        av_frame_free(&avctx->coded_frame);
FF_ENABLE_DEPRECATION_WARNINGS
#endif
    }
    avctx->codec = NULL;
    avctx->active_thread_type = 0;
#endif

    return 0;
}

void avcodec_free_context(AVCodecContext **pavctx)
{
    AVCodecContext *avctx = *pavctx;

    if (!avctx)
        return;

    avcodec_close(avctx);

    av_freep(&avctx->extradata);
    av_freep(&avctx->subtitle_header);
    av_freep(&avctx->intra_matrix);
    av_freep(&avctx->inter_matrix);
    //av_freep(&avctx->rc_override);

    av_freep(pavctx);
}

static AVCodec *find_codec_by_name(const char *name, int (*x)(const AVCodec *))
{
    void *i = 0;
    const AVCodec *p;

    if (!name)
        return NULL;

    while ((p = av_codec_iterate(&i))) {
        if (!x(p))
            continue;
        if (strcmp(name, p->name) == 0)
            return (AVCodec*)p;
    }

    return NULL;
}

AVCodec *avcodec_find_decoder_by_name(const char *name)
{
    return find_codec_by_name(name, av_codec_is_decoder);
}

#if FF_API_NEXT
//FF_DISABLE_DEPRECATION_WARNINGS
static AVOnce av_codec_next_init = AV_ONCE_INIT;

static void av_codec_init_next(void)
{
    AVCodec *prev = NULL, *p;
    void *i = 0;
    while ((p = (AVCodec*)av_codec_iterate(&i))) {
        if (prev)
            prev->next = p;
        prev = p;
    }
}

void avcodec_register(AVCodec *codec)
{
    ff_thread_once(&av_codec_next_init, av_codec_init_next);
}

void avcodec_register_all(void)
{
    ff_thread_once(&av_codec_next_init, av_codec_init_next);
}
//FF_ENABLE_DEPRECATION_WARNINGS
#endif

AVCodec *av_codec_next(const AVCodec *c)
{
    ff_thread_once(&av_codec_next_init, av_codec_init_next);

    if (c)
        return c->next;
    else
        return (AVCodec*)codec_list[0];
}

