/*
 bruce 2020-7-28
 */

#ifndef TSK_COMMON_H
#define TSK_COMMON_H

#include <stddef.h>

#define tsk_true	1
#define tsk_false	0

typedef size_t tsk_size_t;
typedef int tsk_bool_t;

typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#ifdef NULL
#define tsk_null    NULL /**< Null pointer */
#else
#define tsk_null    0  /**< Null pointer */
#endif


#endif