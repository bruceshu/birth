/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-18
 * Description:
 
*********************************/


#ifndef AVCODEC_H
#define AVCODEC_H

#include "libavutil/rational.h"
#include "libavutil/log.h"
#include "libavutil/internal.h"
#include "libavutil/pixfmt.h"
#include "libavutil/samplefmt.h"
#include "libavutil/utils.h"

#include "internal.h"
#include "version.h"

#define AV_INPUT_BUFFER_PADDING_SIZE 64

#define BUFFER_FLAG_READONLY      (1 << 0)
#define BUFFER_FLAG_REALLOCATABLE (1 << 1)

#define AV_CODEC_CAP_DRAW_HORIZ_BAND     (1 <<  0)
#define AV_CODEC_CAP_DR1                 (1 <<  1)
#define AV_CODEC_CAP_TRUNCATED           (1 <<  3)

#define AV_CODEC_CAP_DELAY               (1 <<  5)
#define AV_CODEC_CAP_SMALL_LAST_FRAME    (1 <<  6)

#define AV_CODEC_CAP_SUBFRAMES           (1 <<  8)
#define AV_CODEC_CAP_EXPERIMENTAL        (1 <<  9)
#define AV_CODEC_CAP_CHANNEL_CONF        (1 << 10)

#define AV_CODEC_CAP_FRAME_THREADS       (1 << 12)
#define AV_CODEC_CAP_SLICE_THREADS       (1 << 13)
#define AV_CODEC_CAP_PARAM_CHANGE        (1 << 14)
#define AV_CODEC_CAP_AUTO_THREADS        (1 << 15)
#define AV_CODEC_CAP_VARIABLE_FRAME_SIZE (1 << 16)
#define AV_CODEC_CAP_AVOID_PROBING       (1 << 17)

#define AV_CODEC_CAP_INTRA_ONLY       0x40000000
#define AV_CODEC_CAP_LOSSLESS         0x80000000

#define AV_CODEC_CAP_HARDWARE            (1 << 18)
#define AV_CODEC_CAP_HYBRID              (1 << 19)

enum AVDiscard{
    AVDISCARD_NONE    =-16, ///< discard nothing
    AVDISCARD_DEFAULT =  0, ///< discard useless packets like 0 size packets in avi
    AVDISCARD_NONREF  =  8, ///< discard all non reference
    AVDISCARD_BIDIR   = 16, ///< discard all bidirectional frames
    AVDISCARD_NONINTRA= 24, ///< discard all non intra frames
    AVDISCARD_NONKEY  = 32, ///< discard all frames except keyframes
    AVDISCARD_ALL     = 48, ///< discard all
};

enum AVFieldOrder {
    AV_FIELD_UNKNOWN,
    AV_FIELD_PROGRESSIVE,
    AV_FIELD_TT,          //< Top coded_first, top displayed first
    AV_FIELD_BB,          //< Bottom coded first, bottom displayed first
    AV_FIELD_TB,          //< Top coded first, bottom displayed first
    AV_FIELD_BT,          //< Bottom coded first, top displayed first
};

enum AVPictureStructure {
    AV_PICTURE_STRUCTURE_UNKNOWN,      //< unknown
    AV_PICTURE_STRUCTURE_TOP_FIELD,    //< coded as top field
    AV_PICTURE_STRUCTURE_BOTTOM_FIELD, //< coded as bottom field
    AV_PICTURE_STRUCTURE_FRAME,        //< coded as frame
};

enum AVAudioServiceType {
    AV_AUDIO_SERVICE_TYPE_MAIN              = 0,
    AV_AUDIO_SERVICE_TYPE_EFFECTS           = 1,
    AV_AUDIO_SERVICE_TYPE_VISUALLY_IMPAIRED = 2,
    AV_AUDIO_SERVICE_TYPE_HEARING_IMPAIRED  = 3,
    AV_AUDIO_SERVICE_TYPE_DIALOGUE          = 4,
    AV_AUDIO_SERVICE_TYPE_COMMENTARY        = 5,
    AV_AUDIO_SERVICE_TYPE_EMERGENCY         = 6,
    AV_AUDIO_SERVICE_TYPE_VOICE_OVER        = 7,
    AV_AUDIO_SERVICE_TYPE_KARAOKE           = 8,
    AV_AUDIO_SERVICE_TYPE_NB                   , ///< Not part of ABI
};

