/*
copyright bruceshu

author:bruceshu
date:2018-07-05
description:

*/


#include "protocol.h"
#include "file.h"

#include "libavutils/error.h"


static int file_open(URLContext *url_ctx, const char *filename, int flags)
{
    FileContext *file_ctx = url_ctx->priv_data;
    int access;
    int fd;
    struct stat st;

    av_strstart(filename, "file:", &filename);

    if (flags & AVIO_FLAG_WRITE && flags & AVIO_FLAG_READ) {
        access = O_CREAT | O_RDWR;
        if (file_ctx->trunc)
            access |= O_TRUNC;
    } else if (flags & AVIO_FLAG_WRITE) {
        access = O_CREAT | O_WRONLY;
        if (file_ctx->trunc)
            access |= O_TRUNC;
    } else {
        access = O_RDONLY;
    }
#ifdef O_BINARY
    access |= O_BINARY;
#endif
    fd = avpriv_open(filename, access, 0666);
    if (fd == -1)
        return AVERROR(errno);
    file_ctx->fd = fd;

    //不知道is_streamed有什么作用，当前注释掉
    //url_ctx->is_streamed = !fstat(fd, &st) && S_ISFIFO(st.st_mode);

    /* Buffer writes more than the default 32k to improve throughput especially with networked file systems */
    //!url_ctx->is_streamed && flags & AVIO_FLAG_WRITE)
    //  url_ctx->min_packet_size = url_ctx->max_packet_size = 262144;

    return 0;
}

#define OFFSET(x) offsetof(FileContext, x)
#define D AV_OPT_FLAG_DECODING_PARAM
#define E AV_OPT_FLAG_ENCODING_PARAM

static const AVOption file_options[] = {
    {"truncate", "truncate existing files on write", OFFSET(trunc), AV_OPT_TYPE_BOOL, {.i64 = 1}, 0, 1, E},
    {"blocksize", "set I/O operation maximum block size", OFFSET(block_size), AV_OPT_TYPE_INT, 
        {.i64 = INT_MAX}, 1, INT_MAX, E},
    {"follow", "Follow a file as it is being written", OFFSET(follow), AV_OPT_TYPE_INT, {.i64 = 0}, 0, 1, D},
    {NULL}
};

static const AVClass file_class = {
    .class_name = "file",
    .item_name  = av_default_item_name,
    .option     = file_options,
};

const URLProtocol ff_file_protocol = {
    .name               = "file",
    .url_open           = file_open,
    .priv_data_size     = sizeof(FileContext),
    .priv_data_class    = &file_class,
};













