/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


typedef union {
    uint64_t u64[2];
    uint32_t u32[4];
    uint8_t u8x4[4][4];
    uint8_t u8[16];
} av_aes_block;


typedef struct AVAES {
    // Note: round_key[16] is accessed in the init code, but this only
    // overwrites state, which does not matter (see also commit ba554c0).
    DECLARE_ALIGNED(16, av_aes_block, round_key)[15];
    DECLARE_ALIGNED(16, av_aes_block, state)[2];
    int rounds;
    void (*crypt)(struct AVAES *a, uint8_t *dst, const uint8_t *src, int count, uint8_t *iv, int rounds);
} AVAES;

