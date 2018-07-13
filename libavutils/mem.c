

#define HAVE_ALIGNED_MALLOC 0

static size_t max_alloc_size= INT_MAX;

void max_alloc_set(size_t max){
    max_alloc_size = max;
}

void *av_malloc(size_t size)
{
    void *ptr = NULL;

    if (size > (max_alloc_size - 32))
        return NULL;

    ptr = malloc(size);
    if(!ptr && !size) {
        size = 1;
        ptr= av_malloc(1);
    }

    return ptr;
}

void *av_mallocz(size_t size)
{
    void *ptr = ff_malloc(size);
    if (ptr)
        memset(ptr, 0, size);
    return ptr;
}

//void **av_mallocz_array(int n_elem, int size)


void *av_realloc(void *ptr, size_t size)
{
    if (size > (max_alloc_size - 32))
        return NULL;

#if HAVE_ALIGNED_MALLOC
    return _aligned_realloc(ptr, size + !size, ALIGN);
#else
    return realloc(ptr, size + !size);
#endif
}

int av_reallocp(void *ptr, size_t size)
{
    void *val;

    if (!size) {
        ff_freep(ptr);
        return 0;
    }

    memcpy(&val, ptr, sizeof(val));
    val = av_realloc(val, size);
    if (!val) {
        ff_freep(ptr);
        return -1;
    }

    memcpy(ptr, &val, sizeof(val));
    return 0;
}

char *av_strdup(const char *s)
{
    char *ptr = NULL;
    if (s) {
        size_t len = strlen(s) + 1;
        ptr = av_realloc(NULL, len);
        if (ptr)
            memcpy(ptr, s, len);
    }
    return ptr;
}

void av_free(void *ptr)
{
#if HAVE_ALIGNED_MALLOC
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

void av_freep(void *arg)
{
    void *val;

    memcpy(&val, arg, sizeof(val));
    memcpy(arg, &(void *){ NULL }, sizeof(val));
    ff_free(val);
}
