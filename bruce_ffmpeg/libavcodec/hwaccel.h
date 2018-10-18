/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-18
 * Description:
 
*********************************/


#ifndef HWACCEL_H
#define HWACCEL_H

typedef struct AVHWAccel {
    const char *name;
    enum AVMediaType type;
    enum AVCodecID id;
    enum AVPixelFormat pix_fmt;
    int capabilities;
    
    int (*alloc_frame)(AVCodecContext *avctx, AVFrame *frame);
    int (*start_frame)(AVCodecContext *avctx, const uint8_t *buf, uint32_t buf_size);
    int (*decode_params)(AVCodecContext *avctx, int type, const uint8_t *buf, uint32_t buf_size);
    int (*decode_slice)(AVCodecContext *avctx, const uint8_t *buf, uint32_t buf_size);
    int (*end_frame)(AVCodecContext *avctx);
    int frame_priv_data_size;
    //void (*decode_mb)(struct MpegEncContext *s);
    int (*init)(AVCodecContext *avctx);
    int (*uninit)(AVCodecContext *avctx);
    
    int priv_data_size;
    int caps_internal;
    int (*frame_params)(AVCodecContext *avctx, AVBufferRef *hw_frames_ctx);
} AVHWAccel;

typedef struct AVCodecHWConfigInternal {
    /**
     * This is the structure which will be returned to the user by
     * avcodec_get_hw_config().
     */
    AVCodecHWConfig public;
    /**
     * If this configuration uses a hwaccel, a pointer to it.
     * If not, NULL.
     */
    const AVHWAccel *hwaccel;
} AVCodecHWConfigInternal;


#endif
