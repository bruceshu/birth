/*
copyright bruceshu

author:bruceshu
date:2018-07-05
description:

*/


#ifndef RATIONAL_H
#define RATIONAL_H

typedef struct AVRational{
    int num;
    int den;
}AVRational;

static inline double av_q2d(AVRational a){
    return a.num / (double) a.den;
}


#endif
