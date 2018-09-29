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

#define FFSWAP(type,a,b) do{type SWAP_tmp = b; b = a; a = SWAP_tmp;}while(0)

#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define FFMAX3(a,b,c) FFMAX(FFMAX(a,b),c)
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#define FFMIN3(a,b,c) FFMIN(FFMIN(a,b),c)


#endif