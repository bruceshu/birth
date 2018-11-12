/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-11-12
 * Description:
 
*********************************/



#ifndef FILE_H
#define FILE_H

#include "log_system/log.h"

typedef struct FileContext {
    const AVClass *av_class;
    int fd;
    int trunc;
    int block_size;
    int follow;

}FileContext;















#endif
