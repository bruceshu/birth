/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-014
 * Description:
 
*********************************/



#ifndef OPT_H
#define OPT_H

#include "common.h"
#include "rational.h"

#define AV_OPT_SEARCH_CHILDREN   (1 << 0)
#define AV_OPT_SEARCH_FAKE_OBJ   (1 << 1)
#define AV_OPT_ALLOW_NULL (1 << 2)
#define AV_OPT_MULTI_COMPONENT_RANGE (1 << 12)

enum OptionType{
    OPT_TYPE_FLAGS,
    OPT_TYPE_INT,
    OPT_TYPE_INT64,
    OPT_TYPE_DOUBLE,
    OPT_TYPE_FLOAT,
    OPT_TYPE_STRING,
    OPT_TYPE_RATIONAL,
    OPT_TYPE_BINARY,
    OPT_TYPE_DICT,
    OPT_TYPE_UINT64,
    OPT_TYPE_CONST = 128,
    OPT_TYPE_IMAGE_SIZE = MKBETAG('S','I','Z','E'),
    OPT_TYPE_PIXEL_FMT  = MKBETAG('P','F','M','T'),
    OPT_TYPE_SAMPLE_FMT = MKBETAG('S','F','M','T'),
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
#define OPT_FLAG_ENCODING_PARAM  1
#define OPT_FLAG_DECODING_PARAM  2
#define OPT_FLAG_METADATA        4
#define OPT_FLAG_AUDIO_PARAM     8
#define OPT_FLAG_VIDEO_PARAM     16
#define OPT_FLAG_SUBTITLE_PARAM  32
#define OPT_FLAG_EXPORT          64
#define OPT_FLAG_READONLY        128
#define OPT_FLAG_FILTERING_PARAM (1<<16)
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

int opt_set(void *obj, const char *name, const char *val, int search_flags);
const AVOption *opt_next(const void *obj, const AVOption *last);
void opt_set_defaults(void *s);
void opt_set_defaults2(void *s, int mask, int flags);
void opt_free(void *obj);

#endif
