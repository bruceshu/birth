/*
copyright bruceshu

author: bruceshu
date: 2018-07-05
description:

*/


#ifndef AVIO_H
#define AVIO_H

typedef struct AVIOInterruptCB {
    int (*callback)(void*);
    void *opaque;
}AVIOInterruptCB;


#define AVIO_FLAG_READ  1
#define AVIO_FLAG_WRITE 2
#define AVIO_FLAG_READ_WRITE (AVIO_FLAG_READ|AVIO_FLAG_WRITE)

#define AVIO_FLAG_NONBLOCK 8
#define AVIO_FLAG_DIRECT 0x8000




#endif
