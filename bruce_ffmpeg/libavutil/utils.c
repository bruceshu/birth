/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


#include <string.h>
#include <stdlib.h>

#include "avstring.h"

void av_url_split(char *proto, int proto_size, char *authorization, int authorization_size,
                 char *hostname, int hostname_size, int *port_ptr, char *path, int path_size, const char *url)
{
    const char *p, *ls, *ls2, *at, *at2, *col, *brk;

    if (port_ptr)
        *port_ptr = -1;
    if (proto_size > 0)
        proto[0] = 0;
    if (authorization_size > 0)
        authorization[0] = 0;
    if (hostname_size > 0)
        hostname[0] = 0;
    if (path_size > 0)
        path[0] = 0;

    /* parse protocol */
    if ((p = strchr(url, ':'))) {
        av_strlcpy(proto, url, FFMIN(proto_size, p + 1 - url));
        p++; /* skip ':' */
        if (*p == '/')
            p++;
        if (*p == '/')
            p++;
    } else {
        /* no protocol means plain filename */
        av_strlcpy(path, url, path_size);
        return;
    }

    /* separate path from hostname */
    ls = strchr(p, '/');
    ls2 = strchr(p, '?');
    if (!ls)
        ls = ls2;
    else if (ls && ls2)
        ls = FFMIN(ls, ls2);
    if (ls)
        av_strlcpy(path, ls, path_size);
    else
        ls = &p[strlen(p)];  // XXX

    /* the rest is hostname, use that to parse auth/port */
    if (ls != p) {
        /* authorization (user[:pass]@hostname) */
        at2 = p;
        while ((at = strchr(p, '@')) && at < ls) {
            av_strlcpy(authorization, at2, FFMIN(authorization_size, at + 1 - at2));
            p = at + 1; /* skip '@' */
        }

        if (*p == '[' && (brk = strchr(p, ']')) && brk < ls) {
            /* [host]:port */
            av_strlcpy(hostname, p + 1, FFMIN(hostname_size, brk - p));
            if (brk[1] == ':' && port_ptr)
                *port_ptr = atoi(brk + 2);
        } else if ((col = strchr(p, ':')) && col < ls) {
            av_strlcpy(hostname, p, FFMIN(col + 1 - p, hostname_size));
            if (port_ptr)
                *port_ptr = atoi(col + 1);
        } else {
            av_strlcpy(hostname, p, FFMIN(ls + 1 - p, hostname_size));
        }
    }
}