typedef struct AVCodecTag {
    enum AVCodecID id;
    unsigned int tag;
} AVCodecTag;

typedef struct AVProfile {
    int profile;
    const char *name; ///< short name for the profile
} AVProfile;

typedef struct AVCodecDefault {
    const uint8_t *key;
    const uint8_t *value;
} AVCodecDefault;

typedef struct AVCodecHWConfig {
    enum AVPixelFormat pix_fmt;
    int methods;
    //enum AVHWDeviceType device_type;
} AVCodecHWConfig;

#if 0
typedef struct AVBitStreamFilter {
    const AVClass *priv_class;
    const char *name;
    const enum AVCodecID *codec_ids;
    int priv_data_size;
    int (*init)(AVBSFContext *ctx);
    int (*filter)(AVBSFContext *ctx, AVPacket *pkt);
    void (*close)(AVBSFContext *ctx);
    void (*flush)(AVBSFContext *ctx);
} AVBitStreamFilter;

typedef struct AVBSFInternal {
    AVPacket *buffer_pkt;
    int eof;
} AVBSFInternal;

typedef struct AVBSFContext {
    const AVClass *av_class;
    const struct AVBitStreamFilter *filter;
    AVBSFInternal *internal;
    void *priv_data;
    AVCodecParameters *par_in;
    AVCodecParameters *par_out;
    AVRational time_base_in;
    AVRational time_base_out;
} AVBSFContext;
#endif

typedef struct AVCodec {
    const AVClass *priv_class;              ///< AVClass for the private context
    const char *name;
    const char *long_name;
    enum AVMediaType type;
    enum AVCodecID id;
    int capabilities;
    const AVRational *supported_framerates; ///< array of supported framerates, or NULL if any, array is terminated by {0,0}
    const enum AVPixelFormat *pix_fmts;     ///< array of supported pixel formats, or NULL if unknown, array is terminated by -1
    const int *supported_samplerates;       ///< array of supported audio samplerates, or NULL if unknown, array is terminated by 0
    const enum AVSampleFormat *sample_fmts; ///< array of supported sample formats, or NULL if unknown, array is terminated by -1
    const uint64_t *channel_layouts;         ///< array of support channel layouts, or NULL if unknown. array is terminated by 0
    uint8_t max_lowres;                     ///< maximum value for lowres supported by the decoder
    const AVProfile *profiles;              ///< array of recognized profiles, or NULL if unknown, array is terminated by {FF_PROFILE_UNKNOWN}
    const char *wrapper_name;
    int priv_data_size;
    int (*init_thread_copy)(struct AVCodecContext *);
    int (*update_thread_context)(struct AVCodecContext *dst, const struct AVCodecContext *src);
    const AVCodecDefault *defaults;
    void (*init_static_data)(struct AVCodec *codec);
    int (*init)(struct AVCodecContext *);
    //int (*encode_sub)(AVCodecContext *, uint8_t *buf, int buf_size, const struct AVSubtitle *sub);
    //int (*encode2)(AVCodecContext *avctx, AVPacket *avpkt, const AVFrame *frame, int *got_packet_ptr);
    int (*decode)(struct AVCodecContext *, void *outdata, int *outdata_size, AVPacket *avpkt);
    int (*close)(struct AVCodecContext *);
    int (*send_frame)(struct AVCodecContext *avctx, const AVFrame *frame);
    int (*receive_packet)(struct AVCodecContext *avctx, AVPacket *avpkt);
    int (*receive_frame)(struct AVCodecContext *avctx, AVFrame *frame);
    void (*flush)(struct AVCodecContext *);
    int caps_internal;
    const char *bsfs;
    //const struct AVCodecHWConfigInternal **hw_configs;
    
    struct AVCodec *next;
} AVCodec;

