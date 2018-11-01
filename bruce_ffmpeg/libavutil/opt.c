/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


#include "opt.h"

static int write_number(void *obj, const AVOption *o, void *dst, double num, int den, int64_t intnum)
{
    if (o->type != AV_OPT_TYPE_FLAGS &&
        (!den || o->max * den < num * intnum || o->min * den > num * intnum)) {
        num = den ? num * intnum / den : (num * intnum ? INFINITY : NAN);
        //av_log(obj, AV_LOG_ERROR, "Value %f for parameter '%s' out of range [%g - %g]\n", num, o->name, o->min, o->max);
        return -1;//AVERROR(ERANGE);
    }
    if (o->type == AV_OPT_TYPE_FLAGS) {
        double d = num*intnum/den;
        if (d < -1.5 || d > 0xFFFFFFFF+0.5 || (llrint(d*256) & 255)) {
            /*av_log(obj, AV_LOG_ERROR, "Value %f for parameter '%s' is not a valid set of 32bit integer flags\n",
                   num*intnum/den, o->name);*/
            return -1;//AVERROR(ERANGE);
        }
    }

    switch (o->type) {
    case AV_OPT_TYPE_PIXEL_FMT:
        //*(enum AVPixelFormat *)dst = llrint(num / den) * intnum;
        break;
    case AV_OPT_TYPE_SAMPLE_FMT:
        //*(enum AVSampleFormat *)dst = llrint(num / den) * intnum;
        break;
    case AV_OPT_TYPE_BOOL:
    case AV_OPT_TYPE_FLAGS:
    case AV_OPT_TYPE_INT:
        *(int *)dst = llrint(num / den) * intnum;
        break;
    case AV_OPT_TYPE_DURATION:
    case AV_OPT_TYPE_CHANNEL_LAYOUT:
    case AV_OPT_TYPE_INT64:
        double d = num / den;
        if (intnum == 1 && d == (double)INT64_MAX) {
            *(int64_t *)dst = INT64_MAX;
        } else
            *(int64_t *)dst = llrint(d) * intnum;
        break;
    case AV_OPT_TYPE_UINT64:
        double d = num / den;
        // We must special case uint64_t here as llrint() does not support values
        // outside the int64_t range and there is no portable function which does
        // "INT64_MAX + 1ULL" is used as it is representable exactly as IEEE double
        // while INT64_MAX is not
        if (intnum == 1 && d == (double)UINT64_MAX) {
            *(uint64_t *)dst = UINT64_MAX;
        } else if (d > INT64_MAX + 1ULL) {
            *(uint64_t *)dst = (llrint(d - (INT64_MAX + 1ULL)) + (INT64_MAX + 1ULL))*intnum;
        } else {
            *(uint64_t *)dst = llrint(d) * intnum;
        }
        break;
    case AV_OPT_TYPE_FLOAT:
        *(float *)dst = num * intnum / den;
        break;
    case AV_OPT_TYPE_DOUBLE:
        *(double *)dst = num * intnum / den;
        break;
    case AV_OPT_TYPE_RATIONAL:
    case AV_OPT_TYPE_VIDEO_RATE:
        if ((int) num == num)
            *(AVRational *)dst = (AVRational) { num *intnum, den };
        else
            //*(AVRational *)dst = av_d2q(num * intnum / den, 1 << 24);
        break;
    default:
        return -1;//AVERROR(EINVAL);
    }
    
    return 0;
}

static int set_string_color(void *obj, const AVOption *o, const char *val, uint8_t *dst)
{
    int ret = 0;

    if (!val) {
        return 0;
    } else {
        //ret = av_parse_color(dst, val, -1, obj);
        if (ret < 0)
            //av_log(obj, AV_LOG_ERROR, "Unable to parse option value \"%s\" as color\n", val);
        return ret;
    }
    return 0;
}

static int set_string(void *obj, const AVOption *o, const char *val, uint8_t **dst)
{
    av_freep(dst);
    *dst = av_strdup(val);
    return *dst ? 0 : -1;//AVERROR(ENOMEM);
}

