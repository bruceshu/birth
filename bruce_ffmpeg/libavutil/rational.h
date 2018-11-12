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

AVRational av_mul_q(AVRational b, AVRational c);
int av_reduce(int *dst_num, int *dst_den, int64_t num, int64_t den, int64_t max);


#endif