typedef struct AVCodecDescriptor {
    enum AVCodecID id;
    enum AVMediaType type;
    const char *name;
    const char *long_name;
    int props;
    const char *const *mime_types;
    const struct AVProfile *profiles;
} AVCodecDescriptor;

typedef struct AVCodecContext {
    const AVClass *av_class;
    int log_level_offset;
    enum AVMediaType codec_type; /* see AVMEDIA_TYPE_xxx */
    const struct AVCodec *codec;
    enum AVCodecID codec_id; /* see AV_CODEC_ID_xxx */
    unsigned int codec_tag;
    void *priv_data;
    struct AVCodecInternal *internal;
    void *opaque;
    int64_t bit_rate;
    int bit_rate_tolerance;
    int global_quality;
    int compression_level;
#define FF_COMPRESSION_DEFAULT -1
    int flags;
    int flags2;
    uint8_t *extradata;
    int extradata_size;
    AVRational time_base;
    int ticks_per_frame;
    int delay;
    int width, height;
    int coded_width, coded_height;
    int gop_size;
    enum AVPixelFormat pix_fmt;
    void (*draw_horiz_band)(struct AVCodecContext *s, const AVFrame *src, int offset[AV_NUM_DATA_POINTERS], int y, int type, int height);
    enum AVPixelFormat (*get_format)(struct AVCodecContext *s, const enum AVPixelFormat * fmt);
    int max_b_frames;
    float b_quant_factor;
    float b_quant_offset;
    int has_b_frames;
    float i_quant_factor;
    float i_quant_offset;
    float lumi_masking;
    float temporal_cplx_masking;
    float spatial_cplx_masking;
    float p_masking;
    float dark_masking;
    int slice_count;
    int *slice_offset;
    AVRational sample_aspect_ratio;
    int me_cmp;
    int me_sub_cmp;
    int mb_cmp;
    int ildct_cmp;
#define FF_CMP_SAD          0
#define FF_CMP_SSE          1
#define FF_CMP_SATD         2
#define FF_CMP_DCT          3
#define FF_CMP_PSNR         4
#define FF_CMP_BIT          5
#define FF_CMP_RD           6
#define FF_CMP_ZERO         7
#define FF_CMP_VSAD         8
#define FF_CMP_VSSE         9
#define FF_CMP_NSSE         10
#define FF_CMP_W53          11
#define FF_CMP_W97          12
#define FF_CMP_DCTMAX       13
#define FF_CMP_DCT264       14
#define FF_CMP_MEDIAN_SAD   15
#define FF_CMP_CHROMA       256
    int dia_size;
    int last_predictor_count;
    int me_pre_cmp;
    int pre_dia_size;
    int me_subpel_quality;
    int me_range;
    int slice_flags;
#define SLICE_FLAG_CODED_ORDER    0x0001 ///< draw_horiz_band() is called in coded order instead of display
#define SLICE_FLAG_ALLOW_FIELD    0x0002 ///< allow draw_horiz_band() with field slices (MPEG-2 field pics)
#define SLICE_FLAG_ALLOW_PLANE    0x0004 ///< allow draw_horiz_band() with 1 component at a time (SVQ1)
    int mb_decision;
#define FF_MB_DECISION_SIMPLE 0        ///< uses mb_cmp
#define FF_MB_DECISION_BITS   1        ///< chooses the one which needs the fewest bits
#define FF_MB_DECISION_RD     2        ///< rate distortion
    uint16_t *intra_matrix;
    uint16_t *inter_matrix;
    int intra_dc_precision;
    int skip_top;
    int skip_bottom;
    int mb_lmin;
    int mb_lmax;
    int bidir_refine;
    int keyint_min;
    int refs;
    int mv0_threshold;
    enum AVColorPrimaries color_primaries;
    enum AVColorTransferCharacteristic color_trc;
    enum AVColorSpace colorspace;
    enum AVColorRange color_range;
    enum AVChromaLocation chroma_sample_location;
    int slices;
    enum AVFieldOrder field_order;
    int sample_rate; ///< samples per second
    int channels;    ///< number of audio channels
    enum AVSampleFormat sample_fmt;  ///< sample format
    int frame_size;
    int frame_number;
    int block_align;
    int cutoff;
    uint64_t channel_layout;
    uint64_t request_channel_layout;
    enum AVAudioServiceType audio_service_type;
    enum AVSampleFormat request_sample_fmt;
    int (*get_buffer2)(struct AVCodecContext *s, AVFrame *frame, int flags);
    /* - encoding parameters */
    float qcompress;  ///< amount of qscale change between easy & hard scenes (0.0-1.0)
    float qblur;      ///< amount of qscale smoothing over time (0.0-1.0)
    int qmin;
    int qmax;
    int max_qdiff;
    int rc_buffer_size;
    int rc_override_count;
    //RcOverride *rc_override;
    int64_t rc_max_rate;
    int64_t rc_min_rate;
    float rc_max_available_vbv_use;
    float rc_min_vbv_overflow_use;
    int rc_initial_buffer_occupancy;
    int trellis;
    char *stats_out;
    char *stats_in;
    int workaround_bugs;
#define FF_BUG_AUTODETECT       1  ///< autodetection
#define FF_BUG_XVID_ILACE       4
#define FF_BUG_UMP4             8
#define FF_BUG_NO_PADDING       16
#define FF_BUG_AMV              32
#define FF_BUG_QPEL_CHROMA      64
#define FF_BUG_STD_QPEL         128
#define FF_BUG_QPEL_CHROMA2     256
#define FF_BUG_DIRECT_BLOCKSIZE 512
#define FF_BUG_EDGE             1024
#define FF_BUG_HPEL_CHROMA      2048
#define FF_BUG_DC_CLIP          4096
#define FF_BUG_MS               8192 ///< Work around various bugs in Microsoft's broken decoders.
#define FF_BUG_TRUNCATED       16384
#define FF_BUG_IEDGE           32768
    int strict_std_compliance;
#define FF_COMPLIANCE_VERY_STRICT   2 ///< Strictly conform to an older more strict version of the spec or reference software.
#define FF_COMPLIANCE_STRICT        1 ///< Strictly conform to all the things in the spec no matter what consequences.
#define FF_COMPLIANCE_NORMAL        0
#define FF_COMPLIANCE_UNOFFICIAL   -1 ///< Allow unofficial extensions
#define FF_COMPLIANCE_EXPERIMENTAL -2 ///< Allow nonstandardized experimental things.
    int error_concealment;
#define FF_EC_GUESS_MVS   1
#define FF_EC_DEBLOCK     2
#define FF_EC_FAVOR_INTER 256
    int debug;
#define FF_DEBUG_PICT_INFO   1
#define FF_DEBUG_RC          2
#define FF_DEBUG_BITSTREAM   4
#define FF_DEBUG_MB_TYPE     8
#define FF_DEBUG_QP          16
#if FF_API_DEBUG_MV
#define FF_DEBUG_MV          32
#endif
#define FF_DEBUG_DCT_COEFF   0x00000040
#define FF_DEBUG_SKIP        0x00000080
#define FF_DEBUG_STARTCODE   0x00000100
#define FF_DEBUG_ER          0x00000400
#define FF_DEBUG_MMCO        0x00000800
#define FF_DEBUG_BUGS        0x00001000
#if FF_API_DEBUG_MV
#define FF_DEBUG_VIS_QP      0x00002000
#define FF_DEBUG_VIS_MB_TYPE 0x00004000
#endif
#define FF_DEBUG_BUFFERS     0x00008000
#define FF_DEBUG_THREADS     0x00010000
#define FF_DEBUG_GREEN_MD    0x00800000
#define FF_DEBUG_NOMC        0x01000000
#if FF_API_DEBUG_MV
    int debug_mv;
#define FF_DEBUG_VIS_MV_P_FOR  0x00000001 // visualize forward predicted MVs of P-frames
#define FF_DEBUG_VIS_MV_B_FOR  0x00000002 // visualize forward predicted MVs of B-frames
#define FF_DEBUG_VIS_MV_B_BACK 0x00000004 // visualize backward predicted MVs of B-frames
#endif
    int err_recognition;
#define AV_EF_CRCCHECK  (1<<0)
#define AV_EF_BITSTREAM (1<<1)          ///< detect bitstream specification deviations
#define AV_EF_BUFFER    (1<<2)          ///< detect improper bitstream length
#define AV_EF_EXPLODE   (1<<3)          ///< abort decoding on minor error detection
#define AV_EF_IGNORE_ERR (1<<15)        ///< ignore errors and continue
#define AV_EF_CAREFUL    (1<<16)        ///< consider things that violate the spec, are fast to calculate and have not been seen in the wild as errors
#define AV_EF_COMPLIANT  (1<<17)        ///< consider all spec non compliances as errors
#define AV_EF_AGGRESSIVE (1<<18)        ///< consider things that a sane encoder should not do as an error
    int64_t reordered_opaque;
    //const struct AVHWAccel *hwaccel;
    void *hwaccel_context;
    uint64_t error[AV_NUM_DATA_POINTERS];
    int dct_algo;
#define FF_DCT_AUTO    0
#define FF_DCT_FASTINT 1
#define FF_DCT_INT     2
#define FF_DCT_MMX     3
#define FF_DCT_ALTIVEC 5
#define FF_DCT_FAAN    6
    int idct_algo;
#define FF_IDCT_AUTO          0
#define FF_IDCT_INT           1
#define FF_IDCT_SIMPLE        2
#define FF_IDCT_SIMPLEMMX     3
#define FF_IDCT_ARM           7
#define FF_IDCT_ALTIVEC       8
#define FF_IDCT_SIMPLEARM     10
#define FF_IDCT_XVID          14
#define FF_IDCT_SIMPLEARMV5TE 16
#define FF_IDCT_SIMPLEARMV6   17
#define FF_IDCT_FAAN          20
#define FF_IDCT_SIMPLENEON    22
#define FF_IDCT_NONE          24 /* Used by XvMC to extract IDCT coefficients with FF_IDCT_PERM_NONE */
#define FF_IDCT_SIMPLEAUTO    128
    int bits_per_coded_sample;
    int bits_per_raw_sample;
#if FF_API_LOWRES
    int lowres;
#endif
    int thread_count;
    int thread_type;
#define FF_THREAD_FRAME   1 ///< Decode more than one frame at once
#define FF_THREAD_SLICE   2 ///< Decode more than one part of a single frame at once
    int active_thread_type;
    int thread_safe_callbacks;
    int (*execute)(struct AVCodecContext *c, int (*func)(struct AVCodecContext *c2, void *arg), void *arg2, int *ret, int count, int size);
    int (*execute2)(struct AVCodecContext *c, int (*func)(struct AVCodecContext *c2, void *arg, int jobnr, int threadnr), void *arg2, int *ret, int count);
    int nsse_weight;
    int profile;
#define FF_PROFILE_UNKNOWN -99
#define FF_PROFILE_RESERVED -100
#define FF_PROFILE_AAC_MAIN 0
#define FF_PROFILE_AAC_LOW  1
#define FF_PROFILE_AAC_SSR  2
#define FF_PROFILE_AAC_LTP  3
#define FF_PROFILE_AAC_HE   4
#define FF_PROFILE_AAC_HE_V2 28
#define FF_PROFILE_AAC_LD   22
#define FF_PROFILE_AAC_ELD  38
#define FF_PROFILE_MPEG2_AAC_LOW 128
#define FF_PROFILE_MPEG2_AAC_HE  131
#define FF_PROFILE_DNXHD         0
#define FF_PROFILE_DNXHR_LB      1
#define FF_PROFILE_DNXHR_SQ      2
#define FF_PROFILE_DNXHR_HQ      3
#define FF_PROFILE_DNXHR_HQX     4
#define FF_PROFILE_DNXHR_444     5
#define FF_PROFILE_DTS         20
#define FF_PROFILE_DTS_ES      30
#define FF_PROFILE_DTS_96_24   40
#define FF_PROFILE_DTS_HD_HRA  50
#define FF_PROFILE_DTS_HD_MA   60
#define FF_PROFILE_DTS_EXPRESS 70
#define FF_PROFILE_MPEG2_422    0
#define FF_PROFILE_MPEG2_HIGH   1
#define FF_PROFILE_MPEG2_SS     2
#define FF_PROFILE_MPEG2_SNR_SCALABLE  3
#define FF_PROFILE_MPEG2_MAIN   4
#define FF_PROFILE_MPEG2_SIMPLE 5
#define FF_PROFILE_H264_CONSTRAINED  (1<<9)  // 8+1; constraint_set1_flag
#define FF_PROFILE_H264_INTRA        (1<<11) // 8+3; constraint_set3_flag
#define FF_PROFILE_H264_BASELINE             66
#define FF_PROFILE_H264_CONSTRAINED_BASELINE (66|FF_PROFILE_H264_CONSTRAINED)
#define FF_PROFILE_H264_MAIN                 77
#define FF_PROFILE_H264_EXTENDED             88
#define FF_PROFILE_H264_HIGH                 100
#define FF_PROFILE_H264_HIGH_10              110
#define FF_PROFILE_H264_HIGH_10_INTRA        (110|FF_PROFILE_H264_INTRA)
#define FF_PROFILE_H264_MULTIVIEW_HIGH       118
#define FF_PROFILE_H264_HIGH_422             122
#define FF_PROFILE_H264_HIGH_422_INTRA       (122|FF_PROFILE_H264_INTRA)
#define FF_PROFILE_H264_STEREO_HIGH          128
#define FF_PROFILE_H264_HIGH_444             144
#define FF_PROFILE_H264_HIGH_444_PREDICTIVE  244
#define FF_PROFILE_H264_HIGH_444_INTRA       (244|FF_PROFILE_H264_INTRA)
#define FF_PROFILE_H264_CAVLC_444            44
#define FF_PROFILE_VC1_SIMPLE   0
#define FF_PROFILE_VC1_MAIN     1
#define FF_PROFILE_VC1_COMPLEX  2
#define FF_PROFILE_VC1_ADVANCED 3
#define FF_PROFILE_MPEG4_SIMPLE                     0
#define FF_PROFILE_MPEG4_SIMPLE_SCALABLE            1
#define FF_PROFILE_MPEG4_CORE                       2
#define FF_PROFILE_MPEG4_MAIN                       3
#define FF_PROFILE_MPEG4_N_BIT                      4
#define FF_PROFILE_MPEG4_SCALABLE_TEXTURE           5
#define FF_PROFILE_MPEG4_SIMPLE_FACE_ANIMATION      6
#define FF_PROFILE_MPEG4_BASIC_ANIMATED_TEXTURE     7
#define FF_PROFILE_MPEG4_HYBRID                     8
#define FF_PROFILE_MPEG4_ADVANCED_REAL_TIME         9
#define FF_PROFILE_MPEG4_CORE_SCALABLE             10
#define FF_PROFILE_MPEG4_ADVANCED_CODING           11
#define FF_PROFILE_MPEG4_ADVANCED_CORE             12
#define FF_PROFILE_MPEG4_ADVANCED_SCALABLE_TEXTURE 13
#define FF_PROFILE_MPEG4_SIMPLE_STUDIO             14
#define FF_PROFILE_MPEG4_ADVANCED_SIMPLE           15
#define FF_PROFILE_JPEG2000_CSTREAM_RESTRICTION_0   1
#define FF_PROFILE_JPEG2000_CSTREAM_RESTRICTION_1   2
#define FF_PROFILE_JPEG2000_CSTREAM_NO_RESTRICTION  32768
#define FF_PROFILE_JPEG2000_DCINEMA_2K              3
#define FF_PROFILE_JPEG2000_DCINEMA_4K              4
#define FF_PROFILE_VP9_0                            0
#define FF_PROFILE_VP9_1                            1
#define FF_PROFILE_VP9_2                            2
#define FF_PROFILE_VP9_3                            3
#define FF_PROFILE_HEVC_MAIN                        1
#define FF_PROFILE_HEVC_MAIN_10                     2
#define FF_PROFILE_HEVC_MAIN_STILL_PICTURE          3
#define FF_PROFILE_HEVC_REXT                        4
#define FF_PROFILE_AV1_MAIN                         0
#define FF_PROFILE_AV1_HIGH                         1
#define FF_PROFILE_AV1_PROFESSIONAL                 2
#define FF_PROFILE_MJPEG_HUFFMAN_BASELINE_DCT            0xc0
#define FF_PROFILE_MJPEG_HUFFMAN_EXTENDED_SEQUENTIAL_DCT 0xc1
#define FF_PROFILE_MJPEG_HUFFMAN_PROGRESSIVE_DCT         0xc2
#define FF_PROFILE_MJPEG_HUFFMAN_LOSSLESS                0xc3
#define FF_PROFILE_MJPEG_JPEG_LS                         0xf7
#define FF_PROFILE_SBC_MSBC                         1
     int level;
#define FF_LEVEL_UNKNOWN -99
    enum AVDiscard skip_loop_filter;
    enum AVDiscard skip_idct;
    enum AVDiscard skip_frame;
    uint8_t *subtitle_header;
    int subtitle_header_size;
    int initial_padding;
    AVRational framerate;
    enum AVPixelFormat sw_pix_fmt;
    AVRational pkt_timebase;
    const AVCodecDescriptor *codec_descriptor;
#if !FF_API_LOWRES
     int lowres;
#endif
    int64_t pts_correction_num_faulty_pts; /// Number of incorrect PTS values so far
    int64_t pts_correction_num_faulty_dts; /// Number of incorrect DTS values so far
    int64_t pts_correction_last_pts;       /// PTS of the last frame
    int64_t pts_correction_last_dts;       /// DTS of the last frame
    char *sub_charenc;
    int sub_charenc_mode;
#define FF_SUB_CHARENC_MODE_DO_NOTHING  -1  ///< do nothing (demuxer outputs a stream supposed to be already in UTF-8, or the codec is bitmap for instance)
#define FF_SUB_CHARENC_MODE_AUTOMATIC    0  ///< libavcodec will select the mode itself
#define FF_SUB_CHARENC_MODE_PRE_DECODER  1  ///< the AVPacket data needs to be recoded to UTF-8 before being fed to the decoder, requires iconv
#define FF_SUB_CHARENC_MODE_IGNORE       2  ///< neither convert the subtitles, nor check them for valid UTF-8
    int skip_alpha;
    int seek_preroll;
#if !FF_API_DEBUG_MV
    int debug_mv;
#define FF_DEBUG_VIS_MV_P_FOR  0x00000001 //visualize forward predicted MVs of P frames
#define FF_DEBUG_VIS_MV_B_FOR  0x00000002 //visualize forward predicted MVs of B frames
#define FF_DEBUG_VIS_MV_B_BACK 0x00000004 //visualize backward predicted MVs of B frames
#endif
    uint16_t *chroma_intra_matrix;
    uint8_t *dump_separator;
    char *codec_whitelist;
    unsigned properties;
#define FF_CODEC_PROPERTY_LOSSLESS        0x00000001
#define FF_CODEC_PROPERTY_CLOSED_CAPTIONS 0x00000002
    AVPacketSideData *coded_side_data;
    int nb_coded_side_data;
    AVBufferRef *hw_frames_ctx;
    int sub_text_format;
#define FF_SUB_TEXT_FMT_ASS              0
#if FF_API_ASS_TIMING
#define FF_SUB_TEXT_FMT_ASS_WITH_TIMINGS 1
#endif
    int trailing_padding;
    int64_t max_pixels;
    AVBufferRef *hw_device_ctx;
    int hwaccel_flags;
    int apply_cropping;
    int extra_hw_frames;
} AVCodecContext;

