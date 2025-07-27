/* public domain Simple, Minimalistic, WMA decoder based on libwma
 *	©2018,2025 Yuichiro Nakada
 *
 * Basic usage:
 *	uwma_decode("music.flac", pcm, len, 44100, 16, 2, 9);
 *
 * */

// codeccontext.h
/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id: $
 *
 * Copyright (C) 2005 Dave Chapman
 *
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#ifndef CODECCTX_H
#define CODECCTX_H

/**
 * external api header. 01/04/06 Marsdaddy <pajojo@gmail.com>
 */


#ifdef __cplusplus
extern "C" {
#endif

struct AVCodec;

enum CodecID {
    CODEC_ID_NONE,
    CODEC_ID_MPEG1VIDEO,
    CODEC_ID_MPEG2VIDEO, /* prefered ID for MPEG Video 1 or 2 decoding */
    CODEC_ID_MPEG2VIDEO_XVMC,
    CODEC_ID_H263,
    CODEC_ID_RV10,
    CODEC_ID_RV20,
    CODEC_ID_MP2,
    CODEC_ID_MP3, /* prefered ID for MPEG Audio layer 1, 2 or3 decoding */
    CODEC_ID_VORBIS,
    CODEC_ID_AC3,
    CODEC_ID_MJPEG,
    CODEC_ID_MJPEGB,
    CODEC_ID_LJPEG,
    CODEC_ID_SP5X,
    CODEC_ID_MPEG4,
    CODEC_ID_RAWVIDEO,
    CODEC_ID_MSMPEG4V1,
    CODEC_ID_MSMPEG4V2,
    CODEC_ID_MSMPEG4V3,
    CODEC_ID_WMV1,
    CODEC_ID_WMV2,
    CODEC_ID_H263P,
    CODEC_ID_H263I,
    CODEC_ID_FLV1,
    CODEC_ID_SVQ1,
    CODEC_ID_SVQ3,
    CODEC_ID_DVVIDEO,
    CODEC_ID_DVAUDIO,
    CODEC_ID_WMAV1,
    CODEC_ID_WMAV2,
    CODEC_ID_MACE3,
    CODEC_ID_MACE6,
    CODEC_ID_HUFFYUV,
    CODEC_ID_CYUV,
    CODEC_ID_H264,
    CODEC_ID_INDEO3,
    CODEC_ID_VP3,
    CODEC_ID_THEORA,
    CODEC_ID_AAC,
    CODEC_ID_MPEG4AAC,
    CODEC_ID_ASV1,
    CODEC_ID_ASV2,
    CODEC_ID_FFV1,
    CODEC_ID_4XM,
    CODEC_ID_VCR1,
    CODEC_ID_CLJR,
    CODEC_ID_MDEC,
    CODEC_ID_ROQ,
    CODEC_ID_INTERPLAY_VIDEO,
    CODEC_ID_XAN_WC3,
    CODEC_ID_XAN_WC4,
    CODEC_ID_RPZA,
    CODEC_ID_CINEPAK,
    CODEC_ID_WS_VQA,
    CODEC_ID_MSRLE,
    CODEC_ID_MSVIDEO1,
    CODEC_ID_IDCIN,
    CODEC_ID_8BPS,
    CODEC_ID_SMC,
    CODEC_ID_FLIC,
    CODEC_ID_TRUEMOTION1,
    CODEC_ID_VMDVIDEO,
    CODEC_ID_VMDAUDIO,
    CODEC_ID_MSZH,
    CODEC_ID_ZLIB,
    CODEC_ID_QTRLE,

    /* various pcm "codecs" */
    CODEC_ID_PCM_S16LE,
    CODEC_ID_PCM_S16BE,
    CODEC_ID_PCM_U16LE,
    CODEC_ID_PCM_U16BE,
    CODEC_ID_PCM_S8,
    CODEC_ID_PCM_U8,
    CODEC_ID_PCM_MULAW,
    CODEC_ID_PCM_ALAW,

    /* various adpcm codecs */
    CODEC_ID_ADPCM_IMA_QT,
    CODEC_ID_ADPCM_IMA_WAV,
    CODEC_ID_ADPCM_IMA_DK3,
    CODEC_ID_ADPCM_IMA_DK4,
    CODEC_ID_ADPCM_IMA_WS,
    CODEC_ID_ADPCM_IMA_SMJPEG,
    CODEC_ID_ADPCM_MS,
    CODEC_ID_ADPCM_4XM,
    CODEC_ID_ADPCM_XA,
    CODEC_ID_ADPCM_ADX,
    CODEC_ID_ADPCM_EA,

	/* AMR */
    CODEC_ID_AMR_NB,
    CODEC_ID_AMR_WB,

    /* RealAudio codecs*/
    CODEC_ID_RA_144,
    CODEC_ID_RA_288,

    /* various DPCM codecs */
    CODEC_ID_ROQ_DPCM,
    CODEC_ID_INTERPLAY_DPCM,
    CODEC_ID_XAN_DPCM,

    CODEC_ID_MPEG2TS, /* _FAKE_ codec to indicate a raw MPEG2 transport
                         stream (only used by libavformat) */
};


enum CodecType {
    CODEC_TYPE_UNKNOWN = -1,
    CODEC_TYPE_VIDEO,
    CODEC_TYPE_AUDIO,
    CODEC_TYPE_DATA,
};



/**
 * main external api structure.
 */
typedef struct CodecContext {
    /**
     * the average bitrate.
     * - encoding: set by user. unused for constant quantizer encoding
     * - decoding: set by lavc. 0 or some bitrate if this info is available in the stream
     */
    int bit_rate;

    int bits_per_sample;

    /**
     * number of bits the bitstream is allowed to diverge from the reference.
     *           the reference can be CBR (for CBR pass1) or VBR (for pass2)
     * - encoding: set by user. unused for constant quantizer encoding
     * - decoding: unused
     */
    int bit_rate_tolerance;

    /**
     * CODEC_FLAG_*.
     * - encoding: set by user.
     * - decoding: set by user.
     */
    int flags;

    /**
     * some codecs needs additionnal format info. It is stored here
     * - encoding: set by user.
     * - decoding: set by lavc. (FIXME is this ok?)
     */
    int sub_id;

    /**
     * motion estimation algorithm used for video coding.
     * - encoding: MUST be set by user.
     * - decoding: unused
     */
    int me_method;

    /**
     * some codecs need / can use extra-data like huffman tables.
     * mjpeg: huffman tables
     * rv10: additional flags
     * mpeg4: global headers (they can be in the bitstream or here)
     * - encoding: set/allocated/freed by lavc.
     * - decoding: set/allocated/freed by user.
     */
    void *extradata;
    int extradata_size;

    /* video only */
    /**
     * frames per sec multiplied by frame_rate_base.
     * for variable fps this is the precission, so if the timestamps
     * can be specified in msec precssion then this is 1000*frame_rate_base
     * - encoding: MUST be set by user
     * - decoding: set by lavc. 0 or the frame_rate if available
     */
    int frame_rate;

    /**
     * width / height.
     * - encoding: MUST be set by user.
     * - decoding: set by user if known, codec should override / dynamically change if needed
     */
    int width, height;

#define FF_ASPECT_SQUARE 1
#define FF_ASPECT_4_3_625 2
#define FF_ASPECT_4_3_525 3
#define FF_ASPECT_16_9_625 4
#define FF_ASPECT_16_9_525 5
#define FF_ASPECT_EXTENDED 15

    /**
     * the number of pictures in a group of pitures, or 0 for intra_only.
     * - encoding: set by user.
     * - decoding: unused
     */
    int gop_size;

    /**
     * Frame rate emulation. If not zero lower layer (i.e. format handler)
     * has to read frames at native frame rate.
     * - encoding: set by user.
     * - decoding: unused.
     */
    int rate_emu;

    /* audio only */
    int sample_rate; ///< samples per sec
    int channels;
    int sample_fmt;  ///< sample format, currenly unused

    /* the following data should not be initialized */
    int frame_size;     ///< in samples, initialized when calling 'init'
    int frame_number;   ///< audio or video frame number
    int real_pict_num;  ///< returns the real picture number of previous encoded frame

    struct AVCodec *codec;

    void *priv_data; // PJJ

    /* statistics, used for 2-pass encoding */
    int mv_bits;
    int header_bits;
    int i_tex_bits;
    int p_tex_bits;
    int i_count;
    int p_count;
    int skip_count;
    int misc_bits;

    /**
     * number of bits used for the previously encoded frame.
     * - encoding: set by lavc
     * - decoding: unused
     */
    int frame_bits;

    /**
     * private data of the user, can be used to carry app specific stuff.
     * - encoding: set by user
     * - decoding: set by user
     */
    void *opaque;

    char codec_name[32];
    enum CodecType codec_type; /* see CODEC_TYPE_xxx */
    enum CodecID codec_id; /* see CODEC_ID_xxx */

    /**
     * fourcc (LSB first, so "ABCD" -> ('D'<<24) + ('C'<<16) + ('B'<<8) + 'A').
     * this is used to workaround some encoder bugs
     * - encoding: set by user, if not then the default based on codec_id will be used
     * - decoding: set by user, will be converted to upper case by lavc during init
     */
    unsigned int codec_tag;

    /**
     * workaround bugs in encoders which sometimes cannot be detected automatically.
     * - encoding: unused
     * - decoding: set by user
     */
    int workaround_bugs;


    int block_align; ///< used by some WAV based audio codecs

    int parse_only; /* - decoding only: if true, only parsing is done
                       (function avcodec_parse_frame()). The frame
                       data is returned. Only MPEG codecs support this now. */

    /**
     * 0-> h263 quant 1-> mpeg quant.
     * - encoding: set by user.
     * - decoding: unused
     */
    int mpeg_quant;

    /**
     * pass1 encoding statistics output buffer.
     * - encoding: set by lavc
     * - decoding: unused
     */
    char *stats_out;

    /**
     * pass2 encoding statistics input buffer.
     * concatenated stuff from stats_out of pass1 should be placed here
     * - encoding: allocated/set/freed by user
     * - decoding: unused
     */
    char *stats_in;

    int rc_override_count;

    /**
     * rate control equation.
     * - encoding: set by user
     * - decoding: unused
     */
    char *rc_eq;

    uint64_t error[4];

    /**
     * minimum MB quantizer.
     * - encoding: set by user.
     * - decoding: unused
     */
    int mb_qmin;

    /**
     * maximum MB quantizer.
     * - encoding: set by user.
     * - decoding: unused
     */
    int mb_qmax;

    /**
     * motion estimation compare function.
     * - encoding: set by user.
     * - decoding: unused
     */
    int me_cmp;
    /**
     * subpixel motion estimation compare function.
     * - encoding: set by user.
     * - decoding: unused
     */
    int me_sub_cmp;
    /**
     * macroblock compare function (not supported yet).
     * - encoding: set by user.
     * - decoding: unused
     */
    int mb_cmp;
    /**
     * interlaced dct compare function
     * - encoding: set by user.
     * - decoding: unused
     */
    int ildct_cmp;
#define FF_CMP_SAD  0
#define FF_CMP_SSE  1
#define FF_CMP_SATD 2
#define FF_CMP_DCT  3
#define FF_CMP_PSNR 4
#define FF_CMP_BIT  5
#define FF_CMP_RD   6
#define FF_CMP_ZERO 7
#define FF_CMP_VSAD 8
#define FF_CMP_VSSE 9
#define FF_CMP_CHROMA 256

    /**
     * ME diamond size & shape.
     * - encoding: set by user.
     * - decoding: unused
     */
    int dia_size;

    /**
     * amount of previous MV predictors (2a+1 x 2a+1 square).
     * - encoding: set by user.
     * - decoding: unused
     */
    int last_predictor_count;

    /**
     * pre pass for motion estimation.
     * - encoding: set by user.
     * - decoding: unused
     */
    int pre_me;

    /**
     * motion estimation pre pass compare function.
     * - encoding: set by user.
     * - decoding: unused
     */
    int me_pre_cmp;

    /**
     * ME pre pass diamond size & shape.
     * - encoding: set by user.
     * - decoding: unused
     */
    int pre_dia_size;

    /**
     * subpel ME quality.
     * - encoding: set by user.
     * - decoding: unused
     */
    int me_subpel_quality;

    /**
     * Maximum motion estimation search range in subpel units.
     * if 0 then no limit
     *
     * - encoding: set by user.
     * - decoding: unused.
     */
    int me_range;

    /**
     * frame_rate_base.
     * for variable fps this is 1
     * - encoding: set by user.
     * - decoding: set by lavc.
     * @todo move this after frame_rate
     */

    int frame_rate_base;
    /**
     * intra quantizer bias.
     * - encoding: set by user.
     * - decoding: unused
     */
    int intra_quant_bias;
#define FF_DEFAULT_QUANT_BIAS 999999

    /**
     * inter quantizer bias.
     * - encoding: set by user.
     * - decoding: unused
     */
    int inter_quant_bias;

    /**
     * color table ID.
     * - encoding: unused.
     * - decoding: which clrtable should be used for 8bit RGB images
     *             table have to be stored somewhere FIXME
     */
    int color_table_id;

    /**
     * internal_buffer count.
     * Dont touch, used by lavc default_get_buffer()
     */
    int internal_buffer_count;

    /**
     * internal_buffers.
     * Dont touch, used by lavc default_get_buffer()
     */
    void *internal_buffer;

    /**
     * global quality for codecs which cannot change it per frame.
     * this should be proportional to MPEG1/2/4 qscale.
     * - encoding: set by user.
     * - decoding: unused
     */
    int global_quality;

#define FF_CODER_TYPE_VLC   0
#define FF_CODER_TYPE_AC    1
    /**
     * coder type
     * - encoding: set by user.
     * - decoding: unused
     */
    int coder_type;

    /**
     * context model
     * - encoding: set by user.
     * - decoding: unused
     */
    int context_model;

    /**
     * slice flags
     * - encoding: unused
     * - decoding: set by user.
     */
    int slice_flags;
#define SLICE_FLAG_CODED_ORDER    0x0001 ///< draw_horiz_band() is called in coded order instead of display
#define SLICE_FLAG_ALLOW_FIELD    0x0002 ///< allow draw_horiz_band() with field slices (MPEG2 field pics)
#define SLICE_FLAG_ALLOW_PLANE    0x0004 ///< allow draw_horiz_band() with 1 component at a time (SVQ1)

    /**
     * XVideo Motion Acceleration
     * - encoding: forbidden
     * - decoding: set by decoder
     */
    int xvmc_acceleration;

    /**
     * macroblock decision mode
     * - encoding: set by user.
     * - decoding: unused
     */
    int mb_decision;
#define FF_MB_DECISION_SIMPLE 0        ///< uses mb_cmp
#define FF_MB_DECISION_BITS   1        ///< chooses the one which needs the fewest bits
#define FF_MB_DECISION_RD     2        ///< rate distoration

    /**
     * custom intra quantization matrix
     * - encoding: set by user, can be NULL
     * - decoding: set by lavc
     */
    uint16_t *intra_matrix;

    /**
     * custom inter quantization matrix
     * - encoding: set by user, can be NULL
     * - decoding: set by lavc
     */
    uint16_t *inter_matrix;

    /**
     * fourcc from the AVI stream header (LSB first, so "ABCD" -> ('D'<<24) + ('C'<<16) + ('B'<<8) + 'A').
     * this is used to workaround some encoder bugs
     * - encoding: unused
     * - decoding: set by user, will be converted to upper case by lavc during init
     */
    unsigned int stream_codec_tag;

    /**
     * scene change detection threshold.
     * 0 is default, larger means fewer detected scene changes
     * - encoding: set by user.
     * - decoding: unused
     */
    int scenechange_threshold;

    /**
     * minimum lagrange multipler
     * - encoding: set by user.
     * - decoding: unused
     */
    int lmin;

    /**
     * maximum lagrange multipler
     * - encoding: set by user.
     * - decoding: unused
     */
    int lmax;

    /**
     * noise reduction strength
     * - encoding: set by user.
     * - decoding: unused
     */
    int noise_reduction;

    /**
     * simulates errors in the bitstream to test error concealment.
     * - encoding: set by user.
     * - decoding: unused.
     */
    int error_rate;

    /**
     * MP3 antialias algorithm, see FF_AA_* below.
     * - encoding: unused
     * - decoding: set by user
     */
    int antialias_algo;
#define FF_AA_AUTO    0
#define FF_AA_FASTINT 1 //not implemented yet
#define FF_AA_INT     2
#define FF_AA_FLOAT   3
    /**
     * Quantizer noise shaping.
     * - encoding: set by user
     * - decoding: unused
     */
    int quantizer_noise_shaping;
} CodecContext;


/**
 * AVOption.
 */
typedef struct AVOption {
    /** options' name */
    const char *name; /* if name is NULL, it indicates a link to next */
    /** short English text help or const struct AVOption* subpointer */
    const char *help; //	const struct AVOption* sub;
    /** offset to context structure where the parsed value should be stored */
    int offset;
    /** options' type */
    int type;
#define FF_OPT_TYPE_BOOL 1      ///< boolean - true,1,on  (or simply presence)
#define FF_OPT_TYPE_DOUBLE 2    ///< double
#define FF_OPT_TYPE_INT 3       ///< integer
#define FF_OPT_TYPE_STRING 4    ///< string (finished with \0)
#define FF_OPT_TYPE_MASK 0x1f	///< mask for types - upper bits are various flags
//#define FF_OPT_TYPE_EXPERT 0x20 // flag for expert option
#define FF_OPT_TYPE_FLAG (FF_OPT_TYPE_BOOL | 0x40)
#define FF_OPT_TYPE_RCOVERRIDE (FF_OPT_TYPE_STRING | 0x80)
    /** min value  (min == max   ->  no limits) */
    double min;
    /** maximum value for double/int */
    double max;
    /** default boo [0,1]l/double/int value */
    double defval;
    /**
     * default string value (with optional semicolon delimited extra option-list
     * i.e.   option1;option2;option3
     * defval might select other then first argument as default
     */
    const char *defstr;
#define FF_OPT_MAX_DEPTH 10
} AVOption;

/**
 * AVCodec.
 */
typedef struct AVCodec {
    const char *name;
    enum CodecType type;
    int id;
    int priv_data_size;
    int (*init)(CodecContext *);
    int (*encode)(CodecContext *, uint8_t *buf, int buf_size, void *data);
    int (*close)(CodecContext *);
    int (*decode)(CodecContext *, void *outdata, int *outdata_size,
                  uint8_t *buf, int buf_size);
    int capabilities;
    const AVOption *options;
    struct AVCodec *next;
    void (*flush)(CodecContext *);
} AVCodec;

#endif /* CODECCTX_H */



// wmadeci.h
#ifndef WMADECI_H
#define WMADECI_H

#define NB_LSP_COEFS 10
#define fixed32         int32_t
#define fixed64         int64_t

/**
 * Various WMA tables. 01/04/06 Marsdaddy <pajojo@gmail.com>
 */

static const uint16_t wma_critical_freqs[25] = {
    100,   200,  300, 400,   510,  630,  770,    920,
    1080, 1270, 1480, 1720, 2000, 2320, 2700,   3150,
    3700, 4400, 5300, 6400, 7700, 9500, 12000, 15500,
    24500,
};

/* first value is number of bands */
static const uint8_t exponent_band_22050[3][25] = {
    { 10, 4, 8, 4, 8, 8, 12, 20, 24, 24, 16, },
    { 14, 4, 8, 8, 4, 12, 12, 16, 24, 16, 20, 24, 32, 40, 36, },
    { 23, 4, 4, 4, 8, 4, 4, 8, 8, 8, 8, 8, 12, 12, 16, 16, 24, 24, 32, 44, 48, 60, 84, 72, },
};

static const uint8_t exponent_band_32000[3][25] = {
    { 11, 4, 4, 8, 4, 4, 12, 16, 24, 20, 28, 4, },
    { 15, 4, 8, 4, 4, 8, 8, 16, 20, 12, 20, 20, 28, 40, 56, 8, },
    { 16, 8, 4, 8, 8, 12, 16, 20, 24, 40, 32, 32, 44, 56, 80, 112, 16, },
};

static const uint8_t exponent_band_44100[3][25] = {
    { 12,  4,   4,   4,   4,   4,   8,   8,   8,  12,  16,  20,  36, },
    { 15,  4,   8,   4,   8,   8,   4,   8,   8,  12,  12,  12,  24,  28,  40,  76, },
    { 17,  4,   8,   8,   4,  12,  12,   8,   8,  24,  16,  20,  24,  32,  40,  60,  80, 152, },
};

static const uint16_t hgain_huffcodes[37] = {
 0x00003, 0x002e7, 0x00001, 0x005cd, 0x0005d, 0x005c9, 0x0005e, 0x00003,
 0x00016, 0x0000b, 0x00001, 0x00006, 0x00001, 0x00006, 0x00004, 0x00005,
 0x00004, 0x00007, 0x00003, 0x00007, 0x00004, 0x0000a, 0x0000a, 0x00002,
 0x00003, 0x00000, 0x00005, 0x00002, 0x0005f, 0x00004, 0x00003, 0x00002,
 0x005c8, 0x000b8, 0x005ca, 0x005cb, 0x005cc,
};

static const uint8_t hgain_huffbits[37] = {
 10, 12, 10, 13,  9, 13,  9,  8,
  7,  5,  5,  4,  4,  3,  3,  3,
  4,  3,  4,  4,  5,  5,  6,  8,
  7, 10,  8, 10,  9,  8,  9,  9,
 13, 10, 13, 13, 13,
};

static const fixed32 lsp_codebook[NB_LSP_COEFS][16] = {
{0x1fcc2,0x1fabd,0x1f8c7,0x1f66d,0x1f34c,0x1eef1,0x1e83e,0x1dca6,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
{0x1f8fc,0x1f5f9,0x1f328,0x1f025,0x1ecd8,0x1e8fc,0x1e46f,0x1df1b,0x1d87c,0x1d047,0x1c6b5,0x1bb8f,0x1add8,0x19c0e,0x18220,0x154ca},
{0x1e6ae,0x1dd65,0x1d58e,0x1cd3b,0x1c439,0x1ba69,0x1af5e,0x1a32c,0x195c4,0x18498,0x16fd2,0x156ea,0x13de4,0x11f63,0xf7ae,0xbd90},
{0x1c4fa,0x1ada0,0x19976,0x1891d,0x17986,0x1697f,0x15858,0x145fd,0x1316b,0x11900,0xfcfa,0xdf55,0xbe63,0x9902,0x6e83,0x2e05},
{0x16f2d,0x15205,0x135f3,0x11b14,0x10170,0xe743,0xcdec,0xb504,0x9ab2,0x7f86,0x6296,0x4565,0x24e2,0x90,0xffffd52f,0xffffa172},
{0xffbc,0xd786,0xb521,0x943e,0x7876,0x5ea3,0x44ad,0x2bf0,0x1274,0xfffff829,0xfffe9981,0xffffbfab,0xffffa0bb,0xffff7d3f,0xffff59e3,0xffff3269},
{0x43e1,0x102a,0xffffe94a,0xffffc9fa,0xffffb076,0xffff9a6b,0xffff871c,0xffff7555,0xffff62b4,0xffff4f81,0xffff3bf4,0xffff25f7,0xffff0c0f,0xfffeef53,0xfffecb7e,0xfffe9fb3},
{0xffff75ea,0xffff4325,0xffff1da2,0xfffefd23,0xfffeddb9,0xfffebb51,0xfffe945f,0xfffe6131,0xfffee5fe,0xfffed5ba,0xfffec442,0xfffeb224,0xfffe9f95,0xfffe880e,0xfffe6c7a,0xfffe54c1},
{0xffff9d2e,0xffff709e,0xffff5489,0xffff3d5e,0xffff295b,0xffff1761,0xffff06a2,0xfffef68a,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
{0xfffe7045,0xfffe572f,0xfffe45ea,0xfffe38af,0xfffe2d8f,0xfffe2347,0xfffe18df,0xfffe0d42,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}
};

static const uint32_t scale_huffcodes[121] = {
 0x3ffe8, 0x3ffe6, 0x3ffe7, 0x3ffe5, 0x7fff5, 0x7fff1, 0x7ffed, 0x7fff6,
 0x7ffee, 0x7ffef, 0x7fff0, 0x7fffc, 0x7fffd, 0x7ffff, 0x7fffe, 0x7fff7,
 0x7fff8, 0x7fffb, 0x7fff9, 0x3ffe4, 0x7fffa, 0x3ffe3, 0x1ffef, 0x1fff0,
 0x0fff5, 0x1ffee, 0x0fff2, 0x0fff3, 0x0fff4, 0x0fff1, 0x07ff6, 0x07ff7,
 0x03ff9, 0x03ff5, 0x03ff7, 0x03ff3, 0x03ff6, 0x03ff2, 0x01ff7, 0x01ff5,
 0x00ff9, 0x00ff7, 0x00ff6, 0x007f9, 0x00ff4, 0x007f8, 0x003f9, 0x003f7,
 0x003f5, 0x001f8, 0x001f7, 0x000fa, 0x000f8, 0x000f6, 0x00079, 0x0003a,
 0x00038, 0x0001a, 0x0000b, 0x00004, 0x00000, 0x0000a, 0x0000c, 0x0001b,
 0x00039, 0x0003b, 0x00078, 0x0007a, 0x000f7, 0x000f9, 0x001f6, 0x001f9,
 0x003f4, 0x003f6, 0x003f8, 0x007f5, 0x007f4, 0x007f6, 0x007f7, 0x00ff5,
 0x00ff8, 0x01ff4, 0x01ff6, 0x01ff8, 0x03ff8, 0x03ff4, 0x0fff0, 0x07ff4,
 0x0fff6, 0x07ff5, 0x3ffe2, 0x7ffd9, 0x7ffda, 0x7ffdb, 0x7ffdc, 0x7ffdd,
 0x7ffde, 0x7ffd8, 0x7ffd2, 0x7ffd3, 0x7ffd4, 0x7ffd5, 0x7ffd6, 0x7fff2,
 0x7ffdf, 0x7ffe7, 0x7ffe8, 0x7ffe9, 0x7ffea, 0x7ffeb, 0x7ffe6, 0x7ffe0,
 0x7ffe1, 0x7ffe2, 0x7ffe3, 0x7ffe4, 0x7ffe5, 0x7ffd7, 0x7ffec, 0x7fff4,
 0x7fff3,
};

static const uint8_t scale_huffbits[121] = {
 18, 18, 18, 18, 19, 19, 19, 19,
 19, 19, 19, 19, 19, 19, 19, 19,
 19, 19, 19, 18, 19, 18, 17, 17,
 16, 17, 16, 16, 16, 16, 15, 15,
 14, 14, 14, 14, 14, 14, 13, 13,
 12, 12, 12, 11, 12, 11, 10, 10,
 10,  9,  9,  8,  8,  8,  7,  6,
  6,  5,  4,  3,  1,  4,  4,  5,
  6,  6,  7,  7,  8,  8,  9,  9,
 10, 10, 10, 11, 11, 11, 11, 12,
 12, 13, 13, 13, 14, 14, 16, 15,
 16, 15, 18, 19, 19, 19, 19, 19,
 19, 19, 19, 19, 19, 19, 19, 19,
 19, 19, 19, 19, 19, 19, 19, 19,
 19, 19, 19, 19, 19, 19, 19, 19,
 19,
};

static const uint32_t coef0_huffcodes[666] = {
 0x00258, 0x0003d, 0x00000, 0x00005, 0x00008, 0x00008, 0x0000c, 0x0001b,
 0x0001f, 0x00015, 0x00024, 0x00032, 0x0003a, 0x00026, 0x0002c, 0x0002f,
 0x0004a, 0x0004d, 0x00061, 0x00070, 0x00073, 0x00048, 0x00052, 0x0005a,
 0x0005d, 0x0006e, 0x00099, 0x0009e, 0x000c1, 0x000ce, 0x000e4, 0x000f0,
 0x00093, 0x0009e, 0x000a2, 0x000a1, 0x000b8, 0x000d2, 0x000d3, 0x0012e,
 0x00130, 0x000de, 0x0012d, 0x0019b, 0x001e4, 0x00139, 0x0013a, 0x0013f,
 0x0014f, 0x0016d, 0x001a2, 0x0027c, 0x0027e, 0x00332, 0x0033c, 0x0033f,
 0x0038b, 0x00396, 0x003c5, 0x00270, 0x0027c, 0x0025a, 0x00395, 0x00248,
 0x004bd, 0x004fb, 0x00662, 0x00661, 0x0071b, 0x004e6, 0x004ff, 0x00666,
 0x0071c, 0x0071a, 0x0071f, 0x00794, 0x00536, 0x004e2, 0x0078e, 0x004ee,
 0x00518, 0x00535, 0x004fb, 0x0078d, 0x00530, 0x00680, 0x0068f, 0x005cb,
 0x00965, 0x006a6, 0x00967, 0x0097f, 0x00682, 0x006ae, 0x00cd0, 0x00e28,
 0x00f13, 0x00f1f, 0x009f5, 0x00cd3, 0x00f11, 0x00926, 0x00964, 0x00f32,
 0x00f12, 0x00f30, 0x00966, 0x00d0b, 0x00a68, 0x00b91, 0x009c7, 0x00b73,
 0x012fa, 0x0131d, 0x013f9, 0x01ca0, 0x0199c, 0x01c7a, 0x0198c, 0x01248,
 0x01c74, 0x01c64, 0x0139e, 0x012fd, 0x00a77, 0x012fc, 0x01c7b, 0x012ca,
 0x014cc, 0x014d2, 0x014e3, 0x014dc, 0x012dc, 0x03344, 0x02598, 0x0263c,
 0x0333b, 0x025e6, 0x01a1c, 0x01e3c, 0x014e2, 0x033d4, 0x01a11, 0x03349,
 0x03cce, 0x014e1, 0x01a34, 0x0273e, 0x02627, 0x0273f, 0x038ee, 0x03971,
 0x03c67, 0x03c61, 0x0333d, 0x038c2, 0x0263f, 0x038cd, 0x02638, 0x02e41,
 0x0351f, 0x03348, 0x03c66, 0x03562, 0x02989, 0x027d5, 0x0333c, 0x02e4f,
 0x0343b, 0x02ddf, 0x04bc8, 0x029c0, 0x02e57, 0x04c72, 0x025b7, 0x03547,
 0x03540, 0x029d3, 0x04c45, 0x025bb, 0x06600, 0x04c73, 0x04bce, 0x0357b,
 0x029a6, 0x029d2, 0x0263e, 0x0298a, 0x07183, 0x06602, 0x07958, 0x04b66,
 0x0537d, 0x05375, 0x04fe9, 0x04b67, 0x0799f, 0x04bc9, 0x051fe, 0x06a3b,
 0x05bb6, 0x04fa8, 0x0728f, 0x05376, 0x0492c, 0x0537e, 0x0795a, 0x06a3c,
 0x0e515, 0x07887, 0x0683a, 0x051f9, 0x051fd, 0x0cc6a, 0x06a8a, 0x0cc6d,
 0x05bb3, 0x0683b, 0x051fc, 0x05378, 0x0728e, 0x07886, 0x05bb7, 0x0f2a4,
 0x0795b, 0x0683c, 0x09fc1, 0x0683d, 0x0b752, 0x09678, 0x0a3e8, 0x06ac7,
 0x051f0, 0x0b759, 0x06af3, 0x04b6b, 0x0f2a0, 0x0f2ad, 0x096c3, 0x0e518,
 0x0b75c, 0x0d458, 0x0cc6b, 0x0537c, 0x067aa, 0x04fea, 0x0343a, 0x0cc71,
 0x0967f, 0x09fc4, 0x096c2, 0x0e516, 0x0f2a1, 0x0d45c, 0x0d45d, 0x0d45e,
 0x12fb9, 0x0967e, 0x1982f, 0x09883, 0x096c4, 0x0b753, 0x12fb8, 0x0f2a8,
 0x1ca21, 0x096c5, 0x0e51a, 0x1ca27, 0x12f3c, 0x0d471, 0x0f2aa, 0x0b75b,
 0x12fbb, 0x0f2a9, 0x0f2ac, 0x0d45a, 0x0b74f, 0x096c8, 0x16e91, 0x096ca,
 0x12fbf, 0x0d0a7, 0x13103, 0x0d516, 0x16e99, 0x12cbd, 0x0a3ea, 0x19829,
 0x0b755, 0x29ba7, 0x1ca28, 0x29ba5, 0x16e93, 0x1982c, 0x19828, 0x25994,
 0x0a3eb, 0x1ca29, 0x16e90, 0x1ca25, 0x1982d, 0x1ca26, 0x16e9b, 0x0b756,
 0x0967c, 0x25997, 0x0b75f, 0x198d3, 0x0b757, 0x19a2a, 0x0d45b, 0x0e517,
 0x1ca24, 0x1ca23, 0x1ca22, 0x0b758, 0x16e97, 0x0cd14, 0x13100, 0x00007,
 0x0003b, 0x0006b, 0x00097, 0x00138, 0x00125, 0x00173, 0x00258, 0x00335,
 0x0028e, 0x004c6, 0x00715, 0x00729, 0x004ef, 0x00519, 0x004ed, 0x00532,
 0x0068c, 0x00686, 0x00978, 0x00e5d, 0x00e31, 0x009f4, 0x00b92, 0x012f8,
 0x00d06, 0x00a67, 0x00d44, 0x00a76, 0x00d59, 0x012cd, 0x01c78, 0x01c75,
 0x0199f, 0x0198f, 0x01c67, 0x014c6, 0x01c79, 0x01c76, 0x00b94, 0x00d1b,
 0x01e32, 0x01e31, 0x01ab0, 0x01a05, 0x01aa1, 0x0333a, 0x025e5, 0x02626,
 0x03541, 0x03544, 0x03421, 0x03546, 0x02e55, 0x02e56, 0x0492d, 0x02dde,
 0x0299b, 0x02ddc, 0x0357a, 0x0249c, 0x0668b, 0x1c77f, 0x1ca20, 0x0d45f,
 0x09886, 0x16e9a, 0x0f2a7, 0x0b751, 0x0a3ee, 0x0cf59, 0x0cf57, 0x0b754,
 0x0d0a6, 0x16e98, 0x0b760, 0x06ac6, 0x0a3f0, 0x12fbe, 0x13104, 0x0f2a5,
 0x0a3ef, 0x0d472, 0x12cba, 0x1982e, 0x16e9c, 0x1c77e, 0x198d0, 0x13105,
 0x16e92, 0x0b75d, 0x0d459, 0x0001a, 0x000c0, 0x0016c, 0x003cd, 0x00350,
 0x0067b, 0x0051e, 0x006a9, 0x009f4, 0x00b72, 0x00d09, 0x01249, 0x01e3d,
 0x01ca1, 0x01a1f, 0x01721, 0x01a8a, 0x016e8, 0x03347, 0x01a35, 0x0249d,
 0x0299a, 0x02596, 0x02e4e, 0x0298b, 0x07182, 0x04c46, 0x025ba, 0x02e40,
 0x027d6, 0x04fe8, 0x06607, 0x05310, 0x09884, 0x072e1, 0x06a3d, 0x04b6a,
 0x04c7a, 0x06603, 0x04c7b, 0x03428, 0x06605, 0x09664, 0x09fc0, 0x071de,
 0x06601, 0x05bb2, 0x09885, 0x0a3e2, 0x1c61f, 0x12cbb, 0x0b750, 0x0cf58,
 0x0967d, 0x25995, 0x668ad, 0x0b75a, 0x09fc2, 0x0537f, 0x0b75e, 0x13fae,
 0x12fbc, 0x00031, 0x001c4, 0x004c5, 0x005b8, 0x00cf4, 0x0096f, 0x00d46,
 0x01e57, 0x01a04, 0x02625, 0x03346, 0x028f9, 0x04c47, 0x072e0, 0x04b69,
 0x03420, 0x07957, 0x06639, 0x0799e, 0x07959, 0x07881, 0x04b68, 0x09fc3,
 0x09fd6, 0x0cc70, 0x0a3f1, 0x12cbe, 0x0e30e, 0x0e51b, 0x06af2, 0x12cbc,
 0x1c77d, 0x0f2ab, 0x12fbd, 0x1aa2f, 0x0a3ec, 0x0d473, 0x05377, 0x0a3e9,
 0x1982b, 0x0e300, 0x12f3f, 0x0cf5f, 0x096c0, 0x38c3c, 0x16e94, 0x16e95,
 0x12f3d, 0x29ba4, 0x29ba6, 0x1c77c, 0x6a8ba, 0x3545c, 0x33457, 0x668ac,
 0x6a8bb, 0x16e9d, 0x0e519, 0x25996, 0x12f3e, 0x00036, 0x0033e, 0x006ad,
 0x00d03, 0x012c8, 0x0124a, 0x03c42, 0x03ccd, 0x06606, 0x07880, 0x06852,
 0x06a3a, 0x05bb4, 0x0f2a2, 0x09fc7, 0x12cb9, 0x0cc6c, 0x0a6e8, 0x096c1,
 0x0004a, 0x00355, 0x012f9, 0x014e8, 0x01abe, 0x025b6, 0x0492e, 0x09fc6,
 0x051ff, 0x0cc6f, 0x096cb, 0x0d071, 0x198d1, 0x12cb8, 0x38c3d, 0x13faf,
 0x096c9, 0x0009d, 0x00539, 0x012ce, 0x0341f, 0x029c1, 0x04b33, 0x0a3e3,
 0x0d070, 0x16e96, 0x0b763, 0x000a0, 0x009ce, 0x038cc, 0x0343d, 0x051fa,
 0x09888, 0x12fba, 0x000df, 0x00a75, 0x029a7, 0x09fc5, 0x0e301, 0x0967b,
 0x001e7, 0x012c9, 0x051fb, 0x09889, 0x0f2a6, 0x0016f, 0x01cb9, 0x0cf5a,
 0x12cbf, 0x09679, 0x00272, 0x01a15, 0x0967a, 0x003cb, 0x025f6, 0x0b762,
 0x0028d, 0x03c60, 0x0cf5e, 0x00352, 0x03ccc, 0x0072f, 0x07186, 0x004ec,
 0x05379, 0x0068e, 0x09887, 0x006a7, 0x06af1, 0x00e29, 0x0cf5b, 0x00f31,
 0x0d470, 0x009c6, 0x013fb, 0x13102, 0x019a5, 0x13101, 0x01983, 0x01c65,
 0x0124f, 0x014c7, 0x01726, 0x01abf, 0x03304, 0x02624, 0x03c41, 0x027d7,
 0x02ddd, 0x02e54, 0x0343c, 0x06604, 0x07181, 0x0663a, 0x04fa9, 0x0663b,
 0x05311, 0x0537a, 0x06839, 0x05bb5, 0x0492f, 0x06af0, 0x096c7, 0x0cc6e,
 0x0537b, 0x0cf5c, 0x0cf56, 0x198d2, 0x0cf5d, 0x0a3ed, 0x0f2a3, 0x1982a,
 0x0b761, 0x096c6,
};

static const uint8_t coef0_huffbits[666] = {
 11,  6,  2,  3,  4,  5,  5,  5,
  5,  6,  6,  6,  6,  7,  7,  7,
  7,  7,  7,  7,  7,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,
  9,  9,  9,  9,  9,  9,  9,  9,
  9,  9,  9,  9,  9, 10, 10, 10,
 10, 10, 10, 10, 10, 10, 10, 10,
 10, 10, 10, 11, 11, 11, 10, 11,
 11, 11, 11, 11, 11, 11, 11, 11,
 11, 11, 11, 11, 12, 12, 11, 12,
 12, 12, 12, 11, 12, 12, 12, 12,
 12, 12, 12, 12, 12, 12, 12, 12,
 12, 12, 12, 12, 12, 13, 13, 12,
 12, 12, 13, 13, 13, 13, 13, 13,
 13, 13, 13, 13, 13, 13, 13, 14,
 13, 13, 13, 13, 13, 13, 13, 14,
 14, 14, 14, 14, 14, 14, 14, 14,
 14, 14, 14, 13, 14, 14, 14, 14,
 14, 14, 14, 14, 14, 14, 14, 14,
 14, 14, 14, 14, 14, 14, 14, 15,
 15, 14, 14, 15, 15, 15, 14, 15,
 15, 15, 15, 15, 15, 15, 15, 15,
 15, 15, 15, 15, 15, 15, 15, 15,
 15, 15, 14, 15, 15, 15, 15, 16,
 16, 16, 15, 16, 15, 15, 16, 16,
 16, 16, 15, 16, 16, 16, 15, 16,
 16, 15, 16, 16, 16, 16, 16, 16,
 16, 16, 16, 16, 15, 15, 16, 16,
 15, 16, 16, 16, 17, 17, 17, 16,
 16, 17, 16, 16, 16, 16, 17, 16,
 17, 17, 16, 16, 15, 15, 15, 16,
 17, 16, 17, 16, 16, 17, 17, 17,
 17, 17, 17, 16, 17, 17, 17, 16,
 17, 17, 16, 17, 17, 17, 16, 17,
 17, 16, 16, 17, 17, 17, 18, 17,
 17, 17, 17, 17, 18, 18, 17, 17,
 17, 19, 17, 19, 18, 17, 17, 18,
 17, 17, 18, 17, 17, 17, 18, 17,
 17, 18, 17, 17, 17, 17, 17, 16,
 17, 17, 17, 17, 18, 16, 17,  4,
  6,  8,  9,  9, 10, 10, 10, 10,
 11, 11, 11, 11, 12, 12, 12, 12,
 12, 12, 12, 12, 12, 13, 13, 13,
 13, 13, 13, 13, 13, 13, 13, 13,
 13, 13, 13, 14, 13, 13, 13, 13,
 13, 13, 14, 14, 14, 14, 14, 14,
 15, 15, 15, 15, 15, 15, 16, 15,
 15, 15, 15, 15, 15, 17, 17, 17,
 16, 18, 16, 17, 17, 16, 16, 17,
 17, 18, 17, 16, 17, 17, 17, 16,
 17, 17, 18, 17, 18, 17, 17, 17,
 18, 17, 17,  5,  8, 10, 10, 11,
 11, 12, 12, 12, 13, 13, 14, 13,
 13, 14, 14, 14, 14, 14, 14, 15,
 15, 15, 15, 15, 15, 15, 15, 15,
 15, 15, 15, 16, 16, 15, 16, 16,
 15, 15, 15, 15, 15, 16, 16, 15,
 15, 16, 16, 17, 17, 18, 17, 16,
 17, 18, 19, 17, 16, 16, 17, 17,
 17,  6,  9, 11, 12, 12, 13, 13,
 13, 14, 14, 14, 15, 15, 15, 16,
 15, 15, 15, 15, 15, 15, 16, 16,
 16, 16, 17, 18, 16, 16, 16, 18,
 17, 16, 17, 18, 17, 17, 16, 17,
 17, 16, 17, 16, 17, 18, 18, 18,
 17, 19, 19, 17, 20, 19, 18, 19,
 20, 18, 16, 18, 17,  7, 10, 12,
 13, 13, 14, 14, 14, 15, 15, 16,
 16, 16, 16, 16, 18, 16, 17, 17,
  8, 11, 13, 14, 14, 15, 16, 16,
 16, 16, 17, 17, 17, 18, 18, 17,
 17,  8, 12, 14, 15, 15, 15, 17,
 17, 18, 17,  9, 12, 14, 15, 16,
 16, 17,  9, 13, 15, 16, 16, 17,
  9, 13, 16, 16, 16, 10, 13, 16,
 18, 17, 10, 14, 17, 10, 14, 17,
 11, 14, 16, 11, 14, 11, 15, 12,
 16, 12, 16, 12, 16, 12, 16, 12,
 17, 13, 13, 17, 13, 17, 13, 13,
 14, 14, 14, 14, 14, 14, 14, 15,
 15, 15, 15, 15, 15, 15, 16, 15,
 16, 16, 16, 16, 16, 16, 17, 16,
 16, 16, 16, 17, 16, 17, 16, 17,
 17, 17,
};

static const uint32_t coef1_huffcodes[555] = {
 0x00115, 0x00002, 0x00001, 0x00000, 0x0000d, 0x00007, 0x00013, 0x0001d,
 0x00008, 0x0000c, 0x00023, 0x0002b, 0x0003f, 0x00017, 0x0001b, 0x00043,
 0x00049, 0x00050, 0x00055, 0x00054, 0x00067, 0x00064, 0x0007b, 0x0002d,
 0x00028, 0x0002a, 0x00085, 0x00089, 0x0002b, 0x00035, 0x00090, 0x00091,
 0x00094, 0x00088, 0x000c1, 0x000c6, 0x000f2, 0x000e3, 0x000c5, 0x000e2,
 0x00036, 0x000f0, 0x000a7, 0x000cd, 0x000fb, 0x00059, 0x00116, 0x00103,
 0x00108, 0x0012b, 0x0012d, 0x00188, 0x0012e, 0x0014c, 0x001c3, 0x00187,
 0x001e7, 0x0006f, 0x00094, 0x00069, 0x001e6, 0x001ca, 0x00147, 0x00195,
 0x000a7, 0x00213, 0x00209, 0x00303, 0x00295, 0x00289, 0x0028c, 0x0028d,
 0x00312, 0x00330, 0x0029b, 0x00308, 0x00328, 0x0029a, 0x0025e, 0x003c5,
 0x00384, 0x0039f, 0x00397, 0x00296, 0x0032e, 0x00332, 0x003c6, 0x003e6,
 0x0012d, 0x000d1, 0x00402, 0x000dd, 0x00161, 0x0012b, 0x00127, 0x0045d,
 0x00601, 0x004ab, 0x0045f, 0x00410, 0x004bf, 0x00528, 0x0045c, 0x00424,
 0x00400, 0x00511, 0x00618, 0x0073d, 0x0063a, 0x00614, 0x0073c, 0x007c0,
 0x007cf, 0x00802, 0x00966, 0x00964, 0x00951, 0x008a0, 0x00346, 0x00803,
 0x00a52, 0x0024a, 0x007c1, 0x0063f, 0x00126, 0x00406, 0x00789, 0x008a2,
 0x00960, 0x00967, 0x00c05, 0x00c70, 0x00c79, 0x00a5d, 0x00c26, 0x00c4d,
 0x00372, 0x008a5, 0x00c08, 0x002c5, 0x00f11, 0x00cc4, 0x00f8e, 0x00e16,
 0x00496, 0x00e77, 0x00f9c, 0x00c25, 0x00f1e, 0x00c27, 0x00f1f, 0x00e17,
 0x00ccd, 0x00355, 0x00c09, 0x00c78, 0x00f90, 0x00521, 0x00357, 0x00356,
 0x0068e, 0x00f9d, 0x00c04, 0x00e58, 0x00a20, 0x00a2c, 0x00c4c, 0x0052f,
 0x00f8d, 0x01178, 0x01053, 0x01097, 0x0180f, 0x0180d, 0x012fb, 0x012aa,
 0x0202a, 0x00a40, 0x018ed, 0x01ceb, 0x01455, 0x018e3, 0x012a1, 0x00354,
 0x00353, 0x00f1c, 0x00c7b, 0x00c37, 0x0101d, 0x012cb, 0x01142, 0x0197d,
 0x01095, 0x01e3b, 0x0186b, 0x00588, 0x01c2a, 0x014b8, 0x01e3a, 0x018ec,
 0x01f46, 0x012fa, 0x00a53, 0x01ce8, 0x00a55, 0x01c29, 0x0117b, 0x01052,
 0x012a0, 0x00589, 0x00950, 0x01c2b, 0x00a50, 0x0208b, 0x0180e, 0x02027,
 0x02556, 0x01e20, 0x006e7, 0x01c28, 0x0197a, 0x00684, 0x020a2, 0x01f22,
 0x03018, 0x039cf, 0x03e25, 0x02557, 0x0294c, 0x028a6, 0x00d11, 0x028a9,
 0x02979, 0x00d46, 0x00a56, 0x039ce, 0x030cc, 0x0329a, 0x0149d, 0x0510f,
 0x0451c, 0x02028, 0x03299, 0x01ced, 0x014b9, 0x00f85, 0x00c7a, 0x01800,
 0x00341, 0x012ca, 0x039c8, 0x0329d, 0x00d0d, 0x03e20, 0x05144, 0x00d45,
 0x030d0, 0x0186d, 0x030d5, 0x00d0f, 0x00d40, 0x04114, 0x020a1, 0x0297f,
 0x03e24, 0x032f1, 0x04047, 0x030d4, 0x028a8, 0x00d0e, 0x0451d, 0x04044,
 0x0297e, 0x04042, 0x030d2, 0x030cf, 0x03e21, 0x03e26, 0x028a5, 0x0451a,
 0x00d48, 0x01a16, 0x00d44, 0x04518, 0x0149b, 0x039ca, 0x01498, 0x0403d,
 0x0451b, 0x0149c, 0x032f3, 0x030cb, 0x08073, 0x03e22, 0x0529a, 0x020aa,
 0x039cc, 0x0738a, 0x06530, 0x07389, 0x06193, 0x08071, 0x04043, 0x030ce,
 0x05147, 0x07388, 0x05145, 0x08072, 0x04521, 0x00d47, 0x0297c, 0x030cd,
 0x030ca, 0x0000b, 0x0000c, 0x00083, 0x000e4, 0x00048, 0x00102, 0x001cc,
 0x001f5, 0x00097, 0x0020b, 0x00124, 0x00453, 0x00627, 0x00639, 0x00605,
 0x00517, 0x001b8, 0x00663, 0x00667, 0x007c3, 0x00823, 0x00961, 0x00963,
 0x00e5a, 0x00e59, 0x00a2b, 0x00cbf, 0x00292, 0x00a2d, 0x007d0, 0x00953,
 0x00cc5, 0x00f84, 0x004ab, 0x014a7, 0x0068a, 0x0117a, 0x0052e, 0x01442,
 0x0052c, 0x00c77, 0x00f8f, 0x004aa, 0x01094, 0x01801, 0x012c4, 0x0297b,
 0x00952, 0x01f19, 0x006a5, 0x01149, 0x012c5, 0x01803, 0x022f2, 0x0329b,
 0x04520, 0x0149e, 0x00d13, 0x01f16, 0x01ce9, 0x0101c, 0x006e6, 0x039c9,
 0x06191, 0x07c8e, 0x06192, 0x0ca63, 0x039cd, 0x06190, 0x06884, 0x06885,
 0x07382, 0x00d49, 0x00d41, 0x0450c, 0x0149a, 0x030d1, 0x08077, 0x03e23,
 0x01a15, 0x0e701, 0x0e702, 0x08079, 0x0822a, 0x0a218, 0x07887, 0x0403f,
 0x0520b, 0x0529b, 0x0e700, 0x04519, 0x00007, 0x000e0, 0x000d0, 0x0039b,
 0x003e5, 0x00163, 0x0063e, 0x007c9, 0x00806, 0x00954, 0x01044, 0x01f44,
 0x0197c, 0x01f45, 0x00a51, 0x01f47, 0x00951, 0x0052d, 0x02291, 0x0092f,
 0x00a54, 0x00d12, 0x0297d, 0x00d0c, 0x01499, 0x0329e, 0x032f0, 0x02025,
 0x039c6, 0x00a57, 0x03e46, 0x00d42, 0x0738b, 0x05146, 0x04046, 0x08078,
 0x0510e, 0x07886, 0x02904, 0x04156, 0x04157, 0x06032, 0x030d3, 0x08bce,
 0x04040, 0x0403e, 0x0a414, 0x10457, 0x08075, 0x06887, 0x07c8f, 0x039c7,
 0x07387, 0x08070, 0x08bcf, 0x1482a, 0x10456, 0x1482b, 0x01a17, 0x06886,
 0x0450d, 0x00013, 0x0006b, 0x00615, 0x0080b, 0x0082b, 0x00952, 0x00e5b,
 0x018e2, 0x0186c, 0x01f18, 0x0329f, 0x00d43, 0x03e29, 0x05140, 0x05141,
 0x0ca62, 0x06033, 0x03c42, 0x03e28, 0x0450f, 0x0a21a, 0x07384, 0x0a219,
 0x0e703, 0x0a21b, 0x01a14, 0x07383, 0x045e6, 0x0007a, 0x0012c, 0x00ccc,
 0x0068f, 0x01802, 0x00a52, 0x00953, 0x04045, 0x01a20, 0x0451f, 0x000a4,
 0x00735, 0x01cec, 0x02029, 0x020a3, 0x0451e, 0x00069, 0x00c24, 0x02024,
 0x032f2, 0x05142, 0x00196, 0x00523, 0x000a6, 0x0197b, 0x0030b, 0x0092e,
 0x003e9, 0x03e27, 0x00160, 0x05143, 0x00652, 0x04041, 0x00734, 0x028a7,
 0x0080f, 0x01483, 0x0097c, 0x00340, 0x0068b, 0x00522, 0x01054, 0x01096,
 0x01f17, 0x0202b, 0x01cea, 0x020a0, 0x02978, 0x02026, 0x0297a, 0x039cb,
 0x03e2b, 0x0149f, 0x0329c, 0x07385, 0x08074, 0x0450e, 0x03e2a, 0x05149,
 0x08076, 0x07386, 0x05148,
};

static const uint8_t coef1_huffbits[555] = {
  9,  5,  2,  4,  4,  5,  5,  5,
  6,  6,  6,  6,  6,  7,  7,  7,
  7,  7,  7,  7,  7,  7,  7,  8,
  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,
  9,  8,  8,  8,  8,  9,  9,  9,
  9,  9,  9,  9,  9,  9,  9,  9,
  9, 10, 10, 10,  9,  9,  9,  9,
 10, 10, 10, 10, 10, 10, 10, 10,
 10, 10, 10, 10, 10, 10, 10, 10,
 10, 10, 10, 10, 10, 10, 10, 10,
 11, 11, 11, 11, 11, 11, 11, 11,
 11, 11, 11, 11, 11, 11, 11, 11,
 11, 11, 11, 11, 11, 11, 11, 11,
 11, 12, 12, 12, 12, 12, 12, 12,
 12, 12, 11, 11, 11, 11, 11, 12,
 12, 12, 12, 12, 12, 12, 12, 12,
 13, 12, 12, 12, 12, 12, 12, 12,
 13, 12, 12, 12, 12, 12, 12, 12,
 12, 13, 12, 12, 12, 13, 13, 13,
 13, 12, 12, 12, 12, 12, 12, 13,
 12, 13, 13, 13, 13, 13, 13, 13,
 14, 14, 13, 13, 13, 13, 13, 13,
 13, 12, 12, 12, 13, 13, 13, 13,
 13, 13, 13, 13, 13, 13, 13, 13,
 13, 13, 14, 13, 14, 13, 13, 13,
 13, 13, 14, 13, 14, 14, 13, 14,
 14, 13, 14, 13, 13, 14, 14, 13,
 14, 14, 14, 14, 14, 14, 14, 14,
 14, 15, 14, 14, 14, 14, 15, 15,
 15, 14, 14, 13, 13, 12, 12, 13,
 13, 13, 14, 14, 15, 14, 15, 15,
 14, 13, 14, 15, 15, 15, 14, 14,
 14, 14, 15, 14, 14, 15, 15, 15,
 14, 15, 14, 14, 14, 14, 14, 15,
 15, 16, 15, 15, 15, 14, 15, 15,
 15, 15, 14, 14, 16, 14, 15, 14,
 14, 15, 15, 15, 15, 16, 15, 14,
 15, 15, 15, 16, 15, 15, 14, 14,
 14,  4,  7,  8,  8,  9,  9,  9,
  9, 10, 10, 11, 11, 11, 11, 11,
 11, 12, 11, 11, 11, 12, 12, 12,
 12, 12, 12, 12, 12, 12, 11, 12,
 12, 12, 13, 13, 13, 13, 13, 13,
 13, 12, 12, 13, 13, 13, 13, 14,
 14, 13, 14, 13, 13, 13, 14, 14,
 15, 15, 14, 13, 13, 13, 14, 14,
 15, 15, 15, 16, 14, 15, 17, 17,
 15, 15, 15, 15, 15, 14, 16, 14,
 16, 16, 16, 16, 16, 16, 15, 15,
 17, 15, 16, 15,  6,  8, 10, 10,
 10, 11, 11, 11, 12, 12, 13, 13,
 13, 13, 14, 13, 14, 13, 14, 14,
 14, 14, 14, 15, 15, 14, 14, 14,
 14, 14, 14, 15, 15, 15, 15, 16,
 15, 15, 16, 15, 15, 15, 14, 16,
 15, 15, 18, 17, 16, 17, 15, 14,
 15, 16, 16, 19, 17, 19, 16, 17,
 15,  7, 10, 11, 12, 12, 12, 12,
 13, 13, 13, 14, 15, 14, 15, 15,
 16, 15, 14, 14, 15, 16, 15, 16,
 16, 16, 16, 15, 15,  7, 11, 12,
 13, 13, 14, 14, 15, 15, 15,  8,
 11, 13, 14, 14, 15,  9, 12, 14,
 14, 15,  9, 13, 10, 13, 10, 14,
 10, 14, 11, 15, 11, 15, 11, 14,
 12, 15, 12, 13, 13, 13, 13, 13,
 13, 14, 13, 14, 14, 14, 14, 14,
 14, 15, 14, 15, 16, 15, 14, 15,
 16, 15, 15,
};

static const uint32_t coef2_huffcodes[1336] = {
 0x003e6, 0x000f6, 0x00000, 0x00002, 0x00006, 0x0000f, 0x0001b, 0x00028,
 0x00039, 0x0003f, 0x0006b, 0x00076, 0x000b7, 0x000e8, 0x000ef, 0x00169,
 0x001a7, 0x001d4, 0x001dc, 0x002c4, 0x00349, 0x00355, 0x00391, 0x003dc,
 0x00581, 0x005b2, 0x00698, 0x0070c, 0x00755, 0x0073a, 0x00774, 0x007cf,
 0x00b0a, 0x00b66, 0x00d2e, 0x00d5e, 0x00e1b, 0x00eac, 0x00e5a, 0x00f7e,
 0x00fa1, 0x0163e, 0x01a37, 0x01a52, 0x01c39, 0x01ab3, 0x01d5f, 0x01cb6,
 0x01f52, 0x01dd9, 0x02c04, 0x02c2e, 0x02c2d, 0x02c23, 0x03467, 0x034a3,
 0x0351b, 0x03501, 0x03a5d, 0x0351c, 0x03875, 0x03dea, 0x0397b, 0x039db,
 0x03df1, 0x039d8, 0x03bb4, 0x0580a, 0x0584d, 0x05842, 0x05b13, 0x058ea,
 0x0697d, 0x06a06, 0x068cc, 0x06ac7, 0x06a96, 0x072f4, 0x07543, 0x072b4,
 0x07d20, 0x0b003, 0x073b5, 0x07be6, 0x0d180, 0x07bd1, 0x07cb8, 0x07d06,
 0x07d25, 0x0d2f2, 0x0d19a, 0x0d334, 0x0e1dc, 0x0d529, 0x0d584, 0x0e1d2,
 0x0e5e3, 0x0eec4, 0x0e564, 0x0fa49, 0x16001, 0x0eedc, 0x0f7fa, 0x1a32c,
 0x16131, 0x16003, 0x0f9c8, 0x1ef80, 0x1d2a0, 0x1aa4b, 0x0f7ce, 0x1abfe,
 0x1aa50, 0x1a458, 0x1a816, 0x1cae4, 0x1d2fe, 0x1d52e, 0x1aa4c, 0x2c245,
 0x1d2a1, 0x1a35d, 0x1ca1b, 0x1d5d8, 0x1f531, 0x1ca1c, 0x1f389, 0x1f4af,
 0x3a5e7, 0x351fb, 0x2c24b, 0x34bce, 0x2c24d, 0x2c249, 0x2c24a, 0x72dfc,
 0x357ef, 0x35002, 0x3a5e6, 0x39431, 0x5843b, 0x34a77, 0x58431, 0x3a5f3,
 0x3a5dd, 0x3e5e5, 0x356bd, 0x3976e, 0x6a3d2, 0x3500d, 0x694c4, 0x580bd,
 0x3e5e8, 0x74b95, 0x34a6e, 0x3977c, 0x39432, 0x5b0d2, 0x6a3d8, 0x580b8,
 0x5b0cb, 0x5b0d7, 0x72dee, 0x72ded, 0x72dec, 0x74b9c, 0x3977f, 0x72dea,
 0x74b9e, 0x7be7d, 0x580bf, 0x5b0d5, 0x7cba8, 0x74b91, 0x3e5dd, 0xb6171,
 0xd46b3, 0xd46b9, 0x7cba1, 0x74b9f, 0x72de1, 0xe59f5, 0x3e5eb, 0x00004,
 0x00015, 0x00038, 0x00075, 0x000e8, 0x001d3, 0x00347, 0x0039c, 0x00690,
 0x0074a, 0x00b60, 0x00e93, 0x00f74, 0x0163d, 0x01a5a, 0x01d24, 0x01cbe,
 0x01f4b, 0x03468, 0x03562, 0x03947, 0x03e82, 0x05804, 0x05b12, 0x05803,
 0x0696d, 0x06a9e, 0x0697c, 0x06978, 0x06afb, 0x074b2, 0x072f5, 0x073c0,
 0x07541, 0x06944, 0x074b7, 0x070d3, 0x07ba9, 0x0b0b1, 0x0d1af, 0x0e1dd,
 0x0e5e2, 0x0e1a3, 0x0eec3, 0x1612f, 0x0e961, 0x0eeda, 0x0e78e, 0x0fa48,
 0x1612c, 0x0e511, 0x0e565, 0x0e953, 0x1aa4a, 0x0e59d, 0x1d52c, 0x1a811,
 0x1cae7, 0x1abfc, 0x1d52d, 0x1cacf, 0x1cf05, 0x2c254, 0x34a72, 0x1f4ac,
 0x3976b, 0x34a71, 0x2c6d9, 0x2d873, 0x34a6a, 0x357e7, 0x3464c, 0x3e5f5,
 0x58433, 0x1f53a, 0x3500a, 0x357ea, 0x34a73, 0x3942f, 0x357e5, 0x39775,
 0x694cd, 0x39772, 0x7cba5, 0x6a3ef, 0x35483, 0x74b98, 0x5b0c1, 0x39770,
 0x3a5d7, 0x39433, 0x39434, 0x694ce, 0x580be, 0x3e5ff, 0x6a3ec, 0xb616f,
 0xd46b1, 0x6a3d1, 0x72de5, 0x74b6e, 0x72de9, 0x3e700, 0xd46b6, 0x6a3e9,
 0x74b69, 0xe5675, 0xd46b8, 0x7cbaa, 0x3a5d1, 0x0000c, 0x0003c, 0x000eb,
 0x001f1, 0x003a4, 0x006a8, 0x007d5, 0x00d43, 0x00e77, 0x016c5, 0x01cb1,
 0x02c5d, 0x03a55, 0x03a56, 0x03e51, 0x03bb5, 0x05b0a, 0x06a9f, 0x074b8,
 0x07d28, 0x0d187, 0x0d40e, 0x0d52e, 0x0d425, 0x0eae3, 0x0e1d3, 0x1612e,
 0x0e59e, 0x0eec2, 0x0e578, 0x0e51a, 0x0e579, 0x0e515, 0x0e960, 0x0d183,
 0x0d220, 0x0d2cb, 0x0e512, 0x16c3e, 0x16002, 0x16c42, 0x1cae9, 0x3461a,
 0x1d2fa, 0x1a308, 0x1a849, 0x1cf07, 0x1f38f, 0x34b65, 0x2c253, 0x1ef9e,
 0x1cbc3, 0x1cbc1, 0x2c255, 0x1f384, 0x58435, 0x2c5cd, 0x3a5f7, 0x2c252,
 0x3959c, 0x2c6d8, 0x3a5d3, 0x6ad78, 0x6a3f2, 0x7cba9, 0xb6176, 0x72deb,
 0x39764, 0x3e5f6, 0x3a5d8, 0x74a8c, 0x6a3e6, 0x694d1, 0x6ad79, 0x1a4592,
 0xe59fb, 0x7cbb3, 0x5b0cd, 0x00017, 0x000b5, 0x002c3, 0x005b7, 0x00b1c,
 0x00e5c, 0x0163f, 0x01ab2, 0x01efa, 0x0348a, 0x0396e, 0x058da, 0x06963,
 0x06a30, 0x072cd, 0x073cf, 0x07ce7, 0x0d2ca, 0x0d2d8, 0x0e764, 0x0e794,
 0x16008, 0x16167, 0x1617e, 0x1aa49, 0x1a30b, 0x1a813, 0x2c6da, 0x1a580,
 0x1cbc2, 0x0f9ca, 0x1617f, 0x1d2fe, 0x0f7fc, 0x16c40, 0x0e513, 0x0eec5,
 0x0f7c3, 0x1d508, 0x1a81e, 0x1d2fd, 0x39430, 0x35486, 0x3e5fd, 0x2c24c,
 0x2c75a, 0x34a74, 0x3a5f4, 0x3464d, 0x694ca, 0x3a5f1, 0x1d509, 0x1d5c0,
 0x34648, 0x3464e, 0x6a3d5, 0x6a3e8, 0x6a3e7, 0x5b0c3, 0x2c248, 0x1f38a,
 0x3a5f2, 0x6a3e5, 0x00029, 0x00168, 0x0058c, 0x00b67, 0x00f9d, 0x01c3d,
 0x01cbf, 0x02c20, 0x0351d, 0x03df6, 0x06af9, 0x072b5, 0x0b1d7, 0x0b0b2,
 0x0d40a, 0x0d52b, 0x0e952, 0x0e797, 0x163c3, 0x1c3a0, 0x1f386, 0x1ca21,
 0x34655, 0x2c247, 0x1f53b, 0x2c250, 0x2c24f, 0x1f385, 0x1ef5d, 0x1cf15,
 0x1caea, 0x1ab0a, 0x1cf19, 0x1f53d, 0x1d5c2, 0x1d2fb, 0x1ef58, 0x34a78,
 0x357ec, 0x1f533, 0x3a5e1, 0x694d2, 0x58482, 0x3a5ee, 0x2c6dc, 0x357eb,
 0x5b0c4, 0x39778, 0x6a3e1, 0x7cbb4, 0x3a5e1, 0x74b68, 0x3a5ef, 0x3a5d2,
 0x39424, 0x72de2, 0xe59f6, 0xe59f7, 0x3e702, 0x3e5ec, 0x1f38b, 0x0003b,
 0x001f0, 0x00777, 0x00fa8, 0x01cb2, 0x02d84, 0x03a57, 0x03dd6, 0x06917,
 0x06a11, 0x07d07, 0x0eae2, 0x0e796, 0x0f9c9, 0x0f7fb, 0x16166, 0x16160,
 0x1ab1b, 0x1abfa, 0x2d87b, 0x1d2f7, 0x39768, 0x1f38c, 0x34653, 0x34651,
 0x6a3d9, 0x35001, 0x3abbd, 0x38742, 0x39426, 0x34a76, 0x3a5ec, 0x34a75,
 0x35000, 0x35488, 0x1cf10, 0x2c6db, 0x357ed, 0x357e8, 0x357e9, 0x3a5f0,
 0x694c2, 0xb6178, 0x72df5, 0x39425, 0x3942b, 0x74b6d, 0x74b6f, 0xb6177,
 0xb6179, 0x74b6a, 0xb6172, 0x58487, 0x3e5ee, 0x3e5ed, 0x72df2, 0x72df4,
 0x7cbae, 0x6a3ca, 0x70e86, 0x34bcf, 0x6a3c8, 0x00059, 0x00384, 0x00d5b,
 0x01c38, 0x03560, 0x0395b, 0x0584e, 0x06964, 0x073cd, 0x0b1e7, 0x0e798,
 0x0e78d, 0x0fa43, 0x1a848, 0x1a32f, 0x1aa4e, 0x3464a, 0x1f4ab, 0x1f38d,
 0x3a5eb, 0x3a5d4, 0x3548a, 0x6a3c7, 0x5b0d0, 0x6a3c5, 0x7cbb0, 0x694cb,
 0x3a5e5, 0x3e5e2, 0x3942c, 0x2d872, 0x1f4ae, 0x3a5d5, 0x694d3, 0x58481,
 0x35009, 0x39774, 0x58432, 0xb616c, 0x5b0db, 0x3548b, 0xb6174, 0x1d5d95,
 0xb004c, 0x7cbb2, 0x3a5e5, 0x74a8f, 0xe59f9, 0x72df6, 0xe59fd, 0x7cbad,
 0xd427d, 0x72cff, 0x3977a, 0x5b0d9, 0xb616d, 0xb616b, 0x1a4593, 0x7cbaf,
 0x5b0da, 0x00071, 0x003eb, 0x01603, 0x02c6c, 0x03961, 0x068c8, 0x06a31,
 0x072bd, 0x0d2c2, 0x0e51b, 0x0e5e6, 0x1abfb, 0x1d2ff, 0x1cae5, 0x1ef5c,
 0x1ef5e, 0x1cf13, 0x34a6d, 0x3976d, 0xb616a, 0x3e5f2, 0x6a3c4, 0xb6169,
 0x3e5dc, 0x580b9, 0x74b99, 0x75764, 0x58434, 0x3a5d9, 0x6945a, 0x69459,
 0x3548c, 0x3a5e9, 0x69457, 0x72df1, 0x6945e, 0x6a35e, 0x3e701, 0xb6168,
 0x5b0dd, 0x3a5de, 0x6a3c2, 0xd4278, 0x6a3cc, 0x72dfd, 0xb6165, 0x16009a,
 0x7cbb1, 0xd427c, 0xb6162, 0xe765e, 0x1cecbe, 0x7cbb6, 0x69454, 0xb6160,
 0xd427a, 0x1d5d96, 0xb1d6d, 0xe59f4, 0x72de8, 0x3a5db, 0x0007a, 0x006ae,
 0x01c3c, 0x03aba, 0x058e9, 0x072cc, 0x0d2dd, 0x0d22d, 0x0eec1, 0x0eedb,
 0x1d2a2, 0x1ef5b, 0x357e2, 0x3abbf, 0x1d2f9, 0x35004, 0x3a5dc, 0x351fc,
 0x3976c, 0x6a3c6, 0x6a3cb, 0x3e5ea, 0xe59f3, 0x6a3ce, 0x69452, 0xe59f0,
 0x74b90, 0xd4279, 0xd427b, 0x7cbb5, 0x5b0c5, 0x3a5e3, 0x3a5e2, 0x000d0,
 0x00775, 0x01efe, 0x03dd5, 0x0728c, 0x07cb9, 0x0e1a2, 0x0ea85, 0x0eed8,
 0x1a30a, 0x1aa4f, 0x3a5df, 0x35008, 0x3a5e0, 0x3e5f4, 0x3e5f7, 0xb1d6c,
 0x5843e, 0x34a70, 0x72df8, 0x74b6b, 0xd427f, 0x72df0, 0x5b0bf, 0x5b0c0,
 0xd46b0, 0x72def, 0xe59f8, 0x162e64, 0xb1d6f, 0x3a5e0, 0x39427, 0x69166,
 0x6a3e2, 0x6a3e3, 0x74a8d, 0xd427e, 0x1d5d97, 0xd46b4, 0x5b0d8, 0x6a3d3,
 0x000e0, 0x00b63, 0x034cc, 0x06a33, 0x073c9, 0x0e1a0, 0x0f7fd, 0x0f9cc,
 0x1617d, 0x1caeb, 0x1f4a9, 0x3abb3, 0x69450, 0x39420, 0x39777, 0x3e5e0,
 0x6a3d4, 0x6a3ed, 0xb6166, 0xe59f1, 0xb1d6e, 0xe5676, 0x6a3ea, 0xe5674,
 0xb6163, 0xd46b7, 0x7cba6, 0xd46ba, 0x1d5d94, 0xb6164, 0x6a3f1, 0x7cba2,
 0x69451, 0x72dfa, 0xd46bb, 0x72df7, 0x74b94, 0x1cecbf, 0xe59fa, 0x16009b,
 0x6a3e4, 0x000e6, 0x00e94, 0x03876, 0x070ef, 0x0d52a, 0x16015, 0x16014,
 0x1abf9, 0x1cf17, 0x34a79, 0x34650, 0x3e705, 0x6a3d0, 0x58430, 0x74b9d,
 0x7be7e, 0x5b0be, 0x39773, 0x6a3de, 0x000fb, 0x00f7b, 0x03dd7, 0x07bd0,
 0x0e59c, 0x0f9cd, 0x1cf18, 0x1d2ff, 0x34a7a, 0x39429, 0x3500c, 0x72de0,
 0x69456, 0x7be7c, 0xd46b5, 0xd46b2, 0x6a3dd, 0x001a2, 0x0163b, 0x06913,
 0x0b016, 0x0fa42, 0x1a32d, 0x1cf06, 0x34a7c, 0x34a7d, 0xb6161, 0x35481,
 0x3e5fa, 0x7cba0, 0x7be7f, 0x7cba3, 0x7cba7, 0x5b0d3, 0x72de6, 0x6a3dc,
 0x001a9, 0x01ab4, 0x06a34, 0x0d46a, 0x16130, 0x1ef5f, 0x1f532, 0x1f536,
 0x3942e, 0x58436, 0x6a3db, 0x6945b, 0x001c9, 0x01ca0, 0x0728b, 0x0eed9,
 0x1f539, 0x1ca1d, 0x39765, 0x39766, 0x58439, 0x6945d, 0x39767, 0x001d3,
 0x01f2c, 0x07bfc, 0x16161, 0x34652, 0x3a5ed, 0x3548d, 0x58438, 0x6a3da,
 0x002c1, 0x02c5e, 0x0d335, 0x1ab1a, 0x2d874, 0x35006, 0x35484, 0x5b0cc,
 0x74b9a, 0x72df3, 0x6a3d6, 0x002da, 0x034b3, 0x0d5ae, 0x1caee, 0x2d871,
 0x357e3, 0x74b97, 0x72df9, 0x580ba, 0x5b0d4, 0x0034d, 0x0354e, 0x0f750,
 0x1cbc0, 0x3a5e7, 0x3a5e4, 0x00385, 0x03a58, 0x16c41, 0x2c5cf, 0x3e5e1,
 0x74b6c, 0xe5677, 0x6a3df, 0x00390, 0x03e50, 0x163c2, 0x2d876, 0x35482,
 0x5b0d6, 0x5843a, 0x0039f, 0x0585e, 0x1a583, 0x3500f, 0x74b93, 0x39771,
 0x003e4, 0x06912, 0x16c43, 0x357e1, 0x0058a, 0x0696f, 0x1f538, 0x5b0c9,
 0x6a3cf, 0x005b6, 0x06af8, 0x1f534, 0x58483, 0x6a3e0, 0x00695, 0x07d02,
 0x1cae8, 0x58485, 0x006a2, 0x0754a, 0x357ee, 0x3977b, 0x00748, 0x074b2,
 0x34a7b, 0x00729, 0x0b1e0, 0x34649, 0x3e5e3, 0x0073d, 0x0d2c4, 0x3e5e6,
 0x007bb, 0x0b099, 0x39762, 0x5b0ce, 0x6945f, 0x007d1, 0x0d5ab, 0x39779,
 0x007d3, 0x0d52f, 0x39763, 0x6945c, 0x00b1a, 0x0d2c5, 0x35489, 0x00d23,
 0x0eaed, 0x3e5f8, 0x00d32, 0x16016, 0x3e5fb, 0x00d41, 0x0e768, 0x3a5ed,
 0x00e1f, 0x16017, 0x58027, 0x00ead, 0x0fa07, 0x69455, 0x00e54, 0x1612b,
 0x00e55, 0x1a581, 0x00f78, 0x1a32b, 0x580bc, 0x6a3ee, 0x00f79, 0x1abfd,
 0x00f95, 0x1ab18, 0x6a3f0, 0x01637, 0x1aa4d, 0x0162d, 0x1f53c, 0x6a3f3,
 0x01a31, 0x1a810, 0x39769, 0x01a50, 0x1caef, 0x01a36, 0x1a32e, 0x01a67,
 0x1f38e, 0x01a85, 0x1ef59, 0x01aa6, 0x1ef83, 0x01d51, 0x2c012, 0x01d53,
 0x2d879, 0x01d5e, 0x35005, 0x01cba, 0x1cf04, 0x69453, 0x01d2d, 0x351ff,
 0x01f2d, 0x2d86f, 0x01f29, 0x35007, 0x02c22, 0x351fa, 0x02c03, 0x3a5ec,
 0x02c5f, 0x3a5eb, 0x02c58, 0x34a6b, 0x03469, 0x356be, 0x02c59, 0x34a6c,
 0x0346a, 0x3a5ea, 0x034bd, 0x034bf, 0x356bf, 0x0386a, 0x03ab9, 0x5843f,
 0x0386b, 0x3a5f5, 0x03a4b, 0x39421, 0x03aa4, 0x3a5e9, 0x03a5a, 0x03960,
 0x3977e, 0x03de9, 0x03958, 0x03df7, 0x039e1, 0x3e5e4, 0x0395f, 0x69458,
 0x03e91, 0x03df2, 0x39428, 0x058f2, 0x03e80, 0x6a3c3, 0x03e93, 0x694c0,
 0x058b8, 0x5b0ca, 0x0584f, 0x694c1, 0x058f1, 0x068d6, 0x06a10, 0x06ac3,
 0x06a32, 0x070d2, 0x06911, 0x074b1, 0x07494, 0x06ad4, 0x06ad6, 0x072b8,
 0x06afa, 0x074b3, 0x07540, 0x073ce, 0x0b005, 0x074b3, 0x07495, 0x074b9,
 0x0d336, 0x07bff, 0x07763, 0x073c8, 0x07d29, 0x0b622, 0x0d221, 0x0d181,
 0x0b1d1, 0x074b8, 0x0b1d0, 0x0d19b, 0x0d2c3, 0x0b172, 0x0d2dc, 0x0b623,
 0x0d5aa, 0x0d426, 0x0d182, 0x0e795, 0x0e1d1, 0x0d337, 0x0e96c, 0x0e5e4,
 0x0e514, 0x0eaee, 0x16000, 0x0e767, 0x0e1a1, 0x0e78f, 0x16004, 0x0f7c2,
 0x0e799, 0x0e5e7, 0x0e566, 0x0e769, 0x0f751, 0x0eede, 0x0fa06, 0x16005,
 0x0fa9f, 0x1a5e6, 0x0e766, 0x1636f, 0x0eedd, 0x0eec0, 0x1a309, 0x1ceca,
 0x163cd, 0x0f9cb, 0x0eedf, 0x1a582, 0x1612d, 0x0e5e5, 0x1abf8, 0x1a30c,
 0x1ca1f, 0x163cc, 0x1a35c, 0x1ca1e, 0x1aa51, 0x163ac, 0x1a84e, 0x1a53f,
 0x1cf16, 0x1d2fc, 0x1a5b3, 0x1ab19, 0x1a81f, 0x1d5c3, 0x16c3f, 0x1d5c1,
 0x1d2fc, 0x1f4aa, 0x1a812, 0x1f535, 0x1cf12, 0x1a817, 0x1617c, 0x1ab0b,
 0x1d2f8, 0x1ef82, 0x2d87a, 0x1d52f, 0x1f530, 0x1aa48, 0x35487, 0x1d2fd,
 0x1f4ad, 0x1cf11, 0x3461b, 0x35485, 0x1ca20, 0x1caed, 0x1cae6, 0x1abff,
 0x3464f, 0x34a6f, 0x1ef81, 0x3464b, 0x39d96, 0x1f383, 0x1f537, 0x1cf14,
 0x2c5ce, 0x3500e, 0x2c251, 0x1caec, 0x1f387, 0x34654, 0x357e4, 0x2d878,
 0x3500b, 0x35480, 0x3a5e8, 0x3548e, 0x34b64, 0x1f4a8, 0x35003, 0x3e5df,
 0x2d870, 0x357e6, 0x3e5f0, 0x1ef5a, 0x3a5ea, 0x1f388, 0x3e703, 0x2c24e,
 0x3a5e2, 0x351fd, 0x2c6dd, 0x3e704, 0x351fe, 0x2d875, 0x5b0c7, 0x3976a,
 0x3a5e6, 0x39423, 0x58480, 0x2c246, 0x3a5e3, 0x2d877, 0x3e5f1, 0x3abbe,
 0x58489, 0x3e5f9, 0x357e0, 0x3abbc, 0x5b0c6, 0x69167, 0x69165, 0x3e5e9,
 0x39422, 0x3976f, 0x3977d, 0x3e5de, 0x6a3c9, 0x58b98, 0x3a5f6, 0x3a5d0,
 0x58486, 0x6a3c1, 0x3e5fc, 0x5b0dc, 0x3548f, 0x3942d, 0x694c9, 0x58484,
 0x3a5e8, 0x74b9b, 0x74b96, 0x694d0, 0x58488, 0x3a5e4, 0x3942a, 0x72ec2,
 0x39776, 0x5b0d1, 0x5b0cf, 0x3a5d6, 0xe59fc, 0x5b0c8, 0x3e5e7, 0x7cbb7,
 0x70e87, 0x7cbab, 0x5b0c2, 0x694c3, 0x74a8e, 0x3e5f3, 0x6a3cd, 0x72dfe,
 0x73b2e, 0x72ec0, 0x694c5, 0x58437, 0x694c8, 0x72dff, 0x39435, 0x5843d,
 0x6a3d7, 0x72ec1, 0xd22c8, 0x694cf, 0xb6173, 0x3e5fe, 0x580bb, 0xe59f2,
 0xb616e, 0xb6175, 0x3a5da, 0x5b0bd, 0x694cc, 0x5843c, 0x694c7, 0x74b92,
 0x72ec3, 0x694c6, 0xb6170, 0x7cbac, 0xb1733, 0x7cba4, 0xb6167, 0x72de7,
 0x72de4, 0x6a3c0, 0x3e5ef, 0x162e65, 0x72de3, 0x72dfb, 0x6a35f, 0x6a3eb,
};

static const uint8_t coef2_huffbits[1336] = {
 11,  9,  2,  3,  4,  4,  5,  6,
  6,  7,  7,  8,  8,  8,  9,  9,
  9,  9, 10, 10, 10, 10, 11, 11,
 11, 11, 11, 11, 11, 12, 12, 12,
 12, 12, 12, 12, 12, 12, 13, 13,
 13, 13, 13, 13, 13, 13, 13, 14,
 14, 14, 14, 14, 14, 14, 14, 14,
 14, 14, 14, 14, 14, 15, 15, 15,
 15, 15, 15, 15, 15, 15, 15, 15,
 15, 15, 15, 15, 15, 16, 15, 16,
 16, 16, 16, 16, 16, 16, 16, 16,
 16, 16, 16, 16, 16, 16, 16, 16,
 17, 17, 17, 17, 17, 17, 17, 17,
 17, 17, 17, 18, 17, 17, 17, 17,
 17, 17, 17, 18, 18, 17, 17, 18,
 17, 17, 18, 17, 18, 18, 18, 18,
 19, 18, 18, 18, 18, 18, 18, 20,
 18, 18, 18, 19, 19, 18, 19, 18,
 19, 19, 18, 19, 19, 18, 19, 19,
 19, 19, 18, 19, 19, 19, 19, 19,
 19, 19, 20, 20, 20, 19, 19, 20,
 19, 20, 19, 19, 20, 19, 19, 20,
 20, 20, 20, 19, 20, 21, 19,  3,
  5,  7,  8,  9,  9, 10, 11, 11,
 12, 12, 12, 13, 13, 13, 13, 14,
 14, 14, 14, 15, 15, 15, 15, 15,
 15, 15, 15, 15, 15, 15, 16, 16,
 15, 15, 15, 15, 16, 16, 16, 16,
 17, 16, 17, 17, 16, 17, 17, 17,
 17, 17, 17, 16, 17, 17, 17, 17,
 18, 17, 17, 18, 18, 18, 18, 18,
 19, 18, 18, 18, 18, 18, 18, 19,
 19, 18, 18, 18, 18, 19, 18, 19,
 19, 19, 20, 19, 18, 19, 19, 19,
 19, 19, 19, 19, 19, 19, 19, 20,
 20, 19, 20, 19, 20, 19, 20, 19,
 19, 21, 20, 20, 19,  4,  7,  8,
 10, 11, 11, 12, 12, 13, 13, 14,
 14, 14, 14, 15, 15, 15, 15, 15,
 16, 16, 16, 16, 16, 16, 16, 17,
 17, 17, 17, 17, 17, 17, 16, 16,
 16, 16, 17, 17, 17, 17, 18, 18,
 18, 17, 17, 18, 18, 18, 18, 18,
 18, 18, 18, 18, 19, 18, 18, 18,
 19, 18, 19, 19, 19, 20, 20, 20,
 19, 19, 19, 19, 19, 19, 19, 21,
 21, 20, 19,  5,  8, 10, 11, 12,
 13, 13, 13, 14, 14, 15, 15, 15,
 15, 16, 16, 16, 16, 16, 17, 17,
 17, 17, 17, 17, 17, 17, 18, 17,
 18, 17, 17, 17, 17, 17, 17, 17,
 17, 17, 17, 17, 19, 18, 19, 18,
 18, 18, 18, 18, 19, 18, 17, 17,
 18, 18, 19, 19, 19, 19, 18, 18,
 18, 19,  6,  9, 11, 12, 13, 13,
 14, 14, 14, 15, 15, 16, 16, 16,
 16, 16, 16, 17, 17, 17, 18, 18,
 18, 18, 18, 18, 18, 18, 18, 18,
 18, 17, 18, 18, 17, 18, 18, 18,
 18, 18, 18, 19, 19, 18, 18, 18,
 19, 19, 19, 20, 19, 19, 18, 19,
 19, 20, 21, 21, 19, 19, 18,  6,
 10, 12, 13, 14, 14, 14, 15, 15,
 15, 16, 16, 17, 17, 17, 17, 17,
 17, 17, 18, 18, 19, 18, 18, 18,
 19, 18, 18, 18, 19, 18, 18, 18,
 18, 18, 18, 18, 18, 18, 18, 18,
 19, 20, 20, 19, 19, 19, 19, 20,
 20, 19, 20, 19, 19, 19, 20, 20,
 20, 19, 19, 18, 19,  7, 10, 12,
 13, 14, 15, 15, 15, 16, 16, 17,
 17, 17, 17, 17, 17, 18, 18, 18,
 18, 19, 18, 19, 19, 19, 20, 19,
 18, 19, 19, 18, 18, 19, 19, 19,
 18, 19, 19, 20, 19, 18, 20, 21,
 20, 20, 19, 19, 21, 20, 21, 20,
 20, 20, 19, 19, 20, 20, 21, 20,
 19,  7, 11, 13, 14, 15, 15, 15,
 16, 16, 17, 17, 17, 17, 18, 18,
 18, 18, 18, 19, 20, 19, 19, 20,
 19, 19, 19, 19, 19, 19, 19, 19,
 18, 18, 19, 20, 19, 19, 19, 20,
 19, 19, 19, 20, 19, 20, 20, 21,
 20, 20, 20, 21, 22, 20, 19, 20,
 20, 21, 20, 21, 20, 19,  8, 11,
 13, 14, 15, 16, 16, 16, 17, 17,
 17, 18, 18, 18, 18, 18, 19, 18,
 19, 19, 19, 19, 21, 19, 19, 21,
 19, 20, 20, 20, 19, 18, 18,  8,
 12, 14, 15, 16, 16, 16, 16, 17,
 17, 17, 19, 18, 18, 19, 19, 20,
 19, 18, 20, 19, 20, 20, 19, 19,
 20, 20, 21, 21, 20, 19, 19, 19,
 19, 19, 19, 20, 21, 20, 19, 19,
  8, 12, 14, 15, 16, 16, 17, 17,
 17, 18, 18, 18, 19, 19, 19, 19,
 19, 19, 20, 21, 20, 21, 19, 21,
 20, 20, 20, 20, 21, 20, 19, 20,
 19, 20, 20, 20, 19, 22, 21, 21,
 19,  9, 12, 14, 15, 16, 17, 17,
 17, 18, 18, 18, 19, 19, 19, 19,
 20, 19, 19, 19,  9, 13, 15, 16,
 17, 17, 18, 18, 18, 19, 18, 20,
 19, 20, 20, 20, 19,  9, 13, 15,
 16, 17, 17, 18, 18, 18, 20, 18,
 19, 20, 20, 20, 20, 19, 20, 19,
  9, 13, 15, 16, 17, 18, 18, 18,
 19, 19, 19, 19, 10, 14, 16, 17,
 18, 18, 19, 19, 19, 19, 19, 10,
 14, 16, 17, 18, 18, 18, 19, 19,
 10, 14, 16, 17, 18, 18, 18, 19,
 19, 20, 19, 10, 14, 16, 18, 18,
 18, 19, 20, 19, 19, 10, 14, 17,
 18, 18, 18, 10, 15, 17, 18, 19,
 19, 21, 19, 11, 15, 17, 18, 18,
 19, 19, 11, 15, 17, 18, 19, 19,
 11, 15, 17, 18, 11, 15, 18, 19,
 19, 11, 15, 18, 19, 19, 11, 16,
 18, 19, 11, 15, 18, 19, 11, 16,
 18, 12, 16, 18, 19, 12, 16, 19,
 12, 16, 19, 19, 19, 12, 16, 19,
 12, 16, 19, 19, 12, 16, 18, 12,
 16, 19, 12, 17, 19, 12, 17, 19,
 12, 17, 19, 12, 17, 19, 13, 17,
 13, 17, 13, 17, 19, 19, 13, 17,
 13, 17, 19, 13, 17, 13, 18, 19,
 13, 17, 19, 13, 18, 13, 17, 13,
 18, 13, 18, 13, 18, 13, 18, 13,
 18, 13, 18, 14, 18, 19, 14, 18,
 14, 18, 14, 18, 14, 18, 14, 19,
 14, 19, 14, 18, 14, 18, 14, 18,
 14, 19, 14, 14, 18, 14, 14, 19,
 14, 18, 14, 19, 14, 19, 14, 15,
 19, 15, 15, 15, 15, 19, 15, 19,
 15, 15, 19, 15, 15, 19, 15, 19,
 15, 19, 15, 19, 15, 15, 15, 15,
 15, 15, 15, 15, 15, 15, 15, 16,
 15, 15, 15, 16, 16, 16, 15, 16,
 16, 16, 16, 16, 16, 16, 16, 16,
 16, 16, 16, 16, 16, 16, 16, 16,
 16, 16, 16, 17, 16, 16, 16, 17,
 17, 16, 17, 17, 16, 17, 17, 17,
 17, 17, 17, 17, 17, 17, 17, 17,
 17, 17, 17, 17, 17, 17, 17, 18,
 17, 17, 17, 17, 17, 17, 17, 17,
 18, 17, 17, 18, 17, 17, 17, 17,
 18, 18, 17, 17, 17, 17, 17, 17,
 17, 18, 17, 18, 18, 17, 17, 17,
 18, 18, 18, 17, 18, 17, 18, 18,
 18, 18, 18, 18, 18, 18, 18, 17,
 18, 18, 18, 18, 19, 18, 18, 18,
 18, 18, 18, 18, 18, 18, 18, 18,
 18, 18, 18, 18, 18, 18, 18, 19,
 18, 18, 19, 18, 18, 18, 19, 18,
 19, 18, 18, 19, 18, 18, 19, 19,
 19, 19, 19, 18, 19, 18, 19, 18,
 19, 19, 18, 18, 19, 19, 19, 19,
 19, 19, 19, 19, 19, 19, 18, 19,
 19, 19, 19, 19, 18, 19, 19, 19,
 19, 19, 19, 19, 19, 19, 19, 20,
 19, 19, 19, 19, 21, 19, 19, 20,
 19, 20, 19, 19, 19, 19, 19, 20,
 20, 20, 19, 19, 19, 20, 19, 19,
 19, 20, 20, 19, 20, 19, 19, 21,
 20, 20, 19, 19, 19, 19, 19, 19,
 20, 19, 20, 20, 20, 20, 20, 20,
 20, 19, 19, 21, 20, 20, 19, 19,
};

static const uint32_t coef3_huffcodes[1072] = {
 0x001b2, 0x00069, 0x00000, 0x00004, 0x00006, 0x0000e, 0x00014, 0x00019,
 0x00016, 0x0002b, 0x00030, 0x0003d, 0x0003c, 0x0005a, 0x0005f, 0x0006d,
 0x0007e, 0x0005f, 0x0007f, 0x000b6, 0x000bc, 0x000d8, 0x000f2, 0x000fe,
 0x000bc, 0x000fc, 0x00161, 0x0016e, 0x00174, 0x00176, 0x001a2, 0x001e3,
 0x001f3, 0x00174, 0x0017a, 0x001ea, 0x002a8, 0x002c4, 0x002e6, 0x00314,
 0x00346, 0x00367, 0x003e9, 0x002e5, 0x002ee, 0x003d6, 0x00555, 0x00554,
 0x00557, 0x005c3, 0x005d6, 0x006e0, 0x0062f, 0x006e2, 0x00799, 0x00789,
 0x007fa, 0x005ce, 0x007fe, 0x005ec, 0x007cc, 0x007af, 0x00aa7, 0x00b19,
 0x00b94, 0x00b85, 0x00b9f, 0x00c48, 0x00c45, 0x00dd8, 0x00c4c, 0x00c4b,
 0x00d99, 0x00d1f, 0x00dc2, 0x00f95, 0x00fa2, 0x00bb5, 0x00b9f, 0x00f5d,
 0x00bbf, 0x00f47, 0x0154a, 0x00fd5, 0x00f45, 0x00f7f, 0x0160d, 0x01889,
 0x01757, 0x01722, 0x018b3, 0x0172d, 0x01a39, 0x01a18, 0x01bb3, 0x01b30,
 0x01e63, 0x0173c, 0x01b35, 0x01723, 0x01e80, 0x01fee, 0x01761, 0x01ffc,
 0x01f7f, 0x02c7c, 0x01fa1, 0x0177b, 0x01755, 0x0175a, 0x01fa6, 0x02eab,
 0x0310a, 0x02c69, 0x03669, 0x03127, 0x03103, 0x02e43, 0x03662, 0x03165,
 0x03124, 0x0313b, 0x03111, 0x03668, 0x0343b, 0x03c52, 0x03efc, 0x02e6c,
 0x03fda, 0x03ef8, 0x02e7b, 0x03ee2, 0x03cc5, 0x03d72, 0x058c0, 0x03df8,
 0x02ea9, 0x03e7e, 0x0556d, 0x05c82, 0x03d71, 0x03e7b, 0x03c42, 0x058d7,
 0x03f4e, 0x06200, 0x03d70, 0x05cb2, 0x05c96, 0x05cb0, 0x03f45, 0x05cb1,
 0x02e6d, 0x03110, 0x02f68, 0x05c90, 0x07ca6, 0x07c88, 0x06204, 0x062c8,
 0x078a6, 0x07986, 0x079d5, 0x0b1ad, 0x07989, 0x0b079, 0x05cdd, 0x0aad4,
 0x05de8, 0x07dcd, 0x07987, 0x05d67, 0x05d99, 0x0b91d, 0x07cf1, 0x05d9b,
 0x079d7, 0x0b07b, 0x05c85, 0x05d9a, 0x07dcc, 0x07ebf, 0x07dce, 0x07dfb,
 0x07ec0, 0x07d1a, 0x07a07, 0x05c84, 0x0c471, 0x07cf2, 0x0baef, 0x0b9d2,
 0x05deb, 0x07bd6, 0x0b845, 0x05d98, 0x0b91a, 0x0bae8, 0x0c4e0, 0x0dc31,
 0x0f93d, 0x0bbce, 0x0d1d2, 0x0f7a9, 0x0d9b9, 0x0bbcb, 0x0b900, 0x0aad7,
 0x0babd, 0x0c4e1, 0x0f46f, 0x0c588, 0x0c58b, 0x160e6, 0x0bbcf, 0x0bac3,
 0x0f945, 0x0f7a3, 0x0d1c1, 0x0fb8e, 0x0f7a4, 0x0fb8c, 0x0f40c, 0x0c473,
 0x0fd72, 0x0bbcd, 0x0fffa, 0x0f940, 0x0bbc9, 0x0f7a8, 0x1a1ed, 0x0bbc5,
 0x1f26f, 0x163fd, 0x160c7, 0x1a1f5, 0x0f947, 0x163fc, 0x154b3, 0x0fff6,
 0x163f6, 0x160e9, 0x1a1f0, 0x0bab9, 0x0baba, 0x17086, 0x0b903, 0x0fd75,
 0x0f308, 0x176f3, 0x163ff, 0x0fd7d, 0x1bb78, 0x163fb, 0x188db, 0x1a1f7,
 0x154b2, 0x172fd, 0x163f4, 0x1bb73, 0x172ff, 0x0babc, 0x0f97d, 0x1a1f3,
 0x1bb6d, 0x1ffd5, 0x1a1f4, 0x1f272, 0x17380, 0x17382, 0x1ffe7, 0x0bac8,
 0x0bbc4, 0x188d3, 0x160e0, 0x0fd7b, 0x1725f, 0x172f5, 0x1bb79, 0x1fad9,
 0x1f269, 0x188d0, 0x0bac4, 0x0bac5, 0x31185, 0x188d2, 0x188cc, 0x31187,
 0x3e7fe, 0x188d1, 0x1bb6c, 0x1f268, 0x1fad2, 0x1ffd9, 0x1a1ea, 0x1bb68,
 0x1facb, 0x3fdb2, 0x1e81a, 0x188ce, 0x172fb, 0x1a1ef, 0x1face, 0x1bb70,
 0x0bac1, 0x1bb6b, 0x172f8, 0x1bb66, 0x1ffdf, 0x1bb6a, 0x1ffd7, 0x1f266,
 0x176f8, 0x37653, 0x1fa7e, 0x31182, 0x1fac8, 0x2c7e3, 0x370ee, 0x176ec,
 0x176e9, 0x2e4bc, 0x160c5, 0x3765a, 0x3ce9c, 0x17373, 0x176e8, 0x188d4,
 0x176f1, 0x176ef, 0x37659, 0x1bb7c, 0x1ffde, 0x176f2, 0x3118b, 0x2c7d4,
 0x37651, 0x5ce9f, 0x37650, 0x31191, 0x3f4f6, 0x3f4f5, 0x7a06c, 0x1fac1,
 0x5c97b, 0x2c7e0, 0x79d3a, 0x3e7fd, 0x2c7df, 0x3f4f0, 0x7a06d, 0x376c1,
 0x79d3b, 0x00004, 0x00014, 0x00059, 0x000ab, 0x000b8, 0x00177, 0x001f5,
 0x001f2, 0x00315, 0x003fc, 0x005bd, 0x0062d, 0x006e8, 0x007dd, 0x00b04,
 0x007cd, 0x00b1e, 0x00d1e, 0x00f15, 0x00f3b, 0x00f41, 0x01548, 0x018b0,
 0x0173b, 0x01884, 0x01a1c, 0x01bb4, 0x01f25, 0x017b5, 0x0176d, 0x01ef8,
 0x02e73, 0x03107, 0x03125, 0x03105, 0x02e49, 0x03ce8, 0x03ef9, 0x03e5e,
 0x02e72, 0x03471, 0x03fd9, 0x0623f, 0x078a0, 0x06867, 0x05cb3, 0x06272,
 0x068ec, 0x06e9a, 0x079d4, 0x06e98, 0x0b1aa, 0x06e1a, 0x07985, 0x068ee,
 0x06e9b, 0x05c88, 0x0b1ac, 0x07dfa, 0x05d65, 0x07cf0, 0x07cbf, 0x0c475,
 0x160eb, 0x1bb7e, 0x0f7a6, 0x1fedd, 0x160e3, 0x0fffb, 0x0fb8d, 0x0fff9,
 0x0d1c0, 0x0c58c, 0x1a1e9, 0x0bab8, 0x0f5cf, 0x0fff5, 0x376c5, 0x1a1ec,
 0x160ed, 0x1fede, 0x1fac9, 0x1a1eb, 0x1f224, 0x176ee, 0x0fd79, 0x17080,
 0x17387, 0x1bb7a, 0x1ffe9, 0x176f7, 0x17385, 0x17781, 0x2c7d5, 0x17785,
 0x1ffe3, 0x163f5, 0x1fac2, 0x3e7f9, 0x3118d, 0x3fdb1, 0x1ffe2, 0x1f226,
 0x3118a, 0x2c7d9, 0x31190, 0x3118c, 0x3f4f3, 0x1bb7f, 0x1bb72, 0x31184,
 0xb92f4, 0x3e7fb, 0x6e1d9, 0x1faca, 0x62300, 0x3fdb8, 0x3d037, 0x3e7fc,
 0x62301, 0x3f4f2, 0x1f26a, 0x0000e, 0x00063, 0x000f8, 0x001ee, 0x00377,
 0x003f7, 0x006e3, 0x005cc, 0x00b05, 0x00dd2, 0x00fd4, 0x0172e, 0x0172a,
 0x01e23, 0x01f2d, 0x01763, 0x01769, 0x0176c, 0x02e75, 0x03104, 0x02ec1,
 0x03e58, 0x0583f, 0x03f62, 0x03f44, 0x058c5, 0x0623c, 0x05cf4, 0x07bd7,
 0x05d9d, 0x0aad2, 0x05d66, 0x0b1a9, 0x0b078, 0x07cfe, 0x0b918, 0x0c46f,
 0x0b919, 0x0b847, 0x06e1b, 0x0b84b, 0x0aad8, 0x0fd74, 0x172f4, 0x17081,
 0x0f97c, 0x1f273, 0x0f7a0, 0x0fd7c, 0x172f7, 0x0fd7a, 0x1bb77, 0x172fe,
 0x1f270, 0x0fd73, 0x1bb7b, 0x1a1bc, 0x1bb7d, 0x0bbc3, 0x172f6, 0x0baeb,
 0x0fb8f, 0x3f4f4, 0x3fdb4, 0x376c8, 0x3e7fa, 0x1ffd0, 0x62303, 0xb92f5,
 0x1f261, 0x31189, 0x3fdb5, 0x2c7db, 0x376c9, 0x1fad6, 0x1fad1, 0x00015,
 0x000f0, 0x002e0, 0x0058e, 0x005d7, 0x00c4d, 0x00fa1, 0x00bdb, 0x01756,
 0x01f70, 0x02c19, 0x0313c, 0x0370f, 0x03cc0, 0x02ea8, 0x058c6, 0x058c7,
 0x02eb7, 0x058d0, 0x07d18, 0x0aa58, 0x0b848, 0x05d9e, 0x05d6c, 0x0b84c,
 0x0c589, 0x0b901, 0x163f8, 0x0bac9, 0x0b9c5, 0x0f93c, 0x188d8, 0x0bbc7,
 0x160ec, 0x0fd6f, 0x188d9, 0x160ea, 0x0f7a7, 0x0f944, 0x0baab, 0x0dc3a,
 0x188cf, 0x176fb, 0x2c7d8, 0x2c7d7, 0x1bb75, 0x5ce9e, 0x62302, 0x370ed,
 0x176f4, 0x1ffd1, 0x370ef, 0x3f4f8, 0x376c7, 0x1ffe1, 0x376c6, 0x176ff,
 0x6e1d8, 0x176f6, 0x17087, 0x0f5cd, 0x00035, 0x001a0, 0x0058b, 0x00aac,
 0x00b9a, 0x0175f, 0x01e22, 0x01e8c, 0x01fb2, 0x0310b, 0x058d1, 0x0552e,
 0x05c27, 0x0686e, 0x07ca7, 0x0c474, 0x0dc33, 0x07bf2, 0x05de9, 0x07a35,
 0x0baaa, 0x0b9eb, 0x0fb95, 0x0b9b8, 0x17381, 0x1f262, 0x188cd, 0x17088,
 0x172fa, 0x0f7a2, 0x1fad3, 0x0bac0, 0x3765c, 0x1fedf, 0x1f225, 0x1fad4,
 0x2c7da, 0x5ce9d, 0x3e7f8, 0x1e203, 0x188d7, 0x00054, 0x002c0, 0x007a1,
 0x00f78, 0x01b36, 0x01fa3, 0x0313a, 0x03436, 0x0343a, 0x07d1d, 0x07bd8,
 0x05cdf, 0x0b846, 0x0b189, 0x0d9b8, 0x0fff8, 0x0d9be, 0x0c58a, 0x05dea,
 0x0d1d3, 0x160e4, 0x1f26b, 0x188da, 0x1e202, 0x2c7d2, 0x163fe, 0x31193,
 0x17782, 0x376c2, 0x2c7d1, 0x3fdb0, 0x3765d, 0x2c7d0, 0x1fad0, 0x1e201,
 0x188dd, 0x2c7e2, 0x37657, 0x37655, 0x376c4, 0x376c0, 0x176ea, 0x0006f,
 0x003cf, 0x00dd5, 0x01f23, 0x02c61, 0x02ed0, 0x05d54, 0x0552d, 0x07883,
 0x0b1a8, 0x0b91c, 0x0babf, 0x0b902, 0x0f7aa, 0x0f7a5, 0x1a1e8, 0x1ffd6,
 0x0babe, 0x1a1bf, 0x163f3, 0x1ffd8, 0x1fad7, 0x1f275, 0x1ffdc, 0x0007d,
 0x005bc, 0x01549, 0x02a99, 0x03def, 0x06273, 0x079d6, 0x07d1b, 0x0aad3,
 0x0d0fc, 0x2c7dd, 0x188d6, 0x0bac2, 0x2c7e1, 0x1bb76, 0x1a1bd, 0x31186,
 0x0fd78, 0x1a1be, 0x31183, 0x3fdb6, 0x3f4f1, 0x37652, 0x1fad5, 0x3f4f9,
 0x3e7ff, 0x5ce9c, 0x3765b, 0x31188, 0x17372, 0x000bd, 0x0078b, 0x01f21,
 0x03c43, 0x03ded, 0x0aad6, 0x07ec1, 0x0f942, 0x05c86, 0x17089, 0x0babb,
 0x1ffe8, 0x2c7de, 0x1f26e, 0x1fac4, 0x3f4f7, 0x37656, 0x1fa7d, 0x376c3,
 0x3fdb3, 0x3118f, 0x1fac6, 0x000f8, 0x007ed, 0x01efd, 0x03e7a, 0x05c91,
 0x0aad9, 0x0baec, 0x0dc32, 0x0f46e, 0x1e200, 0x176fa, 0x3765e, 0x3fdb7,
 0x2c7d6, 0x3fdb9, 0x37654, 0x37658, 0x3118e, 0x1ffdb, 0x000f6, 0x00c43,
 0x03106, 0x068ef, 0x0b84d, 0x0b188, 0x0bbcc, 0x1f264, 0x1bb69, 0x17386,
 0x1fac0, 0x00171, 0x00f39, 0x03e41, 0x068ed, 0x0d9bc, 0x0f7a1, 0x1bb67,
 0x1ffdd, 0x176f9, 0x001b9, 0x00f7d, 0x03f63, 0x0d0fd, 0x0b9ea, 0x188dc,
 0x1fac3, 0x1a1f2, 0x31192, 0x1ffe4, 0x001f6, 0x01754, 0x06865, 0x0f309,
 0x160e5, 0x176f5, 0x3765f, 0x1facc, 0x001e9, 0x01a1a, 0x06201, 0x0f105,
 0x176f0, 0x002df, 0x01756, 0x05d6d, 0x163fa, 0x176ed, 0x00342, 0x02e40,
 0x0d0ff, 0x17082, 0x003cd, 0x02a98, 0x0fffc, 0x2c7dc, 0x1fa7f, 0x003fe,
 0x03764, 0x0fffd, 0x176fc, 0x1fac5, 0x002f7, 0x02ed1, 0x0fb97, 0x0058a,
 0x02edc, 0x0bbc8, 0x005d4, 0x0623d, 0x160e8, 0x0062e, 0x05830, 0x163f9,
 0x006eb, 0x06205, 0x1f274, 0x007de, 0x062c9, 0x1f265, 0x005c9, 0x05cde,
 0x1ffd3, 0x005d4, 0x07988, 0x007ce, 0x0b849, 0x00b1b, 0x05c89, 0x1fac7,
 0x00b93, 0x05c83, 0x00b9e, 0x0f14f, 0x00c4a, 0x0b9c7, 0x00dd4, 0x0c470,
 0x1f271, 0x00f38, 0x0fb96, 0x176eb, 0x00fa0, 0x163f7, 0x00bb2, 0x0b91b,
 0x00bbe, 0x0f102, 0x00f44, 0x0f946, 0x1facd, 0x00f79, 0x0d9bd, 0x0154d,
 0x0bbc6, 0x00fd2, 0x160e7, 0x0172b, 0x188cb, 0x0175e, 0x0fd76, 0x0175c,
 0x1bb71, 0x0189f, 0x1a1ee, 0x01f24, 0x1a1f6, 0x01ba7, 0x0bbca, 0x01f7d,
 0x0ffff, 0x01f2e, 0x1bb65, 0x01bb5, 0x172f9, 0x01fef, 0x1f26c, 0x01f3e,
 0x0fd77, 0x01762, 0x1bb6e, 0x01ef9, 0x172fc, 0x01fa0, 0x02ab7, 0x02e4a,
 0x1f267, 0x01fb3, 0x1ffda, 0x02e42, 0x03101, 0x17780, 0x0313d, 0x03475,
 0x17784, 0x03126, 0x1facf, 0x03c51, 0x17783, 0x03e40, 0x1ffe5, 0x03663,
 0x1ffe0, 0x03e8f, 0x1f26d, 0x0343c, 0x03cc1, 0x176fd, 0x03e45, 0x02ec0,
 0x03f61, 0x03dee, 0x03fd8, 0x0583e, 0x02e45, 0x03e59, 0x03d02, 0x05ce8,
 0x05568, 0x176fe, 0x02f69, 0x1fad8, 0x058c1, 0x05c83, 0x1ffe6, 0x06271,
 0x06e1c, 0x062c7, 0x068e1, 0x0552f, 0x06864, 0x06866, 0x06e99, 0x05cbc,
 0x07ca5, 0x078a1, 0x05c82, 0x07dcf, 0x0623b, 0x0623e, 0x068e8, 0x07a36,
 0x05d9c, 0x0b077, 0x07cf3, 0x07a34, 0x07ca4, 0x07d19, 0x079d2, 0x07d1c,
 0x07bd9, 0x0b84a, 0x0fb94, 0x0aad5, 0x0dc30, 0x07bf3, 0x0baee, 0x0b07a,
 0x0c472, 0x0b91e, 0x0d9ba, 0x05d9f, 0x0d0fe, 0x0b9c6, 0x05c87, 0x0f14e,
 0x0baed, 0x0b92e, 0x0f103, 0x0b9c4, 0x0fb91, 0x0d9bb, 0x0b1ab, 0x0c58d,
 0x0fffe, 0x0f93b, 0x0f941, 0x0baea, 0x0b91f, 0x0f5cc, 0x0d9bf, 0x0f943,
 0x0f104, 0x1f260, 0x0fb92, 0x0f93f, 0x0f3a6, 0x0bac7, 0x0f7ab, 0x0bac6,
 0x17383, 0x0fd6d, 0x0bae9, 0x0fd6e, 0x1e74f, 0x188ca, 0x1f227, 0x0fb93,
 0x0fb90, 0x0fff7, 0x17085, 0x17083, 0x160e1, 0x17084, 0x0f93e, 0x160e2,
 0x160c6, 0x1a1f1, 0x1bb6f, 0x17384, 0x0fd70, 0x1f263, 0x188d5, 0x173a6,
 0x0f5ce, 0x163f2, 0x0fd71, 0x1ffd2, 0x160c4, 0x1ffd4, 0x2c7d3, 0x1bb74,
};

static const uint8_t coef3_huffbits[1072] = {
  9,  7,  2,  3,  4,  4,  5,  5,
  6,  6,  6,  6,  7,  7,  7,  7,
  7,  8,  8,  8,  8,  8,  8,  8,
  9,  9,  9,  9,  9,  9,  9,  9,
  9, 10, 10, 10, 10, 10, 10, 10,
 10, 10, 10, 11, 11, 11, 11, 11,
 11, 11, 11, 11, 11, 11, 11, 11,
 11, 12, 11, 12, 12, 12, 12, 12,
 12, 12, 12, 12, 12, 12, 12, 12,
 12, 12, 12, 12, 12, 13, 13, 13,
 13, 13, 13, 13, 13, 13, 13, 13,
 13, 13, 13, 13, 13, 13, 13, 13,
 13, 14, 13, 14, 14, 13, 14, 13,
 13, 14, 14, 14, 14, 14, 14, 14,
 14, 14, 14, 14, 14, 14, 14, 14,
 14, 14, 14, 14, 14, 14, 14, 15,
 14, 14, 15, 14, 14, 15, 15, 15,
 15, 15, 15, 15, 15, 15, 14, 15,
 15, 15, 15, 15, 15, 15, 15, 15,
 15, 14, 15, 15, 15, 15, 15, 15,
 15, 15, 15, 16, 15, 16, 16, 16,
 16, 15, 15, 16, 16, 16, 16, 16,
 15, 16, 16, 16, 15, 16, 15, 15,
 16, 15, 16, 16, 16, 16, 16, 16,
 16, 16, 16, 16, 16, 16, 16, 16,
 16, 17, 16, 17, 16, 17, 17, 16,
 17, 16, 17, 16, 16, 17, 17, 17,
 16, 17, 16, 16, 17, 16, 17, 16,
 17, 17, 16, 16, 17, 17, 17, 17,
 17, 17, 17, 17, 16, 17, 17, 16,
 17, 17, 17, 17, 17, 17, 17, 17,
 16, 18, 17, 17, 17, 17, 17, 17,
 17, 17, 17, 17, 17, 17, 16, 17,
 17, 17, 17, 17, 17, 17, 17, 17,
 17, 17, 17, 17, 17, 17, 17, 18,
 17, 17, 17, 17, 18, 17, 17, 18,
 19, 17, 17, 17, 18, 17, 17, 17,
 18, 18, 18, 17, 17, 17, 18, 17,
 17, 17, 17, 17, 17, 17, 17, 17,
 18, 18, 18, 18, 18, 18, 18, 18,
 18, 18, 17, 18, 18, 18, 18, 17,
 18, 18, 18, 17, 17, 18, 18, 18,
 18, 19, 18, 18, 19, 19, 20, 18,
 19, 18, 19, 19, 18, 19, 20, 18,
 19,  4,  6,  7,  8,  9,  9,  9,
 10, 10, 10, 11, 11, 11, 11, 12,
 12, 12, 12, 12, 12, 13, 13, 13,
 13, 13, 13, 13, 13, 14, 14, 14,
 14, 14, 14, 14, 14, 14, 14, 14,
 14, 14, 14, 15, 15, 15, 15, 15,
 15, 15, 15, 15, 16, 15, 15, 15,
 15, 16, 16, 15, 16, 16, 15, 16,
 17, 17, 17, 17, 17, 16, 16, 16,
 16, 16, 17, 17, 17, 16, 18, 17,
 17, 17, 18, 17, 17, 18, 17, 17,
 17, 17, 17, 18, 17, 18, 18, 18,
 17, 17, 18, 19, 18, 18, 17, 17,
 18, 18, 18, 18, 19, 17, 17, 18,
 20, 19, 19, 18, 19, 18, 19, 19,
 19, 19, 17,  5,  7,  9, 10, 10,
 11, 11, 12, 12, 12, 13, 13, 13,
 13, 13, 14, 14, 14, 14, 14, 15,
 14, 15, 15, 15, 15, 15, 16, 16,
 16, 16, 16, 16, 16, 16, 16, 16,
 16, 16, 15, 16, 16, 17, 17, 17,
 16, 17, 17, 17, 17, 17, 17, 17,
 17, 17, 17, 17, 17, 17, 17, 16,
 16, 19, 18, 18, 19, 17, 19, 20,
 17, 18, 18, 18, 18, 18, 18,  6,
  8, 10, 11, 12, 12, 12, 13, 13,
 13, 14, 14, 14, 14, 15, 15, 15,
 15, 15, 15, 16, 16, 16, 16, 16,
 16, 17, 17, 17, 16, 16, 17, 17,
 17, 17, 17, 17, 17, 16, 16, 16,
 17, 18, 18, 18, 17, 19, 19, 18,
 18, 17, 18, 19, 18, 17, 18, 18,
 19, 18, 17, 17,  6,  9, 11, 12,
 13, 13, 13, 14, 14, 14, 15, 15,
 15, 15, 15, 16, 16, 16, 16, 16,
 16, 17, 16, 17, 17, 17, 17, 17,
 17, 17, 18, 17, 18, 17, 17, 18,
 18, 19, 19, 17, 17,  7, 10, 12,
 13, 13, 14, 14, 14, 14, 15, 16,
 16, 16, 16, 16, 16, 16, 16, 16,
 16, 17, 17, 17, 17, 18, 17, 18,
 18, 18, 18, 18, 18, 18, 18, 17,
 17, 18, 18, 18, 18, 18, 18,  7,
 10, 12, 13, 14, 15, 15, 15, 15,
 16, 16, 17, 17, 17, 17, 17, 17,
 17, 17, 17, 17, 18, 17, 17,  8,
 11, 13, 14, 15, 15, 15, 15, 16,
 16, 18, 17, 17, 18, 17, 17, 18,
 17, 17, 18, 18, 19, 18, 18, 19,
 19, 19, 18, 18, 18,  8, 11, 13,
 14, 15, 16, 16, 16, 16, 17, 17,
 17, 18, 17, 18, 19, 18, 18, 18,
 18, 18, 18,  8, 12, 14, 15, 15,
 16, 16, 16, 17, 17, 18, 18, 18,
 18, 18, 18, 18, 18, 17,  9, 12,
 14, 15, 16, 16, 17, 17, 17, 17,
 18,  9, 12, 14, 15, 16, 17, 17,
 17, 18,  9, 13, 15, 16, 17, 17,
 18, 17, 18, 17,  9, 13, 15, 16,
 17, 18, 18, 18, 10, 13, 15, 16,
 18, 10, 14, 16, 17, 18, 10, 14,
 16, 17, 10, 14, 16, 18, 18, 10,
 14, 16, 18, 18, 11, 15, 16, 11,
 15, 17, 11, 15, 17, 11, 15, 17,
 11, 15, 17, 11, 15, 17, 12, 16,
 17, 12, 15, 12, 16, 12, 16, 18,
 12, 16, 12, 16, 12, 16, 12, 16,
 17, 12, 16, 18, 12, 17, 13, 16,
 13, 16, 13, 16, 18, 13, 16, 13,
 17, 13, 17, 13, 17, 13, 17, 13,
 17, 13, 17, 13, 17, 13, 17, 13,
 16, 13, 17, 13, 17, 13, 17, 14,
 17, 14, 17, 14, 17, 14, 14, 14,
 17, 14, 17, 14, 14, 18, 14, 14,
 18, 14, 18, 14, 18, 14, 17, 14,
 17, 14, 17, 14, 14, 18, 14, 15,
 15, 15, 14, 15, 15, 14, 15, 15,
 15, 18, 15, 18, 15, 15, 17, 15,
 15, 15, 15, 15, 15, 15, 15, 15,
 15, 15, 16, 15, 15, 15, 15, 16,
 16, 16, 16, 16, 15, 15, 15, 15,
 16, 16, 16, 16, 16, 16, 16, 16,
 16, 16, 16, 16, 16, 16, 16, 16,
 16, 16, 16, 16, 16, 16, 16, 16,
 16, 16, 16, 16, 16, 17, 16, 16,
 16, 17, 16, 16, 16, 17, 17, 17,
 17, 17, 16, 17, 17, 17, 17, 16,
 16, 16, 17, 17, 17, 17, 16, 17,
 17, 17, 17, 17, 17, 17, 17, 17,
 17, 17, 17, 17, 17, 17, 18, 17,
};

static const uint32_t coef4_huffcodes[476] = {
 0x00f01, 0x0001e, 0x00000, 0x00004, 0x00006, 0x0000d, 0x0000a, 0x00017,
 0x0001d, 0x00017, 0x0002c, 0x00031, 0x00039, 0x0003e, 0x00039, 0x0005a,
 0x00066, 0x00070, 0x0007b, 0x00070, 0x00077, 0x000af, 0x000c9, 0x000f2,
 0x000f4, 0x000b2, 0x000e3, 0x0015b, 0x0015d, 0x00181, 0x0019d, 0x001e3,
 0x001c5, 0x002b5, 0x002db, 0x00338, 0x003c3, 0x003cc, 0x003f0, 0x002cd,
 0x003fa, 0x003a1, 0x005b4, 0x00657, 0x007ab, 0x0074d, 0x0074c, 0x00ac1,
 0x00ac5, 0x0076b, 0x00ca8, 0x00f04, 0x00f00, 0x00fe3, 0x00f3c, 0x00f10,
 0x00f39, 0x00fe6, 0x00e26, 0x00e90, 0x016c5, 0x01827, 0x01954, 0x015c5,
 0x01958, 0x01f8a, 0x01c4a, 0x02b0f, 0x02b41, 0x02b0e, 0x033c6, 0x03050,
 0x01c4f, 0x02d88, 0x0305c, 0x03c18, 0x02b4f, 0x02cc2, 0x03a47, 0x05680,
 0x0569d, 0x06442, 0x06443, 0x06446, 0x0656e, 0x06444, 0x07120, 0x0748a,
 0x0c1ba, 0x07e22, 0x07aa6, 0x07f25, 0x07aa7, 0x07e20, 0x0c11b, 0x0c118,
 0x07aa5, 0x0ad0a, 0x0f389, 0x19ebb, 0x0caad, 0x0fe42, 0x0fe40, 0x16c34,
 0x2b4e5, 0x33d65, 0x16c30, 0x1e7ae, 0x1e25c, 0x18370, 0x1e703, 0x19eba,
 0x16c37, 0x0e234, 0x16c6e, 0x00004, 0x0002a, 0x00061, 0x00075, 0x000cb,
 0x000ff, 0x00190, 0x001eb, 0x001d1, 0x002b9, 0x00307, 0x00339, 0x0033f,
 0x003fb, 0x003b4, 0x0060c, 0x00679, 0x00645, 0x0067d, 0x0078a, 0x007e3,
 0x00749, 0x00ac4, 0x00ad2, 0x00ae3, 0x00c10, 0x00c16, 0x00ad1, 0x00cf4,
 0x00fe2, 0x01586, 0x00e9d, 0x019f1, 0x01664, 0x01e26, 0x01d38, 0x02b4d,
 0x033c5, 0x01fc2, 0x01fc3, 0x01d28, 0x03c1d, 0x0598e, 0x0f094, 0x07aa4,
 0x0ad38, 0x0ac0c, 0x0c11a, 0x079ea, 0x0c881, 0x0fe44, 0x0b635, 0x0ac0d,
 0x0b61e, 0x05987, 0x07121, 0x0f382, 0x0f387, 0x0e237, 0x0fe47, 0x0f383,
 0x0f091, 0x0f385, 0x0e233, 0x182ee, 0x19eb8, 0x1663e, 0x0f093, 0x00014,
 0x00058, 0x00159, 0x00167, 0x00300, 0x003d4, 0x005b5, 0x0079d, 0x0076a,
 0x00b67, 0x00b60, 0x00f05, 0x00cf0, 0x00f17, 0x00e95, 0x01822, 0x01913,
 0x016c2, 0x0182f, 0x01959, 0x01fcb, 0x01e27, 0x01c40, 0x033c7, 0x01e7b,
 0x01c49, 0x02d89, 0x01e23, 0x01660, 0x03f12, 0x02cc6, 0x033e1, 0x05b34,
 0x0609a, 0x06569, 0x07488, 0x07e21, 0x0cf5f, 0x0712c, 0x0389d, 0x067cf,
 0x07f28, 0x1663f, 0x33d67, 0x1663d, 0x1e25d, 0x3c1ab, 0x15c44, 0x16c36,
 0x0001f, 0x000ec, 0x00323, 0x005b2, 0x0079f, 0x00ac2, 0x00f16, 0x00e9e,
 0x01956, 0x01e0f, 0x019ea, 0x01666, 0x02b89, 0x02b02, 0x02d8c, 0x03c1b,
 0x03c19, 0x032b5, 0x03f9c, 0x02ccf, 0x03897, 0x05b35, 0x0ad02, 0x07f29,
 0x06441, 0x03884, 0x07888, 0x0784e, 0x06568, 0x0c1bb, 0x05986, 0x067cc,
 0x0fe49, 0x0fe48, 0x0c1bc, 0x0fe41, 0x18371, 0x1663c, 0x0e231, 0x0711e,
 0x0ad09, 0x0f092, 0x0002d, 0x001db, 0x00781, 0x00c1a, 0x00f55, 0x01580,
 0x01ea8, 0x02d9b, 0x032af, 0x03f16, 0x03c1c, 0x07834, 0x03c45, 0x0389c,
 0x067ce, 0x06445, 0x0c1b9, 0x07889, 0x07f3a, 0x0784f, 0x07f2b, 0x0ad0b,
 0x0f090, 0x0c11d, 0x0e94e, 0x0711f, 0x0e9f1, 0x0f38e, 0x079e9, 0x0ad03,
 0x0f09b, 0x0caae, 0x0fe46, 0x2b4e6, 0x0e9f0, 0x19eb6, 0x67ac1, 0x67ac0,
 0x33d66, 0x0f388, 0x00071, 0x003a0, 0x00ca9, 0x01829, 0x01d39, 0x02b43,
 0x02cc4, 0x06554, 0x0f09a, 0x0b61f, 0x067cd, 0x0711c, 0x0b636, 0x07f2a,
 0x0b634, 0x0c11f, 0x0cf5e, 0x0b61d, 0x0f06b, 0x0caab, 0x0c1be, 0x0e94c,
 0x0f099, 0x182ed, 0x0e94f, 0x0c119, 0x0e232, 0x2b4e4, 0x0f38a, 0x19eb4,
 0x1e25f, 0x0e94d, 0x000b7, 0x00785, 0x016cc, 0x03051, 0x033c4, 0x0656f,
 0x03891, 0x0711d, 0x0caaf, 0x0f097, 0x07489, 0x0f098, 0x0c880, 0x0caaa,
 0x0f386, 0x19eb7, 0x16c6f, 0x0f384, 0x182e8, 0x182e9, 0x0e230, 0x1e700,
 0x33d62, 0x33d63, 0x33d64, 0x16c33, 0x0e216, 0x000fd, 0x00c15, 0x01665,
 0x03c4a, 0x07f3b, 0x07896, 0x0c11c, 0x0e215, 0x16c32, 0x0f38b, 0x0f38d,
 0x182ea, 0x1e701, 0x712df, 0x15c46, 0x00194, 0x00fe0, 0x03f13, 0x0748b,
 0x0f096, 0x0cf80, 0x1e25e, 0xe25bd, 0x33d61, 0x16c31, 0x001f9, 0x01912,
 0x05710, 0x0f3d0, 0x0c1bf, 0x00301, 0x01e24, 0x0ad08, 0x003cd, 0x01c41,
 0x0c1bd, 0x00563, 0x03a52, 0x0f3d1, 0x00570, 0x02cce, 0x0e217, 0x0067b,
 0x0655d, 0x0074b, 0x06447, 0x00c12, 0x074fb, 0x00f08, 0x0b61c, 0x00e22,
 0x0fe43, 0x016c7, 0x01836, 0x019f2, 0x01c43, 0x01d3f, 0x01fcf, 0x02b4c,
 0x0304c, 0x032b6, 0x03a46, 0x05607, 0x03f17, 0x02cc5, 0x0609b, 0x0655c,
 0x07e23, 0x067c1, 0x07f26, 0x07f27, 0x0f095, 0x0e9f3, 0x0cf81, 0x0c11e,
 0x0caac, 0x0f38f, 0x0e9f2, 0x074fa, 0x0e236, 0x0fe45, 0x1c428, 0x0e235,
 0x182ef, 0x19eb5, 0x0f3d6, 0x182ec, 0x16c35, 0x0f38c, 0x2b4e7, 0x15c47,
 0xe25bc, 0x1e702, 0x1c4b6, 0x0e25a, 0x3c1aa, 0x15c45, 0x1c429, 0x19eb9,
 0x1e7af, 0x182eb, 0x1e0d4, 0x3896e,
};

static const uint8_t coef4_huffbits[476] = {
 12,  6,  2,  3,  4,  4,  5,  5,
  5,  6,  6,  6,  6,  6,  7,  7,
  7,  7,  7,  8,  8,  8,  8,  8,
  8,  9,  9,  9,  9,  9,  9,  9,
 10, 10, 10, 10, 10, 10, 10, 11,
 10, 11, 11, 11, 11, 12, 12, 12,
 12, 12, 12, 12, 12, 12, 12, 12,
 12, 12, 13, 13, 13, 13, 13, 13,
 13, 13, 14, 14, 14, 14, 14, 14,
 14, 14, 14, 14, 14, 15, 15, 15,
 15, 15, 15, 15, 15, 15, 16, 16,
 16, 15, 15, 15, 15, 15, 16, 16,
 15, 16, 16, 17, 16, 16, 16, 17,
 18, 18, 17, 17, 17, 17, 17, 17,
 17, 17, 17,  4,  6,  7,  8,  8,
  8,  9,  9, 10, 10, 10, 10, 10,
 10, 11, 11, 11, 11, 11, 11, 11,
 12, 12, 12, 12, 12, 12, 12, 12,
 12, 13, 13, 13, 14, 13, 14, 14,
 14, 13, 13, 14, 14, 16, 16, 15,
 16, 16, 16, 15, 16, 16, 16, 16,
 16, 16, 16, 16, 16, 17, 16, 16,
 16, 16, 17, 17, 17, 18, 16,  5,
  8,  9, 10, 10, 10, 11, 11, 12,
 12, 12, 12, 12, 12, 13, 13, 13,
 13, 13, 13, 13, 13, 14, 14, 13,
 14, 14, 13, 14, 14, 15, 14, 15,
 15, 15, 16, 15, 16, 16, 15, 15,
 15, 18, 18, 18, 17, 18, 17, 17,
  6,  9, 10, 11, 11, 12, 12, 13,
 13, 13, 13, 14, 14, 14, 14, 14,
 14, 14, 14, 15, 15, 15, 16, 15,
 15, 15, 15, 15, 15, 16, 16, 15,
 16, 16, 16, 16, 17, 18, 17, 16,
 16, 16,  7, 10, 11, 12, 12, 13,
 13, 14, 14, 14, 14, 15, 14, 15,
 15, 15, 16, 15, 15, 15, 15, 16,
 16, 16, 17, 16, 17, 16, 15, 16,
 16, 16, 16, 18, 17, 17, 19, 19,
 18, 16,  7, 11, 12, 13, 14, 14,
 15, 15, 16, 16, 15, 16, 16, 15,
 16, 16, 16, 16, 16, 16, 16, 17,
 16, 17, 17, 16, 17, 18, 16, 17,
 17, 17,  8, 11, 13, 14, 14, 15,
 15, 16, 16, 16, 16, 16, 16, 16,
 16, 17, 17, 16, 17, 17, 17, 17,
 18, 18, 18, 17, 17,  8, 12, 14,
 14, 15, 15, 16, 17, 17, 16, 16,
 17, 17, 20, 17,  9, 12, 14, 16,
 16, 16, 17, 21, 18, 17,  9, 13,
 15, 16, 16, 10, 13, 16, 10, 14,
 16, 11, 15, 16, 11, 15, 17, 11,
 15, 12, 15, 12, 16, 12, 16, 13,
 16, 13, 13, 13, 14, 14, 13, 14,
 14, 14, 15, 15, 14, 15, 15, 15,
 15, 15, 15, 15, 16, 17, 16, 16,
 16, 16, 17, 16, 17, 16, 18, 17,
 17, 17, 16, 17, 17, 16, 18, 17,
 21, 17, 18, 17, 18, 17, 18, 17,
 17, 17, 17, 19,
};

static const uint32_t coef5_huffcodes[435] = {
 0x00347, 0x0000b, 0x00001, 0x00001, 0x0000c, 0x00004, 0x00010, 0x00015,
 0x0001f, 0x0000b, 0x00023, 0x00026, 0x00029, 0x00035, 0x00037, 0x00001,
 0x00015, 0x0001a, 0x0001d, 0x0001c, 0x0001e, 0x0004e, 0x00049, 0x00051,
 0x00078, 0x00004, 0x00000, 0x00008, 0x0000d, 0x0007b, 0x00005, 0x00032,
 0x00095, 0x00091, 0x00096, 0x000a1, 0x000d9, 0x00003, 0x00019, 0x00061,
 0x00066, 0x00060, 0x00017, 0x0000e, 0x00063, 0x001a0, 0x001b7, 0x001e6,
 0x001e7, 0x001b6, 0x00018, 0x001e8, 0x00038, 0x00031, 0x00005, 0x0003d,
 0x00027, 0x001ea, 0x0001a, 0x000c5, 0x000f9, 0x000ff, 0x000db, 0x00250,
 0x000fc, 0x0025c, 0x00008, 0x00075, 0x003d7, 0x003d3, 0x001b0, 0x0007c,
 0x003ca, 0x00036, 0x00189, 0x004a6, 0x004a2, 0x004fb, 0x000c0, 0x0007f,
 0x0009a, 0x00311, 0x0006e, 0x0009b, 0x0068c, 0x006c0, 0x00484, 0x00012,
 0x000c3, 0x0094f, 0x00979, 0x009f9, 0x00d09, 0x00da6, 0x00da8, 0x00901,
 0x000c1, 0x00373, 0x00d08, 0x009fa, 0x00d8b, 0x00d85, 0x00d86, 0x000df,
 0x006e2, 0x000ce, 0x00f24, 0x009fe, 0x001f7, 0x007c1, 0x000cf, 0x009fc,
 0x009ff, 0x00d89, 0x00da9, 0x009fd, 0x001f8, 0x01a36, 0x0128c, 0x0129d,
 0x01a37, 0x00196, 0x003ea, 0x00f8b, 0x00d93, 0x01e45, 0x01e58, 0x01e4b,
 0x01e59, 0x013f1, 0x00309, 0x00265, 0x00308, 0x0243a, 0x027e1, 0x00f89,
 0x00324, 0x03cbc, 0x03c86, 0x03695, 0x0243c, 0x0243b, 0x0243e, 0x01e4a,
 0x003a5, 0x03468, 0x03428, 0x03c84, 0x027e0, 0x025e2, 0x01880, 0x00197,
 0x00325, 0x03cb7, 0x0791e, 0x007ec, 0x06c75, 0x004c8, 0x04bc7, 0x004c6,
 0x00983, 0x0481e, 0x01b53, 0x0251b, 0x01b58, 0x00984, 0x04fa8, 0x03cbb,
 0x00f8a, 0x00322, 0x0346a, 0x0243d, 0x00326, 0x03469, 0x0481f, 0x0481d,
 0x00746, 0x09032, 0x01b50, 0x01d13, 0x0d8e4, 0x0481b, 0x06c74, 0x0796b,
 0x07969, 0x00985, 0x0d8e3, 0x00986, 0x00fa2, 0x01301, 0x06c7c, 0x00987,
 0x03cb8, 0x0f4af, 0x00e88, 0x1b1c0, 0x00fce, 0x033eb, 0x03f6a, 0x03f69,
 0x00fcf, 0x0791f, 0x004c9, 0x04871, 0x00fcd, 0x00982, 0x00fcc, 0x00fa3,
 0x01d12, 0x0796c, 0x01b47, 0x00321, 0x0796a, 0x0d8e2, 0x04872, 0x04873,
 0x0000e, 0x00014, 0x0000a, 0x000a0, 0x00012, 0x0007d, 0x001a2, 0x0003b,
 0x0025f, 0x000dd, 0x0027c, 0x00343, 0x00368, 0x0036b, 0x0003e, 0x001fa,
 0x00485, 0x001b3, 0x0007f, 0x001b1, 0x0019e, 0x004ba, 0x007ad, 0x00339,
 0x00066, 0x007a4, 0x00793, 0x006c6, 0x0007e, 0x000f1, 0x00372, 0x009fb,
 0x00d83, 0x00d8a, 0x00947, 0x009f4, 0x001d0, 0x01b09, 0x01b4b, 0x007ec,
 0x003e1, 0x000ca, 0x003ec, 0x02539, 0x04fa9, 0x01b57, 0x03429, 0x03d2a,
 0x00d97, 0x003a7, 0x00dc0, 0x00d96, 0x00dc1, 0x007eb, 0x03cba, 0x00c43,
 0x00c41, 0x01b52, 0x007ef, 0x00323, 0x03cb9, 0x03c83, 0x007d0, 0x007ed,
 0x06c7f, 0x09033, 0x03f6c, 0x36383, 0x1e95d, 0x06c78, 0x00747, 0x01b51,
 0x00022, 0x00016, 0x00039, 0x00252, 0x00079, 0x00486, 0x00338, 0x00369,
 0x00d88, 0x00026, 0x00d87, 0x00f4b, 0x00d82, 0x00027, 0x001e1, 0x01a15,
 0x007c7, 0x012f0, 0x001e0, 0x006d0, 0x01a16, 0x01e44, 0x01e5f, 0x03690,
 0x00d90, 0x00c42, 0x00daf, 0x00d92, 0x00f80, 0x00cfb, 0x0342f, 0x0487f,
 0x01b46, 0x07968, 0x00d95, 0x00d91, 0x01b55, 0x03f68, 0x04bc6, 0x03cbd,
 0x00f81, 0x00320, 0x00069, 0x000fe, 0x006d5, 0x0033f, 0x000de, 0x007c6,
 0x01e40, 0x00d94, 0x00f88, 0x03c8e, 0x03694, 0x00dae, 0x00dad, 0x00267,
 0x003a6, 0x00327, 0x0487e, 0x007ee, 0x00749, 0x004c7, 0x03692, 0x01b56,
 0x00fd1, 0x07a56, 0x06c77, 0x09031, 0x00748, 0x06c7a, 0x0796d, 0x033ea,
 0x06c76, 0x00fd0, 0x36382, 0x1e417, 0x00745, 0x04faf, 0x0d8e1, 0x03f6b,
 0x1e95c, 0x04fad, 0x0009e, 0x004bd, 0x0067c, 0x01b08, 0x003eb, 0x01b45,
 0x03691, 0x0d8e5, 0x07904, 0x00981, 0x007ea, 0x019f4, 0x06c7d, 0x04fab,
 0x04fac, 0x06c7e, 0x01300, 0x06c7b, 0x0006f, 0x003f7, 0x03c85, 0x004c4,
 0x0001e, 0x006e1, 0x03693, 0x01b44, 0x00241, 0x01e46, 0x0019d, 0x00266,
 0x004bb, 0x02538, 0x007ac, 0x01b54, 0x00902, 0x04870, 0x00da7, 0x00900,
 0x00185, 0x06c79, 0x006e3, 0x003e9, 0x01e94, 0x003ed, 0x003f2, 0x0342e,
 0x0346b, 0x0251a, 0x004c5, 0x01881, 0x0481c, 0x01b59, 0x03c87, 0x04fae,
 0x007e9, 0x03f6d, 0x0f20a, 0x09030, 0x04faa, 0x0d8e6, 0x03f6f, 0x0481a,
 0x03f6e, 0x1e416, 0x0d8e7,
};

static const uint8_t coef5_huffbits[435] = {
 10,  4,  2,  4,  4,  5,  5,  5,
  5,  6,  6,  6,  6,  6,  6,  7,
  7,  7,  7,  7,  7,  7,  7,  7,
  7,  8,  8,  8,  8,  7,  8,  8,
  8,  8,  8,  8,  8,  9,  9,  9,
  9,  9,  9,  9,  9,  9,  9,  9,
  9,  9, 10,  9, 10, 10, 10, 10,
 10,  9, 10, 10, 10, 10, 10, 10,
 10, 10, 11, 11, 10, 10, 11, 11,
 10, 11, 11, 11, 11, 11, 12, 12,
 12, 12, 12, 12, 11, 11, 11, 12,
 12, 12, 12, 12, 12, 12, 12, 12,
 12, 12, 12, 12, 12, 12, 12, 13,
 13, 13, 12, 12, 13, 13, 13, 12,
 12, 12, 12, 12, 13, 13, 13, 13,
 13, 14, 14, 14, 14, 13, 13, 13,
 13, 13, 14, 14, 14, 14, 14, 14,
 15, 14, 14, 14, 14, 14, 14, 13,
 14, 14, 14, 14, 14, 14, 15, 14,
 15, 14, 15, 15, 15, 15, 15, 15,
 16, 15, 15, 14, 15, 16, 15, 14,
 14, 15, 14, 14, 15, 14, 15, 15,
 15, 16, 15, 17, 16, 15, 15, 15,
 15, 16, 16, 16, 16, 17, 15, 16,
 14, 16, 16, 17, 16, 16, 16, 16,
 16, 15, 15, 15, 16, 16, 16, 16,
 17, 15, 15, 15, 15, 16, 15, 15,
  4,  7,  8,  8,  9,  9,  9, 10,
 10, 10, 10, 10, 10, 10, 11, 11,
 11, 11, 11, 11, 11, 11, 11, 12,
 12, 11, 11, 11, 12, 12, 12, 12,
 12, 12, 12, 12, 13, 13, 13, 13,
 12, 13, 14, 14, 15, 15, 14, 14,
 14, 14, 14, 14, 14, 15, 14, 14,
 14, 15, 15, 15, 14, 14, 15, 15,
 15, 16, 16, 18, 17, 15, 15, 15,
  6,  9, 10, 10, 11, 11, 12, 12,
 12, 13, 12, 12, 12, 13, 13, 13,
 13, 13, 13, 13, 13, 13, 13, 14,
 14, 14, 14, 14, 14, 14, 14, 15,
 15, 15, 14, 14, 15, 16, 15, 14,
 14, 15,  7, 10, 11, 12, 13, 13,
 13, 14, 14, 14, 14, 14, 14, 14,
 14, 15, 15, 15, 15, 15, 14, 15,
 16, 15, 15, 16, 15, 15, 15, 16,
 15, 16, 18, 17, 15, 15, 16, 16,
 17, 15,  8, 11, 13, 13, 14, 15,
 14, 16, 15, 16, 15, 15, 15, 15,
 15, 15, 17, 15,  9, 12, 14, 15,
 10, 13, 14, 15, 10, 13, 11, 14,
 11, 14, 11, 15, 12, 15, 12, 12,
 13, 15, 13, 14, 13, 14, 14, 14,
 14, 14, 15, 15, 15, 15, 14, 15,
 15, 16, 16, 16, 15, 16, 16, 15,
 16, 17, 16,
};

static const uint16_t levels0[60] = {
317, 92, 62, 60, 19, 17, 10,  7,
  6,  5,  5,  3,  3,  3,  2,  2,
  2,  2,  2,  2,  2,  1,  2,  2,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,
};

static const uint16_t levels1[40] = {
311, 91, 61, 28, 10,  6,  5,  2,
  2,  2,  2,  2,  2,  2,  2,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
};

static const uint16_t levels2[340] = {
181,110, 78, 63, 61, 62, 60, 61,
 33, 41, 41, 19, 17, 19, 12, 11,
  9, 11, 10,  6,  8,  7,  6,  4,
  5,  5,  4,  4,  3,  4,  3,  5,
  3,  4,  3,  3,  3,  3,  3,  3,
  2,  2,  4,  2,  3,  2,  3,  3,
  2,  2,  2,  2,  2,  2,  2,  2,
  3,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  1,  2,  1,  2,  2,
  2,  2,  1,  2,  1,  1,  1,  2,
  2,  1,  2,  1,  2,  2,  2,  2,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,
};

static const uint16_t levels3[180] = {
351,122, 76, 61, 41, 42, 24, 30,
 22, 19, 11,  9, 10,  8,  5,  5,
  4,  5,  5,  3,  3,  3,  3,  3,
  3,  3,  2,  2,  3,  2,  2,  2,
  3,  3,  2,  2,  2,  3,  2,  2,
  2,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  2,  1,  1,
  2,  2,  1,  2,  1,  2,  2,  2,
  2,  2,  2,  1,  2,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  2,
  2,  1,  2,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,
};

static const uint16_t levels4[70] = {
113, 68, 49, 42, 40, 32, 27, 15,
 10,  5,  3,  3,  3,  3,  2,  2,
  2,  2,  2,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,
};

static const uint16_t levels5[40] = {
214, 72, 42, 40, 18,  4,  4,  2,
  2,  2,  2,  2,  1,  1,  2,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,
};

typedef struct CoefVLCTable
{
    int n; /* total number of codes */
    const uint32_t *huffcodes; /* VLC bit values */
    const uint8_t *huffbits;   /* VLC bit size */
    const uint16_t *levels; /* table to build run/level tables */
} CoefVLCTable;

static const CoefVLCTable coef_vlcs[6] = {
    {
        sizeof(coef0_huffbits), coef0_huffcodes, coef0_huffbits, levels0,
    },
    {
        sizeof(coef1_huffbits), coef1_huffcodes, coef1_huffbits, levels1,
    },
    {
        sizeof(coef2_huffbits), coef2_huffcodes, coef2_huffbits, levels2,
    },
    {
        sizeof(coef3_huffbits), coef3_huffcodes, coef3_huffbits, levels3,
    },
    {
        sizeof(coef4_huffbits), coef4_huffcodes, coef4_huffbits, levels4,
    },
    {
        sizeof(coef5_huffbits), coef5_huffcodes, coef5_huffbits, levels5,
    },
};


const fixed64 pow_table[] =
    {
        0x10000LL,0x11f3dLL,0x14249LL,0x1699cLL,0x195bcLL,0x1c73dLL,0x1fec9LL,0x23d1dLL,0x2830bLL,0x2d182LL,
        0x3298bLL,0x38c53LL,0x3fb28LL,0x47783LL,0x5030aLL,0x59f98LL,0x64f40LL,0x71457LL,0x7f17bLL,0x8e99aLL,
        0xa0000LL,0xb385eLL,0xc96d9LL,0xe2019LL,0xfd954LL,0x11c865LL,0x13f3dfLL,0x166320LL,0x191e6eLL,0x1c2f10LL,
        0x1f9f6eLL,0x237b39LL,0x27cf8bLL,0x2cab1aLL,0x321e65LL,0x383bf0LL,0x3f1882LL,0x46cb6aLL,0x4f6eceLL,0x592006LL,
        0x640000LL,0x7033acLL,0x7de47eLL,0x8d40f6LL,0x9e7d44LL,0xb1d3f4LL,0xc786b7LL,0xdfdf43LL,0xfb304bLL,0x119d69aLL,
        0x13c3a4eLL,0x162d03aLL,0x18e1b70LL,0x1beaf00LL,0x1f52feeLL,0x2325760LL,0x276f514LL,0x2c3f220LL,0x31a5408LL,
        0x37b403cLL,0x3e80000LL,0x46204b8LL,0x4eaece8LL,0x58489a0LL,0x630e4a8LL,0x6f24788LL,0x7cb4328LL,0x8beb8a0LL,
        0x9cfe2f0LL,0xb026200LL,0xc5a4710LL,0xddc2240LL,0xf8d1260LL,0x1172d600LL,0x1393df60LL,0x15f769c0LL,0x18a592c0LL,
        0x1ba77540LL,0x1f074840LL,0x22d08280LL,0x27100000LL,0x2bd42f40LL,0x312d4100LL,0x372d6000LL,0x3de8ee80LL,
        0x4576cb80LL,0x4df09f80LL,0x57733600LL,0x621edd80LL,0x6e17d480LL,0x7b86c700LL,0x8a995700LL,0x9b82b800LL,
        0xae7c5c00LL,0xc3c6b900LL,0xdbaa2200LL,0xf677bc00LL,0x1148a9400LL,0x13648d200LL,0x15c251800LL,0x186a00000LL,
        0x1b649d800LL,0x1ebc48a00LL,0x227c5c000LL,0x26b195000LL,0x2b6a3f000LL,0x30b663c00LL,0x36a801c00LL,0x3d534a400LL,
        0x44cee4800LL,0x4d343c800LL,0x569fd6000LL,0x6131b2800LL,0x6d0db9800LL,0x7a5c33800LL,0x894a55000LL,0x9a0ad6000LL,
        0xacd69d000LL,0xc1ed84000LL,0xd9972f000LL,0xf42400000LL,0x111ee28000LL,0x1335ad6000LL,0x158db98000LL,0x182efd4000LL,
        0x1b22676000LL,0x1e71fe6000LL,0x2229014000LL,0x26540e8000LL,0x2b014f0000LL,0x3040a5c000LL,0x3623e60000LL,0x3cbf0fc000LL,
        0x4428940000LL,0x4c79a08000LL,0x55ce758000LL,0x6046c58000LL,0x6c06220000LL,0x7934728000LL,0x87fe7d0000LL,0x9896800000LL,
        0xab34d90000LL,0xc018c60000LL,0xd7893f0000LL,0xf1d5e40000LL,0x10f580a0000LL,0x13073f00000LL,0x1559a0c0000LL,0x17f48900000LL,
        0x1ae0d160000LL,0x1e286780000LL,0x21d66fc0000LL,0x25f769c0000LL,0x2a995c80000LL,0x2fcc0440000LL,0x35a10940000LL,
        0x3c2c3b80000LL,0x4383d500000LL,0x4bc0c780000LL,0x54ff0e80000LL,0x5f5e1000000LL,0x6b010780000LL,0x780f7c00000LL,
        0x86b5c800000LL,0x9725ae00000LL,0xa9970600000LL,0xbe487500000LL,0xd5804700000LL,0xef8d5a00000LL,0x10cc82e00000LL,
        0x12d940c00000LL,0x152605c00000LL,0x17baa2200000LL,0x1a9fd9c00000LL,0x1ddf82a00000LL,0x2184a5c00000LL,0x259ba5400000LL,
        0x2a3265400000LL,0x2f587cc00000LL,0x351f69000000LL,0x3b9aca000000LL,0x42e0a4800000LL,0x4b09ad800000LL,
        0x54319d000000LL,0x5e778d000000LL,0x69fe64000000LL,0x76ed49800000LL,0x85702c000000LL,0x95b858000000LL,
        0xa7fd1c000000LL,0xbc7c87000000LL,0xd37c3a000000LL,0xed4a55000000LL,0x10a3e82000000LL,0x12abb1a000000LL,
        0x14f2e7a000000LL,0x1781474000000LL,0x1a5f7f4000000LL,0x1d974de000000LL,0x2133a18000000LL
    };

const fixed32 pow_10_to_yover16[]=
{
0x10000,0x127a0,0x15562,0x18a39,0x1c73d,0x20db4,0x25f12,0x2bd09,0x3298b,0x3a6d9,0x4378b,0x4dea3,
0x59f98,0x67e6b,0x77fbb,0x8a8de,0xa0000,0xb8c3e,0xd55d1,0xf6636,0x11c865,0x148906,0x17b6b8,0x1b625b,
0x1f9f6e,0x248475,0x2a2b6e,0x30b25f,0x383bf0,0x40f02c,0x4afd4b,0x5698b0,0x640000,0x737a6b,0x855a26,
0x99fe1f,0xb1d3f4,0xcd5a3e,0xed232b,0x111d78a,0x13c3a4e,0x16d2c94,0x1a5b24e,0x1e6f7b0,0x2325760,
0x28961b4,0x2ede4ec,0x361f6dc,0x3e80000,0x482c830,0x5358580,0x603ed30,0x6f24788,0x8058670,0x9435fb0,
0xab26b70,0xc5a4710,0xe43bdc0,0x1078f700,0x1305ace0,0x15f769c0,0x195dd100,0x1d4af120
};

const fixed32 pow_a_table[] =
{
0x1004,0x1008,0x100c,0x1010,0x1014,0x1018,0x101c,0x1021,0x1025,0x1029,0x102d,0x1031,0x1036,0x103a,
0x103e,0x1043,0x1047,0x104b,0x1050,0x1054,0x1059,0x105d,0x1062,0x1066,0x106b,0x106f,0x1074,0x1078,
0x107d,0x1082,0x1086,0x108b,0x1090,0x1095,0x1099,0x109e,0x10a3,0x10a8,0x10ad,0x10b2,0x10b7,0x10bc,
0x10c1,0x10c6,0x10cb,0x10d0,0x10d5,0x10da,0x10df,0x10e5,0x10ea,0x10ef,0x10f5,0x10fa,0x10ff,0x1105,
0x110a,0x1110,0x1115,0x111b,0x1120,0x1126,0x112c,0x1131,0x1137,0x113d,0x1143,0x1149,0x114f,0x1155,
0x115a,0x1161,0x1167,0x116d,0x1173,0x1179,0x117f,0x1186,0x118c,0x1192,0x1199,0x119f,0x11a6,0x11ac,
0x11b3,0x11b9,0x11c0,0x11c7,0x11ce,0x11d4,0x11db,0x11e2,0x11e9,0x11f0,0x11f8,0x11ff,0x1206,0x120d,
0x1215,0x121c,0x1223,0x122b,0x1233,0x123a,0x1242,0x124a,0x1251,0x1259,0x1261,0x1269,0x1271,0x127a,
0x1282,0x128a,0x1293,0x129b,0x12a4,0x12ac,0x12b5,0x12be,0x12c7,0x12d0,0x12d9,0x12e2,0x12eb,0x12f4,
0x12fe,0x1307
};

const fixed64 lsp_pow_e_table[] =
{
    0xf333f9deLL,		0xf0518db9LL,		0x0LL,		0x7e656b4fLL,		0x7999fcefLL,		0xf828c6dcLL,		0x0LL,
    0x3f32b5a7LL,   0x3cccfe78LL,    0xfc14636eLL,    0x0LL,    0x9f995ad4LL,    0x9e667f3cLL,    0xfe0a31b7LL,
    0x0LL,    0x4fccad6aLL,    0x4f333f9eLL,    0x7f0518dcLL,    0x0LL,    0x27e656b5LL,    0x27999fcfLL,
    0xbf828c6eLL,    0x0LL,    0x13f32b5aLL,    0x13cccfe7LL,    0xdfc14637LL,    0x0LL,    0x89f995adLL,
    0x9e667f4LL,    0x6fe0a31bLL,    0x0LL,    0x44fccad7LL,    0x4f333faLL,    0x37f0518eLL,    0x0LL,
    0xa27e656bLL,    0x827999fdLL,    0x1bf828c7LL,    0x0LL,    0xd13f32b6LL,    0x413cccfeLL,    0xdfc1463LL,
    0x0LL,    0xe89f995bLL,    0xa09e667fLL,    0x6fe0a32LL,    0x0LL,    0x744fccadLL,    0x504f3340LL,
    0x837f0519LL,    0x0LL,    0xba27e657LL,    0xa82799a0LL,    0xc1bf828cLL,    0x0LL,    0x5d13f32bLL,
    0xd413ccd0LL,    0x60dfc146LL,    0x0LL,    0xae89f996LL,    0x6a09e668LL,    0x306fe0a3LL,    0x0LL,
    0xd744fccbLL,    0xb504f334LL,    0x9837f052LL,    0x80000000LL,    0x6ba27e65LL,    0x5a82799aLL,
    0x4c1bf829LL,    0x40000000LL,    0x35d13f33LL,    0x2d413ccdLL,    0x260dfc14LL,    0x20000000LL,
    0x1ae89f99LL,    0x16a09e66LL,    0x1306fe0aLL,    0x10000000LL,    0xd744fcdLL,    0xb504f33LL,
    0x9837f05LL,    0x8000000LL,    0x6ba27e6LL,    0x5a8279aLL,    0x4c1bf83LL,    0x4000000LL,
    0x35d13f3LL,    0x2d413cdLL,    0x260dfc1LL,    0x2000000LL,    0x1ae89faLL,    0x16a09e6LL,
    0x1306fe1LL,    0x1000000LL,    0xd744fdLL,    0xb504f3LL,    0x9837f0LL,    0x800000LL,
    0x6ba27eLL,    0x5a827aLL,    0x4c1bf8LL,    0x400000LL,    0x35d13fLL,    0x2d413dLL,
    0x260dfcLL,    0x200000LL,    0x1ae8a0LL,    0x16a09eLL,    0x1306feLL,    0x100000LL,
    0xd7450LL,    0xb504fLL,    0x9837fLL,    0x80000LL,    0x6ba28LL,    0x5a828LL,    0x4c1c0LL,
    0x40000LL,    0x35d14LL,    0x2d414LL,    0x260e0LL,    0x20000LL,    0x1ae8aLL,    0x16a0aLL,
    0x13070LL,    0x10000LL,    0xd745LL,    0xb505LL,    0x9838LL,    0x8000LL,    0x6ba2LL,
    0x5a82LL,    0x4c1cLL,    0x4000LL,    0x35d1LL,    0x2d41LL,    0x260eLL,    0x2000LL,
    0x1ae9LL,    0x16a1LL,    0x1307LL,    0x1000LL,    0xd74LL,    0xb50LL,    0x983LL,    0x800LL,
    0x6baLL,    0x5a8LL,    0x4c2LL,    0x400LL,    0x35dLL,    0x2d4LL,    0x261LL,    0x200LL,    0x1afLL,
    0x16aLL,    0x130LL,    0x100LL,    0xd7LL,    0xb5LL,    0x98LL,    0x80LL,    0x6cLL,    0x5bLL,
    0x4cLL,    0x40LL,    0x36LL,    0x2dLL,    0x26LL,    0x20LL,    0x1bLL,    0x17LL,    0x13LL,
    0x10LL,    0xdLL,    0xbLL,    0xaLL,    0x8LL,    0x7LL,    0x6LL,    0x5LL,    0x4LL,    0x3LL,
    0x3LL,    0x2LL,    0x2LL,    0x2LL,    0x1LL,    0x1LL,    0x1LL,    0x1LL,    0x1LL,    0x1LL,
    0x1LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,
    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,
    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,
    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,
    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,
    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,    0x0LL,
    0x0LL,    0x0LL
};

/* misc math functions */
const uint8_t ff_sqrt_tab[128]=
    {
        0, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11
    };

const uint8_t ff_log2_tab[256]=
    {
        0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
        5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
    };

#endif /* WMADECI_H */



// wmadeci.c
/*
 * WMA decoder : Fixed (16.16) point decoding based originally on the ffmpeg
 * WMA decoder source : http://ffmpeg.sourceforge.net.
 * This version 01/04/06 Marsdaddy <pajojo@gmail.com>
 */

//#include "malloc.h"
//#include "memory.h"

/* size of blocks */
#define BLOCK_MIN_BITS 7
#define BLOCK_MAX_BITS 11
#define BLOCK_MAX_SIZE (1 << BLOCK_MAX_BITS)

#define BLOCK_NB_SIZES (BLOCK_MAX_BITS - BLOCK_MIN_BITS + 1)

#define HIGH_BAND_MAX_SIZE 16

//#define NB_LSP_COEFS 10

#define MAX_CODED_SUPERFRAME_SIZE 16384

#define M_PI_F  0x3243f /* in fixed 32 format */

#define MAX_CHANNELS 2

#define NOISE_TAB_SIZE 8192

#define LSP_POW_BITS 7

#define VLC_TYPE int16_t

/*typedef unsigned char uint8_t;
typedef char int8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef long long int64_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#define fixed32         int32_t
#define fixed64         int64_t*/

//#include "codeccontext.h"
//#include "wmadeci.h"

#define IGNORE_OVERFLOW
#define FAST_FILTERS
#define PRECISION       16
#define PRECISION64     16

static fixed64 IntTo64(int x)
{
    fixed64 res = 0;
    unsigned char *p = (unsigned char *)&res;

    p[2] = x & 0xff;
    p[3] = (x & 0xff00)>>8;
    p[4] = (x & 0xff0000)>>16;
    p[5] = (x & 0xff000000)>>24;
    return res;
}

static int IntFrom64(fixed64 x)
{
    int res = 0;
    unsigned char *p = (unsigned char *)&x;

    res = p[2] | (p[3]<<8) | (p[4]<<16) | (p[5]<<24);
    return res;
}

static fixed32 Fixed32From64(fixed64 x)
{
  return x & 0xFFFFFFFF;
}

static fixed64 Fixed32To64(fixed32 x)
{
  return (fixed64)x;
}

#define itofix64(x)       (IntTo64(x))
#define itofix32(x)       ((x) << PRECISION)
#define fixtoi32(x)       ((x) >> PRECISION)
#define fixtoi64(x)       (IntFrom64(x))

static fixed64 fixmul64byfixed(fixed64 x, fixed32 y)
{
    return x * y;
}

/* Portions of this 32 bit fixed code modified from the GPL Hawksoft
source <http://www.hawkfost.com> by Phil Frisbie, Jr. <phil@hawksoft.com> */

static fixed32 fixmul32(fixed32 x, fixed32 y)
{
    fixed64 temp;

    temp = x;
    temp *= y;
    temp >>= PRECISION;
#ifndef IGNORE_OVERFLOW

    if(temp > 0x7fffffff)
    {
        return 0x7fffffff;
    }
    else if(temp < -0x7ffffffe)
    {
        return -0x7ffffffe;
    }
#endif
    return (fixed32)temp;
}

static fixed32 fixdiv32(fixed32 x, fixed32 y)
{
    fixed64 temp;

    if(x == 0)
        return 0;
    if(y == 0)
        return 0x7fffffff;
    temp = x;
    temp <<= PRECISION;
    return (fixed32)(temp / y);
}


static fixed64 fixdiv64(fixed64 x, fixed64 y)
{
    fixed64 temp;

    if(x == 0)
        return 0;
    if(y == 0)
        return 0x07ffffffffffffffLL;
    temp = x;
    temp <<= PRECISION64;
    return (fixed64)(temp / y);
}

/*static*/ fixed32 fixsqrt32(fixed32 x)
{

    unsigned long r = 0, s, v = (unsigned long)x;

#define STEP(k) s = r + (1 << k * 2); r >>= 1; \
    if (s <= v) { v -= s; r |= (1 << k * 2); }

    STEP(15);
    STEP(14);
    STEP(13);
    STEP(12);
    STEP(11);
    STEP(10);
    STEP(9);
    STEP(8);
    STEP(7);
    STEP(6);
    STEP(5);
    STEP(4);
    STEP(3);
    STEP(2);
    STEP(1);
    STEP(0);

    return (fixed32)(r << (PRECISION / 2));
}

__inline fixed32 fixsin32(fixed32 x)
{
    fixed64 x2, temp;
    int     sign = 1;

    if(x < 0)
    {
        sign = -1;
        x = -x;
    }
    while (x > 0x19220)
    {
        x -= M_PI_F;
        sign = -sign;
    }
    if (x > 0x19220)
    {
        x = M_PI_F - x;
    }
    x2 = (fixed64)x * x;
    x2 >>= PRECISION;
    if(sign != 1)
    {
        x = -x;
    }
    /**
    temp = ftofix32(-.0000000239f) * x2;
    temp >>= PRECISION;
    **/
    temp = 0; /* PJJ */
    temp = (temp + 0x0) * x2;
    temp >>= PRECISION;
    temp = (temp - 0xd) * x2;
    temp >>= PRECISION;
    temp = (temp + 0x222) * x2;
    temp >>= PRECISION;
    temp = (temp - 0x2aab) * x2;
    temp >>= PRECISION;
    temp += 0x10000;
    temp = temp * x;
    temp >>= PRECISION;

    return  (fixed32)(temp);
}

__inline fixed32 fixcos32(fixed32 x)
{
    return fixsin32(x - (M_PI_F>>1))*-1;
}

__inline fixed32 fixasin32(fixed32 x)
{
    fixed64 temp;
    int     sign = 1;

    if(x > 0x10000 || x < 0xffff0000)
    {
        return 0;
    }
    if(x < 0)
    {
        sign = -1;
        x = -x;
    }
    temp = 0xffffffad * (fixed64)x;
    temp >>= PRECISION;
    temp = (temp + 0x1b5) * x;
    temp >>= PRECISION;
    temp = (temp - 0x460) * x;
    temp >>= PRECISION;
    temp = (temp + 0x7e9) * x;
    temp >>= PRECISION;
    temp = (temp - 0xcd8) * x;
    temp >>= PRECISION;
    temp = (temp + 0x16c7) * x;
    temp >>= PRECISION;
    temp = (temp - 0x36f0) * x;
    temp >>= PRECISION;
    temp = (temp + 0x19220) * fixsqrt32(0x10000 - x);
    temp >>= PRECISION;

    return sign * ((M_PI_F>>1) - (fixed32)temp);
}

#define ALT_BITSTREAM_READER

#define unaligned32(a) (*(uint32_t*)(a))

uint16_t bswap_16(uint16_t x)
{
    uint16_t hi = x & 0xff00;
    uint16_t lo = x & 0x00ff;
    return (hi >> 8) | (lo << 8);
}

uint32_t bswap_32(uint32_t x)
{
    uint32_t b1 = x & 0xff000000;
    uint32_t b2 = x & 0x00ff0000;
    uint32_t b3 = x & 0x0000ff00;
    uint32_t b4 = x & 0x000000ff;
    return (b1 >> 24) | (b2 >> 8) | (b3 << 8) | (b4 << 24);
}

/* PJJ : reinstate macro */
void CMUL(fixed32 *pre,
          fixed32 *pim,
          fixed32 are,
          fixed32 aim,
          fixed32 bre,
          fixed32 bim)
{
    fixed32 _aref = are;
    fixed32 _aimf = aim;
    fixed32 _bref = bre;
    fixed32 _bimf = bim;
    fixed32 _r1 = fixmul32(_aref, _bref);
    fixed32 _r2 = fixmul32(_aimf, _bimf);
    fixed32 _r3 = fixmul32(_aref, _bimf);
    fixed32 _r4 = fixmul32(_aimf, _bref);
    *pre = _r1 - _r2;
    *pim = _r3 + _r4;
}


#ifdef WORDS_BIGENDIAN
#define be2me_16(x) (x)
#define be2me_32(x) (x)
#define be2me_64(x) (x)
#define le2me_16(x) bswap_16(x)
#define le2me_32(x) bswap_32(x)
#define le2me_64(x) bswap_64(x)
#else
#define be2me_16(x) bswap_16(x)
#define be2me_32(x) bswap_32(x)
#define be2me_64(x) bswap_64(x)
#define le2me_16(x) (x)
#define le2me_32(x) (x)
#define le2me_64(x) (x)
#endif

#define NEG_SSR32(a,s) ((( int32_t)(a))>>(32-(s)))
#define NEG_USR32(a,s) (((uint32_t)(a))>>(32-(s)))


static inline int uwma_unaligned32_be(const void *v)
{
#ifdef CONFIG_ALIGN
    const uint8_t *p=v;
    return (((p[0]<<8) | p[1])<<16) | (p[2]<<8) | (p[3]);
#else

    return be2me_32( unaligned32(v)); /* original */
#endif
}

typedef struct GetBitContext
{
    const uint8_t *buffer, *buffer_end;
#ifdef ALT_BITSTREAM_READER

    int index;
#elif defined LIBMPEG2_BITSTREAM_READER

    uint8_t *buffer_ptr;
    uint32_t cache;
    int bit_count;
#elif defined A32_BITSTREAM_READER

    uint32_t *buffer_ptr;
    uint32_t cache0;
    uint32_t cache1;
    int bit_count;
#endif

    int size_in_bits;
}
GetBitContext;

typedef struct VLC
{
    int bits;
    VLC_TYPE (*table)[2];
    int table_size, table_allocated;
}
VLC;

typedef struct FFTComplex
{
    fixed32 re, im;
}
FFTComplex;

typedef struct FFTContext
{
    int nbits;
    int inverse;
    uint16_t *revtab;
    FFTComplex *exptab;
    FFTComplex *exptab1; /* only used by SSE code */
    void (*fft_calc)(struct FFTContext *s, FFTComplex *z);
}
FFTContext;

typedef struct MDCTContext
{
    int n;  /* size of MDCT (i.e. number of input data * 2) */
    int nbits; /* n = 2^nbits */
    /* pre/post rotation tables */
    fixed32 *tcos;
    fixed32 *tsin;
    FFTContext fft;
}
MDCTContext;

typedef struct WMADecodeContext
{
    GetBitContext gb;
    int sample_rate;
    int nb_channels;
    int bit_rate;
    int version; /* 1 = 0x160 (WMAV1), 2 = 0x161 (WMAV2) */
    int block_align;
    int use_bit_reservoir;
    int use_variable_block_len;
    int use_exp_vlc;  /* exponent coding: 0 = lsp, 1 = vlc + delta */
    int use_noise_coding; /* true if perceptual noise is added */
    int byte_offset_bits;
    VLC exp_vlc;
    int exponent_sizes[BLOCK_NB_SIZES];
    uint16_t exponent_bands[BLOCK_NB_SIZES][25];
    int high_band_start[BLOCK_NB_SIZES]; /* index of first coef in high band */
    int coefs_start;               /* first coded coef */
    int coefs_end[BLOCK_NB_SIZES]; /* max number of coded coefficients */
    int exponent_high_sizes[BLOCK_NB_SIZES];
    int exponent_high_bands[BLOCK_NB_SIZES][HIGH_BAND_MAX_SIZE];
    VLC hgain_vlc;

    /* coded values in high bands */
    int high_band_coded[MAX_CHANNELS][HIGH_BAND_MAX_SIZE];
    int high_band_values[MAX_CHANNELS][HIGH_BAND_MAX_SIZE];

    /* there are two possible tables for spectral coefficients */
    VLC coef_vlc[2];
    uint16_t *run_table[2];
    uint16_t *level_table[2];
    /* frame info */
    int frame_len;       /* frame length in samples */
    int frame_len_bits;  /* frame_len = 1 << frame_len_bits */
    int nb_block_sizes;  /* number of block sizes */
    /* block info */
    int reset_block_lengths;
    int block_len_bits; /* log2 of current block length */
    int next_block_len_bits; /* log2 of next block length */
    int prev_block_len_bits; /* log2 of prev block length */
    int block_len; /* block length in samples */
    int block_num; /* block number in current frame */
    int block_pos; /* current position in frame */
    uint8_t ms_stereo; /* true if mid/side stereo mode */
    uint8_t channel_coded[MAX_CHANNELS]; /* true if channel is coded */
    fixed32 exponents[MAX_CHANNELS][BLOCK_MAX_SIZE];
    fixed32 max_exponent[MAX_CHANNELS];
    int16_t coefs1[MAX_CHANNELS][BLOCK_MAX_SIZE];
    fixed32 coefs[MAX_CHANNELS][BLOCK_MAX_SIZE];
    MDCTContext mdct_ctx[BLOCK_NB_SIZES];
    fixed32 *windows[BLOCK_NB_SIZES];
    FFTComplex mdct_tmp[BLOCK_MAX_SIZE]; /* temporary storage for imdct */
    /* output buffer for one frame and the last for IMDCT windowing */
    fixed32 frame_out[MAX_CHANNELS][BLOCK_MAX_SIZE * 2];
    /* last frame info */
    uint8_t last_superframe[MAX_CODED_SUPERFRAME_SIZE + 4]; /* padding added */
    int last_bitoffset;
    int last_superframe_len;
    fixed32 noise_table[NOISE_TAB_SIZE];
    int noise_index;
    fixed32 noise_mult; /* XXX: suppress that and integrate it in the noise array */
    /* lsp_to_curve tables */
    fixed32 lsp_cos_table[BLOCK_MAX_SIZE];
    fixed64 lsp_pow_e_table[256];
    fixed64 lsp_pow_m_table1[(1 << LSP_POW_BITS)];
    fixed64 lsp_pow_m_table2[(1 << LSP_POW_BITS)];

#ifdef TRACE

    int frame_count;
#endif
} WMADecodeContext;

/*** Prototypes ***/

static void wma_lsp_to_curve_init(WMADecodeContext *s, int frame_len);
void fft_calc(FFTContext *s, FFTComplex *z);
void av_free(void *ptr); /* PJJ found below */

static inline int av_log2(unsigned int v)
{
    int n;

    n = 0;
    if (v & 0xffff0000)
    {
        v >>= 16;
        n += 16;
    }
    if (v & 0xff00)
    {
        v >>= 8;
        n += 8;
    }
    n += ff_log2_tab[v];

    return n;
}

#ifdef ALT_BITSTREAM_READER
#   define MIN_CACHE_BITS 25

#   define OPEN_READER(name, gb)\
        int name##_index= (gb)->index;\
        int name##_cache= 0;\

#   define CLOSE_READER(name, gb)\
        (gb)->index= name##_index;\

#   define UPDATE_CACHE(name, gb)\
        name##_cache= uwma_unaligned32_be( ((uint8_t *)(gb)->buffer)+(name##_index>>3) ) << (name##_index&0x07);\

#   define SKIP_CACHE(name, gb, num)\
        name##_cache <<= (num);\

#   define SKIP_COUNTER(name, gb, num)\
        name##_index += (num);\

#   define SKIP_BITS(name, gb, num)\
        {\
            SKIP_CACHE(name, gb, num)\
            SKIP_COUNTER(name, gb, num)\
        }\

#   define LAST_SKIP_BITS(name, gb, num) SKIP_COUNTER(name, gb, num)
#   define LAST_SKIP_CACHE(name, gb, num) ;

#   define SHOW_UBITS(name, gb, num)\
        NEG_USR32(name##_cache, num)

#   define SHOW_SBITS(name, gb, num)\
        NEG_SSR32(name##_cache, num)

#   define GET_CACHE(name, gb)\
        ((uint32_t)name##_cache)

static inline int uwma_get_bits_count(GetBitContext *s)
{
    return s->index;
}
#elif defined LIBMPEG2_BITSTREAM_READER

#   define MIN_CACHE_BITS 17

#   define OPEN_READER(name, gb)\
        int name##_bit_count=(gb)->bit_count;\
        int name##_cache= (gb)->cache;\
        uint8_t * name##_buffer_ptr=(gb)->buffer_ptr;\

#   define CLOSE_READER(name, gb)\
        (gb)->bit_count= name##_bit_count;\
        (gb)->cache= name##_cache;\
        (gb)->buffer_ptr= name##_buffer_ptr;\

#ifdef LIBMPEG2_BITSTREAM_READER_HACK

#   define UPDATE_CACHE(name, gb)\
    if(name##_bit_count >= 0){\
        name##_cache+= (int)be2me_16(*(uint16_t*)name##_buffer_ptr) << name##_bit_count;\
        ((uint16_t*)name##_buffer_ptr)++;\
        name##_bit_count-= 16;\
    }\

#else

#   define UPDATE_CACHE(name, gb)\
    if(name##_bit_count >= 0){\
        name##_cache+= ((name##_buffer_ptr[0]<<8) + name##_buffer_ptr[1]) << name##_bit_count;\
        name##_buffer_ptr+=2;\
        name##_bit_count-= 16;\
    }\

#endif

#   define SKIP_CACHE(name, gb, num)\
        name##_cache <<= (num);\

#   define SKIP_COUNTER(name, gb, num)\
        name##_bit_count += (num);\

#   define SKIP_BITS(name, gb, num)\
        {\
            SKIP_CACHE(name, gb, num)\
            SKIP_COUNTER(name, gb, num)\
        }\

#   define LAST_SKIP_BITS(name, gb, num) SKIP_BITS(name, gb, num)
#   define LAST_SKIP_CACHE(name, gb, num) SKIP_CACHE(name, gb, num)

#   define SHOW_UBITS(name, gb, num)\
        NEG_USR32(name##_cache, num)

#   define SHOW_SBITS(name, gb, num)\
        NEG_SSR32(name##_cache, num)

#   define GET_CACHE(name, gb)\
        ((uint32_t)name##_cache)

static inline int uwma_get_bits_count(GetBitContext *s)
{
    return (s->buffer_ptr - s->buffer)*8 - 16 + s->bit_count;
}

#elif defined A32_BITSTREAM_READER

#   define MIN_CACHE_BITS 32

#   define OPEN_READER(name, gb)\
        int name##_bit_count=(gb)->bit_count;\
        uint32_t name##_cache0= (gb)->cache0;\
        uint32_t name##_cache1= (gb)->cache1;\
        uint32_t * name##_buffer_ptr=(gb)->buffer_ptr;\

#   define CLOSE_READER(name, gb)\
        (gb)->bit_count= name##_bit_count;\
        (gb)->cache0= name##_cache0;\
        (gb)->cache1= name##_cache1;\
        (gb)->buffer_ptr= name##_buffer_ptr;\

#   define UPDATE_CACHE(name, gb)\
    if(name##_bit_count > 0){\
        const uint32_t next= be2me_32( *name##_buffer_ptr );\
        name##_cache0 |= NEG_USR32(next,name##_bit_count);\
        name##_cache1 |= next<<name##_bit_count;\
        name##_buffer_ptr++;\
        name##_bit_count-= 32;\
    }\

#ifdef ARCH_X86
#   define SKIP_CACHE(name, gb, num)\
        asm(\
            "shldl %2, %1, %0  \n\t"\
            "shll %2, %1  \n\t"\
            : "+r" (name##_cache0), "+r" (name##_cache1)\
            : "Ic" ((uint8_t)num)\
           );
#else
#   define SKIP_CACHE(name, gb, num)\
        name##_cache0 <<= (num);\
        name##_cache0 |= NEG_USR32(name##_cache1,num);\
        name##_cache1 <<= (num);
#endif

#   define SKIP_COUNTER(name, gb, num)\
        name##_bit_count += (num);\

#   define SKIP_BITS(name, gb, num)\
        {\
            SKIP_CACHE(name, gb, num)\
            SKIP_COUNTER(name, gb, num)\
        }\

#   define LAST_SKIP_BITS(name, gb, num) SKIP_BITS(name, gb, num)
#   define LAST_SKIP_CACHE(name, gb, num) SKIP_CACHE(name, gb, num)

#   define SHOW_UBITS(name, gb, num)\
        NEG_USR32(name##_cache0, num)

#   define SHOW_SBITS(name, gb, num)\
        NEG_SSR32(name##_cache0, num)

#   define GET_CACHE(name, gb)\
        (name##_cache0)

static inline int uwma_get_bits_count(GetBitContext *s)
{
    return ((uint8_t*)s->buffer_ptr - s->buffer)*8 - 32 + s->bit_count;
}

#endif

/**
 * read mpeg1 dc style vlc (sign bit + mantisse with no MSB).
 * if MSB not set it is negative
 * @param n length in bits
 * @author BERO
 */
static inline int get_xbits(GetBitContext *s, int n)
{
    register int tmp;
    register int32_t cache;
    OPEN_READER(re, s)
    UPDATE_CACHE(re, s)
    cache = GET_CACHE(re,s);
    if ((int32_t)cache<0)
    {
        tmp = NEG_USR32(cache,n);
    }
    else
    {
        tmp = - NEG_USR32(~cache,n);
    }
    LAST_SKIP_BITS(re, s, n)
    CLOSE_READER(re, s)
    return tmp;
}

static inline int get_sbits(GetBitContext *s, int n)
{
    register int tmp;
    OPEN_READER(re, s)
    UPDATE_CACHE(re, s)
    tmp= SHOW_SBITS(re, s, n);
    LAST_SKIP_BITS(re, s, n)
    CLOSE_READER(re, s)
    return tmp;
}

/**
 * reads 0-17 bits.
 * Note, the alt bitstream reader can read upto 25 bits, but the libmpeg2 reader cant
 */
static inline unsigned int uwma_get_bits(GetBitContext *s, int n)
{
    register int tmp;
    OPEN_READER(re, s)
    UPDATE_CACHE(re, s)
    tmp= SHOW_UBITS(re, s, n);
    LAST_SKIP_BITS(re, s, n)
    CLOSE_READER(re, s)
    return tmp;
}


/**
 *
 * if the vlc code is invalid and max_depth=1 than no bits will be removed
 * if the vlc code is invalid and max_depth>1 than the number of bits removed
 * is undefined
 */
#define GET_VLC(code, name, gb, table, bits, max_depth)\
{\
    int n, index, nb_bits;\
\
    index= SHOW_UBITS(name, gb, bits);\
    code = table[index][0];\
    n    = table[index][1];\
\
    if(max_depth > 1 && n < 0){\
        LAST_SKIP_BITS(name, gb, bits)\
        UPDATE_CACHE(name, gb)\
\
        nb_bits = -n;\
\
        index= SHOW_UBITS(name, gb, nb_bits) + code;\
        code = table[index][0];\
        n    = table[index][1];\
        if(max_depth > 2 && n < 0){\
            LAST_SKIP_BITS(name, gb, nb_bits)\
            UPDATE_CACHE(name, gb)\
\
            nb_bits = -n;\
\
            index= SHOW_UBITS(name, gb, nb_bits) + code;\
            code = table[index][0];\
            n    = table[index][1];\
        }\
    }\
    SKIP_BITS(name, gb, n)\
}

static inline void uwma_skip_bits(GetBitContext *s, int n)
{
    OPEN_READER(re, s)
    UPDATE_CACHE(re, s)
    LAST_SKIP_BITS(re, s, n)
    CLOSE_READER(re, s)
}

void uwma_align_get_bits(GetBitContext *s)
{
    int n= (-uwma_get_bits_count(s)) & 7;
    if(n)
        uwma_skip_bits(s, n);
}

/**
 * init GetBitContext.
 * @param buffer bitstream buffer, must be FF_INPUT_BUFFER_PADDING_SIZE bytes larger then the actual read bits
 * because some optimized bitstream readers read 32 or 64 bit at once and could read over the end
 * @param bit_size the size of the buffer in bits
 */
void uwma_init_get_bits(GetBitContext *s,
                   const uint8_t *buffer, int bit_size)
{
    int buffer_size= (bit_size+7)>>3;

    if (buffer_size < 0 || bit_size < 0)
    {
        buffer_size = bit_size = 0;
        buffer = 0;
    }
    s->buffer= buffer;
    s->size_in_bits= bit_size;
    s->buffer_end= buffer + buffer_size;
#ifdef ALT_BITSTREAM_READER

    s->index=0;
#elif defined LIBMPEG2_BITSTREAM_READER
#ifdef LIBMPEG2_BITSTREAM_READER_HACK

    if ((int)buffer&1)
    {
        /* word alignment */
        s->cache = (*buffer++)<<24;
        s->buffer_ptr = buffer;
        s->bit_count = 16-8;
    }
    else
#endif

    {
        s->buffer_ptr = buffer;
        s->bit_count = 16;
        s->cache = 0;
    }
#elif defined A32_BITSTREAM_READER
    s->buffer_ptr = (uint32_t*)buffer;
    s->bit_count = 32;
    s->cache0 = 0;
    s->cache1 = 0;
#endif

    {
        OPEN_READER(re, s)
        UPDATE_CACHE(re, s)
        UPDATE_CACHE(re, s)
        CLOSE_READER(re, s)
    }
#ifdef A32_BITSTREAM_READER
    s->cache1 = 0;
#endif
}

/**
 * Memory allocation of size byte with alignment suitable for all
 * memory accesses (including vectors if available on the
 * CPU). av_malloc(0) must return a non NULL pointer.
 */
void *av_malloc(unsigned int size)
{
    void *ptr;

#if defined (HAVE_MEMALIGN)

    ptr = memalign(16,size);
    /* Why 64?
       Indeed, we should align it:
         on 4 for 386
         on 16 for 486
    on 32 for 586, PPro - k6-III
    on 64 for K7 (maybe for P3 too).
       Because L1 and L2 caches are aligned on those values.
       But I don't want to code such logic here!
     */
    /* Why 16?
       because some cpus need alignment, for example SSE2 on P4, & most RISC cpus
       it will just trigger an exception and the unaligned load will be done in the
       exception handler or it will just segfault (SSE2 on P4)
       Why not larger? because i didnt see a difference in benchmarks ...
    */
    /* benchmarks with p3
       memalign(64)+1  3071,3051,3032
       memalign(64)+2  3051,3032,3041
       memalign(64)+4  2911,2896,2915
       memalign(64)+8  2545,2554,2550
       memalign(64)+16  2543,2572,2563
       memalign(64)+32  2546,2545,2571
       memalign(64)+64  2570,2533,2558

       btw, malloc seems to do 8 byte alignment by default here
    */
#else

    ptr = malloc(size);
#endif

    return ptr;
}

/**
 * av_realloc semantics (same as glibc): if ptr is NULL and size > 0,
 * identical to malloc(size). If size is zero, it is identical to
 * free(ptr) and NULL is returned.
 */
void *av_realloc(void *ptr, unsigned int size)
{
    return realloc(ptr, size);
}

/* NOTE: ptr = NULL is explicitly allowed */
void av_free(void *ptr)
{
    /* XXX: this test should not be needed on most libcs */
    if (ptr)
        free(ptr);
}

/* cannot call it directly because of 'void **' casting is not automatic */
void __av_freepy(void **ptr)
{
    av_free(*ptr);
    *ptr = NULL;
}
#define av_freep(p) __av_freepy((void **)(p))

/**
 * The size of the FFT is 2^nbits. If inverse is TRUE, inverse FFT is
 * done
 */
int fft_inits(FFTContext *s, int nbits, int inverse)
{
    int i, j, m, n;
    fixed32 alpha, c1, s1;
    int s2;

    s->nbits = nbits;
    n = 1 << nbits;

    s->exptab = av_malloc((n >> 1) * sizeof(FFTComplex));
    if (!s->exptab)
        goto fail;
    s->revtab = av_malloc(n * sizeof(uint16_t));
    if (!s->revtab)
        goto fail;
    s->inverse = inverse;

    s2 = inverse ? 1 : -1;

    for(i=0;i<(n/2);++i)
    {
        fixed32 ifix = itofix32(i);
        fixed32 nfix = itofix32(n);
        fixed32 res = fixdiv32(ifix,nfix);
        fixed32 pi2 = fixmul32(0x20000, M_PI_F);
        alpha = fixmul32(pi2, res);
        c1 = fixcos32(alpha);
        s1 = fixsin32(alpha) * s2;
        s->exptab[i].re = c1;
        s->exptab[i].im = s1;
    }

    s->fft_calc = fft_calc;
    s->exptab1 = NULL;
    /* compute constant table for HAVE_SSE version */
#if (defined(HAVE_MMX) && defined(HAVE_BUILTIN_VECTOR)) || defined(HAVE_ALTIVEC)

    {
        int has_vectors = 0;

#if defined(HAVE_MMX)

        has_vectors = mm_support() & MM_SSE;
#endif
#if defined(HAVE_ALTIVEC) && !defined(ALTIVEC_USE_REFERENCE_C_CODE)

        has_vectors = mm_support() & MM_ALTIVEC;
#endif

        if (has_vectors)
        {
            int np, nblocks, np2, l;
            FFTComplex *q;

            np = 1 << nbits;
            nblocks = np >> 3;
            np2 = np >> 1;
            s->exptab1 = av_malloc(np * 2 * sizeof(FFTComplex));
            if (!s->exptab1)
                goto fail;
            q = s->exptab1;
            do
            {
                for(l = 0; l < np2; l += 2 * nblocks)
                {
                    *q++ = s->exptab[l];
                    *q++ = s->exptab[l + nblocks];

                    q->re = -s->exptab[l].im;
                    q->im = s->exptab[l].re;
                    q++;
                    q->re = -s->exptab[l + nblocks].im;
                    q->im = s->exptab[l + nblocks].re;
                    q++;
                }
                nblocks = nblocks >> 1;
            }
            while (nblocks != 0);
            av_freep(&s->exptab);
#if defined(HAVE_MMX)

            s->fft_calc = fft_calc_sse;
#else

            s->fft_calc = fft_calc_altivec;
#endif

        }
    }
#endif

    /* compute bit reverse table */

    for(i=0;i<n;i++)
    {
        m=0;
        for(j=0;j<nbits;j++)
        {
            m |= ((i >> j) & 1) << (nbits-j-1);
        }
        s->revtab[i]=m;
    }
    return 0;
fail:
    av_freep(&s->revtab);
    av_freep(&s->exptab);
    av_freep(&s->exptab1);
    return -1;
}

/* butter fly op */
#define BF(pre, pim, qre, qim, pre1, pim1, qre1, qim1) \
{\
  fixed32 ax, ay, bx, by;\
  bx=pre1;\
  by=pim1;\
  ax=qre1;\
  ay=qim1;\
  pre = (bx + ax);\
  pim = (by + ay);\
  qre = (bx - ax);\
  qim = (by - ay);\
}

/**
 * Do a complex FFT with the parameters defined in fft_init(). The
 * input data must be permuted before with s->revtab table. No
 * 1.0/sqrt(n) normalization is done.
 */
void fft_calc(FFTContext *s, FFTComplex *z)
{
    int ln = s->nbits;
    int j, np, np2;
    int nblocks, nloops;
    register FFTComplex *p, *q;
    FFTComplex *exptab = s->exptab;
    int l;
    fixed32 tmp_re, tmp_im;

    np = 1 << ln;

    /* pass 0 */

    p=&z[0];
    j=(np >> 1);
    do
    {
        BF(p[0].re, p[0].im, p[1].re, p[1].im,
           p[0].re, p[0].im, p[1].re, p[1].im);
        p+=2;
    }
    while (--j != 0);

    /* pass 1 */


    p=&z[0];
    j=np >> 2;
    if (s->inverse)
    {
        do
        {
            BF(p[0].re, p[0].im, p[2].re, p[2].im,
               p[0].re, p[0].im, p[2].re, p[2].im);
            BF(p[1].re, p[1].im, p[3].re, p[3].im,
               p[1].re, p[1].im, -p[3].im, p[3].re);
            p+=4;
        }
        while (--j != 0);
    }
    else
    {
        do
        {
            BF(p[0].re, p[0].im, p[2].re, p[2].im,
               p[0].re, p[0].im, p[2].re, p[2].im);
            BF(p[1].re, p[1].im, p[3].re, p[3].im,
               p[1].re, p[1].im, p[3].im, -p[3].re);
            p+=4;
        }
        while (--j != 0);
    }
    /* pass 2 .. ln-1 */

    nblocks = np >> 3;
    nloops = 1 << 2;
    np2 = np >> 1;
    do
    {
        p = z;
        q = z + nloops;
        for (j = 0; j < nblocks; ++j)
        {
            BF(p->re, p->im, q->re, q->im,
               p->re, p->im, q->re, q->im);

            p++;
            q++;
            for(l = nblocks; l < np2; l += nblocks)
            {
                CMUL(&tmp_re, &tmp_im, exptab[l].re, exptab[l].im, q->re, q->im);
                BF(p->re, p->im, q->re, q->im,
                   p->re, p->im, tmp_re, tmp_im);
                p++;
                q++;
            }

            p += nloops;
            q += nloops;
        }
        nblocks = nblocks >> 1;
        nloops = nloops << 1;
    }
    while (nblocks != 0);
}

/**
 * Do the permutation needed BEFORE calling fft_calc()
 */
void fft_permute(FFTContext *s, FFTComplex *z)
{
    int j, k, np;
    FFTComplex tmp;
    const uint16_t *revtab = s->revtab;

    /* reverse */
    np = 1 << s->nbits;
    for(j=0;j<np;j++)
    {
        k = revtab[j];
        if (k < j)
        {
            tmp = z[k];
            z[k] = z[j];
            z[j] = tmp;
        }
    }
}

void fft_end(FFTContext *s)
{
    av_freep(&s->revtab);
    av_freep(&s->exptab);
    av_freep(&s->exptab1);
}
/* VLC decoding */

#define GET_DATA(v, table, i, wrap, size) \
{\
    const uint8_t *ptr = (const uint8_t *)table + i * wrap;\
    switch(size) {\
    case 1:\
        v = *(const uint8_t *)ptr;\
        break;\
    case 2:\
        v = *(const uint16_t *)ptr;\
        break;\
    default:\
        v = *(const uint32_t *)ptr;\
        break;\
    }\
}

/* deprecated, dont use get_vlc for new code, use get_vlc2 instead or use GET_VLC directly */
static inline int get_vlc(GetBitContext *s, VLC *vlc)
{
    int code;
    VLC_TYPE (*table)[2]= vlc->table;

    OPEN_READER(re, s)
    UPDATE_CACHE(re, s)

    GET_VLC(code, re, s, table, vlc->bits, 3)

    CLOSE_READER(re, s)
    return code;
}

static int uwma_alloc_table(VLC *vlc, int size)
{
    int index;
    index = vlc->table_size;
    vlc->table_size += size;
    if (vlc->table_size > vlc->table_allocated)
    {
        vlc->table_allocated += (1 << vlc->bits);
        vlc->table = av_realloc(vlc->table,
                                sizeof(VLC_TYPE) * 2 * vlc->table_allocated);
        if (!vlc->table)
            return -1;
    }
    return index;
}

static int uwma_build_table(VLC *vlc, int table_nb_bits,
                       int nb_codes,
                       const void *bits, int bits_wrap, int bits_size,
                       const void *codes, int codes_wrap, int codes_size,
                       uint32_t code_prefix, int n_prefix)
{
    int i, j, k, n, table_size, table_index, nb, n1, index;
    uint32_t code;
    VLC_TYPE (*table)[2];

    table_size = 1 << table_nb_bits;
    table_index = uwma_alloc_table(vlc, table_size);
    if (table_index < 0)
        return -1;
    table = &vlc->table[table_index];

    for(i=0;i<table_size;i++)
    {
        table[i][1] = 0;
        table[i][0] = -1;
    }

    /* first pass: map codes and compute auxillary table sizes */
    for(i=0;i<nb_codes;i++)
    {
        GET_DATA(n, bits, i, bits_wrap, bits_size);
        GET_DATA(code, codes, i, codes_wrap, codes_size);
        /* we accept tables with holes */
        if (n <= 0)
            continue;
        /* if code matches the prefix, it is in the table */
        n -= n_prefix;
        if (n > 0 && (code >> n) == code_prefix)
        {
            if (n <= table_nb_bits)
            {
                /* no need to add another table */
                j = (code << (table_nb_bits - n)) & (table_size - 1);
                nb = 1 << (table_nb_bits - n);
                for(k=0;k<nb;k++)
                {
                    table[j][1] = n;
                    table[j][0] = i;
                    j++;
                }
            }
            else
            {
                n -= table_nb_bits;
                j = (code >> n) & ((1 << table_nb_bits) - 1);
                /* compute table size */
                n1 = -table[j][1];
                if (n > n1)
                    n1 = n;
                table[j][1] = -n1;
            }
        }
    }

    /* second pass : fill auxillary tables recursively */
    for(i=0;i<table_size;i++)
    {
        n = table[i][1];
        if (n < 0)
        {
            n = -n;
            if (n > table_nb_bits)
            {
                n = table_nb_bits;
                table[i][1] = -n;
            }
            index = uwma_build_table(vlc, n, nb_codes,
                                bits, bits_wrap, bits_size,
                                codes, codes_wrap, codes_size,
                                (code_prefix << table_nb_bits) | i,
                                n_prefix + table_nb_bits);
            if (index < 0)
                return -1;
            /* note: realloc has been done, so reload tables */
            table = &vlc->table[table_index];
            table[i][0] = index;
        }
    }
    return table_index;
}

/* Build VLC decoding tables suitable for use with get_vlc().

   'nb_bits' set thee decoding table size (2^nb_bits) entries. The
   bigger it is, the faster is the decoding. But it should not be too
   big to save memory and L1 cache. '9' is a good compromise.

   'nb_codes' : number of vlcs codes

   'bits' : table which gives the size (in bits) of each vlc code.

   'codes' : table which gives the bit pattern of of each vlc code.

   'xxx_wrap' : give the number of bytes between each entry of the
   'bits' or 'codes' tables.

   'xxx_size' : gives the number of bytes of each entry of the 'bits'
   or 'codes' tables.

   'wrap' and 'size' allows to use any memory configuration and types
   (byte/word/long) to store the 'bits' and 'codes' tables.
*/
int uwma_init_vlc(VLC *vlc, int nb_bits, int nb_codes,
             const void *bits, int bits_wrap, int bits_size,
             const void *codes, int codes_wrap, int codes_size)
{
    vlc->bits = nb_bits;
    vlc->table = NULL;
    vlc->table_allocated = 0;
    vlc->table_size = 0;

    if (uwma_build_table(vlc, nb_bits, nb_codes,
                    bits, bits_wrap, bits_size,
                    codes, codes_wrap, codes_size,
                    0, 0) < 0)
    {
        av_free(vlc->table);
        return -1;
    }
    return 0;
}

/**
 * init MDCT or IMDCT computation.
 */
int ff_mdct_init(MDCTContext *s, int nbits, int inverse)
{
    int n, n4, i;
    fixed32 alpha;
    memset(s, 0, sizeof(*s));
    n = 1 << nbits;
    s->nbits = nbits;
    s->n = n;
    n4 = n >> 2;
    s->tcos = av_malloc(n4 * sizeof(fixed32));
    if (!s->tcos)
        goto fail;
    s->tsin = av_malloc(n4 * sizeof(fixed32));
    if (!s->tsin)
        goto fail;

    for(i=0;i<n4;i++)
    {
        fixed32 pi2 = fixmul32(0x20000, M_PI_F);
        fixed32 ip = itofix32(i) + 0x2000;
        ip = fixdiv32(ip,itofix32(n)); /* PJJ optimize */
        alpha = fixmul32(pi2, ip);
        s->tcos[i] = -fixcos32(alpha);
        s->tsin[i] = -fixsin32(alpha);
    }
    if (fft_inits(&s->fft, s->nbits - 2, inverse) < 0)
        goto fail;
    return 0;
fail:
    av_freep(&s->tcos);
    av_freep(&s->tsin);
    return -1;
}

/**
 * Compute inverse MDCT of size N = 2^nbits
 * @param output N samples
 * @param input N/2 samples
 * @param tmp N/2 samples
 */
void ff_imdct_calc(MDCTContext *s,
                   fixed32 *output,
                   const fixed32 *input,
                   FFTComplex *tmp)
{
    int k, n8, n4, n2, n, j;
    const uint16_t *revtab = s->fft.revtab;
    const fixed32 *tcos = s->tcos;
    const fixed32 *tsin = s->tsin;
    const fixed32 *in1, *in2;
    FFTComplex *z = (FFTComplex *)tmp;

    n = 1 << s->nbits;
    n2 = n >> 1;
    n4 = n >> 2;
    n8 = n >> 3;

    /* pre rotation */
    in1 = input;
    in2 = input + n2 - 1;
    for(k = 0; k < n4; k++)
    {
        j=revtab[k];
        CMUL(&z[j].re, &z[j].im, *in2, *in1, tcos[k], tsin[k]);
        in1 += 2;
        in2 -= 2;
    }
    fft_calc(&s->fft, z);

    /* post rotation + reordering */
    /* XXX: optimize */
    for(k = 0; k < n4; k++)
    {
        CMUL(&z[k].re, &z[k].im, z[k].re, z[k].im, tcos[k], tsin[k]);
    }
    for(k = 0; k < n8; k++)
    {
        fixed32 r1,r2,r3,r4,r1n,r2n,r3n;

        r1 = z[n8 + k].im;
        r1n = r1 * -1.0;
        r2 = z[n8-1-k].re;
        r2n = r2 * -1.0;
        r3 = z[k+n8].re;
        r3n = r3 * -1.0;
        r4 = z[n8-k-1].im;

        output[2*k] = r1n;
        output[n2-1-2*k] = r1;

        output[2*k+1] = r2;
        output[n2-1-2*k-1] = r2n;

        output[n2 + 2*k]= r3n;
        output[n-1- 2*k]= r3n;

        output[n2 + 2*k+1]= r4;
        output[n-2 - 2 * k] = r4;
    }
}

void ff_mdct_end(MDCTContext *s)
{
    av_freep(&s->tcos);
    av_freep(&s->tsin);
    fft_end(&s->fft);
}

static void init_coef_vlc(VLC *vlc,
                          uint16_t **prun_table, uint16_t **plevel_table,
                          const CoefVLCTable *vlc_table)
{
    int n = vlc_table->n;
    const uint8_t *table_bits = vlc_table->huffbits;
    const uint32_t *table_codes = vlc_table->huffcodes;
    const uint16_t *levels_table = vlc_table->levels;
    uint16_t *run_table, *level_table;
    const uint16_t *p;
    int i, l, j, level;

    uwma_init_vlc(vlc, 9, n, table_bits, 1, 1, table_codes, 4, 4);

    run_table = av_malloc(n * sizeof(uint16_t));
    level_table = av_malloc(n * sizeof(uint16_t));
    p = levels_table;
    i = 2;
    level = 1;
    while (i < n)
    {
        l = *p++;
        for(j=0;j<l;++j)
        {
            run_table[i] = j;
            level_table[i] = level;
            ++i;
        }
        ++level;
    }
    *prun_table = run_table;
    *plevel_table = level_table;
}

/* interpolate values for a bigger or smaller block. The block must
   have multiple sizes */
static void interpolate_array(fixed32 *scale, int old_size, int new_size)
{
    int i, j, jincr, k;
    fixed32 v;

    if (new_size > old_size)
    {
        jincr = new_size / old_size;
        j = new_size;
        for(i = old_size - 1; i >=0; --i)
        {
            v = scale[i];
            k = jincr;
            do
            {
                scale[--j] = v;
            }
            while (--k);
        }
    }
    else if (new_size < old_size)
    {
        j = 0;
        jincr = old_size / new_size;
        for(i = 0; i < new_size; ++i)
        {
            scale[i] = scale[j];
            j += jincr;
        }
    }
}

/* compute x^-0.25 with an exponent and mantissa table. We use linear
   interpolation to reduce the mantissa table size at a small speed
   expense (linear interpolation approximately doubles the number of
   bits of precision). */
static inline fixed32 pow_m1_4(WMADecodeContext *s, fixed32 x)
{
    union {
        fixed64 f;
        unsigned int v;
    } u, t;
    unsigned int e, m;
    fixed64 a, b;

    u.f = x;
    e = u.v >> 23;
    m = (u.v >> (23 - LSP_POW_BITS)) & ((1 << LSP_POW_BITS) - 1);
    /* build interpolation scale: 1 <= t < 2. */
    t.v = ((u.v << LSP_POW_BITS) & ((1 << 23) - 1)) | (127 << 23);
    a = s->lsp_pow_m_table1[m];
    b = s->lsp_pow_m_table2[m];
    return lsp_pow_e_table[e] * (a + b * t.f);
}

static void wma_lsp_to_curve_init(WMADecodeContext *s, int frame_len)
{
    fixed32 wdel, a, b;
    int i, m;

    wdel = fixdiv32(M_PI_F, itofix32(frame_len));
    for (i=0; i<frame_len; ++i)
    {
        s->lsp_cos_table[i] = 0x20000 * fixcos32(wdel * i);
    }

    /* NOTE: these two tables are needed to avoid two operations in
       pow_m1_4 */
    b = itofix32(1);
    int ix = 0;
    for(i=(1 << LSP_POW_BITS) - 1;i>=0;i--)
    {
        m = (1 << LSP_POW_BITS) + i;
        a = m * (0x8000 / (1 << LSP_POW_BITS)); /* PJJ */
        a = pow_a_table[++ix];  /* PJJ : further refinement */
        s->lsp_pow_m_table1[i] = 2 * a - b;
        s->lsp_pow_m_table2[i] = b - a;
        b = a;
    }
}

/* NOTE: We use the same code as Vorbis here */
static void wma_lsp_to_curve(WMADecodeContext *s,
                             fixed32 *out,
                             fixed32 *val_max_ptr,
                             int n,
                             fixed32 *lsp)
{
    int i, j;
    fixed32 p, q, w, v, val_max;

    val_max = 0;
    for(i=0;i<n;++i)
    {
        p = 0x8000;
        q = 0x8000;
        w = s->lsp_cos_table[i];
        for (j=1;j<NB_LSP_COEFS;j+=2)
        {
            q *= w - lsp[j - 1];
            p *= w - lsp[j];
        }
        p *= p * (0x20000 - w);
        q *= q * (0x20000 + w);
        v = p + q;
        v = pow_m1_4(s, v); /* PJJ */
        if (v > val_max)
        {
            val_max = v;
        }
        out[i] = v;
    }
    *val_max_ptr = val_max;
}

void free_vlc(VLC *vlc)
{
    av_free(vlc->table);
}

/* decode exponents coded with LSP coefficients (same idea as Vorbis) */
static void decode_exp_lsp(WMADecodeContext *s, int ch)
{
    fixed32 lsp_coefs[NB_LSP_COEFS];
    int val, i;

    for (i = 0; i < NB_LSP_COEFS; ++i)
    {
        if (i == 0 || i >= 8)
        {
            val = uwma_get_bits(&s->gb, 3);
        }
        else
        {
            val = uwma_get_bits(&s->gb, 4);
        }
        lsp_coefs[i] = lsp_codebook[i][val];
    }

    wma_lsp_to_curve(s,
                     s->exponents[ch],
                     &s->max_exponent[ch],
                     s->block_len,
                     lsp_coefs);
}

/* decode exponents coded with VLC codes */
static int decode_exp_vlc(WMADecodeContext *s, int ch)
{
    int last_exp, n, code;
    const uint16_t *ptr, *band_ptr;
    fixed32 v, max_scale;
    fixed32 *q,*q_end;

    band_ptr = s->exponent_bands[s->frame_len_bits - s->block_len_bits];
    ptr = band_ptr;
    q = s->exponents[ch];
    q_end = q + s->block_len;
    max_scale = 0;
    if (s->version == 1)
    {
        last_exp = uwma_get_bits(&s->gb, 5) + 10;
        /* XXX: use a table */
        v = pow_10_to_yover16[last_exp];
        max_scale = v;
        n = *ptr++;
        do
        {
            *q++ = v;
        }
        while (--n);
    }
    last_exp = 36;
    while (q < q_end)
    {
        code = get_vlc(&s->gb, &s->exp_vlc);
        if (code < 0)
        {
            return -1;
        }
        /* NOTE: this offset is the same as MPEG4 AAC ! */
        last_exp += code - 60;
        /* XXX: use a table */
        v = pow_10_to_yover16[last_exp];
        if (v > max_scale)
        {
            max_scale = v;
        }
        n = *ptr++;
        do
        {
            *q++ = v;
        }
        while (--n);
    }
    s->max_exponent[ch] = max_scale;
    return 0;
}

/* return 0 if OK. return 1 if last block of frame. return -1 if
   unrecorrable error. */
static int wma_decode_block(WMADecodeContext *s)
{
    int n, v, a, ch, code, bsize;
    int coef_nb_bits, total_gain, parse_exponents;
    fixed32 window[BLOCK_MAX_SIZE * 2];
    int nb_coefs[MAX_CHANNELS];
    fixed32 mdct_norm;

    /* compute current block length */
    if (s->use_variable_block_len)
    {
        n = av_log2(s->nb_block_sizes - 1) + 1;
        if (s->reset_block_lengths)
        {
            s->reset_block_lengths = 0;
            v = uwma_get_bits(&s->gb, n);
            if (v >= s->nb_block_sizes)
            {
                return -1;
            }
            s->prev_block_len_bits = s->frame_len_bits - v;
            v = uwma_get_bits(&s->gb, n);
            if (v >= s->nb_block_sizes)
            {
                return -1;
            }
            s->block_len_bits = s->frame_len_bits - v;
        }
        else
        {
            /* update block lengths */
            s->prev_block_len_bits = s->block_len_bits;
            s->block_len_bits = s->next_block_len_bits;
        }
        v = uwma_get_bits(&s->gb, n);
        if (v >= s->nb_block_sizes)
        {
            return -1;
        }
        s->next_block_len_bits = s->frame_len_bits - v;
    }
    else
    {
        /* fixed block len */
        s->next_block_len_bits = s->frame_len_bits;
        s->prev_block_len_bits = s->frame_len_bits;
        s->block_len_bits = s->frame_len_bits;
    }
    /* now check if the block length is coherent with the frame length */
    s->block_len = 1 << s->block_len_bits;

    if ((s->block_pos + s->block_len) > s->frame_len)
    {
        return -1;
    }

    if (s->nb_channels == 2)
    {
        s->ms_stereo = uwma_get_bits(&s->gb, 1);
    }
    v = 0;
    for (ch = 0; ch < s->nb_channels; ++ch)
    {
        a = uwma_get_bits(&s->gb, 1);
        s->channel_coded[ch] = a;
        v |= a;
    }
    /* if no channel coded, no need to go further */
    /* XXX: fix potential framing problems */
    if (!v)
    {
        goto next;
    }

    bsize = s->frame_len_bits - s->block_len_bits;

    /* read total gain and extract corresponding number of bits for
       coef escape coding */
    total_gain = 1;
    for(;;)
    {
        a = uwma_get_bits(&s->gb, 7);
        total_gain += a;
        if (a != 127)
        {
            break;
        }
    }

    if (total_gain < 15)
        coef_nb_bits = 13;
    else if (total_gain < 32)
        coef_nb_bits = 12;
    else if (total_gain < 40)
        coef_nb_bits = 11;
    else if (total_gain < 45)
        coef_nb_bits = 10;
    else
        coef_nb_bits = 9;
    /* compute number of coefficients */
    n = s->coefs_end[bsize] - s->coefs_start;

    for(ch = 0; ch < s->nb_channels; ++ch)
    {
        nb_coefs[ch] = n;
    }
    /* complex coding */

    if (s->use_noise_coding)
    {
        for(ch = 0; ch < s->nb_channels; ++ch)
        {
            if (s->channel_coded[ch])
            {
                int i, n, a;
                n = s->exponent_high_sizes[bsize];
                for(i=0;i<n;++i)
                {
                    a = uwma_get_bits(&s->gb, 1);
                    s->high_band_coded[ch][i] = a;
                    /* if noise coding, the coefficients are not transmitted */
                    if (a)
                        nb_coefs[ch] -= s->exponent_high_bands[bsize][i];
                }
            }
        }
        for(ch = 0; ch < s->nb_channels; ++ch)
        {
            if (s->channel_coded[ch])
            {
                int i, n, val, code;

                n = s->exponent_high_sizes[bsize];
                val = (int)0x80000000;
                for(i=0;i<n;++i)
                {
                    if (s->high_band_coded[ch][i])
                    {
                        if (val == (int)0x80000000)
                        {
                            val = uwma_get_bits(&s->gb, 7) - 19;
                        }
                        else
                        {
                            code = get_vlc(&s->gb, &s->hgain_vlc);
                            if (code < 0)
                            {
                                return -1;
                            }
                            val += code - 18;
                        }
                        s->high_band_values[ch][i] = val;
                    }
                }
            }
        }
    }
    /* exposant can be interpolated in short blocks. */
    parse_exponents = 1;
    if (s->block_len_bits != s->frame_len_bits)
    {
        parse_exponents = uwma_get_bits(&s->gb, 1);
    }

    if (parse_exponents)
    {
        for(ch = 0; ch < s->nb_channels; ++ch)
        {
            if (s->channel_coded[ch])
            {
                if (s->use_exp_vlc)
                {
                    if (decode_exp_vlc(s, ch) < 0)
                    {
                        return -1;
                    }
                }
                else
                {
                    decode_exp_lsp(s, ch);
                }
            }
        }
    }
    else
    {
        for(ch = 0; ch < s->nb_channels; ++ch)
        {
            if (s->channel_coded[ch])
            {
                interpolate_array(s->exponents[ch],
                                  1 << s->prev_block_len_bits,
                                  s->block_len);
            }
        }
    }
    /* parse spectral coefficients : just RLE encoding */
    for(ch = 0; ch < s->nb_channels; ++ch)
    {
        if (s->channel_coded[ch])
        {
            VLC *coef_vlc;
            int level, run, sign, tindex;
            int16_t *ptr, *eptr;
            const int16_t *level_table, *run_table;

            /* special VLC tables are used for ms stereo because
               there is potentially less energy there */
            tindex = (ch == 1 && s->ms_stereo);
            coef_vlc = &s->coef_vlc[tindex];
            run_table = (const int16_t*)s->run_table[tindex];
            level_table = (const int16_t*)s->level_table[tindex];
            /* XXX: optimize */
            ptr = &s->coefs1[ch][0];
            eptr = ptr + nb_coefs[ch];
            memset(ptr, 0, s->block_len * sizeof(int16_t));
            for(;;)
            {
                code = get_vlc(&s->gb, coef_vlc);
                if (code < 0)
                {
                    return -1;
                }
                if (code == 1)
                {
                    /* EOB */
                    break;
                }
                else if (code == 0)
                {
                    /* escape */
                    level = uwma_get_bits(&s->gb, coef_nb_bits);
                    /* NOTE: this is rather suboptimal. reading
                       block_len_bits would be better */
                    run = uwma_get_bits(&s->gb, s->frame_len_bits);
                }
                else
                {
                    /* normal code */
                    run = run_table[code];
                    level = level_table[code];
                }
                sign = uwma_get_bits(&s->gb, 1);
                if (!sign)
                    level = -level;
                ptr += run;
                if (ptr >= eptr)
                {
                    return -1;
                }
                *ptr++ = level;
                /* NOTE: EOB can be omitted */
                if (ptr >= eptr)
                    break;
            }
        }
        if (s->version == 1 && s->nb_channels >= 2)
        {
            uwma_align_get_bits(&s->gb);
        }
    }
    /* normalize */
    {
        int n4 = s->block_len >> 1;
        mdct_norm = 0x10000;
        mdct_norm = fixdiv32(mdct_norm,itofix32(n4));
        if (s->version == 1)
        {
            fixed32 tmp = fixsqrt32(itofix32(n4));
            mdct_norm *= tmp; /* PJJ : exercise this path */
        }
    }
    /* finally compute the MDCT coefficients */
    for(ch = 0; ch < s->nb_channels; ++ch)
    {
        if (s->channel_coded[ch])
        {
            int16_t *coefs1;
            fixed32 *exponents, *exp_ptr;
            fixed32 *coefs;
            fixed64 mult;
            fixed64 mult1;
            fixed32 noise;
            int i, j, n, n1, last_high_band;
            fixed32 exp_power[HIGH_BAND_MAX_SIZE];

            coefs1 = s->coefs1[ch];
            exponents = s->exponents[ch];
            mult = fixdiv64(pow_table[total_gain],Fixed32To64(s->max_exponent[ch]));
            mult = fixmul64byfixed(mult, mdct_norm);
            coefs = s->coefs[ch];
            if (s->use_noise_coding)
            {
                mult1 = mult;
                /* very low freqs : noise */
                for(i = 0;i < s->coefs_start; ++i)
                {
                    *coefs++ = fixmul32(fixmul32(s->noise_table[s->noise_index],(*exponents++)),Fixed32From64(mult1));
                    s->noise_index = (s->noise_index + 1) & (NOISE_TAB_SIZE - 1);
                }

                n1 = s->exponent_high_sizes[bsize];

                /* compute power of high bands */
                exp_ptr = exponents +
                          s->high_band_start[bsize] -
                          s->coefs_start;
                last_high_band = 0; /* avoid warning */
                for (j=0;j<n1;++j)
                {
                    n = s->exponent_high_bands[s->frame_len_bits -
                                               s->block_len_bits][j];
                    if (s->high_band_coded[ch][j])
                    {
                        fixed32 e2, v;
                        e2 = 0;
                        for(i = 0;i < n; ++i)
                        {
                            v = exp_ptr[i];
                            e2 += v * v;
                        }
                        exp_power[j] = fixdiv32(e2,n);
                        last_high_band = j;
                    }
                    exp_ptr += n;
                }

                /* main freqs and high freqs */
                for(j=-1;j<n1;++j)
                {
                    if (j < 0)
                    {
                        n = s->high_band_start[bsize] -
                            s->coefs_start;
                    }
                    else
                    {
                        n = s->exponent_high_bands[s->frame_len_bits -
                                                   s->block_len_bits][j];
                    }
                    if (j >= 0 && s->high_band_coded[ch][j])
                    {
                        /* use noise with specified power */
                        fixed32 tmp = fixdiv32(exp_power[j],exp_power[last_high_band]);
                        mult1 = (fixed64)fixsqrt32(tmp);
                        /* XXX: use a table */
                        mult1 = mult1 * pow_table[s->high_band_values[ch][j]];
                        mult1 = fixdiv64(mult1,fixmul32(s->max_exponent[ch],s->noise_mult));
                        mult1 = fixmul64byfixed(mult1,mdct_norm);
                        for(i = 0;i < n; ++i)
                        {
                            noise = s->noise_table[s->noise_index];
                            s->noise_index = (s->noise_index + 1) & (NOISE_TAB_SIZE - 1);
                            *coefs++ = fixmul32(fixmul32(*exponents,noise),Fixed32From64(mult1));
                            ++exponents;
                        }
                    }
                    else
                    {
                        /* coded values + small noise */
                        for(i = 0;i < n; ++i)
                        {
                            /* PJJ: check code path */
                            noise = s->noise_table[s->noise_index];
                            s->noise_index = (s->noise_index + 1) & (NOISE_TAB_SIZE - 1);
                            *coefs++ = fixmul32(fixmul32(((*coefs1++) + noise),*exponents),mult);
                            ++exponents;
                        }
                    }
                }

                /* very high freqs : noise */
                n = s->block_len - s->coefs_end[bsize];
                mult1 = fixmul32(mult,exponents[-1]);
                for (i = 0; i < n; ++i)
                {
                    *coefs++ = fixmul32(s->noise_table[s->noise_index],Fixed32From64(mult1));
                    s->noise_index = (s->noise_index + 1) & (NOISE_TAB_SIZE - 1);
                }
            }
            else
            {
                /* XXX: optimize more */
                for(i = 0;i < s->coefs_start; ++i)
                    *coefs++ = 0;
                n = nb_coefs[ch];
                for(i = 0;i < n; ++i)
                {
                    *coefs++ = fixmul32(fixmul32(coefs1[i],exponents[i]),mult);
                }
                n = s->block_len - s->coefs_end[bsize];
                for(i = 0;i < n; ++i)
                    *coefs++ = 0;
            }
        }
    }

    if (s->ms_stereo && s->channel_coded[1])
    {
        fixed32 a, b;
        int i;

        /* nominal case for ms stereo: we do it before mdct */
        /* no need to optimize this case because it should almost
           never happen */
        if (!s->channel_coded[0])
        {
            memset(s->coefs[0], 0, sizeof(fixed32) * s->block_len);
            s->channel_coded[0] = 1;
        }

        for(i = 0; i < s->block_len; ++i)
        {
            a = s->coefs[0][i];
            b = s->coefs[1][i];
            s->coefs[0][i] = a + b;
            s->coefs[1][i] = a - b;
        }
    }

    /* build the window : we ensure that when the windows overlap
       their squared sum is always 1 (MDCT reconstruction rule) */
    /* XXX: merge with output */
    {
        int i, next_block_len, block_len, prev_block_len, n;
        fixed32 *wptr;

        block_len = s->block_len;
        prev_block_len = 1 << s->prev_block_len_bits;
        next_block_len = 1 << s->next_block_len_bits;

        /* right part */
        wptr = window + block_len;
        if (block_len <= next_block_len)
        {
            for(i=0;i<block_len;++i)
                *wptr++ = s->windows[bsize][i];
        }
        else
        {
            /* overlap */
            n = (block_len / 2) - (next_block_len / 2);
            for(i=0;i<n;++i)
                *wptr++ = itofix32(1);
            for(i=0;i<next_block_len;++i)
                *wptr++ = s->windows[s->frame_len_bits - s->next_block_len_bits][i];
            for(i=0;i<n;++i)
                *wptr++ = 0;
        }

        /* left part */
        wptr = window + block_len;
        if (block_len <= prev_block_len)
        {
            for(i=0;i<block_len;++i)
                *--wptr = s->windows[bsize][i];
        }
        else
        {
            /* overlap */
            n = (block_len / 2) - (prev_block_len / 2);
            for(i=0;i<n;++i)
                *--wptr = itofix32(1);
            for(i=0;i<prev_block_len;++i)
                *--wptr = s->windows[s->frame_len_bits - s->prev_block_len_bits][i];
            for(i=0;i<n;++i)
                *--wptr = 0;
        }
    }


    for(ch = 0; ch < s->nb_channels; ++ch)
    {
        if (s->channel_coded[ch])
        {
            fixed32 output[BLOCK_MAX_SIZE * 2];
            fixed32 *ptr;
            int i, n4, index, n;

            n = s->block_len;
            n4 = s->block_len / 2;
            ff_imdct_calc(&s->mdct_ctx[bsize],
                          output,
                          s->coefs[ch],
                          s->mdct_tmp);

            /* XXX: optimize all that by build the window and
               multipying/adding at the same time */
            /* multiply by the window */
            for(i=0;i<n * 2;++i)
            {
                output[i] = fixmul32(output[i], window[i]);
            }

            /* add in the frame */
            index = (s->frame_len / 2) + s->block_pos - n4;
            ptr = &s->frame_out[ch][index];
            for(i=0;i<n * 2;++i)
            {
                *ptr += output[i];
                ++ptr;
            }

            /* specific fast case for ms-stereo : add to second
               channel if it is not coded */
            if (s->ms_stereo && !s->channel_coded[1])
            {
                ptr = &s->frame_out[1][index];
                for(i=0;i<n * 2;++i)
                {
                    *ptr += output[i];
                    ++ptr;
                }
            }
        }
    }
next:
    /* update block number */
    ++s->block_num;
    s->block_pos += s->block_len;
    if (s->block_pos >= s->frame_len)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/* decode a frame of frame_len samples */
static int wma_decode_frame(WMADecodeContext *s, int16_t *samples)
{
    int ret, i, n, a, ch, incr;
    int16_t *ptr;
    fixed32 *iptr;

    /* read each block */
    s->block_num = 0;
    s->block_pos = 0;
    for(;;)
    {
        ret = wma_decode_block(s);
        if (ret < 0)
        {
            return -1;
        }
        if (ret)
        {
            break;
        }
    }

    /* convert frame to integer */
    n = s->frame_len;
    incr = s->nb_channels;
    for(ch = 0; ch < s->nb_channels; ++ch)
    {
        ptr = samples + ch;
        iptr = s->frame_out[ch];

        for (i=0;i<n;++i)
        {
            a = fixtoi32(*iptr++);
            if (a > 32767)
            {
                a = 32767;
            }
            else if (a < -32768)
            {
                a = -32768;
            }
            *ptr = a;
            ptr += incr;
        }
        /* prepare for next block */
        memmove(&s->frame_out[ch][0], &s->frame_out[ch][s->frame_len],
                s->frame_len * sizeof(fixed32));
        /* XXX: suppress this */
        memset(&s->frame_out[ch][s->frame_len], 0,
               s->frame_len * sizeof(fixed32));
    }

    return 0;
}

/* Public entry point */
static int wma_decode_init_fixed(CodecContext * avctx)
{
    WMADecodeContext *s = avctx->priv_data;
    int i, flags1, flags2;
    fixed32 *window;
    uint8_t *extradata;
    fixed64 bps1;
    fixed32 high_freq;
    fixed64 bps;
    int sample_rate1;
    int coef_vlc_table;

    s->sample_rate = avctx->sample_rate;
    s->nb_channels = avctx->channels;
    s->bit_rate = avctx->bit_rate;
    s->block_align = avctx->block_align;

    if (avctx->codec->id == CODEC_ID_WMAV1)
    {
        s->version = 1;
    }
    else
    {
        s->version = 2;
    }

    /* extract flag infos */
    flags1 = 0;
    flags2 = 0;
    extradata = avctx->extradata;
    if (s->version == 1 && avctx->extradata_size >= 4)
    {
        flags1 = extradata[0] | (extradata[1] << 8);
        flags2 = extradata[2] | (extradata[3] << 8);
    }
    else if (s->version == 2 && avctx->extradata_size >= 6)
    {
        flags1 = extradata[0] | (extradata[1] << 8) |
                 (extradata[2] << 16) | (extradata[3] << 24);
        flags2 = extradata[4] | (extradata[5] << 8);
    }
    s->use_exp_vlc = flags2 & 0x0001;
    s->use_bit_reservoir = flags2 & 0x0002;
    s->use_variable_block_len = flags2 & 0x0004;

    /* compute MDCT block size */
    if (s->sample_rate <= 16000)
    {
        s->frame_len_bits = 9;
    }
    else if (s->sample_rate <= 22050 ||
             (s->sample_rate <= 32000 && s->version == 1))
    {
        s->frame_len_bits = 10;
    }
    else
    {
        s->frame_len_bits = 11;
    }
    s->frame_len = 1 << s->frame_len_bits;
    if (s->use_variable_block_len)
    {
        int nb_max, nb;
        nb = ((flags2 >> 3) & 3) + 1;
        if ((s->bit_rate / s->nb_channels) >= 32000)
        {
            nb += 2;
        }
        nb_max = s->frame_len_bits - BLOCK_MIN_BITS;
        if (nb > nb_max)
            nb = nb_max;
        s->nb_block_sizes = nb + 1;
    }
    else
    {
        s->nb_block_sizes = 1;
    }

    /* init rate dependant parameters */
    s->use_noise_coding = 1;
    high_freq = fixmul64byfixed(itofix64(s->sample_rate), 0x8000);

    /* if version 2, then the rates are normalized */
    sample_rate1 = s->sample_rate;
    if (s->version == 2)
    {
        if (sample_rate1 >= 44100)
            sample_rate1 = 44100;
        else if (sample_rate1 >= 22050)
            sample_rate1 = 22050;
        else if (sample_rate1 >= 16000)
            sample_rate1 = 16000;
        else if (sample_rate1 >= 11025)
            sample_rate1 = 11025;
        else if (sample_rate1 >= 8000)
            sample_rate1 = 8000;
    }

    fixed64 tmp = itofix64(s->bit_rate);
    fixed64 tmp2 = itofix64(s->nb_channels * s->sample_rate);
    bps = fixdiv64(tmp, tmp2);
    fixed64 tim = fixmul64byfixed(bps, s->frame_len);
    fixed64 tmpi = fixdiv64(tim,itofix64(8));
    s->byte_offset_bits = av_log2(fixtoi64(tmpi)) + 2;

    /* compute high frequency value and choose if noise coding should
       be activated */
    bps1 = bps;
    if (s->nb_channels == 2)
        bps1 = fixmul32(bps,0x1999a);
    if (sample_rate1 == 44100)
    {
        if (bps1 >= 0x9c29)
            s->use_noise_coding = 0;
        else
            high_freq = fixmul64byfixed(high_freq,0x6666);
    }
    else if (sample_rate1 == 22050)
    {
        if (bps1 >= 0x128f6)
            s->use_noise_coding = 0;
        else if (bps1 >= 0xb852)
            high_freq = fixmul64byfixed(high_freq,0xb333);
        else
            high_freq = fixmul64byfixed(high_freq,0x999a);
    }
    else if (sample_rate1 == 16000)
    {
        if (bps > 0x8000)
            high_freq = fixmul64byfixed(high_freq,0x8000);
        else
            high_freq = fixmul64byfixed(high_freq,0x4ccd);
    }
    else if (sample_rate1 == 11025)
    {
        high_freq = fixmul64byfixed(high_freq,0xb3333);
    }
    else if (sample_rate1 == 8000)
    {
        if (bps <= 0xa000)
        {
            high_freq = fixmul64byfixed(high_freq,0x8000);
        }
        else if (bps > 0xc000)
        {
            s->use_noise_coding = 0;
        }
        else
        {
            high_freq = fixmul64byfixed(high_freq,0xa666);
        }
    }
    else
    {
        if (bps >= 0xcccd)
        {
            high_freq = fixmul64byfixed(high_freq,0xc000);
        }
        else if (bps >= 0x999a)
        {
            high_freq = fixmul64byfixed(high_freq,0x999a);
        }
        else
        {
            high_freq = fixmul64byfixed(high_freq,0x8000);
        }
    }

    /* compute the scale factor band sizes for each MDCT block size */
    {
        int a, b, pos, lpos, k, block_len, i, j, n;
        const uint8_t *table;

        if (s->version == 1)
        {
            s->coefs_start = 3;
        }
        else
        {
            s->coefs_start = 0;
        }
        for(k = 0; k < s->nb_block_sizes; ++k)
        {
            block_len = s->frame_len >> k;

            if (s->version == 1)
            {
                lpos = 0;
                for(i=0;i<25;++i)
                {
                    a = wma_critical_freqs[i];
                    b = s->sample_rate;
                    pos = ((block_len * 2 * a)  + (b >> 1)) / b;
                    if (pos > block_len)
                        pos = block_len;
                    s->exponent_bands[0][i] = pos - lpos;
                    if (pos >= block_len)
                    {
                        ++i;
                        break;
                    }
                    lpos = pos;
                }
                s->exponent_sizes[0] = i;
            }
            else
            {
                /* hardcoded tables */
                table = NULL;
                a = s->frame_len_bits - BLOCK_MIN_BITS - k;
                if (a < 3)
                {
                    if (s->sample_rate >= 44100)
                        table = exponent_band_44100[a];
                    else if (s->sample_rate >= 32000)
                        table = exponent_band_32000[a];
                    else if (s->sample_rate >= 22050)
                        table = exponent_band_22050[a];
                }
                if (table)
                {
                    n = *table++;
                    for(i=0;i<n;++i)
                        s->exponent_bands[k][i] = table[i];
                    s->exponent_sizes[k] = n;
                }
                else
                {
                    j = 0;
                    lpos = 0;
                    for(i=0;i<25;++i)
                    {
                        a = wma_critical_freqs[i];
                        b = s->sample_rate;
                        pos = ((block_len * 2 * a)  + (b << 1)) / (4 * b);
                        pos <<= 2;
                        if (pos > block_len)
                            pos = block_len;
                        if (pos > lpos)
                            s->exponent_bands[k][j++] = pos - lpos;
                        if (pos >= block_len)
                            break;
                        lpos = pos;
                    }
                    s->exponent_sizes[k] = j;
                }
            }

            /* max number of coefs */
            s->coefs_end[k] = (s->frame_len - ((s->frame_len * 9) / 100)) >> k;
            /* high freq computation */
            fixed64 tmp = itofix64(block_len<<2);
            tmp = fixmul64byfixed(tmp,high_freq);
            fixed64 tmp2 = itofix64(s->sample_rate);
            tmp2 += 0x8000;
            s->high_band_start[k] = fixtoi64(fixdiv64(tmp,tmp2));

            /*
            s->high_band_start[k] = (int)((block_len * 2 * high_freq) /
                                          s->sample_rate + 0.5);*/

            n = s->exponent_sizes[k];
            j = 0;
            pos = 0;
            for(i=0;i<n;++i)
            {
                int start, end;
                start = pos;
                pos += s->exponent_bands[k][i];
                end = pos;
                if (start < s->high_band_start[k])
                    start = s->high_band_start[k];
                if (end > s->coefs_end[k])
                    end = s->coefs_end[k];
                if (end > start)
                    s->exponent_high_bands[k][j++] = end - start;
            }
            s->exponent_high_sizes[k] = j;
        }
    }

    /* init MDCT */
    for(i = 0; i < s->nb_block_sizes; ++i)
    {
        ff_mdct_init(&s->mdct_ctx[i], s->frame_len_bits - i + 1, 1);
    }

    /* init MDCT windows : simple sinus window */
    for(i = 0; i < s->nb_block_sizes; ++i)
    {
        int n, j;
        fixed32 alpha;
        n = 1 << (s->frame_len_bits - i);
        window = av_malloc(sizeof(fixed32) * n);
        fixed32 n2 = itofix32(n<<1);
        alpha = fixdiv32(M_PI_F, n2);
        for(j=0;j<n;++j)
        {
            fixed32 j2 = itofix32(j) + 0x8000;
            window[n - j - 1] = fixsin32(fixmul32(j2,alpha));
        }
        s->windows[i] = window;
    }

    s->reset_block_lengths = 1;

    if (s->use_noise_coding)
    {
        /* init the noise generator */
        if (s->use_exp_vlc)
        {
            s->noise_mult = 0x51f;
        }
        else
        {
            s->noise_mult = 0xa3d;
        }

        {
            unsigned int seed;
            fixed32 norm;
            seed = 1;
            norm = 0;   /* PJJ: near as makes any diff to 0! */
            for (i=0;i<NOISE_TAB_SIZE;++i)
            {
                seed = seed * 314159 + 1;
                s->noise_table[i] = itofix32((int)seed) * norm;
            }
        }

        uwma_init_vlc(&s->hgain_vlc, 9, sizeof(hgain_huffbits),
                 hgain_huffbits, 1, 1,
                 hgain_huffcodes, 2, 2);
    }

    if (s->use_exp_vlc)
    {
        uwma_init_vlc(&s->exp_vlc, 9, sizeof(scale_huffbits),
                 scale_huffbits, 1, 1,
                 scale_huffcodes, 4, 4);
    }
    else
    {
        wma_lsp_to_curve_init(s, s->frame_len);
    }

    /* choose the VLC tables for the coefficients */
    coef_vlc_table = 2;
    if (s->sample_rate >= 32000)
    {
        if (bps1 < 0xb852)
            coef_vlc_table = 0;
        else if (bps1 < 0x128f6)
            coef_vlc_table = 1;
    }

    init_coef_vlc(&s->coef_vlc[0], &s->run_table[0], &s->level_table[0],
                  &coef_vlcs[coef_vlc_table * 2]);
    init_coef_vlc(&s->coef_vlc[1], &s->run_table[1], &s->level_table[1],
                  &coef_vlcs[coef_vlc_table * 2 + 1]);
    return 0;
}

/* public entry point */
static int wma_decode_superframe(CodecContext *avctx,
                                 void *data,
                                 int *data_size,
                                 uint8_t *buf,
                                 int buf_size)
{
    WMADecodeContext *s = avctx->priv_data;
    int nb_frames, bit_offset, i, pos, len;
    uint8_t *q;
    int16_t *samples;
    if (buf_size==0)
    {
        s->last_superframe_len = 0;
        return 0;
    }
    samples = data;
    uwma_init_get_bits(&s->gb, buf, buf_size*8);
    if (s->use_bit_reservoir)
    {
        /* read super frame header */
        uwma_get_bits(&s->gb, 4); /* super frame index */
        nb_frames = uwma_get_bits(&s->gb, 4) - 1;

        bit_offset = uwma_get_bits(&s->gb, s->byte_offset_bits + 3);
        if (s->last_superframe_len > 0)
        {
            /* add bit_offset bits to last frame */
            if ((s->last_superframe_len + ((bit_offset + 7) >> 3)) >
                    MAX_CODED_SUPERFRAME_SIZE)
            {
                goto fail;
            }
            q = s->last_superframe + s->last_superframe_len;
            len = bit_offset;
            while (len > 0)
            {
                *q++ = (uwma_get_bits)(&s->gb, 8);
                len -= 8;
            }
            if (len > 0)
            {
                *q++ = (uwma_get_bits)(&s->gb, len) << (8 - len);
            }

            /* XXX: bit_offset bits into last frame */
            uwma_init_get_bits(&s->gb, s->last_superframe, MAX_CODED_SUPERFRAME_SIZE*8);
            /* skip unused bits */
            if (s->last_bitoffset > 0)
                uwma_skip_bits(&s->gb, s->last_bitoffset);
            /* this frame is stored in the last superframe and in the
               current one */
            if (wma_decode_frame(s, samples) < 0)
            {
                goto fail;
            }
            samples += s->nb_channels * s->frame_len;
        }

        /* read each frame starting from bit_offset */
        pos = bit_offset + 4 + 4 + s->byte_offset_bits + 3;
        uwma_init_get_bits(&s->gb, buf + (pos >> 3), (MAX_CODED_SUPERFRAME_SIZE - (pos >> 3))*8);
        len = pos & 7;
        if (len > 0)
            uwma_skip_bits(&s->gb, len);

        s->reset_block_lengths = 1;
        for(i=0;i<nb_frames;++i)
        {
            if (wma_decode_frame(s, samples) < 0)
            {
                goto fail;
            }
            samples += s->nb_channels * s->frame_len;
        }

        /* we copy the end of the frame in the last frame buffer */
        pos = uwma_get_bits_count(&s->gb) + ((bit_offset + 4 + 4 + s->byte_offset_bits + 3) & ~7);
        s->last_bitoffset = pos & 7;
        pos >>= 3;
        len = buf_size - pos;
        if (len > MAX_CODED_SUPERFRAME_SIZE || len < 0)
        {
            goto fail;
        }
        s->last_superframe_len = len;
        memcpy(s->last_superframe, buf + pos, len);
    }
    else
    {
        /* single frame decode */
        if (wma_decode_frame(s, samples) < 0)
        {
            goto fail;
        }
        samples += s->nb_channels * s->frame_len;
    }
    *data_size = (int8_t *)samples - (int8_t *)data;
    return s->block_align;
fail:
    /* when error, we reset the bit reservoir */
    s->last_superframe_len = 0;
    return -1;
}

/* public entry point */
static int wma_decode_end(CodecContext *avctx)
{
    WMADecodeContext *s = avctx->priv_data;
    int i;

    for(i = 0; i < s->nb_block_sizes; ++i)
        ff_mdct_end(&s->mdct_ctx[i]);
    for(i = 0; i < s->nb_block_sizes; ++i)
        av_free(s->windows[i]);

    if (s->use_exp_vlc)
    {
        free_vlc(&s->exp_vlc);
    }
    if (s->use_noise_coding)
    {
        free_vlc(&s->hgain_vlc);
    }
    for(i = 0;i < 2; ++i)
    {
        free_vlc(&s->coef_vlc[i]);
        av_free(s->run_table[i]);
        av_free(s->level_table[i]);
    }

    return 0;
}

AVCodec wmav1i_decoder =
    {
        "wmav1",
        CODEC_TYPE_AUDIO,
        CODEC_ID_WMAV1,
        sizeof(WMADecodeContext),
        wma_decode_init_fixed,
        NULL,
        wma_decode_end,
        wma_decode_superframe,
    };

AVCodec wmav2i_decoder =
    {
        "wmav2",
        CODEC_TYPE_AUDIO,
        CODEC_ID_WMAV2,
        sizeof(WMADecodeContext),
        wma_decode_init_fixed,
        NULL,
        wma_decode_end,
        wma_decode_superframe,
    };

#include <stdint.h> // for uint16_t, etc.
// ASF/WMAヘッダからオーディオプロパティを抽出するヘルパー関数
// 参考: https://en.wikipedia.org/wiki/Advanced_Systems_Format
//       https://learn.microsoft.com/en-us/windows/win32/wmformat/audio-media-types
static int parse_wma_header(unsigned char* data, int len, CodecContext *cc) {
    // WaveFormatEx Structure for WMA
    // WORD  wFormatTag;
    // WORD  nChannels;
    // DWORD nSamplesPerSec;
    // DWORD nAvgBytesPerSec;
    // WORD  nBlockAlign;
    // WORD  wBitsPerSample;
    // WORD  cbSize;
    
    // ASFヘッダ内に "WAVEFORMATEX" に相当する構造体を探す。
    // ここでは簡易的に、よく見られるオフセットにあると仮定して探索します。
    // "f8 69 9e 53" (Audio Stream) -> "26 00" (size) -> ...
    // -> "01 01" (WMAv1) or "61 01" (WMAv2)
    
    // 簡単な探索例 (堅牢な実装にはより詳細なASFパーサが必要です)
    for (int i = 0; i < len - 24; ++i) {
        // "WMA"のwFormatTag (0x0161 for WMAv2, 0x0160 for WMAv1) を探す
        if ((data[i] == 0x61 || data[i] == 0x60) && data[i+1] == 0x01) {
            cc->codec_id = data[i] | (data[i+1] << 8);
            cc->channels = data[i+2] | (data[i+3] << 8);
            cc->sample_rate = data[i+4] | (data[i+5] << 8) | (data[i+6] << 16) | (data[i+7] << 24);
            cc->bit_rate = (data[i+8] | (data[i+9] << 8) | (data[i+10] << 16) | (data[i+11] << 24)) * 8;
            cc->block_align = data[i+12] | (data[i+13] << 8);
            
            // Extradataも設定
            if (cc->codec_id == CODEC_ID_WMAV2) {
                // Check if there are enough bytes to read the size field (cbSize)
                if (i + 20 <= len) {
                    uint16_t cbSize = data[i+18] | (data[i+19] << 8);
                    // Check if the actual extradata is within the file bounds
                    if (cbSize > 0 && (i + 20 + cbSize <= len)) {
                        cc->extradata = &data[i+20];
                        cc->extradata_size = cbSize;
                    }
                }
            }
            
            // 妥当な値かチェック
            if (cc->channels > 0 && cc->channels <= 2 && 
                cc->sample_rate > 0 && cc->sample_rate < 200000 && 
                cc->block_align > 0) {
                return 0; // 成功
            }
        }
    }
    return -1; // 失敗
}

#undef UPDATE_CACHE
#undef BF
