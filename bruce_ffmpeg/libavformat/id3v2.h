/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-18
 * Description:
 
*********************************/


#ifndef ID3V2_H
#define ID3V2_H

#define ID3v2_HEADER_SIZE 10

#define ID3v2_DEFAULT_MAGIC "ID3"

#define ID3v2_FLAG_DATALEN     0x0001
#define ID3v2_FLAG_UNSYNCH     0x0002
#define ID3v2_FLAG_ENCRYPTION  0x0004
#define ID3v2_FLAG_COMPRESSION 0x0008

#define ID3v2_PRIV_METADATA_PREFIX "id3v2_priv."


typedef struct ID3v2ExtraMeta {
    const char *tag;
    void *data;
    struct ID3v2ExtraMeta *next;
} ID3v2ExtraMeta;

#endif
