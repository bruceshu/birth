/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/




struct SRTPContext {
    struct AVAES *aes;
    struct AVHMAC *hmac;
    int rtp_hmac_size, rtcp_hmac_size;
    uint8_t master_key[16];
    uint8_t master_salt[14];
    uint8_t rtp_key[16],  rtcp_key[16];
    uint8_t rtp_salt[14], rtcp_salt[14];
    uint8_t rtp_auth[20], rtcp_auth[20];
    int seq_largest, seq_initialized;
    uint32_t roc;

    uint32_t rtcp_index;
};

