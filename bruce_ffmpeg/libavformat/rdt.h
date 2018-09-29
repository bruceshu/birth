/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/




struct RDTDemuxContext {
    AVFormatContext *ic; 
    AVStream **streams;
    int n_streams; /**< streams with identical content in this set */
    void *dynamic_protocol_context;
    DynamicPayloadPacketHandlerProc parse_packet;
    uint32_t prev_timestamp;
    int prev_set_id, prev_stream_id;
};

