/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-18
 * Description:
 
*********************************/


#ifndef AVUTIL_UTILS_H
#define AVUTIL_UTILS_H

#define AV_NOPTS_VALUE ((int64_t)UINT64_C(0x8000000000000000))

#define AV_TIME_BASE            1000000
#define AV_TIME_BASE_Q          (AVRational){1, AV_TIME_BASE}


enum AVMediaType {
    AVMEDIA_TYPE_UNKNOWN = -1,  ///< Usually treated as AVMEDIA_TYPE_DATA
    AVMEDIA_TYPE_VIDEO,
    AVMEDIA_TYPE_AUDIO,
    AVMEDIA_TYPE_DATA,          ///< Opaque data information usually continuous
    AVMEDIA_TYPE_SUBTITLE,
    AVMEDIA_TYPE_ATTACHMENT,    ///< Opaque data information usually sparse
    AVMEDIA_TYPE_NB
};

enum AVPictureType {
    AV_PICTURE_TYPE_NONE = 0, ///< Undefined
    AV_PICTURE_TYPE_I,     ///< Intra
    AV_PICTURE_TYPE_P,     ///< Predicted
    AV_PICTURE_TYPE_B,     ///< Bi-dir predicted
    AV_PICTURE_TYPE_S,     ///< S(GMC)-VOP MPEG-4
    AV_PICTURE_TYPE_SI,    ///< Switching Intra
    AV_PICTURE_TYPE_SP,    ///< Switching Predicted
    AV_PICTURE_TYPE_BI,    ///< BI type
};

void av_url_split(char *proto, int proto_size, 
                 char *authorization, int authorization_size,
                 char *hostname, int hostname_size, 
                 int *port_ptr, 
                 char *path, int path_size, 
                 const char *url);

int av_find_info_tag(char *arg, int arg_size, const char *tag1, const char *info);

#endif