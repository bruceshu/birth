/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-11-08
 * Description:
 
*********************************/


#include "pixdes.h"

static const AVPixFmtDescriptor av_pix_fmt_descriptors[AV_PIX_FMT_NB] = {
    [AV_PIX_FMT_YUV420P] = {
        .name = "yuv420p",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUYV422] = {
        .name = "yuyv422",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 8, 1, 7, 1 },        /* Y */
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* U */
            { 0, 4, 3, 0, 8, 3, 7, 4 },        /* V */
        },
    },
    [AV_PIX_FMT_YVYU422] = {
        .name = "yvyu422",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 8, 1, 7, 1 },        /* Y */
            { 0, 4, 3, 0, 8, 3, 7, 4 },        /* U */
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* V */
        },
    },
    [AV_PIX_FMT_RGB24] = {
        .name = "rgb24",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 3, 0, 0, 8, 2, 7, 1 },        /* R */
            { 0, 3, 1, 0, 8, 2, 7, 2 },        /* G */
            { 0, 3, 2, 0, 8, 2, 7, 3 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_BGR24] = {
        .name = "bgr24",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 3, 2, 0, 8, 2, 7, 3 },        /* R */
            { 0, 3, 1, 0, 8, 2, 7, 2 },        /* G */
            { 0, 3, 0, 0, 8, 2, 7, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_YUV422P] = {
        .name = "yuv422p",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P] = {
        .name = "yuv444p",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV410P] = {
        .name = "yuv410p",
        .nb_components = 3,
        .log2_chroma_w = 2,
        .log2_chroma_h = 2,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV411P] = {
        .name = "yuv411p",
        .nb_components = 3,
        .log2_chroma_w = 2,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUVJ411P] = {
        .name = "yuvj411p",
        .nb_components = 3,
        .log2_chroma_w = 2,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_GRAY8] = {
        .name = "gray",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
        },
        .flags = FF_PSEUDOPAL,
        .alias = "gray8,y8",
    },
    [AV_PIX_FMT_MONOWHITE] = {
        .name = "monow",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 1, 0, 0, 1 },        /* Y */
        },
        .flags = AV_PIX_FMT_FLAG_BITSTREAM,
    },
    [AV_PIX_FMT_MONOBLACK] = {
        .name = "monob",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 7, 1, 0, 0, 1 },        /* Y */
        },
        .flags = AV_PIX_FMT_FLAG_BITSTREAM,
    },
    [AV_PIX_FMT_PAL8] = {
        .name = "pal8",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },
        },
        .flags = AV_PIX_FMT_FLAG_PAL | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVJ420P] = {
        .name = "yuvj420p",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUVJ422P] = {
        .name = "yuvj422p",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUVJ444P] = {
        .name = "yuvj444p",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_XVMC] = {
        .name = "xvmc",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_UYVY422] = {
        .name = "uyvy422",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 1, 0, 8, 1, 7, 2 },        /* Y */
            { 0, 4, 0, 0, 8, 3, 7, 1 },        /* U */
            { 0, 4, 2, 0, 8, 3, 7, 3 },        /* V */
        },
    },
    [AV_PIX_FMT_UYYVYY411] = {
        .name = "uyyvyy411",
        .nb_components = 3,
        .log2_chroma_w = 2,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* Y */
            { 0, 6, 0, 0, 8, 5, 7, 1 },        /* U */
            { 0, 6, 3, 0, 8, 5, 7, 4 },        /* V */
        },
    },
    [AV_PIX_FMT_BGR8] = {
        .name = "bgr8",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 3, 0, 2, 1 },        /* R */
            { 0, 1, 0, 3, 3, 0, 2, 1 },        /* G */
            { 0, 1, 0, 6, 2, 0, 1, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | FF_PSEUDOPAL,
    },
    [AV_PIX_FMT_BGR4] = {
        .name = "bgr4",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 3, 0, 1, 3, 0, 4 },        /* R */
            { 0, 4, 1, 0, 2, 3, 1, 2 },        /* G */
            { 0, 4, 0, 0, 1, 3, 0, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BITSTREAM | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_BGR4_BYTE] = {
        .name = "bgr4_byte",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 1, 0, 0, 1 },        /* R */
            { 0, 1, 0, 1, 2, 0, 1, 1 },        /* G */
            { 0, 1, 0, 3, 1, 0, 0, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | FF_PSEUDOPAL,
    },
    [AV_PIX_FMT_RGB8] = {
        .name = "rgb8",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 6, 2, 0, 1, 1 },        /* R */
            { 0, 1, 0, 3, 3, 0, 2, 1 },        /* G */
            { 0, 1, 0, 0, 3, 0, 2, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | FF_PSEUDOPAL,
    },
    [AV_PIX_FMT_RGB4] = {
        .name = "rgb4",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 0, 0, 1, 3, 0, 1 },        /* R */
            { 0, 4, 1, 0, 2, 3, 1, 2 },        /* G */
            { 0, 4, 3, 0, 1, 3, 0, 4 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BITSTREAM | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_RGB4_BYTE] = {
        .name = "rgb4_byte",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 3, 1, 0, 0, 1 },        /* R */
            { 0, 1, 0, 1, 2, 0, 1, 1 },        /* G */
            { 0, 1, 0, 0, 1, 0, 0, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | FF_PSEUDOPAL,
    },
    [AV_PIX_FMT_NV12] = {
        .name = "nv12",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 2, 0, 0, 8, 1, 7, 1 },        /* U */
            { 1, 2, 1, 0, 8, 1, 7, 2 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_NV21] = {
        .name = "nv21",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 2, 1, 0, 8, 1, 7, 2 },        /* U */
            { 1, 2, 0, 0, 8, 1, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_ARGB] = {
        .name = "argb",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* R */
            { 0, 4, 2, 0, 8, 3, 7, 3 },        /* G */
            { 0, 4, 3, 0, 8, 3, 7, 4 },        /* B */
            { 0, 4, 0, 0, 8, 3, 7, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_RGBA] = {
        .name = "rgba",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 0, 0, 8, 3, 7, 1 },        /* R */
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* G */
            { 0, 4, 2, 0, 8, 3, 7, 3 },        /* B */
            { 0, 4, 3, 0, 8, 3, 7, 4 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_ABGR] = {
        .name = "abgr",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 3, 0, 8, 3, 7, 4 },        /* R */
            { 0, 4, 2, 0, 8, 3, 7, 3 },        /* G */
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* B */
            { 0, 4, 0, 0, 8, 3, 7, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_BGRA] = {
        .name = "bgra",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 2, 0, 8, 3, 7, 3 },        /* R */
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* G */
            { 0, 4, 0, 0, 8, 3, 7, 1 },        /* B */
            { 0, 4, 3, 0, 8, 3, 7, 4 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_0RGB] = {
        .name = "0rgb",
        .nb_components= 3,
        .log2_chroma_w= 0,
        .log2_chroma_h= 0,
        .comp = {
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* R */
            { 0, 4, 2, 0, 8, 3, 7, 3 },        /* G */
            { 0, 4, 3, 0, 8, 3, 7, 4 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_RGB0] = {
        .name = "rgb0",
        .nb_components= 3,
        .log2_chroma_w= 0,
        .log2_chroma_h= 0,
        .comp = {
            { 0, 4, 0, 0, 8, 3, 7, 1 },        /* R */
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* G */
            { 0, 4, 2, 0, 8, 3, 7, 3 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_0BGR] = {
        .name = "0bgr",
        .nb_components= 3,
        .log2_chroma_w= 0,
        .log2_chroma_h= 0,
        .comp = {
            { 0, 4, 3, 0, 8, 3, 7, 4 },        /* R */
            { 0, 4, 2, 0, 8, 3, 7, 3 },        /* G */
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_BGR0] = {
        .name = "bgr0",
        .nb_components= 3,
        .log2_chroma_w= 0,
        .log2_chroma_h= 0,
        .comp = {
            { 0, 4, 2, 0, 8, 3, 7, 3 },        /* R */
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* G */
            { 0, 4, 0, 0, 8, 3, 7, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GRAY9BE] = {
        .name = "gray9be",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },       /* Y */
        },
        .flags = AV_PIX_FMT_FLAG_BE,
        .alias = "y9be",
    },
    [AV_PIX_FMT_GRAY9LE] = {
        .name = "gray9le",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },       /* Y */
        },
        .alias = "y9le",
    },
    [AV_PIX_FMT_GRAY10BE] = {
        .name = "gray10be",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },       /* Y */
        },
        .flags = AV_PIX_FMT_FLAG_BE,
        .alias = "y10be",
    },
    [AV_PIX_FMT_GRAY10LE] = {
        .name = "gray10le",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },       /* Y */
        },
        .alias = "y10le",
    },
    [AV_PIX_FMT_GRAY12BE] = {
        .name = "gray12be",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },       /* Y */
        },
        .flags = AV_PIX_FMT_FLAG_BE,
        .alias = "y12be",
    },
    [AV_PIX_FMT_GRAY12LE] = {
        .name = "gray12le",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },       /* Y */
        },
        .alias = "y12le",
    },
    [AV_PIX_FMT_GRAY14BE] = {
        .name = "gray14be",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 14, 1, 13, 1 },       /* Y */
        },
        .flags = AV_PIX_FMT_FLAG_BE,
        .alias = "y14be",
    },
    [AV_PIX_FMT_GRAY14LE] = {
        .name = "gray14le",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 14, 1, 13, 1 },       /* Y */
        },
        .alias = "y14le",
    },
    [AV_PIX_FMT_GRAY16BE] = {
        .name = "gray16be",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },       /* Y */
        },
        .flags = AV_PIX_FMT_FLAG_BE,
        .alias = "y16be",
    },
    [AV_PIX_FMT_GRAY16LE] = {
        .name = "gray16le",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },       /* Y */
        },
        .alias = "y16le",
    },
    [AV_PIX_FMT_YUV440P] = {
        .name = "yuv440p",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUVJ440P] = {
        .name = "yuvj440p",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV440P10LE] = {
        .name = "yuv440p10le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV440P10BE] = {
        .name = "yuv440p10be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV440P12LE] = {
        .name = "yuv440p12le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV440P12BE] = {
        .name = "yuv440p12be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUVA420P] = {
        .name = "yuva420p",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
            { 3, 1, 0, 0, 8, 0, 7, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA422P] = {
        .name = "yuva422p",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
            { 3, 1, 0, 0, 8, 0, 7, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA444P] = {
        .name = "yuva444p",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
            { 3, 1, 0, 0, 8, 0, 7, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA420P9BE] = {
        .name = "yuva420p9be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
            { 3, 2, 0, 0, 9, 1, 8, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA420P9LE] = {
        .name = "yuva420p9le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
            { 3, 2, 0, 0, 9, 1, 8, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA422P9BE] = {
        .name = "yuva422p9be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
            { 3, 2, 0, 0, 9, 1, 8, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA422P9LE] = {
        .name = "yuva422p9le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
            { 3, 2, 0, 0, 9, 1, 8, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA444P9BE] = {
        .name = "yuva444p9be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
            { 3, 2, 0, 0, 9, 1, 8, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA444P9LE] = {
        .name = "yuva444p9le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
            { 3, 2, 0, 0, 9, 1, 8, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA420P10BE] = {
        .name = "yuva420p10be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
            { 3, 2, 0, 0, 10, 1, 9, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA420P10LE] = {
        .name = "yuva420p10le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
            { 3, 2, 0, 0, 10, 1, 9, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA422P10BE] = {
        .name = "yuva422p10be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
            { 3, 2, 0, 0, 10, 1, 9, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA422P10LE] = {
        .name = "yuva422p10le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
            { 3, 2, 0, 0, 10, 1, 9, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA444P10BE] = {
        .name = "yuva444p10be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
            { 3, 2, 0, 0, 10, 1, 9, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA444P10LE] = {
        .name = "yuva444p10le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
            { 3, 2, 0, 0, 10, 1, 9, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA420P16BE] = {
        .name = "yuva420p16be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
            { 3, 2, 0, 0, 16, 1, 15, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA420P16LE] = {
        .name = "yuva420p16le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
            { 3, 2, 0, 0, 16, 1, 15, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA422P16BE] = {
        .name = "yuva422p16be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
            { 3, 2, 0, 0, 16, 1, 15, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA422P16LE] = {
        .name = "yuva422p16le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
            { 3, 2, 0, 0, 16, 1, 15, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA444P16BE] = {
        .name = "yuva444p16be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
            { 3, 2, 0, 0, 16, 1, 15, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA444P16LE] = {
        .name = "yuva444p16le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
            { 3, 2, 0, 0, 16, 1, 15, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_RGB48BE] = {
        .name = "rgb48be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 6, 0, 0, 16, 5, 15, 1 },       /* R */
            { 0, 6, 2, 0, 16, 5, 15, 3 },       /* G */
            { 0, 6, 4, 0, 16, 5, 15, 5 },       /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BE,
    },
    [AV_PIX_FMT_RGB48LE] = {
        .name = "rgb48le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 6, 0, 0, 16, 5, 15, 1 },       /* R */
            { 0, 6, 2, 0, 16, 5, 15, 3 },       /* G */
            { 0, 6, 4, 0, 16, 5, 15, 5 },       /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_RGBA64BE] = {
        .name = "rgba64be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 8, 0, 0, 16, 7, 15, 1 },       /* R */
            { 0, 8, 2, 0, 16, 7, 15, 3 },       /* G */
            { 0, 8, 4, 0, 16, 7, 15, 5 },       /* B */
            { 0, 8, 6, 0, 16, 7, 15, 7 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_RGBA64LE] = {
        .name = "rgba64le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 8, 0, 0, 16, 7, 15, 1 },       /* R */
            { 0, 8, 2, 0, 16, 7, 15, 3 },       /* G */
            { 0, 8, 4, 0, 16, 7, 15, 5 },       /* B */
            { 0, 8, 6, 0, 16, 7, 15, 7 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_RGB565BE] = {
        .name = "rgb565be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, -1, 3, 5, 1, 4, 0 },        /* R */
            { 0, 2,  0, 5, 6, 1, 5, 1 },        /* G */
            { 0, 2,  0, 0, 5, 1, 4, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_RGB565LE] = {
        .name = "rgb565le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 1, 3, 5, 1, 4, 2 },        /* R */
            { 0, 2, 0, 5, 6, 1, 5, 1 },        /* G */
            { 0, 2, 0, 0, 5, 1, 4, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_RGB555BE] = {
        .name = "rgb555be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, -1, 2, 5, 1, 4, 0 },        /* R */
            { 0, 2,  0, 5, 5, 1, 4, 1 },        /* G */
            { 0, 2,  0, 0, 5, 1, 4, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_RGB555LE] = {
        .name = "rgb555le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 1, 2, 5, 1, 4, 2 },        /* R */
            { 0, 2, 0, 5, 5, 1, 4, 1 },        /* G */
            { 0, 2, 0, 0, 5, 1, 4, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_RGB444BE] = {
        .name = "rgb444be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, -1, 0, 4, 1, 3, 0 },        /* R */
            { 0, 2,  0, 4, 4, 1, 3, 1 },        /* G */
            { 0, 2,  0, 0, 4, 1, 3, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_RGB444LE] = {
        .name = "rgb444le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 1, 0, 4, 1, 3, 2 },        /* R */
            { 0, 2, 0, 4, 4, 1, 3, 1 },        /* G */
            { 0, 2, 0, 0, 4, 1, 3, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_BGR48BE] = {
        .name = "bgr48be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 6, 4, 0, 16, 5, 15, 5 },       /* R */
            { 0, 6, 2, 0, 16, 5, 15, 3 },       /* G */
            { 0, 6, 0, 0, 16, 5, 15, 1 },       /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_BGR48LE] = {
        .name = "bgr48le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 6, 4, 0, 16, 5, 15, 5 },       /* R */
            { 0, 6, 2, 0, 16, 5, 15, 3 },       /* G */
            { 0, 6, 0, 0, 16, 5, 15, 1 },       /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_BGRA64BE] = {
        .name = "bgra64be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 8, 4, 0, 16, 7, 15, 5 },       /* R */
            { 0, 8, 2, 0, 16, 7, 15, 3 },       /* G */
            { 0, 8, 0, 0, 16, 7, 15, 1 },       /* B */
            { 0, 8, 6, 0, 16, 7, 15, 7 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_BGRA64LE] = {
        .name = "bgra64le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 8, 4, 0, 16, 7, 15, 5 },       /* R */
            { 0, 8, 2, 0, 16, 7, 15, 3 },       /* G */
            { 0, 8, 0, 0, 16, 7, 15, 1 },       /* B */
            { 0, 8, 6, 0, 16, 7, 15, 7 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_BGR565BE] = {
        .name = "bgr565be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2,  0, 0, 5, 1, 4, 1 },        /* R */
            { 0, 2,  0, 5, 6, 1, 5, 1 },        /* G */
            { 0, 2, -1, 3, 5, 1, 4, 0 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_BGR565LE] = {
        .name = "bgr565le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 5, 1, 4, 1 },        /* R */
            { 0, 2, 0, 5, 6, 1, 5, 1 },        /* G */
            { 0, 2, 1, 3, 5, 1, 4, 2 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_BGR555BE] = {
        .name = "bgr555be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2,  0, 0, 5, 1, 4, 1 },       /* R */
            { 0, 2,  0, 5, 5, 1, 4, 1 },       /* G */
            { 0, 2, -1, 2, 5, 1, 4, 0 },       /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
     },
    [AV_PIX_FMT_BGR555LE] = {
        .name = "bgr555le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 5, 1, 4, 1 },        /* R */
            { 0, 2, 0, 5, 5, 1, 4, 1 },        /* G */
            { 0, 2, 1, 2, 5, 1, 4, 2 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_BGR444BE] = {
        .name = "bgr444be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2,  0, 0, 4, 1, 3, 1 },       /* R */
            { 0, 2,  0, 4, 4, 1, 3, 1 },       /* G */
            { 0, 2, -1, 0, 4, 1, 3, 0 },       /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
     },
    [AV_PIX_FMT_BGR444LE] = {
        .name = "bgr444le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 4, 1, 3, 1 },        /* R */
            { 0, 2, 0, 4, 4, 1, 3, 1 },        /* G */
            { 0, 2, 1, 0, 4, 1, 3, 2 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
#if FF_API_VAAPI
    [AV_PIX_FMT_VAAPI_MOCO] = {
        .name = "vaapi_moco",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_VAAPI_IDCT] = {
        .name = "vaapi_idct",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_VAAPI_VLD] = {
        .name = "vaapi_vld",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
#else
    [AV_PIX_FMT_VAAPI] = {
        .name = "vaapi",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
#endif
    [AV_PIX_FMT_YUV420P9LE] = {
        .name = "yuv420p9le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV420P9BE] = {
        .name = "yuv420p9be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV420P10LE] = {
        .name = "yuv420p10le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV420P10BE] = {
        .name = "yuv420p10be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV420P12LE] = {
        .name = "yuv420p12le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV420P12BE] = {
        .name = "yuv420p12be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV420P14LE] = {
        .name = "yuv420p14le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 14, 1, 13, 1 },        /* Y */
            { 1, 2, 0, 0, 14, 1, 13, 1 },        /* U */
            { 2, 2, 0, 0, 14, 1, 13, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV420P14BE] = {
        .name = "yuv420p14be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 14, 1, 13, 1 },        /* Y */
            { 1, 2, 0, 0, 14, 1, 13, 1 },        /* U */
            { 2, 2, 0, 0, 14, 1, 13, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV420P16LE] = {
        .name = "yuv420p16le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV420P16BE] = {
        .name = "yuv420p16be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P9LE] = {
        .name = "yuv422p9le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P9BE] = {
        .name = "yuv422p9be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P10LE] = {
        .name = "yuv422p10le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P10BE] = {
        .name = "yuv422p10be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P12LE] = {
        .name = "yuv422p12le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P12BE] = {
        .name = "yuv422p12be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P14LE] = {
        .name = "yuv422p14le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 14, 1, 13, 1 },        /* Y */
            { 1, 2, 0, 0, 14, 1, 13, 1 },        /* U */
            { 2, 2, 0, 0, 14, 1, 13, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P14BE] = {
        .name = "yuv422p14be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 14, 1, 13, 1 },        /* Y */
            { 1, 2, 0, 0, 14, 1, 13, 1 },        /* U */
            { 2, 2, 0, 0, 14, 1, 13, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P16LE] = {
        .name = "yuv422p16le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P16BE] = {
        .name = "yuv422p16be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P16LE] = {
        .name = "yuv444p16le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P16BE] = {
        .name = "yuv444p16be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P10LE] = {
        .name = "yuv444p10le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P10BE] = {
        .name = "yuv444p10be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P9LE] = {
        .name = "yuv444p9le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P9BE] = {
        .name = "yuv444p9be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P12LE] = {
        .name = "yuv444p12le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P12BE] = {
        .name = "yuv444p12be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P14LE] = {
        .name = "yuv444p14le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 14, 1, 13, 1 },        /* Y */
            { 1, 2, 0, 0, 14, 1, 13, 1 },        /* U */
            { 2, 2, 0, 0, 14, 1, 13, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P14BE] = {
        .name = "yuv444p14be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 14, 1, 13, 1 },        /* Y */
            { 1, 2, 0, 0, 14, 1, 13, 1 },        /* U */
            { 2, 2, 0, 0, 14, 1, 13, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_D3D11VA_VLD] = {
        .name = "d3d11va_vld",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_DXVA2_VLD] = {
        .name = "dxva2_vld",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_YA8] = {
        .name = "ya8",
        .nb_components = 2,
        .comp = {
            { 0, 2, 0, 0, 8, 1, 7, 1 },        /* Y */
            { 0, 2, 1, 0, 8, 1, 7, 2 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_ALPHA,
        .alias = "gray8a",
    },
    [AV_PIX_FMT_YA16LE] = {
        .name = "ya16le",
        .nb_components = 2,
        .comp = {
            { 0, 4, 0, 0, 16, 3, 15, 1 },        /* Y */
            { 0, 4, 2, 0, 16, 3, 15, 3 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YA16BE] = {
        .name = "ya16be",
        .nb_components = 2,
        .comp = {
            { 0, 4, 0, 0, 16, 3, 15, 1 },        /* Y */
            { 0, 4, 2, 0, 16, 3, 15, 3 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_VIDEOTOOLBOX] = {
        .name = "videotoolbox_vld",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_GBRP] = {
        .name = "gbrp",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* R */
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* G */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP9LE] = {
        .name = "gbrp9le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* R */
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* G */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP9BE] = {
        .name = "gbrp9be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* R */
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* G */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP10LE] = {
        .name = "gbrp10le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* R */
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* G */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP10BE] = {
        .name = "gbrp10be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* R */
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* G */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP12LE] = {
        .name = "gbrp12le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* R */
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* G */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP12BE] = {
        .name = "gbrp12be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* R */
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* G */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP14LE] = {
        .name = "gbrp14le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 14, 1, 13, 1 },        /* R */
            { 0, 2, 0, 0, 14, 1, 13, 1 },        /* G */
            { 1, 2, 0, 0, 14, 1, 13, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP14BE] = {
        .name = "gbrp14be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 14, 1, 13, 1 },        /* R */
            { 0, 2, 0, 0, 14, 1, 13, 1 },        /* G */
            { 1, 2, 0, 0, 14, 1, 13, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP16LE] = {
        .name = "gbrp16le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 16, 1, 15, 1 },       /* R */
            { 0, 2, 0, 0, 16, 1, 15, 1 },       /* G */
            { 1, 2, 0, 0, 16, 1, 15, 1 },       /* B */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP16BE] = {
        .name = "gbrp16be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 16, 1, 15, 1 },       /* R */
            { 0, 2, 0, 0, 16, 1, 15, 1 },       /* G */
            { 1, 2, 0, 0, 16, 1, 15, 1 },       /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRAP] = {
        .name = "gbrap",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* R */
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* G */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* B */
            { 3, 1, 0, 0, 8, 0, 7, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB |
                 AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_GBRAP16LE] = {
        .name = "gbrap16le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 16, 1, 15, 1 },       /* R */
            { 0, 2, 0, 0, 16, 1, 15, 1 },       /* G */
            { 1, 2, 0, 0, 16, 1, 15, 1 },       /* B */
            { 3, 2, 0, 0, 16, 1, 15, 1 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB |
                 AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_GBRAP16BE] = {
        .name = "gbrap16be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 16, 1, 15, 1 },       /* R */
            { 0, 2, 0, 0, 16, 1, 15, 1 },       /* G */
            { 1, 2, 0, 0, 16, 1, 15, 1 },       /* B */
            { 3, 2, 0, 0, 16, 1, 15, 1 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR |
                 AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_VDPAU] = {
        .name = "vdpau",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_XYZ12LE] = {
        .name = "xyz12le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 6, 0, 4, 12, 5, 11, 1 },       /* X */
            { 0, 6, 2, 4, 12, 5, 11, 3 },       /* Y */
            { 0, 6, 4, 4, 12, 5, 11, 5 },       /* Z */
      },
      /*.flags = -- not used*/
    },
    [AV_PIX_FMT_XYZ12BE] = {
        .name = "xyz12be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 6, 0, 4, 12, 5, 11, 1 },       /* X */
            { 0, 6, 2, 4, 12, 5, 11, 3 },       /* Y */
            { 0, 6, 4, 4, 12, 5, 11, 5 },       /* Z */
       },
        .flags = AV_PIX_FMT_FLAG_BE,
    },

#define BAYER8_DESC_COMMON \
        .nb_components= 3, \
        .log2_chroma_w= 0, \
        .log2_chroma_h= 0, \
        .comp = {          \
            {0,1,0,0,2,0,1,1},\
            {0,1,0,0,4,0,3,1},\
            {0,1,0,0,2,0,1,1},\
        },                 \

#define BAYER16_DESC_COMMON \
        .nb_components= 3, \
        .log2_chroma_w= 0, \
        .log2_chroma_h= 0, \
        .comp = {          \
            {0,2,0,0,4,1,3,1},\
            {0,2,0,0,8,1,7,1},\
            {0,2,0,0,4,1,3,1},\
        },                 \

    [AV_PIX_FMT_BAYER_BGGR8] = {
        .name = "bayer_bggr8",
        BAYER8_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_BGGR16LE] = {
        .name = "bayer_bggr16le",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_BGGR16BE] = {
        .name = "bayer_bggr16be",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_RGGB8] = {
        .name = "bayer_rggb8",
        BAYER8_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_RGGB16LE] = {
        .name = "bayer_rggb16le",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_RGGB16BE] = {
        .name = "bayer_rggb16be",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_GBRG8] = {
        .name = "bayer_gbrg8",
        BAYER8_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_GBRG16LE] = {
        .name = "bayer_gbrg16le",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_GBRG16BE] = {
        .name = "bayer_gbrg16be",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_GRBG8] = {
        .name = "bayer_grbg8",
        BAYER8_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_GRBG16LE] = {
        .name = "bayer_grbg16le",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_GRBG16BE] = {
        .name = "bayer_grbg16be",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_NV16] = {
        .name = "nv16",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 2, 0, 0, 8, 1, 7, 1 },        /* U */
            { 1, 2, 1, 0, 8, 1, 7, 2 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_NV20LE] = {
        .name = "nv20le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 4, 0, 0, 10, 3, 9, 1 },        /* U */
            { 1, 4, 2, 0, 10, 3, 9, 3 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_NV20BE] = {
        .name = "nv20be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 4, 0, 0, 10, 3, 9, 1 },        /* U */
            { 1, 4, 2, 0, 10, 3, 9, 3 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_BE,
    },
    [AV_PIX_FMT_QSV] = {
        .name = "qsv",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_MEDIACODEC] = {
        .name = "mediacodec",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_MMAL] = {
        .name = "mmal",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_CUDA] = {
        .name = "cuda",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_AYUV64LE] = {
        .name = "ayuv64le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 8, 2, 0, 16, 7, 15, 3 },        /* Y */
            { 0, 8, 4, 0, 16, 7, 15, 5 },        /* U */
            { 0, 8, 6, 0, 16, 7, 15, 7 },        /* V */
            { 0, 8, 0, 0, 16, 7, 15, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_AYUV64BE] = {
        .name = "ayuv64be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 8, 2, 0, 16, 7, 15, 3 },        /* Y */
            { 0, 8, 4, 0, 16, 7, 15, 5 },        /* U */
            { 0, 8, 6, 0, 16, 7, 15, 7 },        /* V */
            { 0, 8, 0, 0, 16, 7, 15, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_P010LE] = {
        .name = "p010le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 6, 10, 1, 9, 1 },        /* Y */
            { 1, 4, 0, 6, 10, 3, 9, 1 },        /* U */
            { 1, 4, 2, 6, 10, 3, 9, 3 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_P010BE] = {
        .name = "p010be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 6, 10, 1, 9, 1 },        /* Y */
            { 1, 4, 0, 6, 10, 3, 9, 1 },        /* U */
            { 1, 4, 2, 6, 10, 3, 9, 3 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_BE,
    },
    [AV_PIX_FMT_P016LE] = {
        .name = "p016le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },       /* Y */
            { 1, 4, 0, 0, 16, 3, 15, 1 },       /* U */
            { 1, 4, 2, 0, 16, 3, 15, 3 },       /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_P016BE] = {
        .name = "p016be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },       /* Y */
            { 1, 4, 0, 0, 16, 3, 15, 1 },       /* U */
            { 1, 4, 2, 0, 16, 3, 15, 3 },       /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_BE,
    },
    [AV_PIX_FMT_GBRAP12LE] = {
        .name = "gbrap12le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 12, 1, 11, 1 },       /* R */
            { 0, 2, 0, 0, 12, 1, 11, 1 },       /* G */
            { 1, 2, 0, 0, 12, 1, 11, 1 },       /* B */
            { 3, 2, 0, 0, 12, 1, 11, 1 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB |
                 AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_GBRAP12BE] = {
        .name = "gbrap12be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 12, 1, 11, 1 },       /* R */
            { 0, 2, 0, 0, 12, 1, 11, 1 },       /* G */
            { 1, 2, 0, 0, 12, 1, 11, 1 },       /* B */
            { 3, 2, 0, 0, 12, 1, 11, 1 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR |
                 AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_GBRAP10LE] = {
        .name = "gbrap10le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 10, 1, 9, 1 },       /* R */
            { 0, 2, 0, 0, 10, 1, 9, 1 },       /* G */
            { 1, 2, 0, 0, 10, 1, 9, 1 },       /* B */
            { 3, 2, 0, 0, 10, 1, 9, 1 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB |
                 AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_GBRAP10BE] = {
        .name = "gbrap10be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 10, 1, 9, 1 },       /* R */
            { 0, 2, 0, 0, 10, 1, 9, 1 },       /* G */
            { 1, 2, 0, 0, 10, 1, 9, 1 },       /* B */
            { 3, 2, 0, 0, 10, 1, 9, 1 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR |
                 AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_D3D11] = {
        .name = "d3d11",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_GBRPF32BE] = {
        .name = "gbrpf32be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 4, 0, 0, 32, 3, 31, 1 },        /* R */
            { 0, 4, 0, 0, 32, 3, 31, 1 },        /* G */
            { 1, 4, 0, 0, 32, 3, 31, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR |
                 AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_FLOAT,
    },
    [AV_PIX_FMT_GBRPF32LE] = {
        .name = "gbrpf32le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 4, 0, 0, 32, 3, 31, 1 },        /* R */
            { 0, 4, 0, 0, 32, 3, 31, 1 },        /* G */
            { 1, 4, 0, 0, 32, 3, 31, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_FLOAT | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRAPF32BE] = {
        .name = "gbrapf32be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 4, 0, 0, 32, 3, 31, 1 },        /* R */
            { 0, 4, 0, 0, 32, 3, 31, 1 },        /* G */
            { 1, 4, 0, 0, 32, 3, 31, 1 },        /* B */
            { 3, 4, 0, 0, 32, 3, 31, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR |
                 AV_PIX_FMT_FLAG_ALPHA | AV_PIX_FMT_FLAG_RGB |
                 AV_PIX_FMT_FLAG_FLOAT,
    },
    [AV_PIX_FMT_GBRAPF32LE] = {
        .name = "gbrapf32le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 4, 0, 0, 32, 3, 31, 1 },        /* R */
            { 0, 4, 0, 0, 32, 3, 31, 1 },        /* G */
            { 1, 4, 0, 0, 32, 3, 31, 1 },        /* B */
            { 3, 4, 0, 0, 32, 3, 31, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA |
                 AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_FLOAT,
    },
    [AV_PIX_FMT_DRM_PRIME] = {
        .name = "drm_prime",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_OPENCL] = {
        .name  = "opencl",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_GRAYF32BE] = {
        .name = "grayf32be",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 0, 0, 32, 3, 31, 1 },       /* Y */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_FLOAT,
        .alias = "yf32be",
    },
    [AV_PIX_FMT_GRAYF32LE] = {
        .name = "grayf32le",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 0, 0, 32, 3, 31, 1 },       /* Y */
        },
        .flags = AV_PIX_FMT_FLAG_FLOAT,
        .alias = "yf32le",
    },
};

const AVPixFmtDescriptor *av_pix_fmt_desc_get(enum AVPixelFormat pix_fmt)
{
    if (pix_fmt < 0 || pix_fmt >= AV_PIX_FMT_NB)
        return NULL;
    return &av_pix_fmt_descriptors[pix_fmt];
}


