/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-09
 * Description:
 
*********************************/


#ifndef AVSTRING_H
#define AVSTRING_H

#include <stddef.h>

static inline int av_isdigit(int c)
{
    return c >= '0' && c <= '9';
}

static inline int av_isgraph(int c)
{
    return c > 32 && c < 127;
}

static inline int av_isspace(int c)
{
    return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

static inline int av_toupper(int c)
{
    if (c >= 'a' && c <= 'z')
        c ^= 0x20;
    return c;
}

static inline int av_tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
        c ^= 0x20;
    return c;
}

static inline int av_isxdigit(int c)
{
    c = av_tolower(c);
    return av_isdigit(c) || (c >= 'a' && c <= 'f');
}

int av_strstart(const char *str, const char *pfx, const char **ptr);
int av_stristart(const char *str, const char *pfx, const char **ptr);
char *av_stristr(const char *s1, const char *s2);

size_t av_strlcat(char *dst, size_t size, const char *fmt, ...);
size_t av_strlcatf(char *dst, size_t size, const char *fmt, ...);
size_t av_strlcpy(char *dst, const char *src, size_t size);

int av_strcasecmp(const char *a, const char *b);
int av_strncasecmp(const char *a, const char *b, size_t n);

char *av_strtok(char *s, const char *delim, char **saveptr);


#endif