typedef struct AVCodecParameters {
    enum AVMediaType codec_type;
    enum AVCodecID   codec_id;
    uint32_t         codec_tag;
    
    uint8_t *extradata;
    int extradata_size;
    int format;
    int64_t bit_rate;
    int bits_per_coded_sample;
    int bits_per_raw_sample;
    int profile;
    int level;
    int width;
    int height;
    AVRational sample_aspect_ratio;
    enum AVFieldOrder field_order;
    enum AVColorRange                  color_range;
    enum AVColorPrimaries              color_primaries;
    enum AVColorTransferCharacteristic color_trc;
    enum AVColorSpace                  color_space;
    enum AVChromaLocation              chroma_location;

    int video_delay;
    uint64_t channel_layout;
    int channels;
    int sample_rate;
    int block_align;
    int frame_size;
    int initial_padding;
    int trailing_padding;
    int seek_preroll;
} AVCodecParameters;

#if 0
typedef struct AVCodecParserContext {
    void *priv_data;
    struct AVCodecParser *parser;
    int64_t frame_offset; /* offset of the current frame */
    int64_t cur_offset; 
    int64_t next_frame_offset; /* offset of the next frame */
    int pict_type; /* XXX: Put it back in AVCodecContext. */
    int repeat_pict; /* XXX: Put it back in AVCodecContext. */
    int64_t pts;     /* pts of the current frame */
    int64_t dts;     /* dts of the current frame */
    int64_t last_pts;
    int64_t last_dts;
    int fetch_timestamp;
#define AV_PARSER_PTS_NB 4
    int cur_frame_start_index;
    int64_t cur_frame_offset[AV_PARSER_PTS_NB];
    int64_t cur_frame_pts[AV_PARSER_PTS_NB];
    int64_t cur_frame_dts[AV_PARSER_PTS_NB];
    int flags;
#define PARSER_FLAG_COMPLETE_FRAMES           0x0001
#define PARSER_FLAG_ONCE                      0x0002
#define PARSER_FLAG_FETCHED_OFFSET            0x0004
#define PARSER_FLAG_USE_CODEC_TS              0x1000
    int64_t offset;      ///< byte offset from starting packet start
    int64_t cur_frame_end[AV_PARSER_PTS_NB];
    int key_frame;
    int dts_sync_point;
    int dts_ref_dts_delta;
    int pts_dts_delta;
    int64_t cur_frame_pos[AV_PARSER_PTS_NB];
    int64_t pos;
    int64_t last_pos;
    int duration;
    enum AVFieldOrder field_order;
    enum AVPictureStructure picture_structure;
    int output_picture_number;
    int width;
    int height;
    int coded_width;
    int coded_height;
    int format;
} AVCodecParserContext;

typedef struct AVCodecParser {
    int codec_ids[5]; /* several codec IDs are permitted */
    int priv_data_size;
    int (*parser_init)(AVCodecParserContext *s);
    /* This callback never returns an error, a negative value means that
     * the frame start was in a previous packet. */
    int (*parser_parse)(AVCodecParserContext *s,
                        AVCodecContext *avctx,
                        const uint8_t **poutbuf, int *poutbuf_size,
                        const uint8_t *buf, int buf_size);
    void (*parser_close)(AVCodecParserContext *s);
    int (*split)(AVCodecContext *avctx, const uint8_t *buf, int buf_size);
    struct AVCodecParser *next;
} AVCodecParser;                                                                  } AVCodecTag;
#endif

#endif
