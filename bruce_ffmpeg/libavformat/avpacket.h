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


#endif
