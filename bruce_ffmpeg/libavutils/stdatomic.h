


#ifndef STDATOMIC_H
#define STDATOMIC_H


#define atomic_init(obj, value) \
do {                            \
    *(obj) = (value);           \
} while(0)


#endif