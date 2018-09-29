/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


#define RTP_PT_PRIVATE 96
#define RTP_VERSION 2
#define RTP_MAX_SDES 256   /**< maximum text length for SDES */

#define RTP_FLAG_KEY    0x1 ///< RTP packet contains a keyframe
#define RTP_FLAG_MARKER 0x2 ///< RTP marker bit was set for this packet


/* RTCP packet types */
enum RTCPType {
    RTCP_FIR    = 192,
    RTCP_NACK, // 193
    RTCP_SMPTETC,// 194
    RTCP_IJ,   // 195
    RTCP_SR     = 200,
    RTCP_RR,   // 201
    RTCP_SDES, // 202
    RTCP_BYE,  // 203
    RTCP_APP,  // 204
    RTCP_RTPFB,// 205
    RTCP_PSFB, // 206
    RTCP_XR,   // 207
    RTCP_AVB,  // 208
    RTCP_RSI,  // 209
    RTCP_TOKEN,// 210
};

#define RTP_PT_IS_RTCP(x) (((x) >= RTCP_FIR && (x) <= RTCP_IJ) || ((x) >= RTCP_SR  && (x) <= RTCP_TOKEN))

struct RTPDynamicProtocolHandler {
    const char *enc_name;
    enum AVMediaType codec_type;
    enum AVCodecID codec_id;
    enum AVStreamParseType need_parsing;
    int static_payload_id; /* 0 means no payload id is set. 0 is a valid
                            * payload ID (PCMU), too, but that format doesn't
                            * require any custom depacketization code. */
    int priv_data_size;

    /** Initialize dynamic protocol handler, called after the full rtpmap line is parsed, may be null */
    int (*init)(AVFormatContext *s, int st_index, PayloadContext *priv_data);
    /** Parse the a= line from the sdp field */
    int (*parse_sdp_a_line)(AVFormatContext *s, int st_index, PayloadContext *priv_data, const char *line);
    /** Free any data needed by the rtp parsing for this dynamic data.
      * Don't free the protocol_data pointer itself, that is freed by the
      * caller. This is called even if the init method failed. */
    void (*close)(PayloadContext *protocol_data);
    /** Parse handler for this dynamic packet */
    DynamicPayloadPacketHandlerProc parse_packet;
    int (*need_keyframe)(PayloadContext *context);

    struct RTPDynamicProtocolHandler *next;
};

typedef struct RTPDemuxContext {
    AVFormatContext *ic;
    AVStream *st;
    int payload_type;
    uint32_t ssrc;
    uint16_t seq;
    uint32_t timestamp;
    uint32_t base_timestamp;
    uint32_t cur_timestamp;
    int64_t  unwrapped_timestamp;
    int64_t  range_start_offset;
    int max_payload_size;
    /* used to send back RTCP RR */
    char hostname[256];

    int srtp_enabled;
    struct SRTPContext srtp;

    /** Statistics for this stream (used by RTCP receiver reports) */
    RTPStatistics statistics;

    /** Fields for packet reordering @{ */
    int prev_ret;     ///< The return value of the actual parsing of the previous packet
    RTPPacket* queue; ///< A sorted queue of buffered packets not yet returned
    int queue_len;    ///< The number of packets in queue
    int queue_size;   ///< The size of queue, or 0 if reordering is disabled
    /*@}*/

    /* rtcp sender statistics receive */
    uint64_t last_rtcp_ntp_time;
    int64_t last_rtcp_reception_time;
    uint64_t first_rtcp_ntp_time;
    uint32_t last_rtcp_timestamp;
    int64_t rtcp_ts_offset;

    /* rtcp sender statistics */
    unsigned int packet_count;
    unsigned int octet_count;
    unsigned int last_octet_count;
    int64_t last_feedback_time;

    /* dynamic payload stuff */
    const RTPDynamicProtocolHandler *handler;
    PayloadContext *dynamic_protocol_context;
} RTPDemuxContext;

typedef struct RTPPacket {
    uint16_t seq;
    uint8_t *buf;
    int len;
    int64_t recvtime;
    struct RTPPacket *next;
} RTPPacket;

typedef struct RTPStatistics {
    uint16_t max_seq;           ///< highest sequence number seen
    uint32_t cycles;            ///< shifted count of sequence number cycles
    uint32_t base_seq;          ///< base sequence number
    uint32_t bad_seq;           ///< last bad sequence number + 1
    int probation;              ///< sequence packets till source is valid
    uint32_t received;          ///< packets received
    uint32_t expected_prior;    ///< packets expected in last interval
    uint32_t received_prior;    ///< packets received in last interval
    uint32_t transit;           ///< relative transit time for previous packet
    uint32_t jitter;            ///< estimated jitter.
} RTPStatistics;

