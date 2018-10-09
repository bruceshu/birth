/*
copyright bruceshu

author:bruceshu
date:2018-07-05
description:

*/


#ifndef OPT_H
#define OPT_H

#include "common.h"
#include "rational.h"

#include <stdlib.h>

enum AVOptionType{
    AV_OPT_TYPE_FLAGS,
    AV_OPT_TYPE_INT,
    AV_OPT_TYPE_INT64,
    AV_OPT_TYPE_DOUBLE,
    AV_OPT_TYPE_FLOAT,
    AV_OPT_TYPE_STRING,
    AV_OPT_TYPE_RATIONAL,
    AV_OPT_TYPE_BINARY,
    AV_OPT_TYPE_DICT,
    AV_OPT_TYPE_UINT64,
    AV_OPT_TYPE_CONST = 128,
    AV_OPT_TYPE_IMAGE_SIZE = MKBETAG('S','I','Z','E'),
    AV_OPT_TYPE_PIXEL_FMT  = MKBETAG('P','F','M','T'),
    AV_OPT_TYPE_SAMPLE_FMT = MKBETAG('S','F','M','T'),
    AV_OPT_TYPE_VIDEO_RATE = MKBETAG('V','R','A','T'),
    AV_OPT_TYPE_DURATION   = MKBETAG('D','U','R',' '),
    AV_OPT_TYPE_COLOR      = MKBETAG('C','O','L','R'),
    AV_OPT_TYPE_CHANNEL_LAYOUT = MKBETAG('C','H','L','A'),
    AV_OPT_TYPE_BOOL           = MKBETAG('B','O','O','L'),
};

typedef struct AVOption {
    const char *name;
    int offset;
    enum AVOptionType type;

    union {
        int64_t i64;
        double dbl;
        const char *str;
        AVRational q;
    } default_val;
    double min;
    double max;

    int flags;
#define AV_OPT_FLAG_ENCODING_PARAM  1
#define AV_OPT_FLAG_DECODING_PARAM  2
#define AV_OPT_FLAG_METADATA        4
#define AV_OPT_FLAG_AUDIO_PARAM     8
#define AV_OPT_FLAG_VIDEO_PARAM     16
#define AV_OPT_FLAG_SUBTITLE_PARAM  32
#define AV_OPT_FLAG_EXPORT          64
#define AV_OPT_FLAG_READONLY        128
#define AV_OPT_FLAG_FILTERING_PARAM (1<<16)
    const char *unit;
}AVOption;

typedef struct AVOptionRange {
    const char *str;
    double value_min, value_max;
    double component_min, component_max;
    int is_range;
} AVOptionRange;

typedef struct AVOptionRanges {
    AVOptionRange **range;
    int nb_ranges;
    int nb_components;
} AVOptionRanges;



#endif