static int set_string_image_size(void *obj, const AVOption *o, const char *val, int *dst)
{
    int ret = 0;

    if (!val || !strcmp(val, "none")) {
        dst[0] =
        dst[1] = 0;
        return 0;
    }

    /*
    ret = av_parse_video_size(dst, dst + 1, val);
    if (ret < 0)
        av_log(obj, AV_LOG_ERROR, "Unable to parse option value \"%s\" as image size\n", val);
    */
    
    return ret;
}

static int set_string_video_rate(void *obj, const AVOption *o, const char *val, AVRational *dst)
{
    int ret = 0;
    if (!val) {
        ret = -1;//AVERROR(EINVAL);
    } else {
        //ret = av_parse_video_rate(dst, val);
    }
    
    if (ret < 0)
        //av_log(obj, AV_LOG_ERROR, "Unable to parse option value \"%s\" as video rate\n", val);

    return ret;
}

static int hexchar2int(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

static int set_string_binary(void *obj, const AVOption *o, const char *val, uint8_t **dst)
{
    int *lendst = (int *)(dst + 1);
    uint8_t *bin, *ptr;
    int len;

    av_freep(dst);
    *lendst = 0;

    if (!val || !(len = strlen(val)))
        return 0;

    if (len & 1)
        return -1;//AVERROR(EINVAL);
    len /= 2;

    ptr = bin = av_malloc(len);
    if (!ptr)
        return -1;//AVERROR(ENOMEM);
        
    while (*val) {
        int a = hexchar2int(*val++);
        int b = hexchar2int(*val++);
        if (a < 0 || b < 0) {
            av_free(bin);
            return -1;//AVERROR(EINVAL);
        }
        *ptr++ = (a << 4) | b;
    }
    *dst    = bin;
    *lendst = len;

    return 0;
}

void av_opt_set_defaults2(void *s, int mask, int flags)
{
    const AVOption *opt = NULL;
    while ((opt = av_opt_next(s, opt))) {
        void *dst = ((uint8_t*)s) + opt->offset;

        if ((opt->flags & mask) != flags)
            continue;

        if (opt->flags & AV_OPT_FLAG_READONLY)
            continue;

        switch (opt->type) {
            case AV_OPT_TYPE_CONST:
                /* Nothing to be done here */
                break;
            case AV_OPT_TYPE_BOOL:
            case AV_OPT_TYPE_FLAGS:
            case AV_OPT_TYPE_INT:
            case AV_OPT_TYPE_INT64:
            case AV_OPT_TYPE_UINT64:
            case AV_OPT_TYPE_DURATION:
            case AV_OPT_TYPE_CHANNEL_LAYOUT:
            case AV_OPT_TYPE_PIXEL_FMT:
            case AV_OPT_TYPE_SAMPLE_FMT:
                write_number(s, opt, dst, 1, 1, opt->default_val.i64);
                break;
            case AV_OPT_TYPE_DOUBLE:
            case AV_OPT_TYPE_FLOAT: {
                double val;
                val = opt->default_val.dbl;
                write_number(s, opt, dst, val, 1, 1);
            }
            break;
            case AV_OPT_TYPE_RATIONAL: {
                AVRational val;
                //val = av_d2q(opt->default_val.dbl, INT_MAX);
                write_number(s, opt, dst, 1, val.den, val.num);
            }
            break;
            case AV_OPT_TYPE_COLOR:
                set_string_color(s, opt, opt->default_val.str, dst);
                break;
            case AV_OPT_TYPE_STRING:
                set_string(s, opt, opt->default_val.str, dst);
                break;
            case AV_OPT_TYPE_IMAGE_SIZE:
                set_string_image_size(s, opt, opt->default_val.str, dst);
                break;
            case AV_OPT_TYPE_VIDEO_RATE:
                set_string_video_rate(s, opt, opt->default_val.str, dst);
                break;
            case AV_OPT_TYPE_BINARY:
                set_string_binary(s, opt, opt->default_val.str, dst);
                break;
            case AV_OPT_TYPE_DICT:
                /* Cannot set defaults for these types */
            break;
        default:
            //av_log(s, AV_LOG_DEBUG, "AVOption type %d of option %s not implemented yet\n", opt->type, opt->name);
        }
    }
}

void av_opt_set_defaults(void *s)
{
    av_opt_set_defaults2(s, 0, 0);
}

int av_opt_set_dict2(void *obj, AVDictionary **options, int search_flags)
{
    AVDictionaryEntry *t = NULL;
    AVDictionary    *tmp = NULL;
    int ret = 0;

    if (!options)
        return 0;

    while ((t = av_dict_get(*options, "", t, AV_DICT_IGNORE_SUFFIX))) {
        ret = av_opt_set(obj, t->key, t->value, search_flags);
        if (ret == AVERROR_OPTION_NOT_FOUND)
            ret = av_dict_set(&tmp, t->key, t->value, 0);
        if (ret < 0) {
            av_log(obj, AV_LOG_ERROR, "Error setting option %s to value %s.\n", t->key, t->value);
            av_dict_free(&tmp);
            return ret;
        }
        ret = 0;
    }
    
    av_dict_free(options);
    *options = tmp;
    
    return ret;
}

int av_opt_set_dict(void *obj, AVDictionary **options)
{
    return av_opt_set_dict2(obj, options, 0);
}

void *av_opt_child_next(void *obj, void *prev)
{
    const AVClass *c = *(AVClass **)obj;
    if (c->child_next)
        return c->child_next(obj, prev);
    return NULL;
}

const AVClass *av_opt_child_class_next(const AVClass *parent, const AVClass *prev)
{
    if (parent->child_class_next)
        return parent->child_class_next(prev);
    return NULL;
}

const AVOption *av_opt_find2(void *obj, const char *name, const char *unit, int opt_flags, int search_flags, void **target_obj)
{
    const AVClass  *c;
    const AVOption *o = NULL;

    if(!obj)
        return NULL;

    c= *(AVClass**)obj;

    if (!c)
        return NULL;

    if (search_flags & AV_OPT_SEARCH_CHILDREN) {
        if (search_flags & AV_OPT_SEARCH_FAKE_OBJ) {
            const AVClass *child = NULL;
            while (child = av_opt_child_class_next(c, child))
                if (o = av_opt_find2(&child, name, unit, opt_flags, search_flags, NULL))
                    return o;
        } else {
            void *child = NULL;
            while (child = av_opt_child_next(obj, child))
                if (o = av_opt_find2(child, name, unit, opt_flags, search_flags, target_obj))
                    return o;
        }
    }

    while (o = av_opt_next(obj, o)) {
        if (!strcmp(o->name, name) && (o->flags & opt_flags) == opt_flags &&
            ((!unit && o->type != AV_OPT_TYPE_CONST) ||
             (unit  && o->type == AV_OPT_TYPE_CONST && o->unit && !strcmp(o->unit, unit)))) {
            if (target_obj) {
                if (!(search_flags & AV_OPT_SEARCH_FAKE_OBJ))
                    *target_obj = obj;
                else
                    *target_obj = NULL;
            }
            return o;
        }
    }
    return NULL;
}

void *av_opt_ptr(const AVClass *pstClass, void *obj, const char *name)
{
    const AVOption *opt= av_opt_find2(&pstClass, name, NULL, 0, AV_OPT_SEARCH_FAKE_OBJ, NULL);
    if(!opt)
        return NULL;
    return (uint8_t*)obj + opt->offset;
}

int av_opt_copy(void *dst, const void *src)
{
    const AVOption *o = NULL;
    const AVClass *c;
    int ret = 0;

    if (!src)
        return AVERROR(EINVAL);

    c = *(AVClass **)src;
    if (!c || c != *(AVClass **)dst)
        return AVERROR(EINVAL);

    while ((o = av_opt_next(src, o))) {
        void *field_dst = (uint8_t *)dst + o->offset;
        void *field_src = (uint8_t *)src + o->offset;
        uint8_t **field_dst8 = (uint8_t **)field_dst;
        uint8_t **field_src8 = (uint8_t **)field_src;

        if (o->type == AV_OPT_TYPE_STRING) {
            if (*field_dst8 != *field_src8)
                av_freep(field_dst8);
            *field_dst8 = av_strdup(*field_src8);
            if (*field_src8 && !*field_dst8)
                ret = AVERROR(ENOMEM);
        } else if (o->type == AV_OPT_TYPE_BINARY) {
            int len = *(int *)(field_src8 + 1);
            if (*field_dst8 != *field_src8)
                av_freep(field_dst8);
            *field_dst8 = av_memdup(*field_src8, len);
            if (len && !*field_dst8) {
                ret = AVERROR(ENOMEM);
                len = 0;
            }
            *(int *)(field_dst8 + 1) = len;
        } else if (o->type == AV_OPT_TYPE_CONST) {
            // do nothing
        } else if (o->type == AV_OPT_TYPE_DICT) {
            AVDictionary **sdict = (AVDictionary **) field_src;
            AVDictionary **ddict = (AVDictionary **) field_dst;
            if (*sdict != *ddict)
                av_dict_free(ddict);
            *ddict = NULL;
            av_dict_copy(ddict, *sdict, 0);
            if (av_dict_count(*sdict) != av_dict_count(*ddict))
                ret = AVERROR(ENOMEM);
        } else {
            int size = opt_size(o->type);
            if (size < 0)
                ret = size;
            else
                memcpy(field_dst, field_src, size);
        }
    }
    return ret;
}

static void format_duration(char *buf, size_t size, int64_t d)
{
    char *e;

    av_assert0(size >= 25);
    if (d < 0 && d != INT64_MIN) {
        *(buf++) = '-';
        size--;
        d = -d;
    }
    if (d == INT64_MAX)
        snprintf(buf, size, "INT64_MAX");
    else if (d == INT64_MIN)
        snprintf(buf, size, "INT64_MIN");
    else if (d > (int64_t)3600*1000000)
        snprintf(buf, size, "%"PRId64":%02d:%02d.%06d", d / 3600000000,
                 (int)((d / 60000000) % 60),
                 (int)((d / 1000000) % 60),
                 (int)(d % 1000000));
    else if (d > 60*1000000)
        snprintf(buf, size, "%d:%02d.%06d",
                 (int)(d / 60000000),
                 (int)((d / 1000000) % 60),
                 (int)(d % 1000000));
    else
        snprintf(buf, size, "%d.%06d",
                 (int)(d / 1000000),
                 (int)(d % 1000000));
    e = buf + strlen(buf);
    while (e > buf && e[-1] == '0')
        *(--e) = 0;
    if (e > buf && e[-1] == '.')
        *(--e) = 0;
}

int av_opt_get(void *obj, const char *name, int search_flags, uint8_t **out_val)
{
    void *dst, *target_obj;
    const AVOption *o = av_opt_find2(obj, name, NULL, 0, search_flags, &target_obj);
    uint8_t *bin, buf[128];
    int len, i, ret;
    int64_t i64;

    if (!o || !target_obj || (o->offset<=0 && o->type != AV_OPT_TYPE_CONST))
        return AVERROR_OPTION_NOT_FOUND;

    /*if (o->flags & AV_OPT_FLAG_DEPRECATED) {
        av_log(obj, AV_LOG_WARNING, "The \"%s\" option is deprecated: %s\n", name, o->help);
    }*/
    
    
    dst = (uint8_t *)target_obj + o->offset;

    buf[0] = 0;
    switch (o->type) {
    case AV_OPT_TYPE_BOOL:
        //ret = snprintf(buf, sizeof(buf), "%s", (char *)av_x_if_null(get_bool_name(*(int *)dst), "invalid"));
        break;
    case AV_OPT_TYPE_FLAGS:
        ret = snprintf(buf, sizeof(buf), "0x%08X", *(int *)dst);
        break;
    case AV_OPT_TYPE_INT:
        ret = snprintf(buf, sizeof(buf), "%d", *(int *)dst);
        break;
    case AV_OPT_TYPE_INT64:
        ret = snprintf(buf, sizeof(buf), "%"PRId64, *(int64_t *)dst);
        break;
    case AV_OPT_TYPE_UINT64:
        ret = snprintf(buf, sizeof(buf), "%"PRIu64, *(uint64_t *)dst);
        break;
    case AV_OPT_TYPE_FLOAT:
        ret = snprintf(buf, sizeof(buf), "%f", *(float *)dst);
        break;
    case AV_OPT_TYPE_DOUBLE:
        ret = snprintf(buf, sizeof(buf), "%f", *(double *)dst);
        break;
    case AV_OPT_TYPE_VIDEO_RATE:
    case AV_OPT_TYPE_RATIONAL:
        ret = snprintf(buf, sizeof(buf), "%d/%d", ((AVRational *)dst)->num, ((AVRational *)dst)->den);
        break;
    case AV_OPT_TYPE_CONST:
        ret = snprintf(buf, sizeof(buf), "%f", o->default_val.dbl);
        break;
    case AV_OPT_TYPE_STRING:
        if (*(uint8_t **)dst) {
            *out_val = av_strdup(*(uint8_t **)dst);
        } else if (search_flags & AV_OPT_ALLOW_NULL) {
            *out_val = NULL;
            return 0;
        } else {
            *out_val = av_strdup("");
        }
        return *out_val ? 0 : AVERROR(ENOMEM);
    case AV_OPT_TYPE_BINARY:
        if (!*(uint8_t **)dst && (search_flags & AV_OPT_ALLOW_NULL)) {
            *out_val = NULL;
            return 0;
        }
        len = *(int *)(((uint8_t *)dst) + sizeof(uint8_t *));
        if ((uint64_t)len * 2 + 1 > INT_MAX)
            return AVERROR(EINVAL);
        if (!(*out_val = av_malloc(len * 2 + 1)))
            return AVERROR(ENOMEM);
        if (!len) {
            *out_val[0] = '\0';
            return 0;
        }
        bin = *(uint8_t **)dst;
        for (i = 0; i < len; i++)
            snprintf(*out_val + i * 2, 3, "%02X", bin[i]);
        return 0;
    case AV_OPT_TYPE_IMAGE_SIZE:
        ret = snprintf(buf, sizeof(buf), "%dx%d", ((int *)dst)[0], ((int *)dst)[1]);
        break;
    case AV_OPT_TYPE_PIXEL_FMT:
        //ret = snprintf(buf, sizeof(buf), "%s", (char *)av_x_if_null(av_get_pix_fmt_name(*(enum AVPixelFormat *)dst), "none"));
        break;
    case AV_OPT_TYPE_SAMPLE_FMT:
        //ret = snprintf(buf, sizeof(buf), "%s", (char *)av_x_if_null(av_get_sample_fmt_name(*(enum AVSampleFormat *)dst), "none"));
        break;
    case AV_OPT_TYPE_DURATION:
        i64 = *(int64_t *)dst;
        format_duration(buf, sizeof(buf), i64);
        ret = strlen(buf); // no overflow possible, checked by an assert
        break;
    case AV_OPT_TYPE_COLOR:
        ret = snprintf(buf, sizeof(buf), "0x%02x%02x%02x%02x",
                       (int)((uint8_t *)dst)[0], (int)((uint8_t *)dst)[1],
                       (int)((uint8_t *)dst)[2], (int)((uint8_t *)dst)[3]);
        break;
    case AV_OPT_TYPE_CHANNEL_LAYOUT:
        i64 = *(int64_t *)dst;
        ret = snprintf(buf, sizeof(buf), "0x%"PRIx64, i64);
        break;
    default:
        return AVERROR(EINVAL);
    }

    if (ret >= sizeof(buf))
        return AVERROR(EINVAL);
    *out_val = av_strdup(buf);
    return *out_val ? 0 : AVERROR(ENOMEM);
}

int av_opt_set(void *obj, const char *name, const char *val, int search_flags)
{
    int ret = 0;
    void *dst, *target_obj;
    const AVOption *o = av_opt_find2(obj, name, NULL, 0, search_flags, &target_obj);
    if (!o || !target_obj)
        return AVERROR_OPTION_NOT_FOUND;
    if (!val && (o->type != AV_OPT_TYPE_STRING &&
                 o->type != AV_OPT_TYPE_PIXEL_FMT && o->type != AV_OPT_TYPE_SAMPLE_FMT &&
                 o->type != AV_OPT_TYPE_IMAGE_SIZE && o->type != AV_OPT_TYPE_VIDEO_RATE &&
                 o->type != AV_OPT_TYPE_DURATION && o->type != AV_OPT_TYPE_COLOR &&
                 o->type != AV_OPT_TYPE_CHANNEL_LAYOUT && o->type != AV_OPT_TYPE_BOOL))
        return AVERROR(EINVAL);

    if (o->flags & AV_OPT_FLAG_READONLY)
        return AVERROR(EINVAL);

    if (o->flags & AV_OPT_FLAG_DEPRECATED)
        av_log(obj, AV_LOG_WARNING, "The \"%s\" option is deprecated: %s\n", name, o->help);

    dst = ((uint8_t *)target_obj) + o->offset;
    switch (o->type) {
    case AV_OPT_TYPE_BOOL:
        return set_string_bool(obj, o, val, dst);
    case AV_OPT_TYPE_STRING:
        return set_string(obj, o, val, dst);
    case AV_OPT_TYPE_BINARY:
        return set_string_binary(obj, o, val, dst);
    case AV_OPT_TYPE_FLAGS:
    case AV_OPT_TYPE_INT:
    case AV_OPT_TYPE_INT64:
    case AV_OPT_TYPE_UINT64:
    case AV_OPT_TYPE_FLOAT:
    case AV_OPT_TYPE_DOUBLE:
    case AV_OPT_TYPE_RATIONAL:
        return set_string_number(obj, target_obj, o, val, dst);
    case AV_OPT_TYPE_IMAGE_SIZE:
        return set_string_image_size(obj, o, val, dst);
    case AV_OPT_TYPE_VIDEO_RATE: {
        AVRational tmp;
        ret = set_string_video_rate(obj, o, val, &tmp);
        if (ret < 0)
            return ret;
        return write_number(obj, o, dst, 1, tmp.den, tmp.num);
    }
    case AV_OPT_TYPE_PIXEL_FMT:
        return set_string_pixel_fmt(obj, o, val, dst);
    case AV_OPT_TYPE_SAMPLE_FMT:
        return set_string_sample_fmt(obj, o, val, dst);
    case AV_OPT_TYPE_DURATION:
        if (!val) {
            *(int64_t *)dst = 0;
            return 0;
        } else {
            if ((ret = av_parse_time(dst, val, 1)) < 0)
                av_log(obj, AV_LOG_ERROR, "Unable to parse option value \"%s\" as duration\n", val);
            return ret;
        }
        break;
    case AV_OPT_TYPE_COLOR:
        return set_string_color(obj, o, val, dst);
    case AV_OPT_TYPE_CHANNEL_LAYOUT:
        if (!val || !strcmp(val, "none")) {
            *(int64_t *)dst = 0;
        } else {
            int64_t cl = av_get_channel_layout(val);
            if (!cl) {
                av_log(obj, AV_LOG_ERROR, "Unable to parse option value \"%s\" as channel layout\n", val);
                ret = AVERROR(EINVAL);
            }
            *(int64_t *)dst = cl;
            return ret;
        }
        break;
    }

    av_log(obj, AV_LOG_ERROR, "Invalid option type.\n");
    return AVERROR(EINVAL);
}

const AVOption *av_opt_next(const void *obj, const AVOption *last)
{
    const AVClass *class_para;
    if (!obj)
        return NULL;
    
    class_para = *(const AVClass**)obj;
    if (!last && class_para && class_para->option && class_para->option[0].name)
        return class_para->option;
    
    if (last && last[1].name)
        return ++last;
    
    return NULL;
}

void av_opt_free(void *obj)
{
    const AVOption *o = NULL;
    while ((o = av_opt_next(obj, o))) {
        switch (o->type) {
        case AV_OPT_TYPE_STRING:
        case AV_OPT_TYPE_BINARY:
            av_freep((uint8_t *)obj + o->offset);
            break;

        case AV_OPT_TYPE_DICT:
            av_dict_free((AVDictionary **)(((uint8_t *)obj) + o->offset));
            break;

        default:
            break;
        }
    }
}

