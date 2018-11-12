/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-11-06
 * Description:
 
*********************************/


#ifndef AVPACKET_H
#define AVPACKET_H

#include "libavutil/internal.h"

void av_packet_unref(AVPacket *pkt);
void av_packet_free(AVPacket **pkt);
int av_packet_ref(AVPacket *dst, const AVPacket *src);
void av_init_packet(AVPacket *pkt);
int av_packet_alloc(AVBufferRef **buf, int size);
int av_packet_make_refcounted(AVPacket *pkt);



#endif
