/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-16
 * Description:
 
*********************************/


#ifndef TIME_H
#define TIME_H

#include <stdint.h>

int64_t av_gettime(void);
int64_t av_gettime_relative(void);
int av_usleep(unsigned usec);


#endif
