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


#endif
