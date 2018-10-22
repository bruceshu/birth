/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-22
 * Description:
 
*********************************/


#ifndef AVDEVICE_H
#define AVDEVICE_H

#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "libavutil/dict.h"
#include "libavformat/avformat.h"

typedef struct AVDeviceInfo {
    char *device_name;                   /**< device name, format depends on device */
    char *device_description;            /**< human friendly name */
} AVDeviceInfo;

typedef struct AVDeviceInfoList {
    AVDeviceInfo **devices;              /**< list of autodetected devices */
    int nb_devices;                      /**< number of autodetected devices */
    int default_device;                  /**< index of default device or -1 if no default */
} AVDeviceInfoList;

typedef struct AVDeviceCapabilitiesQuery {
    const AVClass *av_class;
    AVFormatContext *device_context;
    enum AVCodecID codec;
    enum AVSampleFormat sample_format;
    enum AVPixelFormat pixel_format;
    int sample_rate;
    int channels;
    int64_t channel_layout;
    int window_width;
    int window_height;
    int frame_width;
    int frame_height;
    AVRational fps;
} AVDeviceCapabilitiesQuery;


#endif