#if 0 //后续需要时再放开
 AVStream *avformat_new_stream(AVFormatContext *s, const AVCodec *c)
 {
     AVStream *st;
     int i;
     AVStream **streams;
 
     if (s->nb_streams >= FFMIN(s->max_streams, INT_MAX/sizeof(*streams))) {
         if (s->max_streams < INT_MAX/sizeof(*streams))
             av_log(s, AV_LOG_ERROR, "Number of streams exceeds max_streams parameter (%d), see the documentation if you wish to increase it\n", s->max_streams);
         return NULL;
     }
     streams = av_realloc_array(s->streams, s->nb_streams + 1, sizeof(*streams));
     if (!streams)
         return NULL;
     s->streams = streams;
 
     st = av_mallocz(sizeof(AVStream));
     if (!st)
         return NULL;
     if (!(st->info = av_mallocz(sizeof(*st->info)))) {
         av_free(st);
         return NULL;
     }
     st->info->last_dts = AV_NOPTS_VALUE;
 
#if FF_API_LAVF_AVCTX
 FF_DISABLE_DEPRECATION_WARNINGS
     st->codec = avcodec_alloc_context3(c);
     if (!st->codec) {
         av_free(st->info);
         av_free(st);
         return NULL;
     }
 FF_ENABLE_DEPRECATION_WARNINGS
#endif
 
     st->internal = av_mallocz(sizeof(*st->internal));
     if (!st->internal)
         goto fail;
 
     st->codecpar = avcodec_parameters_alloc();
     if (!st->codecpar)
         goto fail;
 
     st->internal->avctx = avcodec_alloc_context3(NULL);
     if (!st->internal->avctx)
         goto fail;
 
     if (s->iformat) {
#if FF_API_LAVF_AVCTX
 FF_DISABLE_DEPRECATION_WARNINGS
         /* no default bitrate if decoding */
         st->codec->bit_rate = 0;
 FF_ENABLE_DEPRECATION_WARNINGS
#endif
 
         /* default pts setting is MPEG-like */
         avpriv_set_pts_info(st, 33, 1, 90000);
         /* we set the current DTS to 0 so that formats without any timestamps
          * but durations get some timestamps, formats with some unknown
          * timestamps have their first few packets buffered and the
          * timestamps corrected before they are returned to the user */
         st->cur_dts = RELATIVE_TS_BASE;
     } else {
         st->cur_dts = AV_NOPTS_VALUE;
     }
 
     st->index      = s->nb_streams;
     st->start_time = AV_NOPTS_VALUE;
     st->duration   = AV_NOPTS_VALUE;
     st->first_dts     = AV_NOPTS_VALUE;
     st->probe_packets = MAX_PROBE_PACKETS;
     st->pts_wrap_reference = AV_NOPTS_VALUE;
     st->pts_wrap_behavior = AV_PTS_WRAP_IGNORE;
 
     st->last_IP_pts = AV_NOPTS_VALUE;
     st->last_dts_for_order_check = AV_NOPTS_VALUE;
     for (i = 0; i < MAX_REORDER_DELAY + 1; i++)
         st->pts_buffer[i] = AV_NOPTS_VALUE;
 
     st->sample_aspect_ratio = (AVRational) { 0, 1 };
 
#if FF_API_R_FRAME_RATE
     st->info->last_dts      = AV_NOPTS_VALUE;
#endif
     st->info->fps_first_dts = AV_NOPTS_VALUE;
     st->info->fps_last_dts  = AV_NOPTS_VALUE;
 
     st->inject_global_side_data = s->internal->inject_global_side_data;
 
     st->internal->need_context_update = 1;
 
     s->streams[s->nb_streams++] = st;
     return st;
 fail:
     free_stream(&st);
     return NULL;
 }

 void avpriv_set_pts_info(AVStream *s, int pts_wrap_bits, unsigned int pts_num, unsigned int pts_den)
 {
     AVRational new_tb;
     if (av_reduce(&new_tb.num, &new_tb.den, pts_num, pts_den, INT_MAX)) {
         if (new_tb.num != pts_num)
             av_log(NULL, AV_LOG_DEBUG,
                    "st:%d removing common factor %d from timebase\n",
                    s->index, pts_num / new_tb.num);
     } else
         av_log(NULL, AV_LOG_WARNING, "st:%d has too large timebase, reducing\n", s->index);
 
     if (new_tb.num <= 0 || new_tb.den <= 0) {
         av_log(NULL, AV_LOG_ERROR, "Ignoring attempt to set invalid timebase %d/%d for st:%d\n",
                new_tb.num, new_tb.den, s->index);
         return;
     }
     s->time_base     = new_tb;
#if FF_API_LAVF_AVCTX
 FF_DISABLE_DEPRECATION_WARNINGS
     av_codec_set_pkt_timebase(s->codec, new_tb);
 FF_ENABLE_DEPRECATION_WARNINGS
#endif
     av_codec_set_pkt_timebase(s->internal->avctx, new_tb);
     s->pts_wrap_bits = pts_wrap_bits;
 }

 int av_parse_time(int64_t *timeval, const char *timestr, int duration)
 {
     const char *p, *q;
     int64_t t, now64;
     time_t now;
     struct tm dt = { 0 }, tmbuf;
     int today = 0, negative = 0, microseconds = 0;
     int i;
     static const char * const date_fmt[] = {
         "%Y - %m - %d",
         "%Y%m%d",
     };
     static const char * const time_fmt[] = {
         "%H:%M:%S",
         "%H%M%S",
     };
     static const char * const tz_fmt[] = {
         "%H:%M",
         "%H%M",
         "%H",
     };
 
     p = timestr;
     q = NULL;
     *timeval = INT64_MIN;
     if (!duration) {
         now64 = av_gettime();
         now = now64 / 1000000;
 
         if (!av_strcasecmp(timestr, "now")) {
             *timeval = now64;
             return 0;
         }
 
         /* parse the year-month-day part */
         for (i = 0; i < FF_ARRAY_ELEMS(date_fmt); i++) {
             q = av_small_strptime(p, date_fmt[i], &dt);
             if (q)
                 break;
         }
 
         /* if the year-month-day part is missing, then take the
          * current year-month-day time */
         if (!q) {
             today = 1;
             q = p;
         }
         p = q;
 
         if (*p == 'T' || *p == 't')
             p++;
         else
             while (av_isspace(*p))
                 p++;
 
         /* parse the hour-minute-second part */
         for (i = 0; i < FF_ARRAY_ELEMS(time_fmt); i++) {
             q = av_small_strptime(p, time_fmt[i], &dt);
             if (q)
                 break;
         }
     } else {
         /* parse timestr as a duration */
         if (p[0] == '-') {
             negative = 1;
             ++p;
         }
         /* parse timestr as HH:MM:SS */
         q = av_small_strptime(p, "%J:%M:%S", &dt);
         if (!q) {
             /* parse timestr as MM:SS */
             q = av_small_strptime(p, "%M:%S", &dt);
             dt.tm_hour = 0;
         }
         if (!q) {
             char *o;
             /* parse timestr as S+ */
             dt.tm_sec = strtol(p, &o, 10);
             if (o == p) /* the parsing didn't succeed */
                 return AVERROR(EINVAL);
             dt.tm_min = 0;
             dt.tm_hour = 0;
             q = o;
         }
     }
 
     /* Now we have all the fields that we can get */
     if (!q)
         return AVERROR(EINVAL);
 
     /* parse the .m... part */
     if (*q == '.') {
         int n;
         q++;
         for (n = 100000; n >= 1; n /= 10, q++) {
             if (!av_isdigit(*q))
                 break;
             microseconds += n * (*q - '0');
         }
         while (av_isdigit(*q))
             q++;
     }
 
     if (duration) {
         t = dt.tm_hour * 3600 + dt.tm_min * 60 + dt.tm_sec;
     } else {
         int is_utc = *q == 'Z' || *q == 'z';
         int tzoffset = 0;
         q += is_utc;
         if (!today && !is_utc && (*q == '+' || *q == '-')) {
             struct tm tz = { 0 };
             int sign = (*q == '+' ? -1 : 1);
             q++;
             p = q;
             for (i = 0; i < FF_ARRAY_ELEMS(tz_fmt); i++) {
                 q = av_small_strptime(p, tz_fmt[i], &tz);
                 if (q)
                     break;
             }
             if (!q)
                 return AVERROR(EINVAL);
             tzoffset = sign * (tz.tm_hour * 60 + tz.tm_min) * 60;
             is_utc = 1;
         }
         if (today) { /* fill in today's date */
             struct tm dt2 = is_utc ? *gmtime_r(&now, &tmbuf) : *localtime_r(&now, &tmbuf);
             dt2.tm_hour = dt.tm_hour;
             dt2.tm_min  = dt.tm_min;
             dt2.tm_sec  = dt.tm_sec;
             dt = dt2;
         }
         dt.tm_isdst = is_utc ? 0 : -1;
         t = is_utc ? av_timegm(&dt) : mktime(&dt);
         t += tzoffset;
     }
 
     /* Check that we are at the end of the string */
     if (*q)
         return AVERROR(EINVAL);
 
     t *= 1000000;
     t += microseconds;
     *timeval = negative ? -t : t;
     return 0;
 }
#endif

 int av_find_info_tag(char *arg, int arg_size, const char *tag1, const char *info)
 {
     const char *p;
     char tag[128], *q;
 
     p = info;
     if (*p == '?')
         p++;
     for(;;) {
         q = tag;
         while (*p != '\0' && *p != '=' && *p != '&') {
             if ((q - tag) < sizeof(tag) - 1)
                 *q++ = *p;
             p++;
         }
         *q = '\0';
         q = arg;
         if (*p == '=') {
             p++;
             while (*p != '&' && *p != '\0') {
                 if ((q - arg) < arg_size - 1) {
                     if (*p == '+')
                         *q++ = ' ';
                     else
                         *q++ = *p;
                 }
                 p++;
             }
         }
         *q = '\0';
         if (!strcmp(tag, tag1))
             return 1;
         if (*p != '&')
             break;
         p++;
     }
     return 0;
 }

