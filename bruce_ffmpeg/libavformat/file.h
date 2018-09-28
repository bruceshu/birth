/*
copyright bruceshu

author: bruceshu
date: 2018-07-05
description:

*/


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
