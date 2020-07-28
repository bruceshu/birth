/*
 bruce 2020-7-28
 */

#ifndef TSK_OBJECT_H
#define TSK_OBJECT_H

#include <stdarg.h>
#include "tsk_common.h"

typedef void tsk_object_t;

typedef struct tsk_object_def_s
{
    tsk_size_t size;
    tsk_object_t *(*constructor)(tsk_object_t *, va_list *);
    tsk_object_t *(*destructor)(tsk_object_t *);
    int (*comparator)(const tsk_object_t *, const tsk_object_t *);
} tsk_object_def_t;

#define TSK_DECLARE_OBJECT   \
    const void *__def__;     \
    volatile long ref_count; \
    volatile long lock;

typedef struct tsk_object_header_s
{
    TSK_DECLARE_OBJECT;
} tsk_object_header_t;

#endif
