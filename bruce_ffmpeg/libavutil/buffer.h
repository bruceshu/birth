

#ifndef BUFFER_H
#define BUFFER_H

#define AV_BUFFER_FLAG_READONLY (1 << 0)
#define BUFFER_FLAG_READONLY      (1 << 0)

struct AVBuffer {
    uint8_t *data; /**< data described by this buffer */
    int      size; /**< size of data in bytes */
    atomic_uint refcount;

    void (*free)(void *opaque, uint8_t *data);
    
    void *opaque;
    int flags;
};

typedef struct AVBufferRef {
    AVBuffer *buffer;

    uint8_t *data;
    int      size;
} AVBufferRef;


#endif