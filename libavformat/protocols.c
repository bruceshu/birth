









//extern const URLProtocol ff_async_protocol;
//extern const URLProtocol ff_bluray_protocol;
//extern const URLProtocol ff_cache_protocol;
//extern const URLProtocol ff_concat_protocol;
//extern const URLProtocol ff_crypto_protocol;
//extern const URLProtocol ff_data_protocol;
//extern const URLProtocol ff_ffrtmpcrypt_protocol;
//extern const URLProtocol ff_ffrtmphttp_protocol;
//extern const URLProtocol ff_file_protocol;
//extern const URLProtocol ff_ftp_protocol;
//extern const URLProtocol ff_gopher_protocol;
//extern const URLProtocol ff_hls_protocol;
extern const URLProtocol ff_http_protocol;
//extern const URLProtocol ff_httpproxy_protocol;
//extern const URLProtocol ff_https_protocol;
//extern const URLProtocol ff_icecast_protocol;
//extern const URLProtocol ff_ijkhttphook_protocol;
//extern const URLProtocol ff_ijklongurl_protocol;
//extern const URLProtocol ff_ijkmediadatasource_protocol;
//extern const URLProtocol ff_ijksegment_protocol;
//extern const URLProtocol ff_ijktcphook_protocol;
//extern const URLProtocol ff_ijkio_protocol;
//extern const URLProtocol ff_mmsh_protocol;
//extern const URLProtocol ff_mmst_protocol;
//extern const URLProtocol ff_md5_protocol;
//extern const URLProtocol ff_pipe_protocol;
//extern const URLProtocol ff_prompeg_protocol;
//extern const URLProtocol ff_rtmp_protocol;
//extern const URLProtocol ff_rtmpe_protocol;
//extern const URLProtocol ff_rtmps_protocol;
//extern const URLProtocol ff_rtmpt_protocol;
//extern const URLProtocol ff_rtmpte_protocol;
//extern const URLProtocol ff_rtmpts_protocol;
//extern const URLProtocol ff_rtp_protocol;
//extern const URLProtocol ff_sctp_protocol;
//extern const URLProtocol ff_srtp_protocol;
//extern const URLProtocol ff_subfile_protocol;
//extern const URLProtocol ff_tee_protocol;
extern const URLProtocol ff_tcp_protocol;
//extern const URLProtocol ff_tls_gnutls_protocol;
//extern const URLProtocol ff_tls_schannel_protocol;
//extern const URLProtocol ff_tls_securetransport_protocol;
//extern const URLProtocol ff_tls_openssl_protocol;
//extern const URLProtocol ff_udp_protocol;
//extern const URLProtocol ff_udplite_protocol;
//extern const URLProtocol ff_unix_protocol;
//extern const URLProtocol ff_librtmp_protocol;
//extern const URLProtocol ff_librtmpe_protocol;
//extern const URLProtocol ff_librtmps_protocol;
//extern const URLProtocol ff_librtmpt_protocol;
//extern const URLProtocol ff_librtmpte_protocol;
//extern const URLProtocol ff_libssh_protocol;
//extern const URLProtocol ff_libsmbclient_protocol;

#include "libavformat/protocol_list.c"



const URLProtocol **ffurl_get_protocols(const char *whitelist, const char *blacklist)
{
    const URLProtocol **ret;
    int i;
    int ret_idx = 0;

    ret = av_mallocz_array(FF_ARRAY_ELEMS(url_protocols), sizeof(*ret));
    if (!ret)
        return NULL;

    for (i = 0; url_protocols[i]; i++) {
        const URLProtocol *up = url_protocols[i];
        ret[ret_idx++] = up;
    }

    return ret;
}


