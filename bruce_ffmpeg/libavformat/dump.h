/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-11-06
 * Description:
 
*********************************/


#ifndef DUMP_H
#define DUMP_H

#include "avformat.h"

void av_dump_format(AVFormatContext *ic, int index, const char *url, int is_output);


#endif
