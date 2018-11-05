/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-12
 * Description:
 
*********************************/


#ifndef MEM_H
#define MEM_H

#include <stddef.h>

#include "error.h"

#if 0 //后续研究
#if defined(__INTEL_COMPILER) && __INTEL_COMPILER < 1110 || defined(__SUNPRO_C)
    #define DECLARE_ALIGNED(n,t,v)      t __attribute__ ((aligned (n))) v
    #define DECLARE_ASM_CONST(n,t,v)    const t __attribute__ ((aligned (n))) v
#elif defined(__TI_COMPILER_VERSION__)
    #define DECLARE_ALIGNED(n,t,v)                      \
        AV_PRAGMA(DATA_ALIGN(v,n))                      \
        t __attribute__((aligned(n))) v
    #define DECLARE_ASM_CONST(n,t,v)                    \
        AV_PRAGMA(DATA_ALIGN(v,n))                      \
        static const t __attribute__((aligned(n))) v
#elif defined(__DJGPP__)
    #define DECLARE_ALIGNED(n,t,v)      t __attribute__ ((aligned (FFMIN(n, 16)))) v
    #define DECLARE_ASM_CONST(n,t,v)    static const t av_used __attribute__ ((aligned (FFMIN(n, 16)))) v
#elif defined(__GNUC__) || defined(__clang__)
    #define DECLARE_ALIGNED(n,t,v)      t __attribute__ ((aligned (n))) v
    #define DECLARE_ASM_CONST(n,t,v)    static const t av_used __attribute__ ((aligned (n))) v
#elif defined(_MSC_VER)
    #define DECLARE_ALIGNED(n,t,v)      __declspec(align(n)) t v
    #define DECLARE_ASM_CONST(n,t,v)    __declspec(align(n)) static const t v
#else
    #define DECLARE_ALIGNED(n,t,v)      t v
    #define DECLARE_ASM_CONST(n,t,v)    static const t v
#endif
#endif

static inline int av_size_mult(size_t a, size_t b, size_t *r)
{
    size_t t = a * b;
    /* Hack inspired from glibc: don't try the division if nelem and elsize are both less than sqrt(SIZE_MAX). */
    if ((a | b) >= ((size_t)1 << (sizeof(size_t) * 4)) && a && t / a != b)
        return AVERROR(EINVAL);
    *r = t;
    return 0;
}


void max_alloc_set(size_t max);
void *av_malloc(size_t size);
void *av_mallocz(size_t size);
void *av_mallocz_array(int n_elem, size_t size);
void *av_realloc(void *ptr, size_t size);
int av_reallocp(void *ptr, size_t size);
void *av_realloc_f(void *ptr, size_t nelem, size_t elsize);

char *av_strdup(const char *s);
char *av_strndup(const char *s, size_t len);
void av_free(void *ptr);
void av_freep(void *arg);


#endif
