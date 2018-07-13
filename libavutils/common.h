/*
copyright bruceshu

author:bruceshu
date:2018-07-05
description:

*/


#ifndef COMMON_H
#define COMMON_H



#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))

#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))



#endif
