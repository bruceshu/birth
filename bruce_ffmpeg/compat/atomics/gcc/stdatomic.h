/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-18
 * Description:
 
*********************************/


#ifndef COMPAT_ATOMICS_GCC_STDATOMIC_H
#define COMPAT_ATOMICS_GCC_STDATOMIC_H

#include <stddef.h>
#include <stdint.h>


#define atomic_fetch_add(object, operand) \
    __sync_fetch_and_add(object, operand)

#define atomic_fetch_add_explicit(object, operand, order) \
    atomic_fetch_add(object, operand)

#define atomic_init(obj, value) \
do {                            \
    *(obj) = (value);           \
} while(0)


#endif /* COMPAT_ATOMICS_GCC_STDATOMIC_H */
