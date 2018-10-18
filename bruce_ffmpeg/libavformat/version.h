/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-16
 * Description:
 
*********************************/


#ifndef AVFORMAT_VERSION_H
#define AVFORMAT_VERSION_H

/**
 * @file
 * @ingroup libavf
 * Libavformat version macros
 */

#include "libavutil/version.h"

// Major bumping may affect Ticket5467, 5421, 5451(compatibility with Chromium)
// Also please add any ticket numbers that you believe might be affected here
#define LIBAVFORMAT_VERSION_MAJOR  57
#define LIBAVFORMAT_VERSION_MINOR  71
#define LIBAVFORMAT_VERSION_MICRO 100

#define LIBAVFORMAT_VERSION_INT AV_VERSION_INT(LIBAVFORMAT_VERSION_MAJOR, \
                                               LIBAVFORMAT_VERSION_MINOR, \
                                               LIBAVFORMAT_VERSION_MICRO)
#define LIBAVFORMAT_VERSION     AV_VERSION(LIBAVFORMAT_VERSION_MAJOR,   \
                                           LIBAVFORMAT_VERSION_MINOR,   \
                                           LIBAVFORMAT_VERSION_MICRO)
#define LIBAVFORMAT_BUILD       LIBAVFORMAT_VERSION_INT

#define LIBAVFORMAT_IDENT       "Lavf" AV_STRINGIFY(LIBAVFORMAT_VERSION)

#ifndef FF_API_LAVF_BITEXACT
#define FF_API_LAVF_BITEXACT            (LIBAVFORMAT_VERSION_MAJOR < 58)
#endif
#ifndef FF_API_LAVF_FRAC
#define FF_API_LAVF_FRAC                (LIBAVFORMAT_VERSION_MAJOR < 58)
#endif
#ifndef FF_API_LAVF_CODEC_TB
#define FF_API_LAVF_CODEC_TB            (LIBAVFORMAT_VERSION_MAJOR < 58)
#endif
#ifndef FF_API_URL_FEOF
#define FF_API_URL_FEOF                 (LIBAVFORMAT_VERSION_MAJOR < 58)
#endif
#ifndef FF_API_LAVF_FMT_RAWPICTURE
#define FF_API_LAVF_FMT_RAWPICTURE      (LIBAVFORMAT_VERSION_MAJOR < 58)
#endif
#ifndef FF_API_COMPUTE_PKT_FIELDS2
#define FF_API_COMPUTE_PKT_FIELDS2      (LIBAVFORMAT_VERSION_MAJOR < 58)
#endif
#ifndef FF_API_OLD_OPEN_CALLBACKS
#define FF_API_OLD_OPEN_CALLBACKS       (LIBAVFORMAT_VERSION_MAJOR < 58)
#endif
#ifndef FF_API_LAVF_AVCTX
#define FF_API_LAVF_AVCTX               (LIBAVFORMAT_VERSION_MAJOR < 58)
#endif
#ifndef FF_API_NOCONST_GET_SIDE_DATA
#define FF_API_NOCONST_GET_SIDE_DATA    (LIBAVFORMAT_VERSION_MAJOR < 58)
#endif
#ifndef FF_API_HTTP_USER_AGENT
#define FF_API_HTTP_USER_AGENT          (LIBAVFORMAT_VERSION_MAJOR < 58)
#endif
#ifndef FF_API_HLS_WRAP
#define FF_API_HLS_WRAP                 (LIBAVFORMAT_VERSION_MAJOR < 58)
#endif
#ifndef FF_API_LAVF_MERGE_SD
#define FF_API_LAVF_MERGE_SD            (LIBAVFORMAT_VERSION_MAJOR < 58)
#endif
#ifndef FF_API_LAVF_KEEPSIDE_FLAG
#define FF_API_LAVF_KEEPSIDE_FLAG       (LIBAVFORMAT_VERSION_MAJOR < 58)
#endif
#ifndef FF_API_OLD_ROTATE_API
#define FF_API_OLD_ROTATE_API           (LIBAVFORMAT_VERSION_MAJOR < 58)
#endif

#ifndef FF_API_R_FRAME_RATE
#define FF_API_R_FRAME_RATE            1
#endif
#endif /* AVFORMAT_VERSION_H */

