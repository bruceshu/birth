/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-18
 * Description:
 
*********************************/


#ifndef HWCONTEXT_H
#define HWCONTEXT_H

enum AVHWDeviceType {
    AV_HWDEVICE_TYPE_NONE,
    AV_HWDEVICE_TYPE_VDPAU,
    AV_HWDEVICE_TYPE_CUDA,
    AV_HWDEVICE_TYPE_VAAPI,
    AV_HWDEVICE_TYPE_DXVA2,
    AV_HWDEVICE_TYPE_QSV,
    AV_HWDEVICE_TYPE_VIDEOTOOLBOX,
    AV_HWDEVICE_TYPE_D3D11VA,
    AV_HWDEVICE_TYPE_DRM,
    AV_HWDEVICE_TYPE_OPENCL,
    AV_HWDEVICE_TYPE_MEDIACODEC,
};

typedef struct AVHWDeviceContext {
    const AVClass *av_class;
    //AVHWDeviceInternal *internal;
    enum AVHWDeviceType type;
    void *hwctx;
    void (*free)(struct AVHWDeviceContext *ctx);
    void *user_opaque;
} AVHWDeviceContext;

typedef struct AVHWFramesContext {
    const AVClass *av_class;
    //AVHWFramesInternal *internal;
    AVBufferRef *device_ref;
    AVHWDeviceContext *device_ctx;
    void *hwctx;
    void (*free)(struct AVHWFramesContext *ctx);
    void *user_opaque;
    AVBufferPool *pool;
    int initial_pool_size;
    enum AVPixelFormat format;
    enum AVPixelFormat sw_format;
    int width, height;
} AVHWFramesContext;

#endif
