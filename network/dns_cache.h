


#ifndef DNS_CACHE_H
#define DNS_CACHE_H

typedef struct DnsCacheEntry {
    volatile int ref_count;
    volatile int delete_flag;
    int64_t expired_time;
    
    // construct by private function, not support ai_next and ai_canonname, can only be  released using free_private_addrinfo
    struct addrinfo *res;  
} DnsCacheEntry;

typedef struct DnsCacheContext {
    AVDictionary *dns_dictionary;
    pthread_mutex_t dns_dictionary_mutex;
    int initialized;
} DnsCacheContext;


#endif


