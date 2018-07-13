



#ifndef HTTP_H
#define HTTP_H

#define BUFFER_SIZE   MAX_URL_SIZE
#define MAX_REDIRECTS 8
#define HTTP_SINGLE   1
#define HTTP_MUTLI    2
#define MAX_EXPIRY    19
#define WHITESPACES " \n\t\r"

#define HTTP_HEADERS_SIZE 4096


typedef enum {
    LOWER_PROTO,
    READ_HEADERS,
    WRITE_REPLY_HEADERS,
    FINISH
}HandshakeState;

typedef enum HTTPAuthType {
    HTTP_AUTH_NONE = 0,
    HTTP_AUTH_BASIC,       /**< HTTP 1.0 Basic auth from RFC 1945 (also in RFC 2617) */
    HTTP_AUTH_DIGEST,      /**< HTTP 1.1 Digest auth from RFC 2617 */
} HTTPAuthType;

typedef struct DigestParams {
    char nonce[300];
    char algorithm[10];

    /* Quality of protection, containing the one  that we've chosen to use, from the alternatives that the server offered.*/
    char qop[30];

    /* A server-specified string that should be included in authentication responses, not included in the actual digest calculation.*/
    char opaque[300];

    /* The server indicated that the auth was ok, but needs to be redone with a new, non-stale nonce. */
    char stale[10];

    /* Nonce count, the number of earlier replies where this particular nonce has been used. */
    int nc;                
} DigestParams;

typedef struct HTTPAuthState {
    char realm[200];
    int auth_type;
    int stale;
    DigestParams digest_params;
} HTTPAuthState;

typedef struct HTTPContext {
    const AVClass *av_class;
    URLContext *pstSubUrlCtx;
    HandshakeState handshake_step;
    AVDictionary *chained_options;
    AVDictionary *cookie_dict;
    HTTPAuthState auth_state;
    HTTPAuthState proxy_auth_state;
    
    unsigned char buffer[BUFFER_SIZE];
    unsigned char *buf_ptr;
    unsigned char *buf_end;
    char *location;
    char *content_type;
    char *headers;
    char *method;
    char *tcp_hook;
    char *http_proxy;
    char *user_agent_deprecated;
    char *user_agent;
    char *cookies;
    uint8_t *post_data;

#if CONFIG_ZLIB
    int compressed;
    z_stream inflate_stream;
    uint8_t *inflate_buffer;
#endif /* CONFIG_ZLIB */
    
    int line_count;
    int end_header;
    int http_code;
    int reply_code;
    int listen;
    int seekable;
    int is_connected_server;
    int chunked_post;
    int send_expect_100;
    int multiple_requests;
    int post_datalen;
    int icy;
    int willclose;
    int end_chunked_post;

    /* Used if "Transfer-Encoding: chunked" otherwise -1. */
    uint64_t chunksize;
    uint64_t off;
    uint64_t end_off;
    uint64_t filesize;
    uint64_t icy_data_read;
    
    int64_t app_ctx_intptr;
    ApplicationContext *app_ctx;
}HTTPContext;




#endif
