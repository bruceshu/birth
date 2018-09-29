/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


#define MAX_HASHLEN 64
#define MAX_BLOCKLEN 128

struct AVHMAC {
    void *hash;
    int blocklen, hashlen;
    void (*final)(void*, uint8_t*);
    void (*update)(void*, const uint8_t*, int len);
    void (*init)(void*);
    uint8_t key[MAX_BLOCKLEN];
    int keylen;
};

void av_hmac_init(AVHMAC *c, const uint8_t *key, unsigned int keylen)
{
    int i;
    uint8_t block[MAX_BLOCKLEN];
    
    if (keylen > c->blocklen) {
        c->init(c->hash);
        c->update(c->hash, key, keylen);
        c->final(c->hash, c->key);
        c->keylen = c->hashlen;
    } else {
        memcpy(c->key, key, keylen);
        c->keylen = keylen;
    }
    
    c->init(c->hash);
    
    for (i = 0; i < c->keylen; i++)
        block[i] = c->key[i] ^ 0x36;
    
    for (i = c->keylen; i < c->blocklen; i++)
        block[i] = 0x36;
    
    c->update(c->hash, block, c->blocklen);
}

void av_hmac_update(AVHMAC *c, const uint8_t *data, unsigned int len)
{
    c->update(c->hash, data, len);
}

int av_hmac_final(AVHMAC *c, uint8_t *out, unsigned int outlen)
{
    uint8_t block[MAX_BLOCKLEN];
    int i;
    
    if (outlen < c->hashlen)
        return AVERROR(EINVAL);
    
    c->final(c->hash, out);
    c->init(c->hash);
    
    for (i = 0; i < c->keylen; i++)
        block[i] = c->key[i] ^ 0x5C;
    
    for (i = c->keylen; i < c->blocklen; i++)
        block[i] = 0x5C;
    
    c->update(c->hash, block, c->blocklen);
    c->update(c->hash, out, c->hashlen);
    c->final(c->hash, out);
    
    return c->hashlen;
}

