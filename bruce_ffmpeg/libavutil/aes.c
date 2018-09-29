/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


static uint8_t     sbox[256];
static uint8_t inv_sbox[256];

#if CONFIG_SMALL
static uint32_t enc_multbl[1][256];
static uint32_t dec_multbl[1][256];
#else
static uint32_t enc_multbl[4][256];
static uint32_t dec_multbl[4][256];
#endif

static void init_multbl2(uint32_t tbl[][256], const int c[4], const uint8_t *log8, const uint8_t *alog8, const uint8_t *sbox)
{
    int i;

    for (i = 0; i < 256; i++) {
        int x = sbox[i];
        if (x) {
            int k, l, m, n;
            x = log8[x];
            k = alog8[x + log8[c[0]]];
            l = alog8[x + log8[c[1]]];
            m = alog8[x + log8[c[2]]];
            n = alog8[x + log8[c[3]]];
            tbl[0][i] = AV_NE(MKBETAG(k, l, m, n), MKTAG(k, l, m, n));
#if !CONFIG_SMALL
            tbl[1][i] = ROT(tbl[0][i], 8);
            tbl[2][i] = ROT(tbl[0][i], 16);
            tbl[3][i] = ROT(tbl[0][i], 24);
#endif
        }
    }
}

int av_aes_init(AVAES *a, const uint8_t *key, int key_bits, int decrypt)
{
    int i, j, t, rconpointer = 0;
    uint8_t tk[8][4];
    int KC = key_bits >> 5;
    int rounds = KC + 6;
    uint8_t log8[256];
    uint8_t alog8[512];

    a->crypt = decrypt ? aes_decrypt : aes_encrypt;

    if (!enc_multbl[FF_ARRAY_ELEMS(enc_multbl) - 1][FF_ARRAY_ELEMS(enc_multbl[0]) - 1]) {
        j = 1;
        for (i = 0; i < 255; i++) {
            alog8[i] = alog8[i + 255] = j;
            log8[j] = i;
            j ^= j + j;
            if (j > 255)
                j ^= 0x11B;
        }
        for (i = 0; i < 256; i++) {
            j = i ? alog8[255 - log8[i]] : 0;
            j ^= (j << 1) ^ (j << 2) ^ (j << 3) ^ (j << 4);
            j = (j ^ (j >> 8) ^ 99) & 255;
            inv_sbox[j] = i;
            sbox[i]     = j;
        }
        init_multbl2(dec_multbl, (const int[4]) { 0xe, 0x9, 0xd, 0xb }, log8, alog8, inv_sbox);
        init_multbl2(enc_multbl, (const int[4]) { 0x2, 0x1, 0x1, 0x3 }, log8, alog8, sbox);
    }

    if (key_bits != 128 && key_bits != 192 && key_bits != 256)
        return AVERROR(EINVAL);

    a->rounds = rounds;

    memcpy(tk, key, KC * 4);
    memcpy(a->round_key[0].u8, key, KC * 4);

    for (t = KC * 4; t < (rounds + 1) * 16; t += KC * 4) {
        for (i = 0; i < 4; i++)
            tk[0][i] ^= sbox[tk[KC - 1][(i + 1) & 3]];
        tk[0][0] ^= rcon[rconpointer++];

        for (j = 1; j < KC; j++) {
            if (KC != 8 || j != KC >> 1)
                for (i = 0; i < 4; i++)
                    tk[j][i] ^= tk[j - 1][i];
            else
                for (i = 0; i < 4; i++)
                    tk[j][i] ^= sbox[tk[j - 1][i]];
        }

        memcpy(a->round_key[0].u8 + t, tk, KC * 4);
    }

    if (decrypt) {
        for (i = 1; i < rounds; i++) {
            av_aes_block tmp[3];
            tmp[2] = a->round_key[i];
            subshift(&tmp[1], 0, sbox);
            mix(tmp, dec_multbl, 1, 3);
            a->round_key[i] = tmp[0];
        }
    } else {
        for (i = 0; i < (rounds + 1) >> 1; i++)
            FFSWAP(av_aes_block, a->round_key[i], a->round_key[rounds - i]);
    }

    return 0;
}

void av_aes_crypt(AVAES *a, uint8_t *dst, const uint8_t *src, int count, uint8_t *iv, int decrypt)
{
    a->crypt(a, dst, src, count, iv, a->rounds);
}

