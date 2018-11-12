/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


#ifndef MATHEMATICS_H
#define MATHEMATICS_H

#include <stdint.h>

#include "rational.h"

enum AVRounding {
    AV_ROUND_ZERO     = 0, ///< Round toward zero.
    AV_ROUND_INF      = 1, ///< Round away from zero.
    AV_ROUND_DOWN     = 2, ///< Round toward -infinity.
    AV_ROUND_UP       = 3, ///< Round toward +infinity.
    AV_ROUND_NEAR_INF = 5, ///< Round to nearest and halfway cases away from zero.
    
    AV_ROUND_PASS_MINMAX = 8192,
};

int64_t av_rescale(int64_t a, int64_t b, int64_t c);
int64_t av_rescale_q(int64_t a, AVRational bq, AVRational cq);
int64_t av_rescale_q_rnd(int64_t a, AVRational bq, AVRational cq, enum AVRounding rnd);
int64_t av_rescale_rnd(int64_t a, int64_t b, int64_t c, enum AVRounding rnd);


#endif
