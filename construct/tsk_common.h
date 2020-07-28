/*
 bruce 2020-7-28
 */

#ifndef TSK_COMMON_H
#define TSK_COMMON_H

#include <stddef.h>

typedef size_t tsk_size_t;

#ifdef NULL
#define tsk_null    NULL /**< Null pointer */
#else
#define tsk_null    0  /**< Null pointer */
#endif


#endif