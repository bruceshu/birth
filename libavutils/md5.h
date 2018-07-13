


#ifndef MD5_H
#define MD5_H

typedef struct AVMD5 {
    uint64_t len;
    uint8_t  block[64];
    uint32_t ABCD[4];
}AVMD5;


#endif
