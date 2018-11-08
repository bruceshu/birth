/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-11-08
 * Description:
 
*********************************/


#include <stdint.h>

#include "version.h"

#define AV_PIX_FMT_FLAG_BE           (1 << 0)
#define AV_PIX_FMT_FLAG_PAL          (1 << 1)
#define AV_PIX_FMT_FLAG_BITSTREAM    (1 << 2)
#define AV_PIX_FMT_FLAG_HWACCEL      (1 << 3)
#define AV_PIX_FMT_FLAG_PLANAR       (1 << 4)
#define AV_PIX_FMT_FLAG_RGB          (1 << 5)
#define AV_PIX_FMT_FLAG_PSEUDOPAL    (1 << 6)
#define AV_PIX_FMT_FLAG_ALPHA        (1 << 7)
#define AV_PIX_FMT_FLAG_BAYER        (1 << 8)
#define AV_PIX_FMT_FLAG_FLOAT        (1 << 9)

typedef struct AVComponentDescriptor {
    int plane;
    int step;
    int offset;
    int shift;
    int depth;
#if FF_API_PLUS1_MINUS1
    int step_minus1;
    int depth_minus1;
    int offset_plus1;
#endif
} AVComponentDescriptor;

typedef struct AVPixFmtDescriptor {
    const char *name;
    uint8_t nb_components;  ///< The number of components each pixel has, (1-4)
    uint8_t log2_chroma_w;
    uint8_t log2_chroma_h;
    uint64_t flags;
    AVComponentDescriptor comp[4];
    const char *alias;
} AVPixFmtDescriptor;

