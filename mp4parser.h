/* LGPL-3.0 Simple, Minimalistic, MP4 parser [https://github.com/gozfree/libraries]
 *	©2018 Yuichiro Nakada
 *
 * Basic usage:
 *
 * */

/******************************************************************************
 * Copyright (C) 2014-2016
 * file:    libmp4parser-patch.h
 * author:  gozfree <gozfree@163.com>
 * created: 2016-08-12 00:05
 * updated: 2016-08-12 00:05
 ******************************************************************************/

#ifndef LIBMP4PARSER_PATCH_H
#define LIBMP4PARSER_PATCH_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#endif
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define typeof __typeof__


/* redefine bool to fix complie error */
#ifndef bool
#define bool uint8_t
#define true    1
#define false   0
#endif


/* define from vlc */
#ifdef WORDS_BIGENDIAN
#   define VLC_FOURCC( a, b, c, d ) \
        ( ((uint32_t)d) | ( ((uint32_t)c) << 8 ) \
           | ( ((uint32_t)b) << 16 ) | ( ((uint32_t)a) << 24 ) )
#   define VLC_TWOCC( a, b ) \
        ( (uint16_t)(b) | ( (uint16_t)(a) << 8 ) )

#else
#   define VLC_FOURCC( a, b, c, d ) \
        ( ((uint32_t)a) | ( ((uint32_t)b) << 8 ) \
           | ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) )
#   define VLC_TWOCC( a, b ) \
        ( (uint16_t)(a) | ( (uint16_t)(b) << 8 ) )

#endif

/* implement stream  */
typedef struct stream {
    void *(*open)(struct stream *stream_s, const char *filename, int mode);
    int (*read)(struct stream *stream_s, void *buf, int size);
    int (*write)(struct stream *stream_s, void *buf, int size);
    int (*peek)(struct stream *stream_s, const uint8_t **buf, int size);
    uint64_t (*seek)(struct stream *stream_s, int64_t offset, int whence);
    int64_t (*tell)(struct stream *stream_s);
    int64_t (*size)(struct stream *stream_s);
    int (*close)(struct stream *stream_s);
    void *opaque;
    void **priv_buf;//store peek malloc buffer
    int priv_buf_num;
} stream_t;

#define MODE_READ             (1)
#define MODE_WRITE            (2)
#define MODE_READWRITEFILTER  (3)
#define MODE_EXISTING         (4)
#define MODE_CREATE           (8)

stream_t* create_file_stream();
void destory_file_stream(stream_t* stream_s);
#define stream_open(s, filename, mode) ((stream_t*)s)->open(((stream_t*)s), filename, mode)
#define stream_close(s) ((stream_t*)s)->close(((stream_t*)s))

#ifdef __cplusplus
}
#endif
#endif



// libmp4parser.h
#ifndef LIBMP4PARSER_H
#define LIBMP4PARSER_H

//#include "libmp4parser-patch.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLOCK16x16 (1<<16)

#define ATOM_root VLC_FOURCC( 'r', 'o', 'o', 't' )
#define ATOM_uuid VLC_FOURCC( 'u', 'u', 'i', 'd' )

#define ATOM_ftyp VLC_FOURCC( 'f', 't', 'y', 'p' )
#define ATOM_moov VLC_FOURCC( 'm', 'o', 'o', 'v' )
#define ATOM_foov VLC_FOURCC( 'f', 'o', 'o', 'v' )
#define ATOM_cmov VLC_FOURCC( 'c', 'm', 'o', 'v' )
#define ATOM_dcom VLC_FOURCC( 'd', 'c', 'o', 'm' )
#define ATOM_cmvd VLC_FOURCC( 'c', 'm', 'v', 'd' )

#define ATOM_moof VLC_FOURCC( 'm', 'o', 'o', 'f' )
#define ATOM_mdat VLC_FOURCC( 'm', 'd', 'a', 't' )
#define ATOM_skip VLC_FOURCC( 's', 'k', 'i', 'p' )
#define ATOM_free VLC_FOURCC( 'f', 'r', 'e', 'e' )
#define ATOM_udta VLC_FOURCC( 'u', 'd', 't', 'a' )
#define ATOM_wide VLC_FOURCC( 'w', 'i', 'd', 'e' )
#define ATOM_binm VLC_FOURCC( 0x82, 0x82, 0x7f, 0x7d ) /* binary Computer Graphics Metafile */

#define ATOM_data VLC_FOURCC( 'd', 'a', 't', 'a' )

#define ATOM_trak VLC_FOURCC( 't', 'r', 'a', 'k' )
#define ATOM_mvhd VLC_FOURCC( 'm', 'v', 'h', 'd' )
#define ATOM_tkhd VLC_FOURCC( 't', 'k', 'h', 'd' )
#define ATOM_tref VLC_FOURCC( 't', 'r', 'e', 'f' )
#define ATOM_mdia VLC_FOURCC( 'm', 'd', 'i', 'a' )
#define ATOM_mdhd VLC_FOURCC( 'm', 'd', 'h', 'd' )
#define ATOM_hdlr VLC_FOURCC( 'h', 'd', 'l', 'r' )
#define ATOM_minf VLC_FOURCC( 'm', 'i', 'n', 'f' )
#define ATOM_vmhd VLC_FOURCC( 'v', 'm', 'h', 'd' )
#define ATOM_smhd VLC_FOURCC( 's', 'm', 'h', 'd' )
#define ATOM_hmhd VLC_FOURCC( 'h', 'm', 'h', 'd' )
#define ATOM_dinf VLC_FOURCC( 'd', 'i', 'n', 'f' )
#define ATOM_url  VLC_FOURCC( 'u', 'r', 'l', ' ' )
#define ATOM_urn  VLC_FOURCC( 'u', 'r', 'n', ' ' )
#define ATOM_dref VLC_FOURCC( 'd', 'r', 'e', 'f' )
#define ATOM_stbl VLC_FOURCC( 's', 't', 'b', 'l' )
#define ATOM_stts VLC_FOURCC( 's', 't', 't', 's' )
#define ATOM_ctts VLC_FOURCC( 'c', 't', 't', 's' )
#define ATOM_stsd VLC_FOURCC( 's', 't', 's', 'd' )
#define ATOM_stsz VLC_FOURCC( 's', 't', 's', 'z' )
#define ATOM_stz2 VLC_FOURCC( 's', 't', 'z', '2' )
#define ATOM_stsc VLC_FOURCC( 's', 't', 's', 'c' )
#define ATOM_stco VLC_FOURCC( 's', 't', 'c', 'o' )
#define ATOM_co64 VLC_FOURCC( 'c', 'o', '6', '4' )
#define ATOM_stss VLC_FOURCC( 's', 't', 's', 's' )
#define ATOM_stsh VLC_FOURCC( 's', 't', 's', 'h' )
#define ATOM_stdp VLC_FOURCC( 's', 't', 'd', 'p' )
#define ATOM_padb VLC_FOURCC( 'p', 'a', 'd', 'b' )
#define ATOM_edts VLC_FOURCC( 'e', 'd', 't', 's' )
#define ATOM_elst VLC_FOURCC( 'e', 'l', 's', 't' )
#define ATOM_mvex VLC_FOURCC( 'm', 'v', 'e', 'x' )
#define ATOM_sdtp VLC_FOURCC( 's', 'd', 't', 'p' )
#define ATOM_trex VLC_FOURCC( 't', 'r', 'e', 'x' )
#define ATOM_mehd VLC_FOURCC( 'm', 'e', 'h', 'd' )
#define ATOM_mfhd VLC_FOURCC( 'm', 'f', 'h', 'd' )
#define ATOM_traf VLC_FOURCC( 't', 'r', 'a', 'f' )
#define ATOM_sidx VLC_FOURCC( 's', 'i', 'd', 'x' )
#define ATOM_tfhd VLC_FOURCC( 't', 'f', 'h', 'd' )
#define ATOM_trun VLC_FOURCC( 't', 'r', 'u', 'n' )
#define ATOM_cprt VLC_FOURCC( 'c', 'p', 'r', 't' )
#define ATOM_iods VLC_FOURCC( 'i', 'o', 'd', 's' )
#define ATOM_pasp VLC_FOURCC( 'p', 'a', 's', 'p' )
#define ATOM_mfra VLC_FOURCC( 'm', 'f', 'r', 'a' )
#define ATOM_mfro VLC_FOURCC( 'm', 'f', 'r', 'o' )
#define ATOM_tfra VLC_FOURCC( 't', 'f', 'r', 'a' )

#define ATOM_nmhd VLC_FOURCC( 'n', 'm', 'h', 'd' )
#define ATOM_mp2v VLC_FOURCC( 'm', 'p', '2', 'v' )
#define ATOM_mp4v VLC_FOURCC( 'm', 'p', '4', 'v' )
#define ATOM_mp4a VLC_FOURCC( 'm', 'p', '4', 'a' )
#define ATOM_mp4s VLC_FOURCC( 'm', 'p', '4', 's' )
#define ATOM_vide VLC_FOURCC( 'v', 'i', 'd', 'e' )
#define ATOM_soun VLC_FOURCC( 's', 'o', 'u', 'n' )
#define ATOM_hint VLC_FOURCC( 'h', 'i', 'n', 't' )
#define ATOM_hdv2 VLC_FOURCC( 'h', 'd', 'v', '2' )

#define ATOM_dpnd VLC_FOURCC( 'd', 'p', 'n', 'd' )
#define ATOM_ipir VLC_FOURCC( 'i', 'p', 'i', 'r' )
#define ATOM_mpod VLC_FOURCC( 'm', 'p', 'o', 'd' )
#define ATOM_hnti VLC_FOURCC( 'h', 'n', 't', 'i' )
#define ATOM_rtp  VLC_FOURCC( 'r', 't', 'p', ' ' )

#define ATOM_isom VLC_FOURCC( 'i', 's', 'o', 'm' )
#define ATOM_3gp4 VLC_FOURCC( '3', 'g', 'p', '4' )
#define ATOM_esds VLC_FOURCC( 'e', 's', 'd', 's' )

#define ATOM_lpcm VLC_FOURCC( 'l', 'p', 'c', 'm' )
#define ATOM__mp3 VLC_FOURCC( '.', 'm', 'p', '3' )
#define ATOM_ms02 VLC_FOURCC( 'm', 's', 0x0, 0x02 )
#define ATOM_ms11 VLC_FOURCC( 'm', 's', 0x0, 0x11 )
#define ATOM_ms55 VLC_FOURCC( 'm', 's', 0x0, 0x55 )
#define ATOM_twos VLC_FOURCC( 't', 'w', 'o', 's' )
#define ATOM_sowt VLC_FOURCC( 's', 'o', 'w', 't' )
#define ATOM_QDMC VLC_FOURCC( 'Q', 'D', 'M', 'C' )
#define ATOM_QDM2 VLC_FOURCC( 'Q', 'D', 'M', '2' )
#define ATOM_ima4 VLC_FOURCC( 'i', 'm', 'a', '4' )
#define ATOM_IMA4 VLC_FOURCC( 'I', 'M', 'A', '4' )
#define ATOM_dvi  VLC_FOURCC( 'd', 'v', 'i', ' ' )
#define ATOM_MAC3 VLC_FOURCC( 'M', 'A', 'C', '3' )
#define ATOM_MAC6 VLC_FOURCC( 'M', 'A', 'C', '6' )
#define ATOM_alaw VLC_FOURCC( 'a', 'l', 'a', 'w' )
#define ATOM_ulaw VLC_FOURCC( 'u', 'l', 'a', 'w' )
#define ATOM_Qclp VLC_FOURCC( 'Q', 'c', 'l', 'p' )
#define ATOM_samr VLC_FOURCC( 's', 'a', 'm', 'r' )
#define ATOM_sawb VLC_FOURCC( 's', 'a', 'w', 'b' )
#define ATOM_OggS VLC_FOURCC( 'O', 'g', 'g', 'S' )
#define ATOM_alac VLC_FOURCC( 'a', 'l', 'a', 'c' )
#define ATOM_dac3 VLC_FOURCC( 'd', 'a', 'c', '3' )
#define ATOM_dec3 VLC_FOURCC( 'd', 'e', 'c', '3' )
#define ATOM_dvc1 VLC_FOURCC( 'd', 'v', 'c', '1' )
#define ATOM_enda VLC_FOURCC( 'e', 'n', 'd', 'a' )
#define ATOM_gnre VLC_FOURCC( 'g', 'n', 'r', 'e' )
#define ATOM_trkn VLC_FOURCC( 't', 'r', 'k', 'n' )

#define ATOM_zlib VLC_FOURCC( 'z', 'l', 'i', 'b' )
#define ATOM_SVQ1 VLC_FOURCC( 'S', 'V', 'Q', '1' )
#define ATOM_SVQ3 VLC_FOURCC( 'S', 'V', 'Q', '3' )
#define ATOM_ZyGo VLC_FOURCC( 'Z', 'y', 'G', 'o' )
#define ATOM_3IV1 VLC_FOURCC( '3', 'I', 'V', '1' )
#define ATOM_3iv1 VLC_FOURCC( '3', 'i', 'v', '1' )
#define ATOM_3IV2 VLC_FOURCC( '3', 'I', 'V', '2' )
#define ATOM_3iv2 VLC_FOURCC( '3', 'i', 'v', '2' )
#define ATOM_3IVD VLC_FOURCC( '3', 'I', 'V', 'D' )
#define ATOM_3ivd VLC_FOURCC( '3', 'i', 'v', 'd' )
#define ATOM_3VID VLC_FOURCC( '3', 'V', 'I', 'D' )
#define ATOM_3vid VLC_FOURCC( '3', 'v', 'i', 'd' )
#define ATOM_h263 VLC_FOURCC( 'h', '2', '6', '3' )
#define ATOM_s263 VLC_FOURCC( 's', '2', '6', '3' )
#define ATOM_DIVX VLC_FOURCC( 'D', 'I', 'V', 'X' )
#define ATOM_XVID VLC_FOURCC( 'X', 'V', 'I', 'D' )
#define ATOM_cvid VLC_FOURCC( 'c', 'v', 'i', 'd' )
#define ATOM_mjpa VLC_FOURCC( 'm', 'j', 'p', 'a' )
#define ATOM_mjpb VLC_FOURCC( 'm', 'j', 'q', 't' )
#define ATOM_mjqt VLC_FOURCC( 'm', 'j', 'h', 't' )
#define ATOM_mjht VLC_FOURCC( 'm', 'j', 'p', 'b' )
#define ATOM_VP31 VLC_FOURCC( 'V', 'P', '3', '1' )
#define ATOM_vp31 VLC_FOURCC( 'v', 'p', '3', '1' )
#define ATOM_h264 VLC_FOURCC( 'h', '2', '6', '4' )
#define ATOM_qdrw VLC_FOURCC( 'q', 'd', 'r', 'w' )

#define ATOM_avc1 VLC_FOURCC( 'a', 'v', 'c', '1' )
#define ATOM_avcC VLC_FOURCC( 'a', 'v', 'c', 'C' )
#define ATOM_m4ds VLC_FOURCC( 'm', '4', 'd', 's' )

#define ATOM_hvcC VLC_FOURCC( 'h', 'v', 'c', 'C' )

#define ATOM_dvc  VLC_FOURCC( 'd', 'v', 'c', ' ' )
#define ATOM_dvp  VLC_FOURCC( 'd', 'v', 'p', ' ' )
#define ATOM_dv5n VLC_FOURCC( 'd', 'v', '5', 'n' )
#define ATOM_dv5p VLC_FOURCC( 'd', 'v', '5', 'p' )
#define ATOM_raw  VLC_FOURCC( 'r', 'a', 'w', ' ' )

#define ATOM_jpeg VLC_FOURCC( 'j', 'p', 'e', 'g' )

#define ATOM_yv12 VLC_FOURCC( 'y', 'v', '1', '2' )
#define ATOM_yuv2 VLC_FOURCC( 'y', 'u', 'v', '2' )

#define ATOM_rmra VLC_FOURCC( 'r', 'm', 'r', 'a' )
#define ATOM_rmda VLC_FOURCC( 'r', 'm', 'd', 'a' )
#define ATOM_rdrf VLC_FOURCC( 'r', 'd', 'r', 'f' )
#define ATOM_rmdr VLC_FOURCC( 'r', 'm', 'd', 'r' )
#define ATOM_rmvc VLC_FOURCC( 'r', 'm', 'v', 'c' )
#define ATOM_rmcd VLC_FOURCC( 'r', 'm', 'c', 'd' )
#define ATOM_rmqu VLC_FOURCC( 'r', 'm', 'q', 'u' )
#define ATOM_alis VLC_FOURCC( 'a', 'l', 'i', 's' )

#define ATOM_gmhd VLC_FOURCC( 'g', 'm', 'h', 'd' )
#define ATOM_wave VLC_FOURCC( 'w', 'a', 'v', 'e' )

#define ATOM_drms VLC_FOURCC( 'd', 'r', 'm', 's' )
#define ATOM_sinf VLC_FOURCC( 's', 'i', 'n', 'f' )
#define ATOM_schi VLC_FOURCC( 's', 'c', 'h', 'i' )
#define ATOM_user VLC_FOURCC( 'u', 's', 'e', 'r' )
#define ATOM_key  VLC_FOURCC( 'k', 'e', 'y', ' ' )
#define ATOM_iviv VLC_FOURCC( 'i', 'v', 'i', 'v' )
#define ATOM_name VLC_FOURCC( 'n', 'a', 'm', 'e' )
#define ATOM_priv VLC_FOURCC( 'p', 'r', 'i', 'v' )
#define ATOM_drmi VLC_FOURCC( 'd', 'r', 'm', 'i' )
#define ATOM_frma VLC_FOURCC( 'f', 'r', 'm', 'a' )
#define ATOM_skcr VLC_FOURCC( 's', 'k', 'c', 'r' )

#define ATOM_text VLC_FOURCC( 't', 'e', 'x', 't' )
#define ATOM_tx3g VLC_FOURCC( 't', 'x', '3', 'g' )
#define ATOM_subp VLC_FOURCC( 's', 'u', 'b', 'p' )
#define ATOM_sbtl VLC_FOURCC( 's', 'b', 't', 'l' )

#define ATOM_0xa9nam VLC_FOURCC( 0xa9, 'n', 'a', 'm' )
#define ATOM_0xa9aut VLC_FOURCC( 0xa9, 'a', 'u', 't' )
#define ATOM_0xa9cpy VLC_FOURCC( 0xa9, 'c', 'p', 'y' )
#define ATOM_0xa9inf VLC_FOURCC( 0xa9, 'i', 'n', 'f' )
#define ATOM_0xa9isr VLC_FOURCC( 0xa9, 'i', 's', 'r' )
#define ATOM_0xa9lab VLC_FOURCC( 0xa9, 'l', 'a', 'b' )
#define ATOM_0xa9lal VLC_FOURCC( 0xa9, 'l', 'a', 'l' )
#define ATOM_0xa9ART VLC_FOURCC( 0xa9, 'A', 'R', 'T' )
#define ATOM_0xa9des VLC_FOURCC( 0xa9, 'd', 'e', 's' )
#define ATOM_0xa9dir VLC_FOURCC( 0xa9, 'd', 'i', 'r' )
#define ATOM_0xa9cmt VLC_FOURCC( 0xa9, 'c', 'm', 't' )
#define ATOM_0xa9req VLC_FOURCC( 0xa9, 'r', 'e', 'q' )
#define ATOM_0xa9day VLC_FOURCC( 0xa9, 'd', 'a', 'y' )
#define ATOM_0xa9fmt VLC_FOURCC( 0xa9, 'f', 'm', 't' )
#define ATOM_0xa9prd VLC_FOURCC( 0xa9, 'p', 'r', 'd' )
#define ATOM_0xa9prf VLC_FOURCC( 0xa9, 'p', 'r', 'f' )
#define ATOM_0xa9src VLC_FOURCC( 0xa9, 's', 'r', 'c' )
#define ATOM_0xa9alb VLC_FOURCC( 0xa9, 'a', 'l', 'b' )
#define ATOM_0xa9dis VLC_FOURCC( 0xa9, 'd', 'i', 's' )
#define ATOM_0xa9enc VLC_FOURCC( 0xa9, 'e', 'n', 'c' )
#define ATOM_0xa9trk VLC_FOURCC( 0xa9, 't', 'r', 'k' )
#define ATOM_0xa9url VLC_FOURCC( 0xa9, 'u', 'r', 'l' )
#define ATOM_0xa9dsa VLC_FOURCC( 0xa9, 'd', 's', 'a' )
#define ATOM_0xa9hst VLC_FOURCC( 0xa9, 'h', 's', 't' )
#define ATOM_0xa9ope VLC_FOURCC( 0xa9, 'o', 'p', 'e' )
#define ATOM_0xa9wrt VLC_FOURCC( 0xa9, 'w', 'r', 't' )
#define ATOM_0xa9com VLC_FOURCC( 0xa9, 'c', 'o', 'm' )
#define ATOM_0xa9gen VLC_FOURCC( 0xa9, 'g', 'e', 'n' )
#define ATOM_0xa9too VLC_FOURCC( 0xa9, 't', 'o', 'o' )
#define ATOM_0xa9wrn VLC_FOURCC( 0xa9, 'w', 'r', 'n' )
#define ATOM_0xa9swr VLC_FOURCC( 0xa9, 's', 'w', 'r' )
#define ATOM_0xa9mak VLC_FOURCC( 0xa9, 'm', 'a', 'k' )
#define ATOM_0xa9mal VLC_FOURCC( 0xa9, 'm', 'a', 'l' )
#define ATOM_0xa9mod VLC_FOURCC( 0xa9, 'm', 'o', 'd' )
#define ATOM_0xa9PRD VLC_FOURCC( 0xa9, 'P', 'R', 'D' )
#define ATOM_0xa9grp VLC_FOURCC( 0xa9, 'g', 'r', 'p' )
#define ATOM_0xa9lyr VLC_FOURCC( 0xa9, 'l', 'y', 'r' )
#define ATOM_0xa9gen VLC_FOURCC( 0xa9, 'g', 'e', 'n' )
#define ATOM_0xa9st3 VLC_FOURCC( 0xa9, 's', 't', '3' )
#define ATOM_0xa9ard VLC_FOURCC( 0xa9, 'a', 'r', 'd' )
#define ATOM_0xa9arg VLC_FOURCC( 0xa9, 'a', 'r', 'g' )
#define ATOM_0xa9cak VLC_FOURCC( 0xa9, 'c', 'a', 'k' )
#define ATOM_0xa9con VLC_FOURCC( 0xa9, 'c', 'o', 'n' )
#define ATOM_0xa9des VLC_FOURCC( 0xa9, 'd', 'e', 's' )
#define ATOM_0xa9lnt VLC_FOURCC( 0xa9, 'l', 'n', 't' )
#define ATOM_0xa9phg VLC_FOURCC( 0xa9, 'p', 'h', 'g' )
#define ATOM_0xa9pub VLC_FOURCC( 0xa9, 'p', 'u', 'b' )
#define ATOM_0xa9sne VLC_FOURCC( 0xa9, 's', 'n', 'e' )
#define ATOM_0xa9snm VLC_FOURCC( 0xa9, 's', 'n', 'm' )
#define ATOM_0xa9sol VLC_FOURCC( 0xa9, 's', 'o', 'l' )
#define ATOM_0xa9thx VLC_FOURCC( 0xa9, 't', 'h', 'x' )
#define ATOM_0xa9xpd VLC_FOURCC( 0xa9, 'x', 'p', 'd' )
#define ATOM_0xa9xyz VLC_FOURCC( 0xa9, 'x', 'y', 'z' )
#define ATOM_chpl VLC_FOURCC( 'c', 'h', 'p', 'l' )
#define ATOM_WLOC VLC_FOURCC( 'W', 'L', 'O', 'C' )

#define ATOM_meta VLC_FOURCC( 'm', 'e', 't', 'a' )
#define ATOM_ilst VLC_FOURCC( 'i', 'l', 's', 't' )
#define ATOM_covr VLC_FOURCC( 'c', 'o', 'v', 'r' )

#define ATOM_chap VLC_FOURCC( 'c', 'h', 'a', 'p' )

/* Do you want some debug information on all read boxes ? */
#ifndef NDEBUG
# define MP4_VERBOSE  1
#endif

struct MP4_Box_s;


/* uuid Universal Unique IDentifiers */
typedef struct UUID_s
{
    uint8_t b[16];
} UUID_t;

/* specific structure for all boxes */

typedef struct MP4_Box_data_tfxd_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint64_t i_fragment_duration;
    uint64_t i_fragment_abs_time;

} MP4_Box_data_tfxd_t;

typedef struct TfrfBoxDataFields_s
{
    uint64_t i_fragment_duration;
    uint64_t i_fragment_abs_time;

} TfrfBoxDataFields_t;

typedef struct MP4_Box_data_tfrf_s
{
    uint8_t  i_version;
    uint8_t  i_fragment_count;
    uint32_t i_flags;

    TfrfBoxDataFields_t *p_tfrf_data_fields;

} MP4_Box_data_tfrf_t;

typedef struct MP4_Box_data_ftyp_s
{
    uint32_t i_major_brand;
    uint32_t i_minor_version;

    uint32_t i_compatible_brands_count;
    uint32_t *i_compatible_brands;

} MP4_Box_data_ftyp_t;

typedef struct MP4_Box_data_mvhd_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint64_t i_creation_time;
    uint64_t i_modification_time;
    uint32_t i_timescale;
    uint64_t i_duration;

    int32_t  i_rate;
    int16_t  i_volume;
    int16_t  i_reserved1;
    uint32_t i_reserved2[2];
    int32_t  i_matrix[9];
    uint32_t i_predefined[6];
    uint32_t i_next_track_id;

} MP4_Box_data_mvhd_t;

#define MP4_TRACK_ENABLED    0x000001
#define MP4_TRACK_IN_MOVIE   0x000002
#define MP4_TRACK_IN_PREVIEW 0x000004
typedef struct MP4_Box_data_tkhd_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint64_t i_creation_time;
    uint64_t i_modification_time;
    uint32_t i_track_ID;
    uint32_t i_reserved;
    uint64_t i_duration;

    uint32_t i_reserved2[2];
    int16_t  i_layer;
    int16_t  i_predefined;

    int16_t  i_volume;
    uint16_t i_reserved3;
    int32_t  i_matrix[9];
    int32_t  i_width;
    int32_t  i_height;
    float    f_rotation;

} MP4_Box_data_tkhd_t;

typedef struct MP4_Box_data_mdhd_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint64_t i_creation_time;
    uint64_t i_modification_time;
    uint32_t i_timescale;
    uint64_t i_duration;

    char     rgs_language[3]; /* ISO-639-2/T or Mac lang table */
    bool     b_mac_encoding;  /* media using mac encoding */

    uint16_t i_predefined;

} MP4_Box_data_mdhd_t;

typedef struct MP4_Box_data_hdlr_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_predefined;
    uint32_t i_handler_type; /* "vide" "soun" "hint" "odsm"
                           "crsm" "sdsm" "m7sm" "ocsm"
                           "ipsm" "mjsm" */

    unsigned char *psz_name; /* in UTF-8 */

} MP4_Box_data_hdlr_t;

typedef struct MP4_Box_data_vmhd_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    int16_t  i_graphics_mode;
    int16_t  i_opcolor[3];

} MP4_Box_data_vmhd_t;

typedef struct MP4_Box_data_smhd_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    int16_t  i_balance;
    int16_t  i_reserved;

} MP4_Box_data_smhd_t;

typedef struct MP4_Box_data_hmhd_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint16_t i_max_PDU_size;
    uint16_t i_avg_PDU_size;
    uint32_t i_max_bitrate;
    uint32_t i_avg_bitrate;
    uint32_t i_reserved;

} MP4_Box_data_hmhd_t;

typedef struct MP4_Box_data_url_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    char *psz_location;

} MP4_Box_data_url_t;

typedef struct MP4_Box_data_urn_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    char *psz_name;
    char *psz_location;

} MP4_Box_data_urn_t;

typedef struct MP4_Box_data_dref_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_entry_count;
/* XXX it's also a container with i_entry_count entry */
} MP4_Box_data_dref_t;

typedef struct MP4_Box_data_stts_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_entry_count;
    uint32_t *pi_sample_count; /* these are array */
    int32_t  *pi_sample_delta;

} MP4_Box_data_stts_t;

typedef struct MP4_Box_data_ctts_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_entry_count;

    uint32_t *pi_sample_count; /* these are array */
    int32_t *pi_sample_offset;

} MP4_Box_data_ctts_t;


typedef struct MP4_Box_data_sample_soun_s
{
    uint8_t  i_reserved1[6];
    uint16_t i_data_reference_index;

    //uint32_t i_reserved2[2];
    uint16_t i_qt_version;
    uint16_t i_qt_revision_level;
    uint32_t i_qt_vendor;

    uint16_t i_channelcount;
    uint16_t i_samplesize;
    uint16_t i_compressionid;
    uint16_t i_reserved3;
    uint16_t i_sampleratehi; /* timescale of track */
    uint16_t i_sampleratelo;

    /* for version 1 (i_reserved1[0] == 1) */
    uint32_t i_sample_per_packet;
    uint32_t i_bytes_per_packet;
    uint32_t i_bytes_per_frame;
    uint32_t i_bytes_per_sample;

    /* v2 */
    uint32_t i_constbitsperchannel; /* consts are nonzero only if constant */
    uint32_t i_formatflags;
    uint32_t i_constbytesperaudiopacket;
    uint32_t i_constLPCMframesperaudiopacket;

    /* XXX hack */
    int     i_qt_description;
    uint8_t *p_qt_description;

} MP4_Box_data_sample_soun_t;

typedef struct MP4_Box_data_sample_vide_s
{
    uint8_t  i_reserved1[6];
    uint16_t i_data_reference_index;

    uint16_t i_qt_version;
    uint16_t i_qt_revision_level;
    uint32_t i_qt_vendor;

    uint32_t i_qt_temporal_quality;
    uint32_t i_qt_spatial_quality;

    int16_t  i_width;
    int16_t  i_height;

    uint32_t i_horizresolution;
    uint32_t i_vertresolution;

    uint32_t i_qt_data_size;
    uint16_t i_qt_frame_count;

    uint8_t  i_compressorname[32];
    int16_t  i_depth;

    int16_t  i_qt_color_table;

    /* XXX hack ImageDescription */
    int     i_qt_image_description;
    uint8_t *p_qt_image_description;

} MP4_Box_data_sample_vide_t;

#define MP4_TEXT_DISPLAY_FLAG_DONT_DISPLAY       (1<<0)
#define MP4_TEXT_DISPLAY_FLAG_AUTO_SCALE         (1<<1)
#define MP4_TEXT_DISPLAY_FLAG_CLIP_TO_TEXT_BOX   (1<<2)
#define MP4_TEXT_DISPLAY_FLAG_USE_MOVIE_BG_COLOR (1<<3)
#define MP4_TEXT_DISPLAY_FLAG_SHRINK_TEXT_BOX_TO_FIT (1<<4)
#define MP4_TEXT_DISPLAY_FLAG_SCROLL_IN          (1<<5)
#define MP4_TEXT_DISPLAY_FLAG_SCROLL_OUT         (1<<6)
#define MP4_TEXT_DISPLAY_FLAG_HORIZONTAL_SCROLL  (1<<7)
#define MP4_TEXT_DISPLAY_FLAG_REVERSE_SCROLL     (1<<8)
#define MP4_TEXT_DISPLAY_FLAG_CONTINUOUS_SCROLL  (1<<9)
#define MP4_TEXT_DISPLAY_FLAG_FLOW_HORIZONTAL    (1<<10)
#define MP4_TEXT_DISPLAY_FLAG_CONTINUOUS_KARAOKE (1<<11)
#define MP4_TEXT_DISPLAY_FLAG_DROP_SHADOW        (1<<12)
#define MP4_TEXT_DISPLAY_FLAG_ANTI_ALIAS         (1<<13)
#define MP4_TEXT_DISPLAY_FLAG_KEYED_TEXT         (1<<14)
#define MP4_TEXT_DISPLAY_FLAG_INVERSE_HILITE     (1<<15)
#define MP4_TEXT_DISPLAY_FLAG_COLOR_HILITE       (1<<16)
#define MP4_TEXT_DISPLAY_FLAG_WRITE_VERTICALLY   (1<<17)

typedef struct
{
    uint32_t i_reserved1;
    uint16_t i_reserved2;

    uint16_t i_data_reference_index;

    uint32_t i_display_flags;   // TextDescription and Tx3gDescription

    int8_t i_justification_horizontal; // left(0), centered(1), right(-1)
    int8_t i_justification_vertical;   // top(0), centered(1), bottom(-1)

    uint16_t i_background_color[4];

    uint16_t i_text_box_top;
    uint16_t i_text_box_left;
    uint16_t i_text_box_bottom;
    uint16_t i_text_box_right;

    uint32_t i_reserved3;

    uint16_t i_font_id;
    uint8_t  i_font_face;
    uint8_t  i_font_size;
    uint32_t i_font_color; //RGBA

    // TODO to complete
} MP4_Box_data_sample_text_t;

typedef struct MP4_Box_data_sample_hint_s
{
    uint8_t  i_reserved1[6];
    uint16_t i_data_reference_index;

    uint8_t *p_data;

} MP4_Box_data_sample_hint_t;

typedef struct MP4_Box_data_moviehintinformation_rtp_s
{
    uint32_t i_description_format;
    unsigned char *psz_text;

} MP4_Box_data_moviehintinformation_rtp_t;



typedef struct MP4_Box_data_stsd_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_entry_count;

    /* it contains SampleEntry handled as if it was Box */

} MP4_Box_data_stsd_t;


typedef struct MP4_Box_data_stsz_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_sample_size;
    uint32_t i_sample_count;

    uint32_t *i_entry_size; /* array , empty if i_sample_size != 0 */

} MP4_Box_data_stsz_t;

typedef struct MP4_Box_data_stz2_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_sample_size; /* 24 bits */
    uint8_t  i_field_size;
    uint32_t i_sample_count;

    uint32_t *i_entry_size; /* array: unsigned int(i_field_size) entry_size */

} MP4_Box_data_stz2_t;

typedef struct MP4_Box_data_stsc_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_entry_count;

    uint32_t *i_first_chunk; /* theses are arrays */
    uint32_t *i_samples_per_chunk;
    uint32_t *i_sample_description_index;

} MP4_Box_data_stsc_t;


typedef struct MP4_Box_data_co64_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_entry_count;

    uint64_t *i_chunk_offset;

} MP4_Box_data_co64_t;


typedef struct MP4_Box_data_stss_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_entry_count;

    uint32_t *i_sample_number;

} MP4_Box_data_stss_t;

typedef struct MP4_Box_data_stsh_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_entry_count;

    uint32_t *i_shadowed_sample_number;
    uint32_t *i_sync_sample_number;

} MP4_Box_data_stsh_t;

typedef struct MP4_Box_data_stdp_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint16_t *i_priority;

} MP4_Box_data_stdp_t;

typedef struct MP4_Box_data_padb_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_sample_count;

    uint16_t *i_reserved1;   /* 1bit  */
    uint16_t *i_pad2;        /* 3bits */
    uint16_t *i_reserved2;   /* 1bit  */
    uint16_t *i_pad1;        /* 3bits */


} MP4_Box_data_padb_t;


typedef struct MP4_Box_data_elst_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_entry_count;

    uint64_t *i_segment_duration;
    int64_t  *i_media_time;
    uint16_t *i_media_rate_integer;
    uint16_t *i_media_rate_fraction;


} MP4_Box_data_elst_t;

typedef struct MP4_Box_data_cprt_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    char     rgs_language[3]; /* ISO-639-2/T */

    char *psz_notice;
} MP4_Box_data_cprt_t;


/* DecoderConfigDescriptor */
typedef struct MP4_descriptor_decoder_config_s
{
    uint8_t i_objectTypeIndication;
    uint8_t i_streamType;
    int     b_upStream;
    int     i_buffer_sizeDB;
    int     i_max_bitrate;
    int     i_avg_bitrate;

    int     i_decoder_specific_info_len;
    uint8_t *p_decoder_specific_info;
    /* some other stuff */

} MP4_descriptor_decoder_config_t;

typedef struct MP4_descriptor_SL_config_s
{

    int i_dummy; /* ANSI C forbids empty structures */

} MP4_descriptor_SL_config_t;


typedef struct MP4_descriptor_ES_s
{
    uint16_t i_ES_ID;
    int      b_stream_dependence;
    int      b_url;
    int      b_OCRstream;
    int      i_stream_priority;

    int      i_depend_on_ES_ID; /* if b_stream_dependence set */

    unsigned char *psz_URL;

    uint16_t i_OCR_ES_ID;       /* if b_OCRstream */
    MP4_descriptor_decoder_config_t *p_decConfigDescr;

    MP4_descriptor_SL_config_t *p_slConfigDescr;

    /* some other stuff ... */

} MP4_descriptor_ES_t;

/* ES descriptor */
typedef struct MP4_Box_data_esds_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    MP4_descriptor_ES_t es_descriptor;

} MP4_Box_data_esds_t;


typedef struct MP4_Box_data_dcom_s
{
    uint32_t i_algorithm; /* fourcc */

} MP4_Box_data_dcom_t;

typedef struct MP4_Box_data_cmvd_s
{
    uint32_t i_uncompressed_size;
    uint32_t i_compressed_size;

    int     b_compressed; /* Set to 1 if compressed data, 0 if uncompressed */
    uint8_t *p_data;

} MP4_Box_data_cmvd_t;

typedef struct MP4_Box_data_cmov_s
{
    struct MP4_Box_s *p_moov; /* uncompressed moov */

} MP4_Box_data_cmov_t;

typedef struct
{
    uint32_t i_type;
} MP4_Box_data_frma_t;

typedef struct
{
    uint32_t i_init;
    uint32_t i_encr;
    uint32_t i_decr;
} MP4_Box_data_skcr_t;

typedef struct
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_ref_type;
    char     *psz_ref;

} MP4_Box_data_rdrf_t;

typedef struct
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_rate;

} MP4_Box_data_rmdr_t;

typedef struct
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_gestaltType;
    uint32_t i_val1;
    uint32_t i_val2;
    uint16_t i_checkType;   /* 0: val1 is version min
                               1: gestalt value & val2 == val1 */

} MP4_Box_data_rmvc_t;

typedef struct
{
    uint8_t  i_version;
    uint32_t i_flags;


} MP4_Box_data_rmcd_t;

typedef struct
{
    uint32_t i_quality;

} MP4_Box_data_rmqu_t;

typedef struct MP4_Box_data_mfhd_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_sequence_number;

} MP4_Box_data_mfhd_t;

typedef struct MP4_Box_sidx_item_s
{
    bool     b_reference_type;
    uint32_t i_referenced_size;
    uint32_t i_subsegment_duration;
    bool     b_starts_with_SAP;
    uint8_t  i_SAP_type;
    uint32_t i_SAP_delta_time;

} MP4_Box_sidx_item_t;

typedef struct MP4_Box_data_sidx_s
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_reference_ID;
    uint32_t i_timescale;
    uint64_t i_earliest_presentation_time;
    uint64_t i_first_offset;
    uint16_t i_reference_count;

    MP4_Box_sidx_item_t *p_items;

} MP4_Box_data_sidx_t;

#define MP4_TFHD_BASE_DATA_OFFSET     (1LL<<0)
#define MP4_TFHD_SAMPLE_DESC_INDEX    (1LL<<1)
#define MP4_TFHD_DFLT_SAMPLE_DURATION (1LL<<3)
#define MP4_TFHD_DFLT_SAMPLE_SIZE     (1LL<<4)
#define MP4_TFHD_DFLT_SAMPLE_FLAGS    (1LL<<5)
#define MP4_TFHD_DURATION_IS_EMPTY    (1LL<<16)
#define MP4_TFHD_DEFAULT_BASE_IS_MOOF (1LL<<17)
typedef struct MP4_Box_data_tfhd_s
{
    uint8_t  i_version;
    bool     b_empty;
    uint32_t i_flags;
    uint32_t i_track_ID;

    /* optional fields */
    uint64_t i_base_data_offset;
    uint32_t i_sample_description_index;
    uint32_t i_default_sample_duration;
    uint32_t i_default_sample_size;
    uint32_t i_default_sample_flags;

} MP4_Box_data_tfhd_t;

#define MP4_TRUN_DATA_OFFSET         (1<<0)
#define MP4_TRUN_FIRST_FLAGS         (1<<2)
#define MP4_TRUN_SAMPLE_DURATION     (1<<8)
#define MP4_TRUN_SAMPLE_SIZE         (1<<9)
#define MP4_TRUN_SAMPLE_FLAGS        (1<<10)
#define MP4_TRUN_SAMPLE_TIME_OFFSET  (1<<11)
typedef struct MP4_descriptor_trun_sample_t
{
    uint32_t i_duration;
    uint32_t i_size;
    uint32_t i_flags;
    uint32_t i_composition_time_offset; /* version == 0 ? signed : unsigned */
} MP4_descriptor_trun_sample_t;

typedef struct MP4_Box_data_trun_s
{
    uint8_t  i_version;
    uint32_t i_flags;
    uint32_t i_sample_count;

    /* optional fields */
    int32_t i_data_offset;
    uint32_t i_first_sample_flags;

    MP4_descriptor_trun_sample_t *p_samples;

} MP4_Box_data_trun_t;


typedef struct
{
    char *psz_text;

} MP4_Box_data_0xa9xxx_t;

typedef struct
{
    char *psz_text;

} MP4_Box_data_name_t;

typedef struct
{
    uint32_t i_entry_count;
    uint32_t *i_track_ID;

} MP4_Box_data_tref_generic_t;

typedef struct
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint8_t i_chapter;
    struct
    {
        char    *psz_name;
        int64_t  i_start;
    } chapter[256];
} MP4_Box_data_chpl_t;

typedef struct
{
    uint8_t i_version;
    uint8_t i_profile;
    uint8_t i_profile_compatibility;
    uint8_t i_level;

    uint8_t i_reserved1;     /* 6 bits */
    uint8_t i_length_size;

    uint8_t i_reserved2;    /* 3 bits */
    uint8_t  i_sps;
    uint16_t *i_sps_length;
    uint8_t  **sps;

    uint8_t  i_pps;
    uint16_t *i_pps_length;
    uint8_t  **pps;

    /* XXX: Hack raw avcC atom payload */
    int     i_avcC;
    uint8_t *p_avcC;

} MP4_Box_data_avcC_t;

typedef struct
{
    uint8_t i_fscod;
    uint8_t i_bsid;
    uint8_t i_bsmod;
    uint8_t i_acmod;
    uint8_t i_lfeon;
    uint8_t i_bitrate_code;

} MP4_Box_data_dac3_t;

typedef struct
{
    uint8_t i_profile_level;

    int i_vc1;
    uint8_t *p_vc1;

} MP4_Box_data_dvc1_t;

typedef struct
{
    uint16_t i_little_endian;

} MP4_Box_data_enda_t;

typedef struct
{
    uint16_t i_genre;

} MP4_Box_data_gnre_t;

typedef struct
{
    uint32_t i_track_number;
    uint32_t i_track_total;

} MP4_Box_data_trkn_t;

typedef struct
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint16_t i_object_descriptor;
    uint8_t i_OD_profile_level;
    uint8_t i_scene_profile_level;
    uint8_t i_audio_profile_level;
    uint8_t i_visual_profile_level;
    uint8_t i_graphics_profile_level;

} MP4_Box_data_iods_t;

typedef struct
{
    uint32_t i_horizontal_spacing;
    uint32_t i_vertical_spacing;
} MP4_Box_data_pasp_t;

typedef struct
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint64_t i_fragment_duration;
} MP4_Box_data_mehd_t;

typedef struct
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_track_ID;
    uint32_t i_default_sample_description_index;
    uint32_t i_default_sample_duration;
    uint32_t i_default_sample_size;
    uint32_t i_default_sample_flags;
} MP4_Box_data_trex_t;

typedef struct
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint8_t *p_sample_table;
} MP4_Box_data_sdtp_t;

typedef struct
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_size;
} MP4_Box_data_mfro_t;

typedef struct
{
    uint8_t  i_version;
    uint32_t i_flags;

    uint32_t i_track_ID;
    uint32_t i_number_of_entries;

    uint8_t i_length_size_of_traf_num;
    uint8_t i_length_size_of_trun_num;
    uint8_t i_length_size_of_sample_num;

    uint32_t *p_time;
    uint32_t *p_moof_offset;
    uint8_t *p_traf_number;
    uint8_t *p_trun_number;
    uint8_t *p_sample_number;
} MP4_Box_data_tfra_t;

typedef struct
{
    uint64_t i_duration;
    uint32_t i_timescale;
    uint16_t i_track_ID;
    uint8_t  i_es_cat;

    uint32_t FourCC;
    uint32_t Bitrate;
    uint32_t MaxWidth;
    uint32_t MaxHeight;
    uint32_t SamplingRate;
    uint32_t Channels;
    uint32_t BitsPerSample;
    uint32_t AudioTag;
    uint16_t nBlockAlign;
    uint8_t  cpd_len;
    uint8_t  *CodecPrivateData;
} MP4_Box_data_stra_t;

typedef struct
{
    uint32_t i_hvcC;
    uint8_t *p_hvcC;
} MP4_Box_data_hvcC_t;

typedef struct
{
    uint8_t *p_blob;
    uint32_t i_blob;
} MP4_Box_data_data_t;

/*
typedef struct MP4_Box_data__s
{
    uint8_t  i_version;
    uint32_t i_flags;

} MP4_Box_data__t;

*/

typedef union MP4_Box_data_s
{
    MP4_Box_data_ftyp_t *p_ftyp;
    MP4_Box_data_mvhd_t *p_mvhd;
    MP4_Box_data_mfhd_t *p_mfhd;
    MP4_Box_data_sidx_t *p_sidx;
    MP4_Box_data_tfhd_t *p_tfhd;
    MP4_Box_data_trun_t *p_trun;
    MP4_Box_data_tkhd_t *p_tkhd;
    MP4_Box_data_mdhd_t *p_mdhd;
    MP4_Box_data_hdlr_t *p_hdlr;
    MP4_Box_data_vmhd_t *p_vmhd;
    MP4_Box_data_smhd_t *p_smhd;
    MP4_Box_data_hmhd_t *p_hmhd;
    MP4_Box_data_url_t  *p_url;
    MP4_Box_data_urn_t  *p_urn;
    MP4_Box_data_dref_t *p_dref;
    MP4_Box_data_stts_t *p_stts;
    MP4_Box_data_ctts_t *p_ctts;
    MP4_Box_data_stsd_t *p_stsd;
    MP4_Box_data_sample_vide_t *p_sample_vide;
    MP4_Box_data_sample_soun_t *p_sample_soun;
    MP4_Box_data_sample_text_t *p_sample_text;
    MP4_Box_data_sample_hint_t *p_sample_hint;

    MP4_Box_data_esds_t *p_esds;
    MP4_Box_data_avcC_t *p_avcC;
    MP4_Box_data_dac3_t *p_dac3;
    MP4_Box_data_dvc1_t *p_dvc1;
    MP4_Box_data_enda_t *p_enda;
    MP4_Box_data_gnre_t *p_gnre;
    MP4_Box_data_trkn_t *p_trkn;
    MP4_Box_data_iods_t *p_iods;
    MP4_Box_data_pasp_t *p_pasp;
    MP4_Box_data_trex_t *p_trex;
    MP4_Box_data_mehd_t *p_mehd;
    MP4_Box_data_sdtp_t *p_sdtp;

    MP4_Box_data_tfra_t *p_tfra;
    MP4_Box_data_mfro_t *p_mfro;
    MP4_Box_data_stra_t *p_stra;

    MP4_Box_data_stsz_t *p_stsz;
    MP4_Box_data_stz2_t *p_stz2;
    MP4_Box_data_stsc_t *p_stsc;
    MP4_Box_data_co64_t *p_co64;
    MP4_Box_data_stss_t *p_stss;
    MP4_Box_data_stsh_t *p_stsh;
    MP4_Box_data_stdp_t *p_stdp;
    MP4_Box_data_padb_t *p_padb;
    MP4_Box_data_elst_t *p_elst;
    MP4_Box_data_cprt_t *p_cprt;

    MP4_Box_data_dcom_t *p_dcom;
    MP4_Box_data_cmvd_t *p_cmvd;
    MP4_Box_data_cmov_t *p_cmov;

    MP4_Box_data_moviehintinformation_rtp_t p_moviehintinformation_rtp;

    MP4_Box_data_frma_t *p_frma;
    MP4_Box_data_skcr_t *p_skcr;

    MP4_Box_data_rdrf_t *p_rdrf;
    MP4_Box_data_rmdr_t *p_rmdr;
    MP4_Box_data_rmqu_t *p_rmqu;
    MP4_Box_data_rmvc_t *p_rmvc;

    MP4_Box_data_0xa9xxx_t *p_0xa9xxx;
    MP4_Box_data_chpl_t *p_chpl;
    MP4_Box_data_tref_generic_t *p_tref_generic;
    MP4_Box_data_name_t *p_name;

    MP4_Box_data_tfrf_t *p_tfrf;
    MP4_Box_data_tfxd_t *p_tfxd;
    MP4_Box_data_hvcC_t *p_hvcC;

    MP4_Box_data_data_t *p_data;

    void                *p_payload; /* for unknow type */
} MP4_Box_data_t;



/* the most basic structure */
typedef struct MP4_Box_s
{
    off_t        i_pos;      /* absolute position */

    uint32_t     i_type;
    uint32_t     i_shortsize;
    uint32_t     i_handler;  /* stsd handler */

    UUID_t       i_uuid;  /* Set if i_type == "uuid" */

    uint64_t     i_size; /* always set so use it */

    MP4_Box_data_t   data;   /* union of pointers on extended data depending
                                on i_type (or i_usertype) */

    struct MP4_Box_s *p_father; /* pointer on the father Box */

    struct MP4_Box_s *p_first;  /* pointer on the first child Box */
    struct MP4_Box_s *p_last;

    struct MP4_Box_s *p_next;   /* pointer on the next boxes at the same level */

} MP4_Box_t;

static inline size_t mp4_box_headersize( MP4_Box_t *p_box )
{
    return 8
        + ( p_box->i_shortsize == 1 ? 8 : 0 )
        + ( p_box->i_type == ATOM_uuid ? 16 : 0 );
}

#define MP4_GETX_PRIVATE(dst, code, size) do { \
    if( (i_read) >= (size) ) { dst = (code); p_peek += (size); } \
    else { dst = 0; }   \
    i_read -= (size);   \
  } while(0)

#define MP4_GET1BYTE( dst )  MP4_GETX_PRIVATE( dst, *p_peek, 1 )
#define MP4_GET2BYTES( dst ) MP4_GETX_PRIVATE( dst, GetWBE(p_peek), 2 )
#define MP4_GET3BYTES( dst ) MP4_GETX_PRIVATE( dst, Get24bBE(p_peek), 3 )
#define MP4_GET4BYTES( dst ) MP4_GETX_PRIVATE( dst, GetDWBE(p_peek), 4 )
#define MP4_GET8BYTES( dst ) MP4_GETX_PRIVATE( dst, GetQWBE(p_peek), 8 )
#define MP4_GETFOURCC( dst ) MP4_GETX_PRIVATE( dst, \
                VLC_FOURCC(p_peek[0],p_peek[1],p_peek[2],p_peek[3]), 4)

#define MP4_GETVERSIONFLAGS( p_void ) \
    MP4_GET1BYTE( p_void->i_version ); \
    MP4_GET3BYTES( p_void->i_flags )

#define MP4_GETSTRINGZ( p_str )         \
    if( (i_read > 0) && (p_peek[0]) )   \
    {       \
        const int __i_copy__ = strnlen( (char*)p_peek, i_read-1 );  \
        p_str = (typeof(p_str))malloc( __i_copy__+1 );               \
        if( p_str ) \
        { \
             memcpy( p_str, p_peek, __i_copy__ ); \
             p_str[__i_copy__] = 0; \
        } \
        p_peek += __i_copy__ + 1;   \
        i_read -= __i_copy__ + 1;   \
    }       \
    else    \
    {       \
        p_str = NULL; \
    }

#define MP4_READBOX_ENTER( MP4_Box_data_TYPE_t ) \
    int64_t  i_read = p_box->i_size; \
    uint8_t *p_peek, *p_buff; \
    int i_actually_read; \
    if( !( p_peek = p_buff = (uint8_t *)malloc( i_read ) ) ) \
    { \
        return( 0 ); \
    } \
    i_actually_read = stream_Read( p_stream, p_peek, i_read ); \
    if( i_actually_read < 0 || (int64_t)i_actually_read < i_read )\
    { \
        msg_Warn( p_stream, "MP4_READBOX_ENTER: I got %i bytes, "\
        "but I requested %" PRId64 "", i_actually_read, i_read );\
        free( p_buff ); \
        return( 0 ); \
    } \
    p_peek += mp4_box_headersize( p_box ); \
    i_read -= mp4_box_headersize( p_box ); \
    if( !( p_box->data.p_payload = calloc( 1, sizeof( MP4_Box_data_TYPE_t ) ) ) ) \
    { \
        free( p_buff ); \
        return( 0 ); \
    }

#define MP4_READBOX_EXIT( i_code ) \
    do \
    { \
        free( p_buff ); \
        if( i_read < 0 ) \
            msg_Warn( p_stream, "Not enough data" ); \
        return( i_code ); \
    } while (0)


/* This macro is used when we want to printf the box type
 * APPLE annotation box is :
 *  either 0xA9 + 24-bit ASCII text string (and 0xA9 isn't printable)
 *  either 32-bit ASCII text string
 */
#define MP4_BOX_TYPE_ASCII() ( ((char*)&p_box->i_type)[0] != (char)0xA9 )

static inline uint32_t Get24bBE( const uint8_t *p )
{
    return( ( p[0] <<16 ) + ( p[1] <<8 ) + p[2] );
}

static inline void GetUUID( UUID_t *p_uuid, const uint8_t *p_buff )
{
    memcpy( p_uuid, p_buff, 16 );
}

static inline int CmpUUID( const UUID_t *u1, const UUID_t *u2 )
{
    return memcmp( u1, u2, 16 );
}

static inline void CreateUUID( UUID_t *p_uuid, uint32_t i_fourcc )
{
    /* made by 0xXXXXXXXX-0011-0010-8000-00aa00389b71
            where XXXXXXXX is the fourcc */
    /* FIXME implement this */
    (void)p_uuid;
    (void)i_fourcc;
}

static const UUID_t TfrfBoxUUID = {
                { 0xd4, 0x80, 0x7e, 0xf2, 0xca, 0x39, 0x46, 0x95,
                  0x8e, 0x54, 0x26, 0xcb, 0x9e, 0x46, 0xa7, 0x9f } };

static const UUID_t TfxdBoxUUID = {
                { 0x6d, 0x1d, 0x9b, 0x05, 0x42, 0xd5, 0x44, 0xe6,
                  0x80, 0xe2, 0x14, 0x1d, 0xaf, 0xf7, 0x57, 0xb2 } };

static const UUID_t SmooBoxUUID = {
                { 0xe1, 0xda, 0x72, 0xba, 0x24, 0xd7, 0x43, 0xc3,
                  0xa6, 0xa5, 0x1b, 0x57, 0x59, 0xa1, 0xa9, 0x2c } };

static const UUID_t StraBoxUUID = {
                { 0xb0, 0x3e, 0xf7, 0x70, 0x33, 0xbd, 0x4b, 0xac,
                  0x96, 0xc7, 0xbf, 0x25, 0xf9, 0x7e, 0x24, 0x47 } };

MP4_Box_t *MP4_BoxGetSmooBox( stream_t * );

/*****************************************************************************
 * MP4_BoxGetNextChunk : Parse the entire moof box.
 *****************************************************************************
 *  The first box is a virtual box "root" and is the father of the boxes
 *  'moof' and, possibly, 'sidx'.
 *****************************************************************************/
MP4_Box_t *MP4_BoxGetNextChunk( stream_t * );

/*****************************************************************************
 * MP4_BoxGetRoot : Parse the entire file, and create all boxes in memory
 *****************************************************************************
 *  The first box is a virtual box "root" and is the father for all first
 *  level boxes
 *****************************************************************************/
MP4_Box_t *MP4_BoxGetRoot( stream_t * );

/*****************************************************************************
 * MP4_FreeBox : free memory allocated after read with MP4_ReadBox
 *               or MP4_BoxGetRoot, this means also children boxes
 * XXX : all children have to be allocated by a malloc !! and
 *         p_box is freed
 *****************************************************************************/
void MP4_BoxFree( stream_t *, MP4_Box_t *p_box );

/*****************************************************************************
 * MP4_DumpBoxStructure: print the structure of the p_box
 *****************************************************************************
 * Useful while debugging
 *****************************************************************************/
void MP4_BoxDumpStructure( stream_t *p_input, MP4_Box_t *p_box );

/*****************************************************************************
 * MP4_BoxGet: find a box given a path relative to p_box
 *****************************************************************************
 * Path Format: . .. / as usual
 *              [number] to specifie box number ex: trak[12]
 *
 * ex: /moov/trak[12]
 *     ../mdia
 *****************************************************************************/
MP4_Box_t *MP4_BoxGet( MP4_Box_t *p_box, const char *psz_fmt, ... );

/*****************************************************************************
 * MP4_BoxCount: find number of box given a path relative to p_box
 *****************************************************************************
 * Path Format: . .. / as usual
 *              [number] to specifie box number ex: trak[12]
 *
 * ex: /moov/trak
 *     ../mdia
 *****************************************************************************/
int MP4_BoxCount( MP4_Box_t *p_box, const char *psz_fmt, ... );

/* Internal functions exposed for MKV demux */
int MP4_ReadBoxCommon( stream_t *p_stream, MP4_Box_t *p_box );
int MP4_ReadBoxContainerChildren( stream_t *p_stream, MP4_Box_t *p_container,
                                  uint32_t i_last_child );
int MP4_ReadBox_sample_vide( stream_t *p_stream, MP4_Box_t *p_box );
void MP4_FreeBox_sample_vide( MP4_Box_t *p_box );

#ifdef __cplusplus
}
#endif
#endif



// patch.h
#ifndef PATCH_H
#define pATCH_H

#include <stdlib.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif



typedef uint32_t vlc_fourcc_t;
#ifdef __GNUC__
#   define likely(p)   __builtin_expect(!!(p), 1)
#   define unlikely(p) __builtin_expect(!!(p), 0)
#else
#   define likely(p)   (!!(p))
#   define unlikely(p) (!!(p))
#endif

#define min(a, b)           ((a) > (b) ? (b) : (a))
#define max(a, b)           ((a) > (b) ? (a) : (b))

#define FREENULL(a) do { if (a) {free( a ); a = NULL;} } while(0)

void msg_log(int log_lvl, const char *fmt, ...);
#define MSG_DGB  1
#define MSG_WARN 2
#define MSG_ERR  3

#define msg_Dbg(fmt, ...) msg_log(MSG_DGB, __VA_ARGS__)
#define msg_Warn(fmt, ...) msg_log(MSG_WARN, __VA_ARGS__)
#define msg_Err(fmt, ...) msg_log(MSG_ERR, __VA_ARGS__)

#define VLC_UNUSED(x) (void)(x)
#   define __MIN(a, b)   ( ((a) < (b)) ? (a) : (b) )
#   define __MAX(a, b)   ( ((a) > (b)) ? (a) : (b) )


#ifdef __GNUC__
# define VLC_GCC_VERSION(maj,min) \
    ((__GNUC__ > (maj)) || (__GNUC__ == (maj) && __GNUC_MINOR__ >= (min)))
#else
# define VLC_GCC_VERSION(maj,min) (0)
#endif

static inline uint16_t bswap16 (uint16_t x)
{
    return (x << 8) | (x >> 8);
}

/** Byte swap (32 bits) */
static inline uint32_t bswap32 (uint32_t x)
{
#if VLC_GCC_VERSION(4,3) || defined(__clang__)
    return __builtin_bswap32 (x);
#else
    return ((x & 0x000000FF) << 24)
         | ((x & 0x0000FF00) <<  8)
         | ((x & 0x00FF0000) >>  8)
         | ((x & 0xFF000000) >> 24);
#endif
}

/** Byte swap (64 bits) */
static inline uint64_t bswap64 (uint64_t x)
{
#if VLC_GCC_VERSION(4,3) || defined(__clang__)
    return __builtin_bswap64 (x);
#elif !defined (__cplusplus)
    return ((x & 0x00000000000000FF) << 56)
         | ((x & 0x000000000000FF00) << 40)
         | ((x & 0x0000000000FF0000) << 24)
         | ((x & 0x00000000FF000000) <<  8)
         | ((x & 0x000000FF00000000) >>  8)
         | ((x & 0x0000FF0000000000) >> 24)
         | ((x & 0x00FF000000000000) >> 40)
         | ((x & 0xFF00000000000000) >> 56);
#else
    return ((x & 0x00000000000000FFULL) << 56)
         | ((x & 0x000000000000FF00ULL) << 40)
         | ((x & 0x0000000000FF0000ULL) << 24)
         | ((x & 0x00000000FF000000ULL) <<  8)
         | ((x & 0x000000FF00000000ULL) >>  8)
         | ((x & 0x0000FF0000000000ULL) >> 24)
         | ((x & 0x00FF000000000000ULL) >> 40)
         | ((x & 0xFF00000000000000ULL) >> 56);
#endif
}

#ifdef WORDS_BIGENDIAN
# define hton16(i) ((uint16_t)(i))
# define hton32(i) ((uint32_t)(i))
# define hton64(i) ((uint64_t)(i))
#else
# define hton16(i) bswap16(i)
# define hton32(i) bswap32(i)
# define hton64(i) bswap64(i)
#endif
#define ntoh16(i) hton16(i)
#define ntoh32(i) hton32(i)
#define ntoh64(i) hton64(i)


/** Reads 16 bits in network byte order */
static inline uint16_t U16_AT (const void *p)
{
    uint16_t x;

    memcpy (&x, p, sizeof (x));
    return ntoh16 (x);
}

/** Reads 32 bits in network byte order */
static inline uint32_t U32_AT (const void *p)
{
    uint32_t x;

    memcpy (&x, p, sizeof (x));
    return ntoh32 (x);
}

/** Reads 64 bits in network byte order */
static inline uint64_t U64_AT (const void *p)
{
    uint64_t x;

    memcpy (&x, p, sizeof (x));
    return ntoh64 (x);
}

#define GetWBE(p)  U16_AT(p)
#define GetDWBE(p) U32_AT(p)
#define GetQWBE(p) U64_AT(p)


#define stream_Read(s, buf, size) ((stream_t*)s)->read(((stream_t*)s), buf, size)
#define stream_write(s, buf, size) ((stream_t*)s)->write(((stream_t*)s), buf, size)
#define stream_Peek(s, buf, size) ((stream_t*)s)->peek(((stream_t*)s), buf, size)
#define stream_Seek(s, offset) ((stream_t*)s)->seek(((stream_t*)s), offset, SEEK_SET)
#define stream_Tell(s) ((stream_t*)s)->tell(((stream_t*)s))
#define stream_Size(s) ((stream_t*)s)->size(((stream_t*)s))

bool decodeQtLanguageCode( uint16_t i_language_code, char *psz_iso,
                                  bool *b_mactables );

#ifdef __cplusplus
}
#endif
#endif



// patch.c
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <assert.h>
//#include "libmp4parser-patch.h"
//#include "patch.h"

bool decodeQtLanguageCode( uint16_t i_language_code, char *psz_iso,
                                  bool *b_mactables )
{
    static const char * psz_qt_to_iso639_2T_lower =
            "eng"    "fra"    "deu"    "ita"    "nld"
            "swe"    "spa"    "dan"    "por"    "nor" /* 5-9 */
            "heb"    "jpn"    "ara"    "fin"    "gre"
            "isl"    "mlt"    "tur"    "hrv"    "zho" /* 15-19 */
            "urd"    "hin"    "tha"    "kor"    "lit"
            "pol"    "hun"    "est"    "lav"    "sme" /* 25-29 */
            "fao"    "fas"    "rus"    "zho"    "nld" /* nld==flemish */
            "gle"    "sqi"    "ron"    "ces"    "slk" /* 35-39 */
            "slv"    "yid"    "srp"    "mkd"    "bul"
            "ukr"    "bel"    "uzb"    "kaz"    "aze" /* 45-49 */
            "aze"    "hye"    "kat"    "mol"    "kir"
            "tgk"    "tuk"    "mon"    "mon"    "pus" /* 55-59 */
            "kur"    "kas"    "snd"    "bod"    "nep"
            "san"    "mar"    "ben"    "asm"    "guj" /* 65-69 */
            "pan"    "ori"    "mal"    "kan"    "tam"
            "tel"    "sin"    "mya"    "khm"    "lao" /* 75-79 */
            "vie"    "ind"    "tgl"    "msa"    "msa"
            "amh"    "tir"    "orm"    "som"    "swa" /* 85-89 */
            "kin"    "run"    "nya"    "mlg"    "epo" /* 90-94 */
            ;

    static const char * psz_qt_to_iso639_2T_upper =
            "cym"    "eus"    "cat"    "lat"    "que" /* 128-132 */
            "grn"    "aym"    "tat"    "uig"    "dzo"
            "jaw"    "sun"    "glg"    "afr"    "bre" /* 138-142 */
            "iku"    "gla"    "glv"    "gle"    "ton"
            "gre"                                     /* 148 */
            ;

    if ( i_language_code < 0x400 || i_language_code == 0x7FFF )
    {
        const void *p_data;
        *b_mactables = true;
        if ( i_language_code <= 94 )
        {
            p_data = psz_qt_to_iso639_2T_lower + i_language_code * 3;
        }
        else if ( i_language_code >= 128 && i_language_code <= 148 )
        {
            i_language_code -= 128;
            p_data = psz_qt_to_iso639_2T_upper + i_language_code * 3;
        }
        else
            return false;
        memcpy( psz_iso, p_data, 3 );
    }
    else
    {
        *b_mactables = false;
        /*
         * to build code: ( ( 'f' - 0x60 ) << 10 ) | ( ('r' - 0x60) << 5 ) | ('a' - 0x60)
         */
        if( i_language_code == 0x55C4 ) /* "und" */
        {
            memset( psz_iso, 0, 3 );
            return false;
        }

        for( unsigned i = 0; i < 3; i++ )
            psz_iso[i] = ( ( i_language_code >> ( (2-i)*5 ) )&0x1f ) + 0x60;
    }
    return true;
}

void msg_log(int log_lvl, const char *fmt, ...)
{
    va_list ap;
    char buf[512] = {0};

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    switch (log_lvl) {
    case MSG_DGB:
        printf("debug: %s\n", buf);
        break;
    case MSG_WARN:
        printf("warn: %s\n", buf);
        break;
    case MSG_ERR:
        printf("err: %s\n", buf);
        break;
    default:
        break;
    }
}

static void* file_open(stream_t *stream_s, const char* filename, int mode)
{
    FILE* file = NULL;
    const char* mode_fopen = NULL;
    if ((mode & MODE_READWRITEFILTER) == MODE_READ) {
        mode_fopen = "rb";
    } else if (mode & MODE_EXISTING) {
        mode_fopen = "r+b";
    } else if (mode & MODE_CREATE) {
        mode_fopen = "wb";
    }
    if ((filename != NULL) && (mode_fopen != NULL)) {
        file = fopen(filename, mode_fopen);
    }
    stream_s->opaque = (void*)file;
    return file;
}

static int file_read(stream_t *stream_s, void* buf, int size)
{
    FILE* file = (FILE*)stream_s->opaque;
    return fread(buf, 1, size, file);
}

static int file_write(stream_t *stream_s, void *buf, int size)
{
    FILE* file = (FILE*)stream_s->opaque;
    return fwrite(buf, 1, size, file);
}

static uint64_t file_seek(stream_t *stream_s, int64_t offset, int whence)
{
    FILE* file = (FILE*)stream_s->opaque;
    return fseek(file, offset, whence);
}

static int64_t file_tell(stream_t *stream_s)
{
    FILE* file = (FILE*)stream_s->opaque;
    return ftell(file);
}

static int file_peek(stream_t *stream_s, const uint8_t **buf, int size)
{
    uint32_t offset = file_tell(stream_s);
    *buf = (uint8_t *)calloc(1, size);
    stream_s->priv_buf_num++;
    stream_s->priv_buf = (void **)realloc(stream_s->priv_buf, stream_s->priv_buf_num * sizeof(uint8_t*));
    stream_s->priv_buf[stream_s->priv_buf_num-1] = (void *)*buf;
    int ret = file_read(stream_s, (void *)*buf, size);
    file_seek(stream_s, offset, SEEK_SET);
    return ret;
}

static int64_t file_size(stream_t *stream_s)
{
    FILE* file = (FILE*)stream_s->opaque;
    long size;
    long tmp = ftell(file);
    fseek(file, 0L, SEEK_END);
    size = ftell(file);
    fseek(file, tmp, SEEK_SET);
    return (size_t)size;
}

static int file_close(stream_t *stream_s)
{
    FILE* file = (FILE*)stream_s->opaque;
    return fclose(file);
}

stream_t* create_file_stream()
{
    stream_t* s = (stream_t*)calloc(1, sizeof(stream_t));
    s->open = file_open;
    s->read = file_read;
    s->write = file_write;
    s->peek = file_peek;
    s->seek = file_seek;
    s->tell = file_tell;
    s->size = file_size;
    s->close = file_close;
    s->priv_buf_num = 0;
    s->priv_buf = (void **)calloc(1, sizeof(uint32_t));
    return s;
}

void destory_file_stream(stream_t* stream_s)
{
    for (int i = 0; i < stream_s->priv_buf_num; i++) {
        free(stream_s->priv_buf[i]);
    }
    free(stream_s->priv_buf);
    free(stream_s);
}



// libmp4parser.c
#ifdef HAVE_ZLIB_H
#   include <zlib.h>                                  /* for compressed moov */
#endif

//#include "libmp4parser.h"
//#include "patch.h"

#include <math.h>

/* Some assumptions:
 * The input method HAS to be seekable
 */

/* convert 16.16 fixed point to floating point */
static double conv_fx( int32_t fx ) {
    double fp = fx;
    fp /= 65536.;
    return fp;
}

/* some functions for mp4 encoding of variables */
#ifdef MP4_VERBOSE
static void MP4_ConvertDate2Str( char *psz, uint64_t i_date, bool b_relative )
{
    int i_day;
    int i_hour;
    int i_min;
    int i_sec;

    /* date begin at 1 jan 1904 */
    if ( !b_relative )
        i_date += ((INT64_C(1904) * 365) + 17) * 24 * 60 * 60;

    i_day = i_date / ( 60*60*24);
    i_hour = ( i_date /( 60*60 ) ) % 60;
    i_min  = ( i_date / 60 ) % 60;
    i_sec =  i_date % 60;
    sprintf( psz, "%dd-%2.2dh:%2.2dm:%2.2ds", i_day, i_hour, i_min, i_sec );
}
#endif

/*****************************************************************************
 * Some prototypes.
 *****************************************************************************/
static MP4_Box_t *MP4_ReadBox( stream_t *p_stream, MP4_Box_t *p_father );


/*****************************************************************************
 * MP4_ReadBoxCommon : Load only common parameters for all boxes
 *****************************************************************************
 * p_box need to be an already allocated MP4_Box_t, and all data
 *  will only be peek not read
 *
 * RETURN : 0 if it fail, 1 otherwise
 *****************************************************************************/
int MP4_ReadBoxCommon( stream_t *p_stream, MP4_Box_t *p_box )
{
    int      i_read;
    const uint8_t  *p_peek;

    if( ( ( i_read = stream_Peek( p_stream, &p_peek, 32 ) ) < 8 ) )
    {
        return 0;
    }
    p_box->i_pos = stream_Tell( p_stream );

    p_box->data.p_payload = NULL;
    p_box->p_father = NULL;
    p_box->p_first  = NULL;
    p_box->p_last  = NULL;
    p_box->p_next   = NULL;

    MP4_GET4BYTES( p_box->i_shortsize );
    MP4_GETFOURCC( p_box->i_type );

    /* Now special case */

    if( p_box->i_shortsize == 1 )
    {
        /* get the true size on 64 bits */
        MP4_GET8BYTES( p_box->i_size );
    }
    else
    {
        p_box->i_size = p_box->i_shortsize;
        /* XXX size of 0 means that the box extends to end of file */
    }

    if( p_box->i_type == ATOM_uuid && i_read >= 16 )
    {
        /* get extented type on 16 bytes */
        GetUUID( &p_box->i_uuid, p_peek );
    }
    else
    {
        CreateUUID( &p_box->i_uuid, p_box->i_type );
    }
#ifdef MP4_VERBOSE
    if( p_box->i_size )
    {
        if MP4_BOX_TYPE_ASCII()
            msg_Dbg( p_stream, "found Box: %4.4s size %" PRId64" %" PRId64,
                    (char*)&p_box->i_type, p_box->i_size, p_box->i_pos );
        else
            msg_Dbg( p_stream, "found Box: c%3.3s size %" PRId64,
                    (char*)&p_box->i_type+1, p_box->i_size );
    }
#endif

    return 1;
}

/*****************************************************************************
 * MP4_NextBox : Go to the next box
 *****************************************************************************
 * if p_box == NULL, go to the next box in which we are( at the begining ).
 *****************************************************************************/
static int MP4_NextBox( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_Box_t box;

    if( !p_box )
    {
        MP4_ReadBoxCommon( p_stream, &box );
        p_box = &box;
    }

    if( !p_box->i_size )
    {
        return 2; /* Box with infinite size */
    }

    if( p_box->p_father )
    {
        /* if father's size == 0, it means unknown or infinite size,
         * and we skip the followong check */
        if( p_box->p_father->i_size > 0 )
        {
            const off_t i_box_end = p_box->i_size + p_box->i_pos;
            const off_t i_father_end = p_box->p_father->i_size + p_box->p_father->i_pos;

            /* check if it's within p-father */
            if( i_box_end >= i_father_end )
            {
                if( i_box_end > i_father_end )
                    msg_Dbg( p_stream, "out of bound child" );
                return 0; /* out of bound */
            }
        }
    }
    if( stream_Seek( p_stream, p_box->i_size + p_box->i_pos ) )
    {
        return 0;
    }

    return 1;
}

/*****************************************************************************
 * For all known box a loader is given,
 *  XXX: all common struct have to be already read by MP4_ReadBoxCommon
 *       after called one of theses functions, file position is unknown
 *       you need to call MP4_GotoBox to go where you want
 *****************************************************************************/
int MP4_ReadBoxContainerChildren( stream_t *p_stream,
                                  MP4_Box_t *p_container, uint32_t i_last_child )
{
    MP4_Box_t *p_box;

    /* Size of root container is set to 0 when unknown, for exemple
     * with a DASH stream. In that case, we skip the following check */
    if( p_container->i_size
            && ( stream_Tell( p_stream ) + 8 >
        (off_t)(p_container->i_pos + p_container->i_size) )
      )
    {
        /* there is no box to load */
        return 0;
    }

    do
    {
        if( ( p_box = MP4_ReadBox( p_stream, p_container ) ) == NULL ) break;

        /* chain this box with the father and the other at same level */
        if( !p_container->p_first ) p_container->p_first = p_box;
        else p_container->p_last->p_next = p_box;
        p_container->p_last = p_box;

        if( p_box->i_type == i_last_child )
        {
            MP4_NextBox( p_stream, p_box );
            break;
        }

    } while( MP4_NextBox( p_stream, p_box ) == 1 );

    return 1;
}

static int MP4_ReadBoxContainerRaw( stream_t *p_stream, MP4_Box_t *p_container )
{
    return MP4_ReadBoxContainerChildren( p_stream, p_container, 0 );
}

static int MP4_ReadBoxContainer( stream_t *p_stream, MP4_Box_t *p_container )
{
    if( p_container->i_size &&
        ( p_container->i_size <= (size_t)mp4_box_headersize(p_container ) + 8 ) )
    {
        /* container is empty, 8 stand for the first header in this box */
        return 1;
    }

    /* enter box */
    stream_Seek( p_stream, p_container->i_pos +
                 mp4_box_headersize( p_container ) );

    return MP4_ReadBoxContainerRaw( p_stream, p_container );
}

static void MP4_FreeBox_Common( MP4_Box_t *p_box )
{
    /* Up to now do nothing */
    (void)p_box;
}

static int MP4_ReadBoxSkip( stream_t *p_stream, MP4_Box_t *p_box )
{
    /* XXX sometime moov is hiden in a free box */
    if( p_box->p_father &&
        p_box->p_father->i_type == ATOM_root &&
        p_box->i_type == ATOM_free )
    {
        const uint8_t *p_peek;
        int     i_read;
        vlc_fourcc_t i_fcc;

        i_read  = stream_Peek( p_stream, &p_peek, 44 );

        p_peek += mp4_box_headersize( p_box ) + 4;
        i_read -= mp4_box_headersize( p_box ) + 4;

        if( i_read >= 8 )
        {
            i_fcc = VLC_FOURCC( p_peek[0], p_peek[1], p_peek[2], p_peek[3] );

            if( i_fcc == ATOM_cmov || i_fcc == ATOM_mvhd )
            {
                msg_Warn( p_stream, "detected moov hidden in a free box ..." );

                p_box->i_type = ATOM_foov;
                return MP4_ReadBoxContainer( p_stream, p_box );
            }
        }
    }

    /* Nothing to do */
#ifdef MP4_VERBOSE
    if MP4_BOX_TYPE_ASCII()
        msg_Dbg( p_stream, "skip box: \"%4.4s\"", (char*)&p_box->i_type );
    else
        msg_Dbg( p_stream, "skip box: \"c%3.3s\"", (char*)&p_box->i_type+1 );
#endif
    return 1;
}

static int MP4_ReadBox_ftyp( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_ftyp_t );

    MP4_GETFOURCC( p_box->data.p_ftyp->i_major_brand );
    MP4_GET4BYTES( p_box->data.p_ftyp->i_minor_version );

    if( ( p_box->data.p_ftyp->i_compatible_brands_count = i_read / 4 ) )
    {
        uint32_t *tab = p_box->data.p_ftyp->i_compatible_brands =
            (uint32_t *)calloc( p_box->data.p_ftyp->i_compatible_brands_count,
                    sizeof(uint32_t));

        if( unlikely( tab == NULL ) )
            MP4_READBOX_EXIT( 0 );

        for( unsigned i = 0; i < p_box->data.p_ftyp->i_compatible_brands_count; i++ )
        {
            MP4_GETFOURCC( tab[i] );
        }
    }
    else
    {
        p_box->data.p_ftyp->i_compatible_brands = NULL;
    }

    MP4_READBOX_EXIT( 1 );
}

static void MP4_FreeBox_ftyp( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_ftyp->i_compatible_brands );
}


static int MP4_ReadBox_mvhd(  stream_t *p_stream, MP4_Box_t *p_box )
{
#ifdef MP4_VERBOSE
    char s_creation_time[128];
    char s_modification_time[128];
    char s_duration[128];
#endif
    MP4_READBOX_ENTER( MP4_Box_data_mvhd_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_mvhd );

    if( p_box->data.p_mvhd->i_version )
    {
        MP4_GET8BYTES( p_box->data.p_mvhd->i_creation_time );
        MP4_GET8BYTES( p_box->data.p_mvhd->i_modification_time );
        MP4_GET4BYTES( p_box->data.p_mvhd->i_timescale );
        MP4_GET8BYTES( p_box->data.p_mvhd->i_duration );
    }
    else
    {
        MP4_GET4BYTES( p_box->data.p_mvhd->i_creation_time );
        MP4_GET4BYTES( p_box->data.p_mvhd->i_modification_time );
        MP4_GET4BYTES( p_box->data.p_mvhd->i_timescale );
        MP4_GET4BYTES( p_box->data.p_mvhd->i_duration );
    }
    MP4_GET4BYTES( p_box->data.p_mvhd->i_rate );
    MP4_GET2BYTES( p_box->data.p_mvhd->i_volume );
    MP4_GET2BYTES( p_box->data.p_mvhd->i_reserved1 );


    for( unsigned i = 0; i < 2; i++ )
    {
        MP4_GET4BYTES( p_box->data.p_mvhd->i_reserved2[i] );
    }
    for( unsigned i = 0; i < 9; i++ )
    {
        MP4_GET4BYTES( p_box->data.p_mvhd->i_matrix[i] );
    }
    for( unsigned i = 0; i < 6; i++ )
    {
        MP4_GET4BYTES( p_box->data.p_mvhd->i_predefined[i] );
    }

    MP4_GET4BYTES( p_box->data.p_mvhd->i_next_track_id );


#ifdef MP4_VERBOSE
    MP4_ConvertDate2Str( s_creation_time, p_box->data.p_mvhd->i_creation_time, false );
    MP4_ConvertDate2Str( s_modification_time,
                         p_box->data.p_mvhd->i_modification_time, false );
    if( p_box->data.p_mvhd->i_rate && p_box->data.p_mvhd->i_timescale )
    {
        MP4_ConvertDate2Str( s_duration, p_box->data.p_mvhd->i_duration / p_box->data.p_mvhd->i_timescale, true );
    }
    else
    {
        s_duration[0] = 0;
    }
    msg_Dbg( p_stream, "read box: \"mvhd\" creation %s modification %s time scale %d duration %s(%f) rate %f volume %f next track id %d",
                  s_creation_time,
                  s_modification_time,
                  (uint32_t)p_box->data.p_mvhd->i_timescale,
                  s_duration,
                  (float)(p_box->data.p_mvhd->i_duration*1.0/p_box->data.p_mvhd->i_timescale*1.0),
                  (float)p_box->data.p_mvhd->i_rate / (1<<16 ),
                  (float)p_box->data.p_mvhd->i_volume / 256 ,
                  (uint32_t)p_box->data.p_mvhd->i_next_track_id );
#endif
    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_mfhd(  stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_mfhd_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_mvhd );

    MP4_GET4BYTES( p_box->data.p_mfhd->i_sequence_number );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"mfhd\" sequence number %d",
                  p_box->data.p_mfhd->i_sequence_number );
#endif
    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_tfxd(  stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_tfxd_t );

    MP4_Box_data_tfxd_t *p_tfxd_data = p_box->data.p_tfxd;
    MP4_GETVERSIONFLAGS( p_tfxd_data );

    if( p_tfxd_data->i_version == 0 )
    {
        MP4_GET4BYTES( p_tfxd_data->i_fragment_abs_time );
        MP4_GET4BYTES( p_tfxd_data->i_fragment_duration );
    }
    else
    {
        MP4_GET8BYTES( p_tfxd_data->i_fragment_abs_time );
        MP4_GET8BYTES( p_tfxd_data->i_fragment_duration );
    }

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"tfxd\" version %d, flags 0x%x, "\
            "fragment duration %" PRIu64", fragment abs time %" PRIu64,
                p_tfxd_data->i_version,
                p_tfxd_data->i_flags,
                p_tfxd_data->i_fragment_duration,
                p_tfxd_data->i_fragment_abs_time
           );
#endif

    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_tfrf(  stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_tfxd_t );

    MP4_Box_data_tfrf_t *p_tfrf_data = p_box->data.p_tfrf;
    MP4_GETVERSIONFLAGS( p_tfrf_data );

    MP4_GET1BYTE( p_tfrf_data->i_fragment_count );

    p_tfrf_data->p_tfrf_data_fields = (TfrfBoxDataFields_t *)calloc( p_tfrf_data->i_fragment_count,
                                              sizeof( TfrfBoxDataFields_t ) );
    if( !p_tfrf_data->p_tfrf_data_fields )
        MP4_READBOX_EXIT( 0 );

    for( uint8_t i = 0; i < p_tfrf_data->i_fragment_count; i++ )
    {
        TfrfBoxDataFields_t *TfrfBoxDataField = &p_tfrf_data->p_tfrf_data_fields[i];
        if( p_tfrf_data->i_version == 0 )
        {
            MP4_GET4BYTES( TfrfBoxDataField->i_fragment_abs_time );
            MP4_GET4BYTES( TfrfBoxDataField->i_fragment_duration );
        }
        else
        {
            MP4_GET8BYTES( TfrfBoxDataField->i_fragment_abs_time );
            MP4_GET8BYTES( TfrfBoxDataField->i_fragment_duration );
        }
    }

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"tfrf\" version %d, flags 0x%x, "\
            "fragment count %" PRIu8, p_tfrf_data->i_version,
                p_tfrf_data->i_flags, p_tfrf_data->i_fragment_count );

    for( uint8_t i = 0; i < p_tfrf_data->i_fragment_count; i++ )
    {
        TfrfBoxDataFields_t *TfrfBoxDataField = &p_tfrf_data->p_tfrf_data_fields[i];
        msg_Dbg( p_stream, "\"tfrf\" fragment duration %" PRIu64", "\
                                    "fragment abs time %" PRIu64,
                    TfrfBoxDataField->i_fragment_duration,
                    TfrfBoxDataField->i_fragment_abs_time );
    }

#endif

    MP4_READBOX_EXIT( 1 );
}

static void MP4_FreeBox_tfrf( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_tfrf->p_tfrf_data_fields );
}

static int MP4_ReadBox_stra( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_stra_t );
    MP4_Box_data_stra_t *p_stra = p_box->data.p_stra;

    uint8_t i_reserved;
    VLC_UNUSED(i_reserved);
    MP4_GET1BYTE( p_stra->i_es_cat );
    MP4_GET1BYTE( i_reserved );
    MP4_GET2BYTES( p_stra->i_track_ID );

    MP4_GET4BYTES( p_stra->i_timescale );
    MP4_GET8BYTES( p_stra->i_duration );

    MP4_GET4BYTES( p_stra->FourCC );
    MP4_GET4BYTES( p_stra->Bitrate );
    MP4_GET4BYTES( p_stra->MaxWidth );
    MP4_GET4BYTES( p_stra->MaxHeight );
    MP4_GET4BYTES( p_stra->SamplingRate );
    MP4_GET4BYTES( p_stra->Channels );
    MP4_GET4BYTES( p_stra->BitsPerSample );
    MP4_GET4BYTES( p_stra->AudioTag );
    MP4_GET2BYTES( p_stra->nBlockAlign );

    MP4_GET1BYTE( i_reserved );
    MP4_GET1BYTE( i_reserved );
    MP4_GET1BYTE( i_reserved );
    MP4_GET1BYTE( p_stra->cpd_len );
    if( p_stra->cpd_len > i_read )
        goto error;
    p_stra->CodecPrivateData = (uint8_t *)malloc( p_stra->cpd_len );
    if( unlikely( p_stra->CodecPrivateData == NULL ) )
        goto error;
    memcpy( p_stra->CodecPrivateData, p_peek, p_stra->cpd_len );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "es_cat is %" PRIu8", birate is %" PRIu32,
              p_stra->i_es_cat, p_stra->Bitrate );
#endif

    MP4_READBOX_EXIT( 1 );
error:
    MP4_READBOX_EXIT( 0 );
}

static void MP4_FreeBox_stra( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_stra->CodecPrivateData );
}

static int MP4_ReadBox_uuid( stream_t *p_stream, MP4_Box_t *p_box )
{
    if( !CmpUUID( &p_box->i_uuid, &TfrfBoxUUID ) )
        return MP4_ReadBox_tfrf( p_stream, p_box );
    if( !CmpUUID( &p_box->i_uuid, &TfxdBoxUUID ) )
        return MP4_ReadBox_tfxd( p_stream, p_box );
    if( !CmpUUID( &p_box->i_uuid, &SmooBoxUUID ) )
        return MP4_ReadBoxContainer( p_stream, p_box );
    if( !CmpUUID( &p_box->i_uuid, &StraBoxUUID ) )
        return MP4_ReadBox_stra( p_stream, p_box );

    msg_Warn( p_stream, "Unknown uuid type box" );
    return 1;
}

static void MP4_FreeBox_uuid( MP4_Box_t *p_box )
{
    if( !CmpUUID( &p_box->i_uuid, &TfrfBoxUUID ) )
        return MP4_FreeBox_tfrf( p_box );
    if( !CmpUUID( &p_box->i_uuid, &TfxdBoxUUID ) )
        return MP4_FreeBox_Common( p_box );
    if( !CmpUUID( &p_box->i_uuid, &SmooBoxUUID ) )
        return MP4_FreeBox_Common( p_box );
    if( !CmpUUID( &p_box->i_uuid, &StraBoxUUID ) )
        return MP4_FreeBox_stra( p_box );
}

static int MP4_ReadBox_sidx(  stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_sidx_t );

    MP4_Box_data_sidx_t *p_sidx_data = p_box->data.p_sidx;
    MP4_GETVERSIONFLAGS( p_sidx_data );

    MP4_GET4BYTES( p_sidx_data->i_reference_ID );
    MP4_GET4BYTES( p_sidx_data->i_timescale );

    if( p_sidx_data->i_version == 0 )
    {
        MP4_GET4BYTES( p_sidx_data->i_earliest_presentation_time );
        MP4_GET4BYTES( p_sidx_data->i_first_offset );
    }
    else
    {
        MP4_GET8BYTES( p_sidx_data->i_earliest_presentation_time );
        MP4_GET8BYTES( p_sidx_data->i_first_offset );
    }

    uint16_t i_reserved;
    VLC_UNUSED(i_reserved);
    MP4_GET2BYTES( i_reserved );
    MP4_GET2BYTES( p_sidx_data->i_reference_count );
    uint16_t i_count = p_sidx_data->i_reference_count;

    p_sidx_data->p_items = (MP4_Box_sidx_item_t *)calloc( i_count, sizeof( MP4_Box_sidx_item_t ) );
    uint32_t tmp;
    for( unsigned i = 0; i < i_count; i++ )
    {
        MP4_GET4BYTES( tmp );
        p_sidx_data->p_items[i].b_reference_type = (bool)((tmp & 0x80000000)>>24);
        p_sidx_data->p_items[i].i_referenced_size = tmp & 0x7fffffff;
        MP4_GET4BYTES( p_sidx_data->p_items[i].i_subsegment_duration );

        MP4_GET4BYTES( tmp );
        p_sidx_data->p_items[i].b_starts_with_SAP = (bool)((tmp & 0x80000000)>>24);
        p_sidx_data->p_items[i].i_SAP_type = (tmp & 0x70000000)>>24;
        p_sidx_data->p_items[i].i_SAP_delta_time = tmp & 0xfffffff;
    }

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"sidx\" version %d, flags 0x%x, "\
            "ref_ID %" PRIu32", timescale %" PRIu32", ref_count %" PRIu16", "\
            "first subsegmt duration %" PRIu32,
                p_sidx_data->i_version,
                p_sidx_data->i_flags,
                p_sidx_data->i_reference_ID,
                p_sidx_data->i_timescale,
                p_sidx_data->i_reference_count,
                p_sidx_data->p_items[0].i_subsegment_duration
           );
#endif

    MP4_READBOX_EXIT( 1 );
}

static void MP4_FreeBox_sidx( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_sidx->p_items );
}

static int MP4_ReadBox_tfhd(  stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_tfhd_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_tfhd );

    if( p_box->data.p_tfhd->i_version != 0 )
    {
        msg_Warn( p_stream, "'tfhd' box with version != 0. "\
                " Don't know what to do with that, please patch" );
        MP4_READBOX_EXIT( 0 );
    }

    MP4_GET4BYTES( p_box->data.p_tfhd->i_track_ID );

    if( p_box->data.p_tfhd->i_flags & MP4_TFHD_DURATION_IS_EMPTY )
    {
        msg_Dbg( p_stream, "'duration-is-empty' flag is present "\
                "=> no samples for this time interval." );
        p_box->data.p_tfhd->b_empty = true;
    }
    else
        p_box->data.p_tfhd->b_empty = false;

    if( p_box->data.p_tfhd->i_flags & MP4_TFHD_BASE_DATA_OFFSET )
        MP4_GET8BYTES( p_box->data.p_tfhd->i_base_data_offset );
    if( p_box->data.p_tfhd->i_flags & MP4_TFHD_SAMPLE_DESC_INDEX )
        MP4_GET4BYTES( p_box->data.p_tfhd->i_sample_description_index );
    if( p_box->data.p_tfhd->i_flags & MP4_TFHD_DFLT_SAMPLE_DURATION )
        MP4_GET4BYTES( p_box->data.p_tfhd->i_default_sample_duration );
    if( p_box->data.p_tfhd->i_flags & MP4_TFHD_DFLT_SAMPLE_SIZE )
        MP4_GET4BYTES( p_box->data.p_tfhd->i_default_sample_size );
    if( p_box->data.p_tfhd->i_flags & MP4_TFHD_DFLT_SAMPLE_FLAGS )
        MP4_GET4BYTES( p_box->data.p_tfhd->i_default_sample_flags );

#ifdef MP4_VERBOSE
    char psz_base[128] = "\0";
    char psz_desc[128] = "\0";
    char psz_dura[128] = "\0";
    char psz_size[128] = "\0";
    char psz_flag[128] = "\0";
    if( p_box->data.p_tfhd->i_flags & MP4_TFHD_BASE_DATA_OFFSET )
        snprintf(psz_base, sizeof(psz_base), "base offset %" PRId64, p_box->data.p_tfhd->i_base_data_offset);
    if( p_box->data.p_tfhd->i_flags & MP4_TFHD_SAMPLE_DESC_INDEX )
        snprintf(psz_desc, sizeof(psz_desc), "sample description index %d", p_box->data.p_tfhd->i_sample_description_index);
    if( p_box->data.p_tfhd->i_flags & MP4_TFHD_DFLT_SAMPLE_DURATION )
        snprintf(psz_dura, sizeof(psz_dura), "sample duration %d", p_box->data.p_tfhd->i_default_sample_duration);
    if( p_box->data.p_tfhd->i_flags & MP4_TFHD_DFLT_SAMPLE_SIZE )
        snprintf(psz_size, sizeof(psz_size), "sample size %d", p_box->data.p_tfhd->i_default_sample_size);
    if( p_box->data.p_tfhd->i_flags & MP4_TFHD_DFLT_SAMPLE_FLAGS )
        snprintf(psz_flag, sizeof(psz_flag), "sample flags 0x%x", p_box->data.p_tfhd->i_default_sample_flags);

    msg_Dbg( p_stream, "read box: \"tfhd\" version %d flags 0x%x track ID %d %s %s %s %s %s",
                p_box->data.p_tfhd->i_version,
                p_box->data.p_tfhd->i_flags,
                p_box->data.p_tfhd->i_track_ID,
                psz_base, psz_desc, psz_dura, psz_size, psz_flag );
#endif

    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_trun(  stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_trun_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_trun );

    MP4_GET4BYTES( p_box->data.p_trun->i_sample_count );

    if( p_box->data.p_trun->i_flags & MP4_TRUN_DATA_OFFSET )
        MP4_GET4BYTES( p_box->data.p_trun->i_data_offset );
    if( p_box->data.p_trun->i_flags & MP4_TRUN_FIRST_FLAGS )
        MP4_GET4BYTES( p_box->data.p_trun->i_first_sample_flags );

    p_box->data.p_trun->p_samples =
      (MP4_descriptor_trun_sample_t *)calloc( p_box->data.p_trun->i_sample_count, sizeof(MP4_descriptor_trun_sample_t) );
    if ( p_box->data.p_trun->p_samples == NULL )
        MP4_READBOX_EXIT( 0 );

    for( unsigned int i = 0; i<p_box->data.p_trun->i_sample_count; i++ )
    {
        MP4_descriptor_trun_sample_t *p_sample = &p_box->data.p_trun->p_samples[i];
        if( p_box->data.p_trun->i_flags & MP4_TRUN_SAMPLE_DURATION )
            MP4_GET4BYTES( p_sample->i_duration );
        if( p_box->data.p_trun->i_flags & MP4_TRUN_SAMPLE_SIZE )
            MP4_GET4BYTES( p_sample->i_size );
        if( p_box->data.p_trun->i_flags & MP4_TRUN_SAMPLE_FLAGS )
            MP4_GET4BYTES( p_sample->i_flags );
        if( p_box->data.p_trun->i_flags & MP4_TRUN_SAMPLE_TIME_OFFSET )
            MP4_GET4BYTES( p_sample->i_composition_time_offset );
    }

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"trun\" version %u flags 0x%x sample count %u",
                  p_box->data.p_trun->i_version,
                  p_box->data.p_trun->i_flags,
                  p_box->data.p_trun->i_sample_count );

    for( unsigned int i = 0; i<p_box->data.p_trun->i_sample_count; i++ )
    {
        MP4_descriptor_trun_sample_t *p_sample = &p_box->data.p_trun->p_samples[i];
        msg_Dbg( p_stream, "read box: \"trun\" sample %4.4u flags 0x%x "\
            "duration %" PRIu32" size %" PRIu32" composition time offset %" PRIu32,
                        i, p_sample->i_flags, p_sample->i_duration,
                        p_sample->i_size, p_sample->i_composition_time_offset );
    }
#endif

    MP4_READBOX_EXIT( 1 );
}

static void MP4_FreeBox_trun( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_trun->p_samples );
}


static int MP4_ReadBox_tkhd(  stream_t *p_stream, MP4_Box_t *p_box )
{
#ifdef MP4_VERBOSE
    char s_creation_time[128];
    char s_modification_time[128];
    char s_duration[128];
#endif
    MP4_READBOX_ENTER( MP4_Box_data_tkhd_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_tkhd );

    if( p_box->data.p_tkhd->i_version )
    {
        MP4_GET8BYTES( p_box->data.p_tkhd->i_creation_time );
        MP4_GET8BYTES( p_box->data.p_tkhd->i_modification_time );
        MP4_GET4BYTES( p_box->data.p_tkhd->i_track_ID );
        MP4_GET4BYTES( p_box->data.p_tkhd->i_reserved );
        MP4_GET8BYTES( p_box->data.p_tkhd->i_duration );
    }
    else
    {
        MP4_GET4BYTES( p_box->data.p_tkhd->i_creation_time );
        MP4_GET4BYTES( p_box->data.p_tkhd->i_modification_time );
        MP4_GET4BYTES( p_box->data.p_tkhd->i_track_ID );
        MP4_GET4BYTES( p_box->data.p_tkhd->i_reserved );
        MP4_GET4BYTES( p_box->data.p_tkhd->i_duration );
    }

    for( unsigned i = 0; i < 2; i++ )
    {
        MP4_GET4BYTES( p_box->data.p_tkhd->i_reserved2[i] );
    }
    MP4_GET2BYTES( p_box->data.p_tkhd->i_layer );
    MP4_GET2BYTES( p_box->data.p_tkhd->i_predefined );
    MP4_GET2BYTES( p_box->data.p_tkhd->i_volume );
    MP4_GET2BYTES( p_box->data.p_tkhd->i_reserved3 );

    for( unsigned i = 0; i < 9; i++ )
    {
        MP4_GET4BYTES( p_box->data.p_tkhd->i_matrix[i] );
    }
    MP4_GET4BYTES( p_box->data.p_tkhd->i_width );
    MP4_GET4BYTES( p_box->data.p_tkhd->i_height );

    double rotation;    //angle in degrees to be rotated clockwise
    double scale[2];    // scale factor; sx = scale[0] , sy = scale[1]
    double translate[2];// amount to translate; tx = translate[0] , ty = translate[1]

    int32_t *matrix = p_box->data.p_tkhd->i_matrix;

    translate[0] = conv_fx(matrix[6]);
    translate[1] = conv_fx(matrix[7]);
    translate[1] = translate[1];

    scale[0] = sqrt(conv_fx(matrix[0]) * conv_fx(matrix[0]) +
                    conv_fx(matrix[3]) * conv_fx(matrix[3]));
    scale[1] = sqrt(conv_fx(matrix[1]) * conv_fx(matrix[1]) +
                    conv_fx(matrix[4]) * conv_fx(matrix[4]));

    rotation = atan2(conv_fx(matrix[1]) / scale[1], conv_fx(matrix[0]) / scale[0]) * 180 / M_PI;

    if (rotation < 0)
        rotation += 360.;

    p_box->data.p_tkhd->f_rotation = rotation;

#ifdef MP4_VERBOSE
    MP4_ConvertDate2Str( s_creation_time, p_box->data.p_mvhd->i_creation_time, false );
    MP4_ConvertDate2Str( s_modification_time, p_box->data.p_mvhd->i_modification_time, false );
    MP4_ConvertDate2Str( s_duration, p_box->data.p_mvhd->i_duration, true );

    msg_Dbg( p_stream, "read box: \"tkhd\" creation %s modification %s duration %s track ID %d layer %d volume %f rotation %f scaleX %f scaleY %f translateX %f translateY %f width %f height %f. "
            "Matrix: %i %i %i %i %i %i %i %i %i",
                  s_creation_time,
                  s_modification_time,
                  s_duration,
                  p_box->data.p_tkhd->i_track_ID,
                  p_box->data.p_tkhd->i_layer,
                  (float)p_box->data.p_tkhd->i_volume / 256 ,
                  rotation,
                  scale[0],
                  scale[1],
                  translate[0],
                  translate[1],
                  (float)p_box->data.p_tkhd->i_width / BLOCK16x16,
                  (float)p_box->data.p_tkhd->i_height / BLOCK16x16,
                  p_box->data.p_tkhd->i_matrix[0],
                  p_box->data.p_tkhd->i_matrix[1],
                  p_box->data.p_tkhd->i_matrix[2],
                  p_box->data.p_tkhd->i_matrix[3],
                  p_box->data.p_tkhd->i_matrix[4],
                  p_box->data.p_tkhd->i_matrix[5],
                  p_box->data.p_tkhd->i_matrix[6],
                  p_box->data.p_tkhd->i_matrix[7],
                  p_box->data.p_tkhd->i_matrix[8] );
#endif
    MP4_READBOX_EXIT( 1 );
}


static int MP4_ReadBox_mdhd( stream_t *p_stream, MP4_Box_t *p_box )
{
    uint16_t i_language;
#ifdef MP4_VERBOSE
    char s_creation_time[128];
    char s_modification_time[128];
    char s_duration[128];
#endif
    MP4_READBOX_ENTER( MP4_Box_data_mdhd_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_mdhd );

    if( p_box->data.p_mdhd->i_version )
    {
        MP4_GET8BYTES( p_box->data.p_mdhd->i_creation_time );
        MP4_GET8BYTES( p_box->data.p_mdhd->i_modification_time );
        MP4_GET4BYTES( p_box->data.p_mdhd->i_timescale );
        MP4_GET8BYTES( p_box->data.p_mdhd->i_duration );
    }
    else
    {
        MP4_GET4BYTES( p_box->data.p_mdhd->i_creation_time );
        MP4_GET4BYTES( p_box->data.p_mdhd->i_modification_time );
        MP4_GET4BYTES( p_box->data.p_mdhd->i_timescale );
        MP4_GET4BYTES( p_box->data.p_mdhd->i_duration );
    }

    MP4_GET2BYTES( i_language );
    decodeQtLanguageCode( i_language, p_box->data.p_mdhd->rgs_language,
                          &p_box->data.p_mdhd->b_mac_encoding );

    MP4_GET2BYTES( p_box->data.p_mdhd->i_predefined );

#ifdef MP4_VERBOSE
    MP4_ConvertDate2Str( s_creation_time, p_box->data.p_mdhd->i_creation_time, false );
    MP4_ConvertDate2Str( s_modification_time, p_box->data.p_mdhd->i_modification_time, false );
    MP4_ConvertDate2Str( s_duration, p_box->data.p_mdhd->i_duration, true );
    msg_Dbg( p_stream, "read box: \"mdhd\" creation %s modification %s time scale %d duration %s language %3.3s",
                  s_creation_time,
                  s_modification_time,
                  (uint32_t)p_box->data.p_mdhd->i_timescale,
                  s_duration,
                  (char*) &p_box->data.p_mdhd->rgs_language );
#endif
    MP4_READBOX_EXIT( 1 );
}


static int MP4_ReadBox_hdlr( stream_t *p_stream, MP4_Box_t *p_box )
{
    int32_t i_reserved;
    VLC_UNUSED(i_reserved);

    MP4_READBOX_ENTER( MP4_Box_data_hdlr_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_hdlr );

    MP4_GETFOURCC( p_box->data.p_hdlr->i_predefined );
    MP4_GETFOURCC( p_box->data.p_hdlr->i_handler_type );

    MP4_GET4BYTES( i_reserved );
    MP4_GET4BYTES( i_reserved );
    MP4_GET4BYTES( i_reserved );
    p_box->data.p_hdlr->psz_name = NULL;

    if( i_read > 0 )
    {
        uint8_t *psz = p_box->data.p_hdlr->psz_name = (unsigned char *)malloc( i_read + 1 );
        if( unlikely( psz == NULL ) )
            MP4_READBOX_EXIT( 0 );

        /* Yes, I love .mp4 :( */
        if( p_box->data.p_hdlr->i_predefined == VLC_FOURCC( 'm', 'h', 'l', 'r' ) )
        {
            uint8_t i_len;
            int i_copy;

            MP4_GET1BYTE( i_len );
            i_copy = __MIN( i_read, i_len );

            memcpy( psz, p_peek, i_copy );
            p_box->data.p_hdlr->psz_name[i_copy] = '\0';
        }
        else
        {
            memcpy( psz, p_peek, i_read );
            p_box->data.p_hdlr->psz_name[i_read] = '\0';
        }
    }

#ifdef MP4_VERBOSE
        msg_Dbg( p_stream, "read box: \"hdlr\" handler type: \"%4.4s\" name: \"%s\"",
                   (char*)&p_box->data.p_hdlr->i_handler_type,
                   p_box->data.p_hdlr->psz_name );

#endif
    MP4_READBOX_EXIT( 1 );
}

static void MP4_FreeBox_hdlr( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_hdlr->psz_name );
}

static int MP4_ReadBox_vmhd( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_vmhd_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_vmhd );

    MP4_GET2BYTES( p_box->data.p_vmhd->i_graphics_mode );
    for( unsigned i = 0; i < 3; i++ )
    {
        MP4_GET2BYTES( p_box->data.p_vmhd->i_opcolor[i] );
    }

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"vmhd\" graphics-mode %d opcolor (%d, %d, %d)",
                      p_box->data.p_vmhd->i_graphics_mode,
                      p_box->data.p_vmhd->i_opcolor[0],
                      p_box->data.p_vmhd->i_opcolor[1],
                      p_box->data.p_vmhd->i_opcolor[2] );
#endif
    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_smhd( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_smhd_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_smhd );



    MP4_GET2BYTES( p_box->data.p_smhd->i_balance );

    MP4_GET2BYTES( p_box->data.p_smhd->i_reserved );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"smhd\" balance %f",
                      (float)p_box->data.p_smhd->i_balance / 256 );
#endif
    MP4_READBOX_EXIT( 1 );
}


static int MP4_ReadBox_hmhd( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_hmhd_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_hmhd );

    MP4_GET2BYTES( p_box->data.p_hmhd->i_max_PDU_size );
    MP4_GET2BYTES( p_box->data.p_hmhd->i_avg_PDU_size );

    MP4_GET4BYTES( p_box->data.p_hmhd->i_max_bitrate );
    MP4_GET4BYTES( p_box->data.p_hmhd->i_avg_bitrate );

    MP4_GET4BYTES( p_box->data.p_hmhd->i_reserved );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"hmhd\" maxPDU-size %d avgPDU-size %d max-bitrate %d avg-bitrate %d",
                      p_box->data.p_hmhd->i_max_PDU_size,
                      p_box->data.p_hmhd->i_avg_PDU_size,
                      p_box->data.p_hmhd->i_max_bitrate,
                      p_box->data.p_hmhd->i_avg_bitrate );
#endif
    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_url( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_url_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_url );
    MP4_GETSTRINGZ( p_box->data.p_url->psz_location );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"url\" url: %s",
                       p_box->data.p_url->psz_location );

#endif
    MP4_READBOX_EXIT( 1 );
}


static void MP4_FreeBox_url( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_url->psz_location );
}

static int MP4_ReadBox_urn( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_urn_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_urn );

    MP4_GETSTRINGZ( p_box->data.p_urn->psz_name );
    MP4_GETSTRINGZ( p_box->data.p_urn->psz_location );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"urn\" name %s location %s",
                      p_box->data.p_urn->psz_name,
                      p_box->data.p_urn->psz_location );
#endif
    MP4_READBOX_EXIT( 1 );
}
static void MP4_FreeBox_urn( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_urn->psz_name );
    FREENULL( p_box->data.p_urn->psz_location );
}


static int MP4_ReadBox_dref( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_dref_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_dref );

    MP4_GET4BYTES( p_box->data.p_dref->i_entry_count );

    stream_Seek( p_stream, p_box->i_pos + mp4_box_headersize( p_box ) + 8 );
    MP4_ReadBoxContainerRaw( p_stream, p_box );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"dref\" entry-count %d",
                      p_box->data.p_dref->i_entry_count );

#endif
    MP4_READBOX_EXIT( 1 );
}

static void MP4_FreeBox_stts( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_stts->pi_sample_count );
    FREENULL( p_box->data.p_stts->pi_sample_delta );
}

static int MP4_ReadBox_stts( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_stts_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_stts );
    MP4_GET4BYTES( p_box->data.p_stts->i_entry_count );

    p_box->data.p_stts->pi_sample_count =
        (uint32_t *)calloc( p_box->data.p_stts->i_entry_count, sizeof(uint32_t) );
    p_box->data.p_stts->pi_sample_delta =
        (int32_t *)calloc( p_box->data.p_stts->i_entry_count, sizeof(int32_t) );
    if( p_box->data.p_stts->pi_sample_count == NULL
     || p_box->data.p_stts->pi_sample_delta == NULL )
    {
        MP4_READBOX_EXIT( 0 );
    }

    uint32_t i = 0;
    for( ; (i < p_box->data.p_stts->i_entry_count )&&( i_read >=8 ); i++ )
    {
        MP4_GET4BYTES( p_box->data.p_stts->pi_sample_count[i] );
        MP4_GET4BYTES( p_box->data.p_stts->pi_sample_delta[i] );
    }

    if ( i < p_box->data.p_stts->i_entry_count )
        p_box->data.p_stts->i_entry_count = i;

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"stts\" entry-count %d",
                      p_box->data.p_stts->i_entry_count );

#endif
    MP4_READBOX_EXIT( 1 );
}


static void MP4_FreeBox_ctts( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_ctts->pi_sample_count );
    FREENULL( p_box->data.p_ctts->pi_sample_offset );
}

static int MP4_ReadBox_ctts( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_ctts_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_ctts );

    MP4_GET4BYTES( p_box->data.p_ctts->i_entry_count );

    p_box->data.p_ctts->pi_sample_count =
        (uint32_t *)calloc( p_box->data.p_ctts->i_entry_count, sizeof(uint32_t) );
    p_box->data.p_ctts->pi_sample_offset =
        (int32_t *)calloc( p_box->data.p_ctts->i_entry_count, sizeof(int32_t) );
    if( ( p_box->data.p_ctts->pi_sample_count == NULL )
     || ( p_box->data.p_ctts->pi_sample_offset == NULL ) )
    {
        MP4_READBOX_EXIT( 0 );
    }

    uint32_t i = 0;
    for( ; (i < p_box->data.p_ctts->i_entry_count )&&( i_read >=8 ); i++ )
    {
        MP4_GET4BYTES( p_box->data.p_ctts->pi_sample_count[i] );
        MP4_GET4BYTES( p_box->data.p_ctts->pi_sample_offset[i] );
    }
    if ( i < p_box->data.p_ctts->i_entry_count )
        p_box->data.p_ctts->i_entry_count = i;

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"ctts\" entry-count %d",
                      p_box->data.p_ctts->i_entry_count );

#endif
    MP4_READBOX_EXIT( 1 );
}


static int MP4_ReadLengthDescriptor( uint8_t **pp_peek, int64_t  *i_read )
{
    unsigned int i_b;
    unsigned int i_len = 0;
    do
    {
        i_b = **pp_peek;

        (*pp_peek)++;
        (*i_read)--;
        i_len = ( i_len << 7 ) + ( i_b&0x7f );
    } while( i_b&0x80 );
    return( i_len );
}


static void MP4_FreeBox_esds( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_esds->es_descriptor.psz_URL );
    if( p_box->data.p_esds->es_descriptor.p_decConfigDescr )
    {
        FREENULL( p_box->data.p_esds->es_descriptor.p_decConfigDescr->p_decoder_specific_info );
        FREENULL( p_box->data.p_esds->es_descriptor.p_decConfigDescr );
    }
}

static int MP4_ReadBox_esds( stream_t *p_stream, MP4_Box_t *p_box )
{
#define es_descriptor p_box->data.p_esds->es_descriptor
    unsigned int i_len;
    unsigned int i_flags;
    unsigned int i_type;

    MP4_READBOX_ENTER( MP4_Box_data_esds_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_esds );


    MP4_GET1BYTE( i_type );
    if( i_type == 0x03 ) /* MP4ESDescrTag */
    {
        i_len = MP4_ReadLengthDescriptor( &p_peek, &i_read );

#ifdef MP4_VERBOSE
        msg_Dbg( p_stream, "found esds MPEG4ESDescr (%dBytes)",
                 i_len );
#endif

        MP4_GET2BYTES( es_descriptor.i_ES_ID );
        MP4_GET1BYTE( i_flags );
        es_descriptor.b_stream_dependence = ( (i_flags&0x80) != 0);
        es_descriptor.b_url = ( (i_flags&0x40) != 0);
        es_descriptor.b_OCRstream = ( (i_flags&0x20) != 0);

        es_descriptor.i_stream_priority = i_flags&0x1f;
        if( es_descriptor.b_stream_dependence )
        {
            MP4_GET2BYTES( es_descriptor.i_depend_on_ES_ID );
        }
        if( es_descriptor.b_url )
        {
            unsigned int i_len;

            MP4_GET1BYTE( i_len );
            i_len = __MIN(i_read, i_len);
            es_descriptor.psz_URL = (unsigned char *)malloc( i_len + 1 );
            if( es_descriptor.psz_URL )
            {
                memcpy( es_descriptor.psz_URL, p_peek, i_len );
                es_descriptor.psz_URL[i_len] = 0;
            }
            p_peek += i_len;
            i_read -= i_len;
        }
        else
        {
            es_descriptor.psz_URL = NULL;
        }
        if( es_descriptor.b_OCRstream )
        {
            MP4_GET2BYTES( es_descriptor.i_OCR_ES_ID );
        }
        MP4_GET1BYTE( i_type ); /* get next type */
    }

    if( i_type != 0x04)/* MP4DecConfigDescrTag */
    {
         es_descriptor.p_decConfigDescr = NULL;
         MP4_READBOX_EXIT( 1 ); /* rest isn't interesting up to now */
    }

    i_len = MP4_ReadLengthDescriptor( &p_peek, &i_read );

#ifdef MP4_VERBOSE
        msg_Dbg( p_stream, "found esds MP4DecConfigDescr (%dBytes)",
                 i_len );
#endif

    es_descriptor.p_decConfigDescr =
            (MP4_descriptor_decoder_config_t *)calloc( 1, sizeof( MP4_descriptor_decoder_config_t ));
    if( unlikely( es_descriptor.p_decConfigDescr == NULL ) )
        MP4_READBOX_EXIT( 0 );

    MP4_GET1BYTE( es_descriptor.p_decConfigDescr->i_objectTypeIndication );
    MP4_GET1BYTE( i_flags );
    es_descriptor.p_decConfigDescr->i_streamType = i_flags >> 2;
    es_descriptor.p_decConfigDescr->b_upStream = ( i_flags >> 1 )&0x01;
    MP4_GET3BYTES( es_descriptor.p_decConfigDescr->i_buffer_sizeDB );
    MP4_GET4BYTES( es_descriptor.p_decConfigDescr->i_max_bitrate );
    MP4_GET4BYTES( es_descriptor.p_decConfigDescr->i_avg_bitrate );
    MP4_GET1BYTE( i_type );
    if( i_type !=  0x05 )/* MP4DecSpecificDescrTag */
    {
        es_descriptor.p_decConfigDescr->i_decoder_specific_info_len = 0;
        es_descriptor.p_decConfigDescr->p_decoder_specific_info  = NULL;
        MP4_READBOX_EXIT( 1 );
    }

    i_len = MP4_ReadLengthDescriptor( &p_peek, &i_read );

#ifdef MP4_VERBOSE
        msg_Dbg( p_stream, "found esds MP4DecSpecificDescr (%dBytes)",
                 i_len );
#endif
    if( i_len > i_read )
        MP4_READBOX_EXIT( 0 );

    es_descriptor.p_decConfigDescr->i_decoder_specific_info_len = i_len;
    es_descriptor.p_decConfigDescr->p_decoder_specific_info = (uint8_t *)malloc( i_len );
    if( unlikely( es_descriptor.p_decConfigDescr->p_decoder_specific_info == NULL ) )
        MP4_READBOX_EXIT( 0 );

    memcpy( es_descriptor.p_decConfigDescr->p_decoder_specific_info,
            p_peek, i_len );

    MP4_READBOX_EXIT( 1 );
#undef es_descriptor
}

static void MP4_FreeBox_hvcC(MP4_Box_t *p_box )
{
    MP4_Box_data_hvcC_t *p_hvcC =  p_box->data.p_hvcC;
    if( p_hvcC->i_hvcC > 0 ) FREENULL( p_hvcC->p_hvcC) ;
}

static int MP4_ReadBox_hvcC( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_Box_data_hvcC_t *p_hvcC;

    MP4_READBOX_ENTER( MP4_Box_data_hvcC_t );
    p_hvcC = p_box->data.p_hvcC;

    p_hvcC->i_hvcC = i_read;
    if( p_hvcC->i_hvcC > 0 )
    {
        uint8_t * p = p_hvcC->p_hvcC = (uint8_t *)malloc( p_hvcC->i_hvcC );
        if( p )
            memcpy( p, p_peek, i_read );
    }
    MP4_READBOX_EXIT( 1 );
}

static void MP4_FreeBox_avcC( MP4_Box_t *p_box )
{
    MP4_Box_data_avcC_t *p_avcC = p_box->data.p_avcC;
    int i;

    if( p_avcC->i_avcC > 0 ) FREENULL( p_avcC->p_avcC );

    if( p_avcC->sps )
    {
        for( i = 0; i < p_avcC->i_sps; i++ )
            FREENULL( p_avcC->sps[i] );
    }
    if( p_avcC->pps )
    {
        for( i = 0; i < p_avcC->i_pps; i++ )
            FREENULL( p_avcC->pps[i] );
    }
    if( p_avcC->i_sps > 0 ) FREENULL( p_avcC->sps );
    if( p_avcC->i_sps > 0 ) FREENULL( p_avcC->i_sps_length );
    if( p_avcC->i_pps > 0 ) FREENULL( p_avcC->pps );
    if( p_avcC->i_pps > 0 ) FREENULL( p_avcC->i_pps_length );
}

static int MP4_ReadBox_avcC( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_Box_data_avcC_t *p_avcC;
    int i;

    MP4_READBOX_ENTER( MP4_Box_data_avcC_t );
    p_avcC = p_box->data.p_avcC;

    p_avcC->i_avcC = i_read;
    if( p_avcC->i_avcC > 0 )
    {
        uint8_t * p = p_avcC->p_avcC = (uint8_t *)malloc( p_avcC->i_avcC );
        if( p )
            memcpy( p, p_peek, i_read );
    }

    MP4_GET1BYTE( p_avcC->i_version );
    MP4_GET1BYTE( p_avcC->i_profile );
    MP4_GET1BYTE( p_avcC->i_profile_compatibility );
    MP4_GET1BYTE( p_avcC->i_level );
    MP4_GET1BYTE( p_avcC->i_reserved1 );
    p_avcC->i_length_size = (p_avcC->i_reserved1&0x03) + 1;
    p_avcC->i_reserved1 >>= 2;

    MP4_GET1BYTE( p_avcC->i_reserved2 );
    p_avcC->i_sps = p_avcC->i_reserved2&0x1f;
    p_avcC->i_reserved2 >>= 5;

    if( p_avcC->i_sps > 0 )
    {
        p_avcC->i_sps_length = (uint16_t *)calloc( p_avcC->i_sps, sizeof( uint16_t ) );
        p_avcC->sps = (uint8_t **)calloc( p_avcC->i_sps, sizeof( uint8_t* ) );

        if( !p_avcC->i_sps_length || !p_avcC->sps )
            goto error;

        for( i = 0; i < p_avcC->i_sps && i_read > 2; i++ )
        {
            MP4_GET2BYTES( p_avcC->i_sps_length[i] );
            if ( p_avcC->i_sps_length[i] > i_read )
                goto error;
            p_avcC->sps[i] = (uint8_t *)malloc( p_avcC->i_sps_length[i] );
            if( p_avcC->sps[i] )
                memcpy( p_avcC->sps[i], p_peek, p_avcC->i_sps_length[i] );

            p_peek += p_avcC->i_sps_length[i];
            i_read -= p_avcC->i_sps_length[i];
        }
        if ( i != p_avcC->i_sps )
            goto error;
    }

    MP4_GET1BYTE( p_avcC->i_pps );
    if( p_avcC->i_pps > 0 )
    {
        p_avcC->i_pps_length = (uint16_t *)calloc( p_avcC->i_pps, sizeof( uint16_t ) );
        p_avcC->pps = (uint8_t **)calloc( p_avcC->i_pps, sizeof( uint8_t* ) );

        if( !p_avcC->i_pps_length || !p_avcC->pps )
            goto error;

        for( i = 0; i < p_avcC->i_pps && i_read > 2; i++ )
        {
            MP4_GET2BYTES( p_avcC->i_pps_length[i] );
            if( p_avcC->i_pps_length[i] > i_read )
                goto error;
            p_avcC->pps[i] = (uint8_t *)malloc( p_avcC->i_pps_length[i] );
            if( p_avcC->pps[i] )
                memcpy( p_avcC->pps[i], p_peek, p_avcC->i_pps_length[i] );

            p_peek += p_avcC->i_pps_length[i];
            i_read -= p_avcC->i_pps_length[i];
        }
        if ( i != p_avcC->i_pps )
            goto error;
    }
#ifdef MP4_VERBOSE
    msg_Dbg( p_stream,
             "read box: \"avcC\" version=%d profile=0x%x level=0x%x length size=%d sps=%d pps=%d",
             p_avcC->i_version, p_avcC->i_profile, p_avcC->i_level,
             p_avcC->i_length_size,
             p_avcC->i_sps, p_avcC->i_pps );
    for( i = 0; i < p_avcC->i_sps; i++ )
    {
        msg_Dbg( p_stream, "         - sps[%d] length=%d",
                 i, p_avcC->i_sps_length[i] );
    }
    for( i = 0; i < p_avcC->i_pps; i++ )
    {
        msg_Dbg( p_stream, "         - pps[%d] length=%d",
                 i, p_avcC->i_pps_length[i] );
    }

#endif
    MP4_READBOX_EXIT( 1 );

error:
    MP4_FreeBox_avcC( p_box );
    MP4_READBOX_EXIT( 0 );
}

static int MP4_ReadBox_dac3( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_Box_data_dac3_t *p_dac3;
    MP4_READBOX_ENTER( MP4_Box_data_dac3_t );

    p_dac3 = p_box->data.p_dac3;

    unsigned i_header;
    MP4_GET3BYTES( i_header );

    p_dac3->i_fscod = ( i_header >> 22 ) & 0x03;
    p_dac3->i_bsid  = ( i_header >> 17 ) & 0x01f;
    p_dac3->i_bsmod = ( i_header >> 14 ) & 0x07;
    p_dac3->i_acmod = ( i_header >> 11 ) & 0x07;
    p_dac3->i_lfeon = ( i_header >> 10 ) & 0x01;
    p_dac3->i_bitrate_code = ( i_header >> 5) & 0x1f;

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream,
             "read box: \"dac3\" fscod=0x%x bsid=0x%x bsmod=0x%x acmod=0x%x lfeon=0x%x bitrate_code=0x%x",
             p_dac3->i_fscod, p_dac3->i_bsid, p_dac3->i_bsmod, p_dac3->i_acmod, p_dac3->i_lfeon, p_dac3->i_bitrate_code );
#endif
    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_dvc1( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_Box_data_dvc1_t *p_dvc1;

    MP4_READBOX_ENTER( MP4_Box_data_dvc1_t );
    p_dvc1 = p_box->data.p_dvc1;

    MP4_GET1BYTE( p_dvc1->i_profile_level ); /* profile is on 4bits, level 3bits */
    uint8_t i_profile = (p_dvc1->i_profile_level & 0xf0) >> 4;
    if( i_profile != 0x06 && i_profile != 0x0c )
    {
        msg_Warn( p_stream, "unsupported VC-1 profile (%" PRIu8"), please report", i_profile );
        MP4_READBOX_EXIT( 0 );
    }


    p_dvc1->i_vc1 = p_box->i_size - 7; /* Header + profile_level */

    if( p_dvc1->i_vc1 > 0 )
    {
        uint8_t *p = p_dvc1->p_vc1 = (uint8_t *)malloc( p_dvc1->i_vc1 );
        if( p )
            memcpy( p, p_peek, i_read );
    }

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream,
             "read box: \"dvc1\" profile=%" PRIu8" level=%i",
             i_profile, p_dvc1->i_profile_level & 0x0e >> 1 );
#endif

    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_enda( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_Box_data_enda_t *p_enda;
    MP4_READBOX_ENTER( MP4_Box_data_enda_t );

    p_enda = p_box->data.p_enda;

    MP4_GET2BYTES( p_enda->i_little_endian );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream,
             "read box: \"enda\" little_endian=%d", p_enda->i_little_endian );
#endif
    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_gnre( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_Box_data_gnre_t *p_gnre;
    MP4_READBOX_ENTER( MP4_Box_data_gnre_t );

    p_gnre = p_box->data.p_gnre;

    uint32_t i_data_len;
    uint32_t i_data_tag;

    MP4_GET4BYTES( i_data_len );
    MP4_GETFOURCC( i_data_tag );
    if( i_data_len < 10 || i_data_tag != ATOM_data )
        MP4_READBOX_EXIT( 0 );

    uint32_t i_version;
    VLC_UNUSED(i_version);
    uint32_t i_reserved;
    VLC_UNUSED(i_reserved);
    MP4_GET4BYTES( i_version );
    MP4_GET4BYTES( i_reserved );
    MP4_GET2BYTES( p_gnre->i_genre );
    if( p_gnre->i_genre == 0 )
        MP4_READBOX_EXIT( 0 );
#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"gnre\" genre=%i", p_gnre->i_genre );
#endif

    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_trkn( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_Box_data_trkn_t *p_trkn;
    MP4_READBOX_ENTER( MP4_Box_data_trkn_t );

    p_trkn = p_box->data.p_trkn;

    uint32_t i_data_len;
    uint32_t i_data_tag;

    MP4_GET4BYTES( i_data_len );
    MP4_GETFOURCC( i_data_tag );
    if( i_data_len < 12 || i_data_tag != ATOM_data )
        MP4_READBOX_EXIT( 0 );

    uint32_t i_version;
    VLC_UNUSED(i_version);
    uint32_t i_reserved;
    VLC_UNUSED(i_reserved);
    MP4_GET4BYTES( i_version );
    MP4_GET4BYTES( i_reserved );
    MP4_GET2BYTES( i_reserved );
    MP4_GET2BYTES( p_trkn->i_track_number );
#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"trkn\" number=%i", p_trkn->i_track_number );
#endif
    if( i_data_len > 15 )
    {
       MP4_GET2BYTES( p_trkn->i_track_total );
#ifdef MP4_VERBOSE
       msg_Dbg( p_stream, "read box: \"trkn\" total=%i", p_trkn->i_track_total );
#endif
    }

    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_sample_soun( stream_t *p_stream, MP4_Box_t *p_box )
{
    p_box->i_handler = ATOM_soun;
    MP4_READBOX_ENTER( MP4_Box_data_sample_soun_t );
    p_box->data.p_sample_soun->p_qt_description = NULL;

    /* Sanity check needed because the "wave" box does also contain an
     * "mp4a" box that we don't understand. */
    if( i_read < 28 )
    {
        i_read -= 30;
        MP4_READBOX_EXIT( 1 );
    }

    for( unsigned i = 0; i < 6 ; i++ )
    {
        MP4_GET1BYTE( p_box->data.p_sample_soun->i_reserved1[i] );
    }

    MP4_GET2BYTES( p_box->data.p_sample_soun->i_data_reference_index );

    /*
     * XXX hack -> produce a copy of the nearly complete chunk
     */
    p_box->data.p_sample_soun->i_qt_description = 0;
    p_box->data.p_sample_soun->p_qt_description = NULL;
    if( i_read > 0 )
    {
        p_box->data.p_sample_soun->p_qt_description = (uint8_t *)malloc( i_read );
        if( p_box->data.p_sample_soun->p_qt_description )
        {
            p_box->data.p_sample_soun->i_qt_description = i_read;
            memcpy( p_box->data.p_sample_soun->p_qt_description, p_peek, i_read );
        }
    }

    MP4_GET2BYTES( p_box->data.p_sample_soun->i_qt_version );
    MP4_GET2BYTES( p_box->data.p_sample_soun->i_qt_revision_level );
    MP4_GET4BYTES( p_box->data.p_sample_soun->i_qt_vendor );

    MP4_GET2BYTES( p_box->data.p_sample_soun->i_channelcount );
    MP4_GET2BYTES( p_box->data.p_sample_soun->i_samplesize );
    MP4_GET2BYTES( p_box->data.p_sample_soun->i_compressionid );
    MP4_GET2BYTES( p_box->data.p_sample_soun->i_reserved3 );
    MP4_GET2BYTES( p_box->data.p_sample_soun->i_sampleratehi );
    MP4_GET2BYTES( p_box->data.p_sample_soun->i_sampleratelo );

    if( p_box->data.p_sample_soun->i_qt_version == 1 && i_read >= 16 )
    {
        /* SoundDescriptionV1 */
        MP4_GET4BYTES( p_box->data.p_sample_soun->i_sample_per_packet );
        MP4_GET4BYTES( p_box->data.p_sample_soun->i_bytes_per_packet );
        MP4_GET4BYTES( p_box->data.p_sample_soun->i_bytes_per_frame );
        MP4_GET4BYTES( p_box->data.p_sample_soun->i_bytes_per_sample );

#ifdef MP4_VERBOSE
        msg_Dbg( p_stream,
                 "read box: \"soun\" qt3+ sample/packet=%d bytes/packet=%d "
                 "bytes/frame=%d bytes/sample=%d",
                 p_box->data.p_sample_soun->i_sample_per_packet,
                 p_box->data.p_sample_soun->i_bytes_per_packet,
                 p_box->data.p_sample_soun->i_bytes_per_frame,
                 p_box->data.p_sample_soun->i_bytes_per_sample );
#endif
        stream_Seek( p_stream, p_box->i_pos +
                        mp4_box_headersize( p_box ) + 44 );
    }
    else if( p_box->data.p_sample_soun->i_qt_version == 2 && i_read >= 36 )
    {
        /* SoundDescriptionV2 */
        double f_sample_rate;
        int64_t i_dummy64;
        uint32_t i_channel, i_extoffset, i_dummy32;

        /* Checks */
        if ( p_box->data.p_sample_soun->i_channelcount != 0x3  ||
             p_box->data.p_sample_soun->i_samplesize != 0x0010 ||
             p_box->data.p_sample_soun->i_compressionid != 0xFFFE ||
             p_box->data.p_sample_soun->i_reserved3 != 0x0     ||
             p_box->data.p_sample_soun->i_sampleratehi != 0x1  ||//65536
             p_box->data.p_sample_soun->i_sampleratelo != 0x0 )  //remainder
        {
            msg_Err( p_stream, "invalid stsd V2 box defaults" );
            MP4_READBOX_EXIT( 0 );
        }
        /* !Checks */

        MP4_GET4BYTES( i_extoffset ); /* offset to stsd extentions */
        MP4_GET8BYTES( i_dummy64 );
        memcpy( &f_sample_rate, &i_dummy64, 8 );
        msg_Dbg( p_stream, "read box: %f Hz", f_sample_rate );
        p_box->data.p_sample_soun->i_sampleratehi = (int)f_sample_rate % BLOCK16x16;
        p_box->data.p_sample_soun->i_sampleratelo = f_sample_rate / BLOCK16x16;

        MP4_GET4BYTES( i_channel );
        p_box->data.p_sample_soun->i_channelcount = i_channel;

        MP4_GET4BYTES( i_dummy32 );
        if ( i_dummy32 != 0x7F000000 )
        {
            msg_Err( p_stream, "invalid stsd V2 box" );
            MP4_READBOX_EXIT( 0 );
        }

        MP4_GET4BYTES( p_box->data.p_sample_soun->i_constbitsperchannel );
        MP4_GET4BYTES( p_box->data.p_sample_soun->i_formatflags );
        MP4_GET4BYTES( p_box->data.p_sample_soun->i_constbytesperaudiopacket );
        MP4_GET4BYTES( p_box->data.p_sample_soun->i_constLPCMframesperaudiopacket );

#ifdef MP4_VERBOSE
        msg_Dbg( p_stream, "read box: \"soun\" V2 rate=%f bitsperchannel=%u "
                           "flags=%u bytesperpacket=%u lpcmframesperpacket=%u",
                 f_sample_rate,
                 p_box->data.p_sample_soun->i_constbitsperchannel,
                 p_box->data.p_sample_soun->i_formatflags,
                 p_box->data.p_sample_soun->i_constbytesperaudiopacket,
                 p_box->data.p_sample_soun->i_constLPCMframesperaudiopacket );
#endif
        if ( i_extoffset < p_box->i_size )
            stream_Seek( p_stream, p_box->i_pos + i_extoffset );
        else
            stream_Seek( p_stream, p_box->i_pos + p_box->i_size );
    }
    else
    {
        p_box->data.p_sample_soun->i_sample_per_packet = 0;
        p_box->data.p_sample_soun->i_bytes_per_packet = 0;
        p_box->data.p_sample_soun->i_bytes_per_frame = 0;
        p_box->data.p_sample_soun->i_bytes_per_sample = 0;

#ifdef MP4_VERBOSE
        msg_Dbg( p_stream, "read box: \"soun\" mp4 or qt1/2 (rest=%" PRId64")",
                 i_read );
#endif
        stream_Seek( p_stream, p_box->i_pos +
                        mp4_box_headersize( p_box ) + 28 );
    }

    if( p_box->i_type == ATOM_drms )
    {
        msg_Warn( p_stream, "DRM protected streams are not supported." );
        MP4_READBOX_EXIT( 0 );
    }

    if( p_box->i_type == ATOM_samr || p_box->i_type == ATOM_sawb )
    {
        /* Ignore channelcount for AMR (3gpp AMRSpecificBox) */
        p_box->data.p_sample_soun->i_channelcount = 1;
    }

    /* Loads extensions */
    MP4_ReadBoxContainerRaw( p_stream, p_box ); /* esds/wave/... */

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"soun\" in stsd channel %d "
             "sample size %d sample rate %f",
             p_box->data.p_sample_soun->i_channelcount,
             p_box->data.p_sample_soun->i_samplesize,
             (float)p_box->data.p_sample_soun->i_sampleratehi +
             (float)p_box->data.p_sample_soun->i_sampleratelo / BLOCK16x16 );

#endif
    MP4_READBOX_EXIT( 1 );
}


static void MP4_FreeBox_sample_soun( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_sample_soun->p_qt_description );
}


int MP4_ReadBox_sample_vide( stream_t *p_stream, MP4_Box_t *p_box )
{
    p_box->i_handler = ATOM_vide;
    MP4_READBOX_ENTER( MP4_Box_data_sample_vide_t );

    for( unsigned i = 0; i < 6 ; i++ )
    {
        MP4_GET1BYTE( p_box->data.p_sample_vide->i_reserved1[i] );
    }

    MP4_GET2BYTES( p_box->data.p_sample_vide->i_data_reference_index );

    /*
     * XXX hack -> produce a copy of the nearly complete chunk
     */
    if( i_read > 0 )
    {
        p_box->data.p_sample_vide->p_qt_image_description = (uint8_t *)malloc( i_read );
        if( unlikely( p_box->data.p_sample_vide->p_qt_image_description == NULL ) )
            MP4_READBOX_EXIT( 0 );
        p_box->data.p_sample_vide->i_qt_image_description = i_read;
        memcpy( p_box->data.p_sample_vide->p_qt_image_description,
                p_peek, i_read );
    }
    else
    {
        p_box->data.p_sample_vide->i_qt_image_description = 0;
        p_box->data.p_sample_vide->p_qt_image_description = NULL;
    }

    MP4_GET2BYTES( p_box->data.p_sample_vide->i_qt_version );
    MP4_GET2BYTES( p_box->data.p_sample_vide->i_qt_revision_level );
    MP4_GET4BYTES( p_box->data.p_sample_vide->i_qt_vendor );

    MP4_GET4BYTES( p_box->data.p_sample_vide->i_qt_temporal_quality );
    MP4_GET4BYTES( p_box->data.p_sample_vide->i_qt_spatial_quality );

    MP4_GET2BYTES( p_box->data.p_sample_vide->i_width );
    MP4_GET2BYTES( p_box->data.p_sample_vide->i_height );

    MP4_GET4BYTES( p_box->data.p_sample_vide->i_horizresolution );
    MP4_GET4BYTES( p_box->data.p_sample_vide->i_vertresolution );

    MP4_GET4BYTES( p_box->data.p_sample_vide->i_qt_data_size );
    MP4_GET2BYTES( p_box->data.p_sample_vide->i_qt_frame_count );

    if ( i_read < 32 )
        MP4_READBOX_EXIT( 0 );
    memcpy( &p_box->data.p_sample_vide->i_compressorname, p_peek, 32 );
    p_peek += 32; i_read -= 32;

    MP4_GET2BYTES( p_box->data.p_sample_vide->i_depth );
    MP4_GET2BYTES( p_box->data.p_sample_vide->i_qt_color_table );

    stream_Seek( p_stream, p_box->i_pos + mp4_box_headersize( p_box ) + 78);

    if( p_box->i_type == ATOM_drmi )
    {
        msg_Warn( p_stream, "DRM protected streams are not supported." );
        MP4_READBOX_EXIT( 0 );
    }

    MP4_ReadBoxContainerRaw( p_stream, p_box );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"vide\" in stsd %dx%d depth %d",
                      p_box->data.p_sample_vide->i_width,
                      p_box->data.p_sample_vide->i_height,
                      p_box->data.p_sample_vide->i_depth );

#endif
    MP4_READBOX_EXIT( 1 );
}


void MP4_FreeBox_sample_vide( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_sample_vide->p_qt_image_description );
}

static int MP4_ReadBox_sample_mp4s( stream_t *p_stream, MP4_Box_t *p_box )
{
    stream_Seek( p_stream, p_box->i_pos + mp4_box_headersize( p_box ) + 8 );
    MP4_ReadBoxContainerRaw( p_stream, p_box );
    return 1;
}

static int MP4_ReadBox_sample_text( stream_t *p_stream, MP4_Box_t *p_box )
{
    int32_t t;

    p_box->i_handler = ATOM_text;
    MP4_READBOX_ENTER( MP4_Box_data_sample_text_t );

    MP4_GET4BYTES( p_box->data.p_sample_text->i_reserved1 );
    MP4_GET2BYTES( p_box->data.p_sample_text->i_reserved2 );

    MP4_GET2BYTES( p_box->data.p_sample_text->i_data_reference_index );

    MP4_GET4BYTES( p_box->data.p_sample_text->i_display_flags );

    MP4_GET4BYTES( t );
    switch( t )
    {
        /* FIXME search right signification */
        case 1: // Center
            p_box->data.p_sample_text->i_justification_horizontal = 1;
            p_box->data.p_sample_text->i_justification_vertical = 1;
            break;
        case -1:    // Flush Right
            p_box->data.p_sample_text->i_justification_horizontal = -1;
            p_box->data.p_sample_text->i_justification_vertical = -1;
            break;
        case -2:    // Flush Left
            p_box->data.p_sample_text->i_justification_horizontal = 0;
            p_box->data.p_sample_text->i_justification_vertical = 0;
            break;
        case 0: // Flush Default
        default:
            p_box->data.p_sample_text->i_justification_horizontal = 1;
            p_box->data.p_sample_text->i_justification_vertical = -1;
            break;
    }

    MP4_GET2BYTES( p_box->data.p_sample_text->i_background_color[0] );
    MP4_GET2BYTES( p_box->data.p_sample_text->i_background_color[1] );
    MP4_GET2BYTES( p_box->data.p_sample_text->i_background_color[2] );
    p_box->data.p_sample_text->i_background_color[3] = 0xFF;

    MP4_GET2BYTES( p_box->data.p_sample_text->i_text_box_top );
    MP4_GET2BYTES( p_box->data.p_sample_text->i_text_box_left );
    MP4_GET2BYTES( p_box->data.p_sample_text->i_text_box_bottom );
    MP4_GET2BYTES( p_box->data.p_sample_text->i_text_box_right );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"text\" in stsd text" );
#endif
    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_sample_tx3g( stream_t *p_stream, MP4_Box_t *p_box )
{
    p_box->i_handler = ATOM_text;
    MP4_READBOX_ENTER( MP4_Box_data_sample_text_t );

    MP4_GET4BYTES( p_box->data.p_sample_text->i_reserved1 );
    MP4_GET2BYTES( p_box->data.p_sample_text->i_reserved2 );

    MP4_GET2BYTES( p_box->data.p_sample_text->i_data_reference_index );

    MP4_GET4BYTES( p_box->data.p_sample_text->i_display_flags );

    MP4_GET1BYTE ( p_box->data.p_sample_text->i_justification_horizontal );
    MP4_GET1BYTE ( p_box->data.p_sample_text->i_justification_vertical );

    MP4_GET1BYTE ( p_box->data.p_sample_text->i_background_color[0] );
    MP4_GET1BYTE ( p_box->data.p_sample_text->i_background_color[1] );
    MP4_GET1BYTE ( p_box->data.p_sample_text->i_background_color[2] );
    MP4_GET1BYTE ( p_box->data.p_sample_text->i_background_color[3] );

    MP4_GET2BYTES( p_box->data.p_sample_text->i_text_box_top );
    MP4_GET2BYTES( p_box->data.p_sample_text->i_text_box_left );
    MP4_GET2BYTES( p_box->data.p_sample_text->i_text_box_bottom );
    MP4_GET2BYTES( p_box->data.p_sample_text->i_text_box_right );

    MP4_GET4BYTES( p_box->data.p_sample_text->i_reserved3 );

    MP4_GET2BYTES( p_box->data.p_sample_text->i_font_id );
    MP4_GET1BYTE ( p_box->data.p_sample_text->i_font_face );
    MP4_GET1BYTE ( p_box->data.p_sample_text->i_font_size );
    MP4_GET4BYTES( p_box->data.p_sample_text->i_font_color );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"tx3g\" in stsd text" );
#endif
    MP4_READBOX_EXIT( 1 );
}


#if 0
/* We can't easily call it, and anyway ~ 20 bytes lost isn't a real problem */
static void MP4_FreeBox_sample_text( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_sample_text->psz_text_name );
}
#endif


static int MP4_ReadBox_stsd( stream_t *p_stream, MP4_Box_t *p_box )
{

    MP4_READBOX_ENTER( MP4_Box_data_stsd_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_stsd );

    MP4_GET4BYTES( p_box->data.p_stsd->i_entry_count );

    stream_Seek( p_stream, p_box->i_pos + mp4_box_headersize( p_box ) + 8 );

    MP4_ReadBoxContainerRaw( p_stream, p_box );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"stsd\" entry-count %d",
                      p_box->data.p_stsd->i_entry_count );

#endif
    MP4_READBOX_EXIT( 1 );
}


static int MP4_ReadBox_stsz( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_stsz_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_stsz );

    MP4_GET4BYTES( p_box->data.p_stsz->i_sample_size );
    MP4_GET4BYTES( p_box->data.p_stsz->i_sample_count );

    if( p_box->data.p_stsz->i_sample_size == 0 )
    {
        p_box->data.p_stsz->i_entry_size =
            (uint32_t *)calloc( p_box->data.p_stsz->i_sample_count, sizeof(uint32_t) );
        if( unlikely( !p_box->data.p_stsz->i_entry_size ) )
            MP4_READBOX_EXIT( 0 );

        for( unsigned int i = 0; (i<p_box->data.p_stsz->i_sample_count)&&(i_read >= 4 ); i++ )
        {
            MP4_GET4BYTES( p_box->data.p_stsz->i_entry_size[i] );
        }
    }
    else
        p_box->data.p_stsz->i_entry_size = NULL;

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"stsz\" sample-size %d sample-count %d",
                      p_box->data.p_stsz->i_sample_size,
                      p_box->data.p_stsz->i_sample_count );

#endif
    MP4_READBOX_EXIT( 1 );
}

static void MP4_FreeBox_stsz( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_stsz->i_entry_size );
}

static void MP4_FreeBox_stsc( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_stsc->i_first_chunk );
    FREENULL( p_box->data.p_stsc->i_samples_per_chunk );
    FREENULL( p_box->data.p_stsc->i_sample_description_index );
}

static int MP4_ReadBox_stsc( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_stsc_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_stsc );

    MP4_GET4BYTES( p_box->data.p_stsc->i_entry_count );

    p_box->data.p_stsc->i_first_chunk =
        (uint32_t *)calloc( p_box->data.p_stsc->i_entry_count, sizeof(uint32_t) );
    p_box->data.p_stsc->i_samples_per_chunk =
        (uint32_t *)calloc( p_box->data.p_stsc->i_entry_count, sizeof(uint32_t) );
    p_box->data.p_stsc->i_sample_description_index =
        (uint32_t *)calloc( p_box->data.p_stsc->i_entry_count, sizeof(uint32_t) );
    if( unlikely( p_box->data.p_stsc->i_first_chunk == NULL
     || p_box->data.p_stsc->i_samples_per_chunk == NULL
     || p_box->data.p_stsc->i_sample_description_index == NULL ) )
    {
        MP4_READBOX_EXIT( 0 );
    }

    for( unsigned int i = 0; (i < p_box->data.p_stsc->i_entry_count )&&( i_read >= 12 );i++ )
    {
        MP4_GET4BYTES( p_box->data.p_stsc->i_first_chunk[i] );
        MP4_GET4BYTES( p_box->data.p_stsc->i_samples_per_chunk[i] );
        MP4_GET4BYTES( p_box->data.p_stsc->i_sample_description_index[i] );
    }

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"stsc\" entry-count %d",
                      p_box->data.p_stsc->i_entry_count );

#endif
    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_stco_co64( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_co64_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_co64 );

    MP4_GET4BYTES( p_box->data.p_co64->i_entry_count );

    p_box->data.p_co64->i_chunk_offset =
        (uint64_t *)calloc( p_box->data.p_co64->i_entry_count, sizeof(uint64_t) );
    if( p_box->data.p_co64->i_chunk_offset == NULL )
        MP4_READBOX_EXIT( 0 );

    for( unsigned int i = 0; i < p_box->data.p_co64->i_entry_count; i++ )
    {
        if( p_box->i_type == ATOM_stco )
        {
            if( i_read < 4 )
            {
                break;
            }
            MP4_GET4BYTES( p_box->data.p_co64->i_chunk_offset[i] );
        }
        else
        {
            if( i_read < 8 )
            {
                break;
            }
            MP4_GET8BYTES( p_box->data.p_co64->i_chunk_offset[i] );
        }
    }

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"co64\" entry-count %d",
                      p_box->data.p_co64->i_entry_count );

#endif
    MP4_READBOX_EXIT( 1 );
}

static void MP4_FreeBox_stco_co64( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_co64->i_chunk_offset );
}

static int MP4_ReadBox_stss( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_stss_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_stss );

    MP4_GET4BYTES( p_box->data.p_stss->i_entry_count );

    p_box->data.p_stss->i_sample_number =
        (uint32_t *)calloc( p_box->data.p_stss->i_entry_count, sizeof(uint32_t) );
    if( unlikely( p_box->data.p_stss->i_sample_number == NULL ) )
        MP4_READBOX_EXIT( 0 );

    unsigned int i;
    for( i = 0; (i < p_box->data.p_stss->i_entry_count )&&( i_read >= 4 ); i++ )
    {

        MP4_GET4BYTES( p_box->data.p_stss->i_sample_number[i] );
        /* XXX in libmp4 sample begin at 0 */
        p_box->data.p_stss->i_sample_number[i]--;
    }
    if ( i < p_box->data.p_stss->i_entry_count )
        p_box->data.p_stss->i_entry_count = i;

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"stss\" entry-count %d",
                      p_box->data.p_stss->i_entry_count );

#endif
    MP4_READBOX_EXIT( 1 );
}

static void MP4_FreeBox_stss( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_stss->i_sample_number );
}

static void MP4_FreeBox_stsh( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_stsh->i_shadowed_sample_number );
    FREENULL( p_box->data.p_stsh->i_sync_sample_number );
}

static int MP4_ReadBox_stsh( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_stsh_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_stsh );


    MP4_GET4BYTES( p_box->data.p_stsh->i_entry_count );

    p_box->data.p_stsh->i_shadowed_sample_number =
        (uint32_t *)calloc( p_box->data.p_stsh->i_entry_count, sizeof(uint32_t) );
    p_box->data.p_stsh->i_sync_sample_number =
        (uint32_t *)calloc( p_box->data.p_stsh->i_entry_count, sizeof(uint32_t) );

    if( p_box->data.p_stsh->i_shadowed_sample_number == NULL
     || p_box->data.p_stsh->i_sync_sample_number == NULL )
    {
        MP4_READBOX_EXIT( 0 );
    }

    unsigned i;
    for( i = 0; (i < p_box->data.p_stss->i_entry_count )&&( i_read >= 8 ); i++ )
    {
        MP4_GET4BYTES( p_box->data.p_stsh->i_shadowed_sample_number[i] );
        MP4_GET4BYTES( p_box->data.p_stsh->i_sync_sample_number[i] );
    }
    if ( i < p_box->data.p_stss->i_entry_count )
        p_box->data.p_stss->i_entry_count = i;

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"stsh\" entry-count %d",
                      p_box->data.p_stsh->i_entry_count );
#endif
    MP4_READBOX_EXIT( 1 );
}


static int MP4_ReadBox_stdp( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_stdp_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_stdp );

    p_box->data.p_stdp->i_priority =
        (uint16_t *)calloc( i_read / 2, sizeof(uint16_t) );

    if( unlikely( !p_box->data.p_stdp->i_priority ) )
        MP4_READBOX_EXIT( 0 );

    for( unsigned i = 0; i < i_read / 2 ; i++ )
    {
        MP4_GET2BYTES( p_box->data.p_stdp->i_priority[i] );
    }

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"stdp\" entry-count %" PRId64,
                      i_read / 2 );

#endif
    MP4_READBOX_EXIT( 1 );
}

static void MP4_FreeBox_stdp( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_stdp->i_priority );
}

static void MP4_FreeBox_padb( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_padb->i_reserved1 );
    FREENULL( p_box->data.p_padb->i_pad2 );
    FREENULL( p_box->data.p_padb->i_reserved2 );
    FREENULL( p_box->data.p_padb->i_pad1 );
}

static int MP4_ReadBox_padb( stream_t *p_stream, MP4_Box_t *p_box )
{
    uint32_t count;

    MP4_READBOX_ENTER( MP4_Box_data_padb_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_padb );

    MP4_GET4BYTES( p_box->data.p_padb->i_sample_count );
    count = (p_box->data.p_padb->i_sample_count + 1) / 2;

    p_box->data.p_padb->i_reserved1 = (uint16_t *)calloc( count, sizeof(uint16_t) );
    p_box->data.p_padb->i_pad2 = (uint16_t *)calloc( count, sizeof(uint16_t) );
    p_box->data.p_padb->i_reserved2 = (uint16_t *)calloc( count, sizeof(uint16_t) );
    p_box->data.p_padb->i_pad1 = (uint16_t *)calloc( count, sizeof(uint16_t) );
    if( p_box->data.p_padb->i_reserved1 == NULL
     || p_box->data.p_padb->i_pad2 == NULL
     || p_box->data.p_padb->i_reserved2 == NULL
     || p_box->data.p_padb->i_pad1 == NULL )
    {
        MP4_READBOX_EXIT( 0 );
    }

    for( unsigned int i = 0; i < i_read / 2 ; i++ )
    {
        if( i >= count )
        {
            MP4_READBOX_EXIT( 0 );
        }
        p_box->data.p_padb->i_reserved1[i] = ( (*p_peek) >> 7 )&0x01;
        p_box->data.p_padb->i_pad2[i] = ( (*p_peek) >> 4 )&0x07;
        p_box->data.p_padb->i_reserved1[i] = ( (*p_peek) >> 3 )&0x01;
        p_box->data.p_padb->i_pad1[i] = ( (*p_peek) )&0x07;

        p_peek += 1; i_read -= 1;
    }

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"stdp\" entry-count %" PRId64,
                      i_read / 2 );

#endif
    MP4_READBOX_EXIT( 1 );
}

static void MP4_FreeBox_elst( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_elst->i_segment_duration );
    FREENULL( p_box->data.p_elst->i_media_time );
    FREENULL( p_box->data.p_elst->i_media_rate_integer );
    FREENULL( p_box->data.p_elst->i_media_rate_fraction );
}

static int MP4_ReadBox_elst( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_elst_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_elst );


    MP4_GET4BYTES( p_box->data.p_elst->i_entry_count );

    p_box->data.p_elst->i_segment_duration =
        (uint64_t *)calloc( p_box->data.p_elst->i_entry_count, sizeof(uint64_t) );
    p_box->data.p_elst->i_media_time =
        (int64_t *)calloc( p_box->data.p_elst->i_entry_count, sizeof(int64_t) );
    p_box->data.p_elst->i_media_rate_integer =
        (uint16_t *)calloc( p_box->data.p_elst->i_entry_count, sizeof(uint16_t) );
    p_box->data.p_elst->i_media_rate_fraction =
        (uint16_t *)calloc( p_box->data.p_elst->i_entry_count, sizeof(uint16_t) );
    if( p_box->data.p_elst->i_segment_duration == NULL
     || p_box->data.p_elst->i_media_time == NULL
     || p_box->data.p_elst->i_media_rate_integer == NULL
     || p_box->data.p_elst->i_media_rate_fraction == NULL )
    {
        MP4_READBOX_EXIT( 0 );
    }

    unsigned i;
    for( i = 0; i < p_box->data.p_elst->i_entry_count; i++ )
    {
        if( p_box->data.p_elst->i_version == 1 )
        {
            if ( i_read < 20 )
                break;
            MP4_GET8BYTES( p_box->data.p_elst->i_segment_duration[i] );

            MP4_GET8BYTES( p_box->data.p_elst->i_media_time[i] );
        }
        else
        {
            if ( i_read < 12 )
                break;
            MP4_GET4BYTES( p_box->data.p_elst->i_segment_duration[i] );

            MP4_GET4BYTES( p_box->data.p_elst->i_media_time[i] );
            p_box->data.p_elst->i_media_time[i] = (int32_t)p_box->data.p_elst->i_media_time[i];
        }

        MP4_GET2BYTES( p_box->data.p_elst->i_media_rate_integer[i] );
        MP4_GET2BYTES( p_box->data.p_elst->i_media_rate_fraction[i] );
    }
    if ( i < p_box->data.p_elst->i_entry_count )
        p_box->data.p_elst->i_entry_count = i;
#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"elst\" entry-count %lu",
             (unsigned long)p_box->data.p_elst->i_entry_count );
#endif
    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_cprt( stream_t *p_stream, MP4_Box_t *p_box )
{
    uint16_t i_language;
    bool b_mac;

    MP4_READBOX_ENTER( MP4_Box_data_cprt_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_cprt );

    MP4_GET2BYTES( i_language );
    decodeQtLanguageCode( i_language, p_box->data.p_cprt->rgs_language, &b_mac );

    MP4_GETSTRINGZ( p_box->data.p_cprt->psz_notice );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"cprt\" language %3.3s notice %s",
                      p_box->data.p_cprt->rgs_language,
                      p_box->data.p_cprt->psz_notice );

#endif
    MP4_READBOX_EXIT( 1 );
}

static void MP4_FreeBox_cprt( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_cprt->psz_notice );
}


static int MP4_ReadBox_dcom( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_dcom_t );

    MP4_GETFOURCC( p_box->data.p_dcom->i_algorithm );
#ifdef MP4_VERBOSE
    msg_Dbg( p_stream,
             "read box: \"dcom\" compression algorithm : %4.4s",
                      (char*)&p_box->data.p_dcom->i_algorithm );
#endif
    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_cmvd( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_cmvd_t );

    MP4_GET4BYTES( p_box->data.p_cmvd->i_uncompressed_size );

    p_box->data.p_cmvd->i_compressed_size = i_read;

    if( !( p_box->data.p_cmvd->p_data = (uint8_t *)malloc( i_read ) ) )
        MP4_READBOX_EXIT( 0 );

    /* now copy compressed data */
    memcpy( p_box->data.p_cmvd->p_data, p_peek,i_read);

    p_box->data.p_cmvd->b_compressed = 1;

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"cmvd\" compressed data size %d",
                      p_box->data.p_cmvd->i_compressed_size );
#endif

    MP4_READBOX_EXIT( 1 );
}
static void MP4_FreeBox_cmvd( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_cmvd->p_data );
}


static int MP4_ReadBox_cmov( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_Box_t *p_dcom;
    MP4_Box_t *p_cmvd;

#ifdef HAVE_ZLIB_H
    stream_t *p_stream_memory;
    z_stream z_data;
    uint8_t *p_data;
    int i_result;
#endif

    if( !( p_box->data.p_cmov = (MP4_Box_data_cmov_t *)calloc(1, sizeof( MP4_Box_data_cmov_t ) ) ) )
        return 0;

    if( !p_box->p_father ||
        ( p_box->p_father->i_type != ATOM_moov &&
          p_box->p_father->i_type != ATOM_foov ) )
    {
        msg_Warn( p_stream, "Read box: \"cmov\" box alone" );
        return 1;
    }

    if( !MP4_ReadBoxContainer( p_stream, p_box ) )
    {
        return 0;
    }

    if( ( p_dcom = MP4_BoxGet( p_box, "dcom" ) ) == NULL ||
        ( p_cmvd = MP4_BoxGet( p_box, "cmvd" ) ) == NULL ||
        p_cmvd->data.p_cmvd->p_data == NULL )
    {
        msg_Warn( p_stream, "read box: \"cmov\" incomplete" );
        return 0;
    }

    if( p_dcom->data.p_dcom->i_algorithm != ATOM_zlib )
    {
        msg_Dbg( p_stream, "read box: \"cmov\" compression algorithm : %4.4s "
                 "not supported", (char*)&p_dcom->data.p_dcom->i_algorithm );
        return 0;
    }

#ifndef HAVE_ZLIB_H
    msg_Dbg( p_stream, "read box: \"cmov\" zlib unsupported" );
    return 0;

#else
    /* decompress data */
    /* allocate a new buffer */
    if( !( p_data = (uint8_t *)malloc( p_cmvd->data.p_cmvd->i_uncompressed_size ) ) )
        return 0;
    /* init default structures */
    z_data.next_in   = p_cmvd->data.p_cmvd->p_data;
    z_data.avail_in  = p_cmvd->data.p_cmvd->i_compressed_size;
    z_data.next_out  = p_data;
    z_data.avail_out = p_cmvd->data.p_cmvd->i_uncompressed_size;
    z_data.zalloc    = (alloc_func)Z_NULL;
    z_data.zfree     = (free_func)Z_NULL;
    z_data.opaque    = (voidpf)Z_NULL;

    /* init zlib */
    if( inflateInit( &z_data ) != Z_OK )
    {
        msg_Err( p_stream, "read box: \"cmov\" error while uncompressing" );
        free( p_data );
        return 0;
    }

    /* uncompress */
    i_result = inflate( &z_data, Z_NO_FLUSH );
    if( i_result != Z_OK && i_result != Z_STREAM_END )
    {
        msg_Err( p_stream, "read box: \"cmov\" error while uncompressing" );
        free( p_data );
        return 0;
    }

    if( p_cmvd->data.p_cmvd->i_uncompressed_size != z_data.total_out )
    {
        msg_Warn( p_stream, "read box: \"cmov\" uncompressing data size "
                  "mismatch" );
    }
    p_cmvd->data.p_cmvd->i_uncompressed_size = z_data.total_out;

    /* close zlib */
    if( inflateEnd( &z_data ) != Z_OK )
    {
        msg_Warn( p_stream, "read box: \"cmov\" error while uncompressing "
                  "data (ignored)" );
    }

    free( p_cmvd->data.p_cmvd->p_data );
    p_cmvd->data.p_cmvd->p_data = p_data;
    p_cmvd->data.p_cmvd->b_compressed = 0;

    msg_Dbg( p_stream, "read box: \"cmov\" box successfully uncompressed" );

    /* now create a memory stream */
    p_stream_memory =
        stream_MemoryNew( VLC_OBJECT(p_stream), p_cmvd->data.p_cmvd->p_data,
                          p_cmvd->data.p_cmvd->i_uncompressed_size, true );

    /* and read uncompressd moov */
    p_box->data.p_cmov->p_moov = MP4_ReadBox( p_stream_memory, NULL );

    stream_Delete( p_stream_memory );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"cmov\" compressed movie header completed");
#endif

    return p_box->data.p_cmov->p_moov ? 1 : 0;
#endif /* HAVE_ZLIB_H */
}

static int MP4_ReadBox_rdrf( stream_t *p_stream, MP4_Box_t *p_box )
{
    uint32_t i_len;
    MP4_READBOX_ENTER( MP4_Box_data_rdrf_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_rdrf );
    MP4_GETFOURCC( p_box->data.p_rdrf->i_ref_type );
    MP4_GET4BYTES( i_len );
    i_len++;

    if( i_len > 0 )
    {
        p_box->data.p_rdrf->psz_ref = (char *)malloc( i_len );
        if( p_box->data.p_rdrf->psz_ref == NULL )
            MP4_READBOX_EXIT( 0 );
        i_len--;

        for( unsigned i = 0; i < i_len; i++ )
        {
            MP4_GET1BYTE( p_box->data.p_rdrf->psz_ref[i] );
        }
        p_box->data.p_rdrf->psz_ref[i_len] = '\0';
    }
    else
    {
        p_box->data.p_rdrf->psz_ref = NULL;
    }

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream,
            "read box: \"rdrf\" type:%4.4s ref %s",
            (char*)&p_box->data.p_rdrf->i_ref_type,
            p_box->data.p_rdrf->psz_ref );
#endif
    MP4_READBOX_EXIT( 1 );
}

static void MP4_FreeBox_rdrf( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_rdrf->psz_ref );
}


static int MP4_ReadBox_rmdr( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_rmdr_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_rmdr );

    MP4_GET4BYTES( p_box->data.p_rmdr->i_rate );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream,
             "read box: \"rmdr\" rate:%d",
             p_box->data.p_rmdr->i_rate );
#endif
    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_rmqu( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_rmqu_t );

    MP4_GET4BYTES( p_box->data.p_rmqu->i_quality );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream,
             "read box: \"rmqu\" quality:%d",
             p_box->data.p_rmqu->i_quality );
#endif
    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_rmvc( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_rmvc_t );
    MP4_GETVERSIONFLAGS( p_box->data.p_rmvc );

    MP4_GETFOURCC( p_box->data.p_rmvc->i_gestaltType );
    MP4_GET4BYTES( p_box->data.p_rmvc->i_val1 );
    MP4_GET4BYTES( p_box->data.p_rmvc->i_val2 );
    MP4_GET2BYTES( p_box->data.p_rmvc->i_checkType );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream,
             "read box: \"rmvc\" gestaltType:%4.4s val1:0x%x val2:0x%x checkType:0x%x",
             (char*)&p_box->data.p_rmvc->i_gestaltType,
             p_box->data.p_rmvc->i_val1,p_box->data.p_rmvc->i_val2,
             p_box->data.p_rmvc->i_checkType );
#endif

    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_frma( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_frma_t );

    MP4_GETFOURCC( p_box->data.p_frma->i_type );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"frma\" i_type:%4.4s",
             (char *)&p_box->data.p_frma->i_type );
#endif

    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_skcr( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_skcr_t );

    MP4_GET4BYTES( p_box->data.p_skcr->i_init );
    MP4_GET4BYTES( p_box->data.p_skcr->i_encr );
    MP4_GET4BYTES( p_box->data.p_skcr->i_decr );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"skcr\" i_init:%d i_encr:%d i_decr:%d",
             p_box->data.p_skcr->i_init,
             p_box->data.p_skcr->i_encr,
             p_box->data.p_skcr->i_decr );
#endif

    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_drms( stream_t *p_stream, MP4_Box_t *p_box )
{
    VLC_UNUSED(p_box);
    /* ATOMs 'user', 'key', 'iviv', and 'priv' will be skipped,
     * so unless data decrypt itself by magic, there will be no playback,
     * but we never know... */
    msg_Warn( p_stream, "DRM protected streams are not supported." );
    return 1;
}

static int MP4_ReadBox_name( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_name_t );

    if( p_box->i_size < 8 || p_box->i_size > SIZE_MAX )
        MP4_READBOX_EXIT( 0 );

    p_box->data.p_name->psz_text = (char *)malloc( p_box->i_size + 1 - 8 ); /* +\0, -name, -size */
    if( p_box->data.p_name->psz_text == NULL )
        MP4_READBOX_EXIT( 0 );

    memcpy( p_box->data.p_name->psz_text, p_peek, p_box->i_size - 8 );
    p_box->data.p_name->psz_text[p_box->i_size - 8] = '\0';

#ifdef MP4_VERBOSE
        msg_Dbg( p_stream, "read box: \"name\" text=`%s'",
                 p_box->data.p_name->psz_text );
#endif
    MP4_READBOX_EXIT( 1 );
}

static void MP4_FreeBox_name( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_name->psz_text );
}

static int MP4_ReadBox_data( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_data_t );

    /* What's that header ?? */
    if ( i_read <= 8 )
        MP4_READBOX_EXIT( 0 );

    p_box->data.p_data->p_blob = (uint8_t *)malloc( i_read - 8 );
    if ( !p_box->data.p_data->p_blob )
        MP4_READBOX_EXIT( 0 );

    p_box->data.p_data->i_blob = i_read - 8;
    memcpy( p_box->data.p_data->p_blob, p_peek + 8, i_read - 8 );

    MP4_READBOX_EXIT( 1 );
}

static void MP4_FreeBox_data( MP4_Box_t *p_box )
{
    free( p_box->data.p_data->p_blob );
}

static int MP4_ReadBox_0xa9xxx( stream_t *p_stream, MP4_Box_t *p_box )
{
    uint16_t i16;

    MP4_READBOX_ENTER( MP4_Box_data_0xa9xxx_t );

    p_box->data.p_0xa9xxx->psz_text = NULL;

    MP4_GET2BYTES( i16 );

    if( i16 > 0 )
    {
        int i_length = i16;

        MP4_GET2BYTES( i16 );
        if( i_length >= i_read ) i_length = i_read + 1;

        p_box->data.p_0xa9xxx->psz_text = (char *)malloc( i_length );
        if( p_box->data.p_0xa9xxx->psz_text == NULL )
            MP4_READBOX_EXIT( 0 );

        i_length--;
        memcpy( p_box->data.p_0xa9xxx->psz_text,
                p_peek, i_length );
        p_box->data.p_0xa9xxx->psz_text[i_length] = '\0';

#ifdef MP4_VERBOSE
        msg_Dbg( p_stream,
                 "read box: \"c%3.3s\" text=`%s'",
                 ((char*)&p_box->i_type + 1),
                 p_box->data.p_0xa9xxx->psz_text );
#endif
    }
    else
    {
        /* try iTune/Quicktime format, rewind to start */
        p_peek -= 2; i_read += 2;
        // we are expecting a 'data' box
        uint32_t i_data_len;
        uint32_t i_data_tag;

        MP4_GET4BYTES( i_data_len );
        if( i_data_len > i_read ) i_data_len = i_read;
        MP4_GETFOURCC( i_data_tag );
        if( (i_data_len > 0) && (i_data_tag == ATOM_data) )
        {
            /* data box contains a version/flags field */
            uint32_t i_version;
            uint32_t i_reserved;
            VLC_UNUSED(i_reserved);
            MP4_GET4BYTES( i_version );
            MP4_GET4BYTES( i_reserved );
            // version should be 0, flags should be 1 for text, 0 for data
            if( ( i_version == 0x00000001 ) && (i_data_len >= 12 ) )
            {
                // the rest is the text
                i_data_len -= 12;
                p_box->data.p_0xa9xxx->psz_text = (char *)malloc( i_data_len + 1 );
                if( p_box->data.p_0xa9xxx->psz_text == NULL )
                    MP4_READBOX_EXIT( 0 );

                memcpy( p_box->data.p_0xa9xxx->psz_text,
                        p_peek, i_data_len );
                p_box->data.p_0xa9xxx->psz_text[i_data_len] = '\0';
#ifdef MP4_VERBOSE
        msg_Dbg( p_stream,
                 "read box: \"c%3.3s\" text=`%s'",
                 ((char*)&p_box->i_type+1),
                 p_box->data.p_0xa9xxx->psz_text );
#endif
            }
            else
            {
                // TODO: handle data values for ID3 tag values, track num or cover art,etc...
            }
        }
    }

    MP4_READBOX_EXIT( 1 );
}
static void MP4_FreeBox_0xa9xxx( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_0xa9xxx->psz_text );
}

/* Chapter support */
static void MP4_FreeBox_chpl( MP4_Box_t *p_box )
{
    MP4_Box_data_chpl_t *p_chpl = p_box->data.p_chpl;
    for( unsigned i = 0; i < p_chpl->i_chapter; i++ )
        free( p_chpl->chapter[i].psz_name );
}

static int MP4_ReadBox_chpl( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_Box_data_chpl_t *p_chpl;
    uint32_t i_dummy;
    VLC_UNUSED(i_dummy);
    int i;
    MP4_READBOX_ENTER( MP4_Box_data_chpl_t );

    p_chpl = p_box->data.p_chpl;

    MP4_GETVERSIONFLAGS( p_chpl );

    MP4_GET4BYTES( i_dummy );

    MP4_GET1BYTE( p_chpl->i_chapter );

    for( i = 0; i < p_chpl->i_chapter; i++ )
    {
        uint64_t i_start;
        uint8_t i_len;
        int i_copy;
        if ( i_read < 9 )
            break;
        MP4_GET8BYTES( i_start );
        MP4_GET1BYTE( i_len );

        p_chpl->chapter[i].psz_name = (char *)malloc( i_len + 1 );
        if( !p_chpl->chapter[i].psz_name )
            MP4_READBOX_EXIT( 0 );

        i_copy = __MIN( i_len, i_read );
        if( i_copy > 0 )
            memcpy( p_chpl->chapter[i].psz_name, p_peek, i_copy );
        p_chpl->chapter[i].psz_name[i_copy] = '\0';
        p_chpl->chapter[i].i_start = i_start;

        p_peek += i_copy;
        i_read -= i_copy;
    }

    if ( i != p_chpl->i_chapter )
        p_chpl->i_chapter = i;

    /* Bubble sort by increasing start date */
    do
    {
        for( i = 0; i < p_chpl->i_chapter - 1; i++ )
        {
            if( p_chpl->chapter[i].i_start > p_chpl->chapter[i+1].i_start )
            {
                char *psz = p_chpl->chapter[i+1].psz_name;
                int64_t i64 = p_chpl->chapter[i+1].i_start;

                p_chpl->chapter[i+1].psz_name = p_chpl->chapter[i].psz_name;
                p_chpl->chapter[i+1].i_start = p_chpl->chapter[i].i_start;

                p_chpl->chapter[i].psz_name = psz;
                p_chpl->chapter[i].i_start = i64;

                i = -1;
                break;
            }
        }
    } while( i == -1 );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "read box: \"chpl\" %d chapters",
                       p_chpl->i_chapter );
#endif
    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_tref_generic( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_tref_generic_t );

    p_box->data.p_tref_generic->i_track_ID = NULL;
    p_box->data.p_tref_generic->i_entry_count = i_read / sizeof(uint32_t);
    if( p_box->data.p_tref_generic->i_entry_count > 0 )
        p_box->data.p_tref_generic->i_track_ID = (uint32_t *)calloc( p_box->data.p_tref_generic->i_entry_count, sizeof(uint32_t) );
    if( p_box->data.p_tref_generic->i_track_ID == NULL )
        MP4_READBOX_EXIT( 0 );

    for( unsigned i = 0; i < p_box->data.p_tref_generic->i_entry_count; i++ )
    {
        MP4_GET4BYTES( p_box->data.p_tref_generic->i_track_ID[i] );
    }
#ifdef MP4_VERBOSE
        msg_Dbg( p_stream, "read box: \"chap\" %d references",
                 p_box->data.p_tref_generic->i_entry_count );
#endif

    MP4_READBOX_EXIT( 1 );
}
static void MP4_FreeBox_tref_generic( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_tref_generic->i_track_ID );
}

static int MP4_ReadBox_meta( stream_t *p_stream, MP4_Box_t *p_box )
{
    uint8_t meta_data[8];
    int i_actually_read;

    // skip over box header
    i_actually_read = stream_Read( p_stream, meta_data, 8 );
    if( i_actually_read < 8 )
        return 0;

    if ( !p_box->p_father )
        return 0;

    switch( p_box->p_father->i_type )
    {
    case ATOM_udta: /* itunes udta/meta */
        /* meta content starts with a 4 byte version/flags value (should be 0) */
        i_actually_read = stream_Read( p_stream, meta_data, 4 );
        if( i_actually_read < 4 )
            return 0;

        /* then it behaves like a container */
        return MP4_ReadBoxContainerRaw( p_stream, p_box );

    default: /* regular meta atom */

        i_actually_read = stream_Read( p_stream, meta_data, 8 );
        if( i_actually_read < 8 )
            return 0;

        /* Mandatory */
        if ( VLC_FOURCC( meta_data[4], meta_data[5], meta_data[6], meta_data[7] ) != ATOM_hdlr )
            return 0;

        //ft
    }
    return 1;
}

static int MP4_ReadBox_iods( stream_t *p_stream, MP4_Box_t *p_box )
{
    char i_unused;
    VLC_UNUSED(i_unused);

    MP4_READBOX_ENTER( MP4_Box_data_iods_t );
    MP4_GETVERSIONFLAGS( p_box->data.p_iods );

    MP4_GET1BYTE( i_unused ); /* tag */
    MP4_GET1BYTE( i_unused ); /* length */

    MP4_GET2BYTES( p_box->data.p_iods->i_object_descriptor ); /* 10bits, 6 other bits
                                                              are used for other flags */
    MP4_GET1BYTE( p_box->data.p_iods->i_OD_profile_level );
    MP4_GET1BYTE( p_box->data.p_iods->i_scene_profile_level );
    MP4_GET1BYTE( p_box->data.p_iods->i_audio_profile_level );
    MP4_GET1BYTE( p_box->data.p_iods->i_visual_profile_level );
    MP4_GET1BYTE( p_box->data.p_iods->i_graphics_profile_level );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream,
             "read box: \"iods\" objectDescriptorId: %i, OD: %i, scene: %i, audio: %i, "
             "visual: %i, graphics: %i",
             p_box->data.p_iods->i_object_descriptor >> 6,
             p_box->data.p_iods->i_OD_profile_level,
             p_box->data.p_iods->i_scene_profile_level,
             p_box->data.p_iods->i_audio_profile_level,
             p_box->data.p_iods->i_visual_profile_level,
             p_box->data.p_iods->i_graphics_profile_level );
#endif

    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_pasp( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_pasp_t );

    MP4_GET4BYTES( p_box->data.p_pasp->i_horizontal_spacing );
    MP4_GET4BYTES( p_box->data.p_pasp->i_vertical_spacing );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream,
             "read box: \"paps\" %dx%d",
             p_box->data.p_pasp->i_horizontal_spacing,
             p_box->data.p_pasp->i_vertical_spacing);
#endif

    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_mehd( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_mehd_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_mehd );
    if( p_box->data.p_mehd->i_version == 1 )
        MP4_GET8BYTES( p_box->data.p_mehd->i_fragment_duration );
    else /* version == 0 */
        MP4_GET4BYTES( p_box->data.p_mehd->i_fragment_duration );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream,
             "read box: \"mehd\" frag dur. %" PRIu64"",
             p_box->data.p_mehd->i_fragment_duration );
#endif

    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_trex( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_trex_t );
    MP4_GETVERSIONFLAGS( p_box->data.p_trex );

    MP4_GET4BYTES( p_box->data.p_trex->i_track_ID );
    MP4_GET4BYTES( p_box->data.p_trex->i_default_sample_description_index );
    MP4_GET4BYTES( p_box->data.p_trex->i_default_sample_duration );
    MP4_GET4BYTES( p_box->data.p_trex->i_default_sample_size );
    MP4_GET4BYTES( p_box->data.p_trex->i_default_sample_flags );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream,
             "read box: \"trex\" trackID: %" PRIu32"",
             p_box->data.p_trex->i_track_ID );
#endif

    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_sdtp( stream_t *p_stream, MP4_Box_t *p_box )
{
    uint32_t i_sample_count;
    MP4_READBOX_ENTER( MP4_Box_data_sdtp_t );
    MP4_Box_data_sdtp_t *p_sdtp = p_box->data.p_sdtp;
    MP4_GETVERSIONFLAGS( p_box->data.p_sdtp );
    i_sample_count = i_read;

    p_sdtp->p_sample_table = (uint8_t *)calloc( i_sample_count, 1 );

    if( !p_sdtp->p_sample_table )
        MP4_READBOX_EXIT( 0 );

    for( uint32_t i = 0; i < i_sample_count; i++ )
        MP4_GET1BYTE( p_sdtp->p_sample_table[i] );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream, "i_sample_count is %" PRIu32"", i_sample_count );
    if ( i_sample_count > 3 )
        msg_Dbg( p_stream,
             "read box: \"sdtp\" head: %" PRIx8" %" PRIx8" %" PRIx8" %" PRIx8"",
                 p_sdtp->p_sample_table[0],
                 p_sdtp->p_sample_table[1],
                 p_sdtp->p_sample_table[2],
                 p_sdtp->p_sample_table[3] );
#endif

    MP4_READBOX_EXIT( 1 );
}

static void MP4_FreeBox_sdtp( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_sdtp->p_sample_table );
}

static int MP4_ReadBox_mfro( stream_t *p_stream, MP4_Box_t *p_box )
{
    MP4_READBOX_ENTER( MP4_Box_data_mfro_t );

    MP4_GETVERSIONFLAGS( p_box->data.p_mfro );
    MP4_GET4BYTES( p_box->data.p_mfro->i_size );

#ifdef MP4_VERBOSE
    msg_Dbg( p_stream,
             "read box: \"mfro\" size: %" PRIu32"",
             p_box->data.p_mfro->i_size);
#endif

    MP4_READBOX_EXIT( 1 );
}

static int MP4_ReadBox_tfra( stream_t *p_stream, MP4_Box_t *p_box )
{
    uint32_t i_number_of_entries;
    MP4_READBOX_ENTER( MP4_Box_data_tfra_t );
    MP4_Box_data_tfra_t *p_tfra = p_box->data.p_tfra;
    MP4_GETVERSIONFLAGS( p_box->data.p_tfra );

    MP4_GET4BYTES( p_tfra->i_track_ID );
    uint32_t i_lengths = 0;
    int i_fields_length = 0;
    MP4_GET4BYTES( i_lengths );
    MP4_GET4BYTES( p_tfra->i_number_of_entries );
    i_number_of_entries = p_tfra->i_number_of_entries;
    p_tfra->i_length_size_of_traf_num = i_lengths >> 4;
    p_tfra->i_length_size_of_trun_num = ( i_lengths & 0x0c ) >> 2;
    p_tfra->i_length_size_of_sample_num = i_lengths & 0x03;

    size_t size = 4 + 4*p_tfra->i_version; /* size in {4, 8} */
    p_tfra->p_time = (uint32_t *)calloc( i_number_of_entries, size );
    p_tfra->p_moof_offset = (uint32_t *)calloc( i_number_of_entries, size );

    size = 1 + p_tfra->i_length_size_of_traf_num; /* size in [|1, 4|] */
    p_tfra->p_traf_number = (uint8_t *)calloc( i_number_of_entries, size );
    size = 1 + p_tfra->i_length_size_of_trun_num;
    p_tfra->p_trun_number = (uint8_t *)calloc( i_number_of_entries, size );
    size = 1 + p_tfra->i_length_size_of_sample_num;
    p_tfra->p_sample_number = (uint8_t *)calloc( i_number_of_entries, size );

    if( !p_tfra->p_time || !p_tfra->p_moof_offset || !p_tfra->p_traf_number
                        || !p_tfra->p_trun_number || !p_tfra->p_sample_number )
        goto error;

    i_fields_length = 3 + p_tfra->i_length_size_of_traf_num
            + p_tfra->i_length_size_of_trun_num
            + p_tfra->i_length_size_of_sample_num;

    uint32_t i;
    for( i = 0; i < i_number_of_entries; i++ )
    {

        if( p_tfra->i_version == 1 )
        {
            if ( i_read < i_fields_length + 16 )
                break;
            MP4_GET8BYTES( p_tfra->p_time[i*2] );
            MP4_GET8BYTES( p_tfra->p_moof_offset[i*2] );
        }
        else
        {
            if ( i_read < i_fields_length + 8 )
                break;
            MP4_GET4BYTES( p_tfra->p_time[i] );
            MP4_GET4BYTES( p_tfra->p_moof_offset[i] );
        }
        switch (p_tfra->i_length_size_of_traf_num)
        {
            case 0:
                MP4_GET1BYTE( p_tfra->p_traf_number[i] );
                break;
            case 1:
                MP4_GET2BYTES( p_tfra->p_traf_number[i*2] );
                break;
            case 2:
                MP4_GET3BYTES( p_tfra->p_traf_number[i*3] );
                break;
            case 3:
                MP4_GET4BYTES( p_tfra->p_traf_number[i*4] );
                break;
            default:
                goto error;
        }

        switch (p_tfra->i_length_size_of_trun_num)
        {
            case 0:
                MP4_GET1BYTE( p_tfra->p_trun_number[i] );
                break;
            case 1:
                MP4_GET2BYTES( p_tfra->p_trun_number[i*2] );
                break;
            case 2:
                MP4_GET3BYTES( p_tfra->p_trun_number[i*3] );
                break;
            case 3:
                MP4_GET4BYTES( p_tfra->p_trun_number[i*4] );
                break;
            default:
                goto error;
        }

        switch (p_tfra->i_length_size_of_sample_num)
        {
            case 0:
                MP4_GET1BYTE( p_tfra->p_sample_number[i] );
                break;
            case 1:
                MP4_GET2BYTES( p_tfra->p_sample_number[i*2] );
                break;
            case 2:
                MP4_GET3BYTES( p_tfra->p_sample_number[i*3] );
                break;
            case 3:
                MP4_GET4BYTES( p_tfra->p_sample_number[i*4] );
                break;
            default:
                goto error;
        }
    }
    if ( i < i_number_of_entries )
        i_number_of_entries = i;

#ifdef MP4_VERBOSE
    if( p_tfra->i_version == 0 )
    {
        msg_Dbg( p_stream, "time[0]: %" PRIu32", moof_offset[0]: %" PRIx32"",
                         p_tfra->p_time[0], p_tfra->p_moof_offset[0] );

        msg_Dbg( p_stream, "time[1]: %" PRIu32", moof_offset[1]: %" PRIx32"",
                         p_tfra->p_time[1], p_tfra->p_moof_offset[1] );
    }
    else
    {
        msg_Dbg( p_stream, "time[0]: %" PRIu64", moof_offset[0]: %" PRIx64"",
                ((uint64_t *)(p_tfra->p_time))[0],
                ((uint64_t *)(p_tfra->p_moof_offset))[0] );

        msg_Dbg( p_stream, "time[1]: %" PRIu64", moof_offset[1]: %" PRIx64"",
                ((uint64_t *)(p_tfra->p_time))[1],
                ((uint64_t *)(p_tfra->p_moof_offset))[1] );
    }

    msg_Dbg( p_stream, "number_of_entries is %" PRIu32"", i_number_of_entries );
    msg_Dbg( p_stream, "track ID is: %" PRIu32"", p_tfra->i_track_ID );
#endif

    MP4_READBOX_EXIT( 1 );
error:
    MP4_READBOX_EXIT( 0 );
}

static void MP4_FreeBox_tfra( MP4_Box_t *p_box )
{
    FREENULL( p_box->data.p_tfra->p_time );
    FREENULL( p_box->data.p_tfra->p_moof_offset );
    FREENULL( p_box->data.p_tfra->p_traf_number );
    FREENULL( p_box->data.p_tfra->p_trun_number );
    FREENULL( p_box->data.p_tfra->p_sample_number );
}


/* For generic */
static int MP4_ReadBox_default( stream_t *p_stream, MP4_Box_t *p_box )
{
    if( !p_box->p_father )
    {
        goto unknown;
    }
    if( p_box->p_father->i_type == ATOM_stsd )
    {
        MP4_Box_t *p_mdia = MP4_BoxGet( p_box, "../../../.." );
        MP4_Box_t *p_hdlr;

        if( p_mdia == NULL || p_mdia->i_type != ATOM_mdia ||
            (p_hdlr = MP4_BoxGet( p_mdia, "hdlr" )) == NULL )
        {
            goto unknown;
        }
        switch( p_hdlr->data.p_hdlr->i_handler_type )
        {
            case ATOM_soun:
                return MP4_ReadBox_sample_soun( p_stream, p_box );
            case ATOM_vide:
                return MP4_ReadBox_sample_vide( p_stream, p_box );
            case ATOM_text:
                return MP4_ReadBox_sample_text( p_stream, p_box );
            case ATOM_tx3g:
            case ATOM_sbtl:
                return MP4_ReadBox_sample_tx3g( p_stream, p_box );
            default:
                msg_Warn( p_stream,
                          "unknown handler type in stsd (incompletely loaded)" );
                return 1;
        }
    }

unknown:
    if MP4_BOX_TYPE_ASCII()
        msg_Warn( p_stream,
                "unknown box type %4.4s (incompletely loaded)",
                (char*)&p_box->i_type );
    else
        msg_Warn( p_stream,
                "unknown box type c%3.3s (incompletely loaded)",
                (char*)&p_box->i_type+1 );

    return 1;
}

/**** ------------------------------------------------------------------- ****/
/****                   "Higher level" Functions                          ****/
/**** ------------------------------------------------------------------- ****/

static const struct
{
    uint32_t i_type;
    int  (*MP4_ReadBox_function )( stream_t *p_stream, MP4_Box_t *p_box );
    void (*MP4_FreeBox_function )( MP4_Box_t *p_box );
    uint32_t i_parent; /* set parent to restrict, duplicating if needed; 0 for any */
} MP4_Box_Function [] =
{
    /* Containers */
    { ATOM_moov,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, 0 },
    { ATOM_trak,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, ATOM_moov },
    { ATOM_mdia,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, ATOM_trak },
    { ATOM_moof,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, 0 },
    { ATOM_minf,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, ATOM_mdia },
    { ATOM_stbl,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, ATOM_minf },
    { ATOM_dinf,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, ATOM_minf },
    { ATOM_dinf,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, ATOM_meta },
    { ATOM_edts,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, ATOM_trak },
    { ATOM_udta,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, 0 },
    { ATOM_nmhd,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, ATOM_minf },
    { ATOM_hnti,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, ATOM_udta },
    { ATOM_rmra,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, ATOM_moov },
    { ATOM_rmda,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, ATOM_rmra },
    { ATOM_tref,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, ATOM_trak },
    { ATOM_gmhd,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, ATOM_minf },
    { ATOM_wave,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, 0 },
    { ATOM_ilst,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, ATOM_meta },
    { ATOM_mvex,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, ATOM_moov },
    { ATOM_mvex,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, ATOM_ftyp },

    /* specific box */
    { ATOM_ftyp,    MP4_ReadBox_ftyp,         MP4_FreeBox_ftyp, 0 },
    { ATOM_cmov,    MP4_ReadBox_cmov,         MP4_FreeBox_Common, 0 },
    { ATOM_mvhd,    MP4_ReadBox_mvhd,         MP4_FreeBox_Common, ATOM_moov },
    { ATOM_tkhd,    MP4_ReadBox_tkhd,         MP4_FreeBox_Common, ATOM_trak },
    { ATOM_mdhd,    MP4_ReadBox_mdhd,         MP4_FreeBox_Common, ATOM_mdia },
    { ATOM_hdlr,    MP4_ReadBox_hdlr,         MP4_FreeBox_hdlr,   ATOM_mdia },
    { ATOM_hdlr,    MP4_ReadBox_hdlr,         MP4_FreeBox_hdlr,   ATOM_meta },
    { ATOM_vmhd,    MP4_ReadBox_vmhd,         MP4_FreeBox_Common, ATOM_minf },
    { ATOM_smhd,    MP4_ReadBox_smhd,         MP4_FreeBox_Common, ATOM_minf },
    { ATOM_hmhd,    MP4_ReadBox_hmhd,         MP4_FreeBox_Common, ATOM_minf },
    { ATOM_url,     MP4_ReadBox_url,          MP4_FreeBox_url, 0 },
    { ATOM_urn,     MP4_ReadBox_urn,          MP4_FreeBox_urn, 0 },
    { ATOM_dref,    MP4_ReadBox_dref,         MP4_FreeBox_Common, 0 },
    { ATOM_stts,    MP4_ReadBox_stts,         MP4_FreeBox_stts,   ATOM_stbl },
    { ATOM_ctts,    MP4_ReadBox_ctts,         MP4_FreeBox_ctts,   ATOM_stbl },
    { ATOM_stsd,    MP4_ReadBox_stsd,         MP4_FreeBox_Common, ATOM_stbl },
    { ATOM_stsz,    MP4_ReadBox_stsz,         MP4_FreeBox_stsz,   ATOM_stbl },
    { ATOM_stsc,    MP4_ReadBox_stsc,         MP4_FreeBox_stsc,   ATOM_stbl },
    { ATOM_stco,    MP4_ReadBox_stco_co64,    MP4_FreeBox_stco_co64, ATOM_stbl },
    { ATOM_co64,    MP4_ReadBox_stco_co64,    MP4_FreeBox_stco_co64, ATOM_stbl },
    { ATOM_stss,    MP4_ReadBox_stss,         MP4_FreeBox_stss, ATOM_stbl },
    { ATOM_stsh,    MP4_ReadBox_stsh,         MP4_FreeBox_stsh, ATOM_stbl },
    { ATOM_stdp,    MP4_ReadBox_stdp,         MP4_FreeBox_stdp, 0 },
    { ATOM_padb,    MP4_ReadBox_padb,         MP4_FreeBox_padb, 0 },
    { ATOM_elst,    MP4_ReadBox_elst,         MP4_FreeBox_elst, ATOM_edts },
    { ATOM_cprt,    MP4_ReadBox_cprt,         MP4_FreeBox_cprt, 0 },
    { ATOM_esds,    MP4_ReadBox_esds,         MP4_FreeBox_esds, 0 },
    { ATOM_dcom,    MP4_ReadBox_dcom,         MP4_FreeBox_Common, 0 },
    { ATOM_cmvd,    MP4_ReadBox_cmvd,         MP4_FreeBox_cmvd, 0 },
    { ATOM_avcC,    MP4_ReadBox_avcC,         MP4_FreeBox_avcC, 0 },
    { ATOM_hvcC,    MP4_ReadBox_hvcC,         MP4_FreeBox_hvcC, 0 },
    { ATOM_dac3,    MP4_ReadBox_dac3,         MP4_FreeBox_Common, 0 },
    { ATOM_dvc1,    MP4_ReadBox_dvc1,         MP4_FreeBox_Common, 0 },
    { ATOM_enda,    MP4_ReadBox_enda,         MP4_FreeBox_Common, 0 },
    { ATOM_iods,    MP4_ReadBox_iods,         MP4_FreeBox_Common, 0 },
    { ATOM_pasp,    MP4_ReadBox_pasp,         MP4_FreeBox_Common, 0 },

    /* Nothing to do with this box */
    { ATOM_mdat,    MP4_ReadBoxSkip,          MP4_FreeBox_Common, 0 },
    { ATOM_skip,    MP4_ReadBoxSkip,          MP4_FreeBox_Common, 0 },
    { ATOM_free,    MP4_ReadBoxSkip,          MP4_FreeBox_Common, 0 },
    { ATOM_wide,    MP4_ReadBoxSkip,          MP4_FreeBox_Common, 0 },
    { ATOM_binm,    MP4_ReadBoxSkip,          MP4_FreeBox_Common, 0 },

    /* Subtitles */
    { ATOM_tx3g,    MP4_ReadBox_sample_tx3g,      MP4_FreeBox_Common, 0 },
    //{ ATOM_text,    MP4_ReadBox_sample_text,      MP4_FreeBox_Common, 0 },

    /* for codecs */
    { ATOM_soun,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_lpcm,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_ms02,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_ms11,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_ms55,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM__mp3,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_mp4a,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_twos,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_sowt,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_QDMC,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_QDM2,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_ima4,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_IMA4,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_dvi,     MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_alaw,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_ulaw,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_raw,     MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_MAC3,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_MAC6,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_Qclp,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_samr,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_sawb,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_OggS,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },
    { ATOM_alac,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, ATOM_stsd },

    { ATOM_drmi,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_vide,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_mp4v,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_SVQ1,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_SVQ3,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_ZyGo,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_DIVX,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_XVID,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_h263,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_s263,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_cvid,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_3IV1,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_3iv1,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_3IV2,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_3iv2,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_3IVD,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_3ivd,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_3VID,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_3vid,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_mjpa,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_mjpb,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_qdrw,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_mp2v,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_hdv2,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },

    { ATOM_mjqt,    MP4_ReadBox_default,      NULL, 0 }, /* found in mjpa/b */
    { ATOM_mjht,    MP4_ReadBox_default,      NULL, 0 },

    { ATOM_dvc,     MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_dvp,     MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_dv5n,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_dv5p,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_VP31,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_vp31,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_h264,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },

    { ATOM_jpeg,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },
    { ATOM_avc1,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, ATOM_stsd },

    { ATOM_yv12,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, 0 },
    { ATOM_yuv2,    MP4_ReadBox_sample_vide,  MP4_FreeBox_sample_vide, 0 },

    { ATOM_mp4s,    MP4_ReadBox_sample_mp4s,  MP4_FreeBox_Common, 0 },

    /* XXX there is 2 box where we could find this entry stbl and tref*/
    { ATOM_hint,    MP4_ReadBox_default,      MP4_FreeBox_Common, 0 },

    /* found in tref box */
    { ATOM_dpnd,    MP4_ReadBox_default,      NULL, 0 },
    { ATOM_ipir,    MP4_ReadBox_default,      NULL, 0 },
    { ATOM_mpod,    MP4_ReadBox_default,      NULL, 0 },
    { ATOM_chap,    MP4_ReadBox_tref_generic, MP4_FreeBox_tref_generic, 0 },

    /* found in hnti */
    { ATOM_rtp,     MP4_ReadBox_default,      NULL, 0 },

    /* found in rmra/rmda */
    { ATOM_rdrf,    MP4_ReadBox_rdrf,         MP4_FreeBox_rdrf  , ATOM_rmda },
    { ATOM_rmdr,    MP4_ReadBox_rmdr,         MP4_FreeBox_Common, ATOM_rmda },
    { ATOM_rmqu,    MP4_ReadBox_rmqu,         MP4_FreeBox_Common, ATOM_rmda },
    { ATOM_rmvc,    MP4_ReadBox_rmvc,         MP4_FreeBox_Common, ATOM_rmda },

    { ATOM_drms,    MP4_ReadBox_sample_soun,  MP4_FreeBox_sample_soun, 0 },
    { ATOM_sinf,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, 0 },
    { ATOM_schi,    MP4_ReadBoxContainer,     MP4_FreeBox_Common, 0 },
    { ATOM_user,    MP4_ReadBox_drms,         MP4_FreeBox_Common, 0 },
    { ATOM_key,     MP4_ReadBox_drms,         MP4_FreeBox_Common, 0 },
    { ATOM_iviv,    MP4_ReadBox_drms,         MP4_FreeBox_Common, 0 },
    { ATOM_priv,    MP4_ReadBox_drms,         MP4_FreeBox_Common, 0 },
    { ATOM_frma,    MP4_ReadBox_frma,         MP4_FreeBox_Common, 0 },
    { ATOM_skcr,    MP4_ReadBox_skcr,         MP4_FreeBox_Common, 0 },

    /* ilst meta tags */
    { ATOM_0xa9ART, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_ilst },
    { ATOM_0xa9alb, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_ilst },
    { ATOM_0xa9cmt, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_ilst },
    { ATOM_0xa9com, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_ilst },
    { ATOM_0xa9day, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_ilst },
    { ATOM_0xa9des, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_ilst },
    { ATOM_0xa9enc, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_ilst },
    { ATOM_0xa9gen, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_ilst },
    { ATOM_0xa9grp, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_ilst },
    { ATOM_0xa9lyr, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_ilst },
    { ATOM_0xa9nam, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_ilst },
    { ATOM_0xa9too, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_ilst },
    { ATOM_0xa9trk, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_ilst },
    { ATOM_0xa9wrt, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_ilst },
    { ATOM_covr,    MP4_ReadBoxContainer,     MP4_FreeBox_Common,  ATOM_ilst },
    { ATOM_gnre,    MP4_ReadBox_gnre,         MP4_FreeBox_Common,  ATOM_ilst },

    /* udta */
    { ATOM_0xa9ART, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9alb, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9ard, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9arg, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9aut, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9cak, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9cmt, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9con, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9com, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9cpy, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9day, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9des, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9dir, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9dis, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9dsa, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9fmt, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9gen, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9grp, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9hst, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9inf, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9isr, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9lab, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9lal, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9lnt, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9lyr, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9mak, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9mal, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9mod, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9ope, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9phg, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9PRD, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9prd, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9prf, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9pub, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9req, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9sne, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9snm, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9sol, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9src, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9st3, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9swr, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9thx, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9too, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9trk, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9url, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9wrn, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9xpd, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_0xa9xyz, MP4_ReadBox_0xa9xxx,      MP4_FreeBox_0xa9xxx, ATOM_udta },
    { ATOM_chpl,    MP4_ReadBox_chpl,         MP4_FreeBox_chpl,    ATOM_udta },
    { ATOM_covr,    MP4_ReadBoxContainer,     MP4_FreeBox_Common,  ATOM_udta },
    { ATOM_gnre,    MP4_ReadBox_gnre,         MP4_FreeBox_Common,  ATOM_udta },
    { ATOM_name,    MP4_ReadBox_name,         MP4_FreeBox_name,    ATOM_udta },
    { ATOM_trkn,    MP4_ReadBox_trkn,         MP4_FreeBox_Common,  ATOM_udta },

    /* iTunes/Quicktime meta info */
    { ATOM_meta,    MP4_ReadBox_meta,         MP4_FreeBox_Common,  0 },
    { ATOM_data,    MP4_ReadBox_data,         MP4_FreeBox_data,    0 },

    /* found in smoothstreaming */
    { ATOM_traf,    MP4_ReadBoxContainer,     MP4_FreeBox_Common,  ATOM_moof },
    { ATOM_mfra,    MP4_ReadBoxContainer,     MP4_FreeBox_Common,  0 },
    { ATOM_mfhd,    MP4_ReadBox_mfhd,         MP4_FreeBox_Common,  ATOM_moof },
    { ATOM_sidx,    MP4_ReadBox_sidx,         MP4_FreeBox_sidx,    0 },
    { ATOM_tfhd,    MP4_ReadBox_tfhd,         MP4_FreeBox_Common,  ATOM_traf },
    { ATOM_trun,    MP4_ReadBox_trun,         MP4_FreeBox_trun,    ATOM_traf },
    { ATOM_trex,    MP4_ReadBox_trex,         MP4_FreeBox_Common,  ATOM_mvex },
    { ATOM_mehd,    MP4_ReadBox_mehd,         MP4_FreeBox_Common,  ATOM_mvex },
    { ATOM_sdtp,    MP4_ReadBox_sdtp,         MP4_FreeBox_sdtp,    0 },
    { ATOM_tfra,    MP4_ReadBox_tfra,         MP4_FreeBox_tfra,    ATOM_mfra },
    { ATOM_mfro,    MP4_ReadBox_mfro,         MP4_FreeBox_Common,  ATOM_mfra },
    { ATOM_uuid,    MP4_ReadBox_uuid,         MP4_FreeBox_uuid,    0 },

    /* Last entry */
    { 0,              MP4_ReadBox_default,      NULL, 0 }
};


/*****************************************************************************
 * MP4_ReadBox : parse the actual box and the children
 *  XXX : Do not go to the next box
 *****************************************************************************/
static MP4_Box_t *MP4_ReadBox( stream_t *p_stream, MP4_Box_t *p_father )
{
    MP4_Box_t *p_box = (MP4_Box_t *)calloc( 1, sizeof( MP4_Box_t ) ); /* Needed to ensure simple on error handler */
    unsigned int i_index;

    if( p_box == NULL )
        return NULL;

    if( !MP4_ReadBoxCommon( p_stream, p_box ) )
    {
        msg_Warn( p_stream, "cannot read one box" );
        free( p_box );
        return NULL;
    }

    if( p_father && p_father->i_size > 0 &&
        p_father->i_pos + p_father->i_size < p_box->i_pos + p_box->i_size )
    {
        msg_Dbg( p_stream, "out of bound child" );
        free( p_box );
        return NULL;
    }

    if( !p_box->i_size )
    {
        msg_Dbg( p_stream, "found an empty box (null size)" );
        free( p_box );
        return NULL;
    }
    p_box->p_father = p_father;

    /* Now search function to call */
    for( i_index = 0; ; i_index++ )
    {
        if ( MP4_Box_Function[i_index].i_parent &&
             p_box->p_father &&
             p_box->p_father->i_type != MP4_Box_Function[i_index].i_parent )
            continue;

        if( ( MP4_Box_Function[i_index].i_type == p_box->i_type )||
            ( MP4_Box_Function[i_index].i_type == 0 ) )
        {
            break;
        }
    }

    if( !(MP4_Box_Function[i_index].MP4_ReadBox_function)( p_stream, p_box ) )
    {
        MP4_BoxFree( p_stream, p_box );
        return NULL;
    }

    return p_box;
}

/*****************************************************************************
 * MP4_FreeBox : free memory after read with MP4_ReadBox and all
 * the children
 *****************************************************************************/
void MP4_BoxFree( stream_t *s, MP4_Box_t *p_box )
{
    unsigned int i_index;
    MP4_Box_t    *p_child;

    if( !p_box )
        return; /* hehe */

    for( p_child = p_box->p_first; p_child != NULL; )
    {
        MP4_Box_t *p_next;

        p_next = p_child->p_next;
        MP4_BoxFree( s, p_child );
        p_child = p_next;
    }

    /* Now search function to call */
    if( p_box->data.p_payload )
    {
        for( i_index = 0; ; i_index++ )
        {
            if ( MP4_Box_Function[i_index].i_parent &&
                 p_box->p_father &&
                 p_box->p_father->i_type != MP4_Box_Function[i_index].i_parent )
                continue;

            if( ( MP4_Box_Function[i_index].i_type == p_box->i_type )||
                ( MP4_Box_Function[i_index].i_type == 0 ) )
            {
                break;
            }
        }
        if( MP4_Box_Function[i_index].MP4_FreeBox_function == NULL )
        {
            /* Should not happen */
            if MP4_BOX_TYPE_ASCII()
                msg_Warn( s,
                        "cannot free box %4.4s, type unknown",
                        (char*)&p_box->i_type );
            else
                msg_Warn( s,
                        "cannot free box c%3.3s, type unknown",
                        (char*)&p_box->i_type+1 );
        }
        else
        {
            MP4_Box_Function[i_index].MP4_FreeBox_function( p_box );
        }
        free( p_box->data.p_payload );
    }
    free( p_box );
}

/* SmooBox is a very simple MP4 box, VLC specific, used only for the stream_filter to
 * send information to the demux. SmooBox is actually a simplified moov box (we wanted
 * to avoid the hassle of building a moov box at the stream_filter level) */
MP4_Box_t *MP4_BoxGetSmooBox( stream_t *s )
{
    /* p_chunk is a virtual root container for the smoo box */
    MP4_Box_t *p_chunk;
    MP4_Box_t *p_smoo;

    p_chunk = (MP4_Box_t *)calloc( 1, sizeof( MP4_Box_t ) );
    if( unlikely( p_chunk == NULL ) )
        return NULL;

    p_chunk->i_type = ATOM_root;
    p_chunk->i_shortsize = 1;

    p_smoo = MP4_ReadBox( s, p_chunk );
    if( !p_smoo || p_smoo->i_type != ATOM_uuid || CmpUUID( &p_smoo->i_uuid, &SmooBoxUUID ) )
    {
        msg_Warn( s, "no smoo box found!");
        goto error;
    }

    p_chunk->p_first = p_smoo;
    p_chunk->p_last = p_smoo;

    return p_chunk;

error:
    free( p_chunk );
    return NULL;
}

MP4_Box_t *MP4_BoxGetNextChunk( stream_t *s )
{
    /* p_chunk is a virtual root container for the moof and mdat boxes */
    MP4_Box_t *p_chunk;
    MP4_Box_t *p_tmp_box = NULL;

    p_tmp_box = (MP4_Box_t *)calloc( 1, sizeof( MP4_Box_t ) );
    if( unlikely( p_tmp_box == NULL ) )
        return NULL;

    /* We might get a ftyp box or a SmooBox */
    MP4_ReadBoxCommon( s, p_tmp_box );

    if( (p_tmp_box->i_type == ATOM_uuid && !CmpUUID( &p_tmp_box->i_uuid, &SmooBoxUUID )) )
    {
        free( p_tmp_box );
        return MP4_BoxGetSmooBox( s );
    }
    else if( p_tmp_box->i_type == ATOM_ftyp )
    {
        free( p_tmp_box );
        return MP4_BoxGetRoot( s );
    }
    free( p_tmp_box );

    p_chunk = (MP4_Box_t *)calloc( 1, sizeof( MP4_Box_t ) );
    if( unlikely( p_chunk == NULL ) )
        return NULL;

    p_chunk->i_type = ATOM_root;
    p_chunk->i_shortsize = 1;

    MP4_ReadBoxContainerChildren( s, p_chunk, ATOM_moof );

    p_tmp_box = p_chunk->p_first;
    while( p_tmp_box )
    {
        p_chunk->i_size += p_tmp_box->i_size;
        p_tmp_box = p_tmp_box->p_next;
    }

    return p_chunk;
}

/*****************************************************************************
 * MP4_BoxGetRoot : Parse the entire file, and create all boxes in memory
 *****************************************************************************
 *  The first box is a virtual box "root" and is the father for all first
 *  level boxes for the file, a sort of virtual contener
 *****************************************************************************/
MP4_Box_t *MP4_BoxGetRoot( stream_t *s )
{
    MP4_Box_t *p_root;
    stream_t *p_stream;
    int i_result;

    p_root = (MP4_Box_t *)malloc( sizeof( MP4_Box_t ) );
    if( p_root == NULL )
        return NULL;

    p_root->i_pos = 0;
    p_root->i_type = ATOM_root;
    p_root->i_shortsize = 1;
    /* could be a DASH stream for exemple, 0 means unknown or infinite size */
    p_root->i_size = 0;
    CreateUUID( &p_root->i_uuid, p_root->i_type );

    p_root->data.p_payload = NULL;
    p_root->p_father    = NULL;
    p_root->p_first     = NULL;
    p_root->p_last      = NULL;
    p_root->p_next      = NULL;

    p_stream = s;

    /* First get the moov */
    i_result = MP4_ReadBoxContainerChildren( p_stream, p_root, ATOM_moov );

    if( !i_result )
        goto error;
    /* If there is a mvex box, it means fragmented MP4, and we're done */
    else if( MP4_BoxCount( p_root, "moov/mvex" ) > 0 )
        return p_root;

    p_root->i_size = stream_Size( s );
    if( stream_Tell( s ) + 8 < stream_Size( s ) )
    {
        /* Get the rest of the file */
        i_result = MP4_ReadBoxContainerRaw( p_stream, p_root );

        if( !i_result )
            goto error;
    }

    MP4_Box_t *p_moov;
    MP4_Box_t *p_cmov;

    /* check if there is a cmov, if so replace
      compressed moov by  uncompressed one */
    if( ( ( p_moov = MP4_BoxGet( p_root, "moov" ) ) &&
          ( p_cmov = MP4_BoxGet( p_root, "moov/cmov" ) ) ) ||
        ( ( p_moov = MP4_BoxGet( p_root, "foov" ) ) &&
          ( p_cmov = MP4_BoxGet( p_root, "foov/cmov" ) ) ) )
    {
        /* rename the compressed moov as a box to skip */
        p_moov->i_type = ATOM_skip;

        /* get uncompressed p_moov */
        p_moov = p_cmov->data.p_cmov->p_moov;
        p_cmov->data.p_cmov->p_moov = NULL;

        /* make p_root father of this new moov */
        p_moov->p_father = p_root;

        /* insert this new moov box as first child of p_root */
        p_moov->p_next = p_root->p_first;
        p_root->p_first = p_moov;
    }

    return p_root;

error:
    free( p_root );
    stream_Seek( p_stream, 0 );
    return NULL;
}


static void MP4_BoxDumpStructure_Internal( stream_t *s,
                                    MP4_Box_t *p_box, unsigned int i_level )
{
    MP4_Box_t *p_child;

    if( !i_level )
    {
        if MP4_BOX_TYPE_ASCII()
            msg_Dbg( s, "dumping root Box \"%4.4s\"",
                              (char*)&p_box->i_type );
        else
            msg_Dbg( s, "dumping root Box \"c%3.3s\"",
                              (char*)&p_box->i_type+1 );
    }
    else
    {
        char str[512];
        if( i_level >= (sizeof(str) - 1)/4 )
            return;

        memset( str, ' ', sizeof(str) );
        for( unsigned i = 0; i < i_level; i++ )
        {
            str[i*4] = '|';
        }
        if( MP4_BOX_TYPE_ASCII() )
            snprintf( &str[i_level * 4], sizeof(str) - 4*i_level,
                      "+ %4.4s size %" PRIu64" offset %ld",
                        (char*)&p_box->i_type, p_box->i_size, p_box->i_pos );
        else
            snprintf( &str[i_level * 4], sizeof(str) - 4*i_level,
                      "+ c%3.3s size %" PRIu64" offset %ld",
                        (char*)&p_box->i_type+1, p_box->i_size, p_box->i_pos );
        msg_Dbg( s, "%s", str );
    }
    p_child = p_box->p_first;
    while( p_child )
    {
        MP4_BoxDumpStructure_Internal( s, p_child, i_level + 1 );
        p_child = p_child->p_next;
    }
}

void MP4_BoxDumpStructure( stream_t *s, MP4_Box_t *p_box )
{
    MP4_BoxDumpStructure_Internal( s, p_box, 0 );
}


/*****************************************************************************
 *****************************************************************************
 **
 **  High level methods to acces an MP4 file
 **
 *****************************************************************************
 *****************************************************************************/
static void get_token( char **ppsz_path, char **ppsz_token, int *pi_number )
{
    size_t i_len ;
    if( !*ppsz_path[0] )
    {
        *ppsz_token = NULL;
        *pi_number = 0;
        return;
    }
    i_len = strcspn( *ppsz_path, "/[" );
    if( !i_len && **ppsz_path == '/' )
    {
        i_len = 1;
    }
    *ppsz_token = strndup( *ppsz_path, i_len );
    if( unlikely(!*ppsz_token) )
        abort();

    *ppsz_path += i_len;

    if( **ppsz_path == '[' )
    {
        (*ppsz_path)++;
        *pi_number = strtol( *ppsz_path, NULL, 10 );
        while( **ppsz_path && **ppsz_path != ']' )
        {
            (*ppsz_path)++;
        }
        if( **ppsz_path == ']' )
        {
            (*ppsz_path)++;
        }
    }
    else
    {
        *pi_number = 0;
    }
    while( **ppsz_path == '/' )
    {
        (*ppsz_path)++;
    }
}

static void MP4_BoxGet_Internal( MP4_Box_t **pp_result,
                          MP4_Box_t *p_box, const char *psz_fmt, va_list args)
{
    char *psz_dup;
    char *psz_path;
    char *psz_token;

    if( !p_box )
    {
        *pp_result = NULL;
        return;
    }

    if (vasprintf(&psz_path, psz_fmt, args) == -1)
        psz_path = NULL;

    if (!psz_path || !psz_path[0]) {
        free( psz_path );
        *pp_result = NULL;
        return;
    }

//    fprintf( stderr, "path:'%s'\n", psz_path );
    psz_dup = psz_path; /* keep this pointer, as it need to be unallocated */
    for( ; ; )
    {
        int i_number;

        get_token( &psz_path, &psz_token, &i_number );
//        fprintf( stderr, "path:'%s', token:'%s' n:%d\n",
//                 psz_path,psz_token,i_number );
        if( !psz_token )
        {
            free( psz_dup );
            *pp_result = p_box;
            return;
        }
        else
        if( !strcmp( psz_token, "/" ) )
        {
            /* Find root box */
            while( p_box && p_box->i_type != ATOM_root )
            {
                p_box = p_box->p_father;
            }
            if( !p_box )
            {
                goto error_box;
            }
        }
        else
        if( !strcmp( psz_token, "." ) )
        {
            /* Do nothing */
        }
        else
        if( !strcmp( psz_token, ".." ) )
        {
            p_box = p_box->p_father;
            if( !p_box )
            {
                goto error_box;
            }
        }
        else
        if( strlen( psz_token ) == 4 )
        {
            uint32_t i_fourcc;
            i_fourcc = VLC_FOURCC( psz_token[0], psz_token[1],
                                   psz_token[2], psz_token[3] );
            p_box = p_box->p_first;
            for( ; ; )
            {
                if( !p_box )
                {
                    goto error_box;
                }
                if( p_box->i_type == i_fourcc )
                {
                    if( !i_number )
                    {
                        break;
                    }
                    i_number--;
                }
                p_box = p_box->p_next;
            }
        }
        else
        if( *psz_token == '\0' )
        {
            p_box = p_box->p_first;
            for( ; ; )
            {
                if( !p_box )
                {
                    goto error_box;
                }
                if( !i_number )
                {
                    break;
                }
                i_number--;
                p_box = p_box->p_next;
            }
        }
        else
        {
//            fprintf( stderr, "Argg malformed token \"%s\"",psz_token );
            goto error_box;
        }

        FREENULL( psz_token );
    }

    return;

error_box:
    free( psz_token );
    free( psz_dup );
    *pp_result = NULL;
    return;
}

/*****************************************************************************
 * MP4_BoxGet: find a box given a path relative to p_box
 *****************************************************************************
 * Path Format: . .. / as usual
 *              [number] to specifie box number ex: trak[12]
 *
 * ex: /moov/trak[12]
 *     ../mdia
 *****************************************************************************/
MP4_Box_t *MP4_BoxGet( MP4_Box_t *p_box, const char *psz_fmt, ... )
{
    va_list args;
    MP4_Box_t *p_result;

    va_start( args, psz_fmt );
    MP4_BoxGet_Internal( &p_result, p_box, psz_fmt, args );
    va_end( args );

    return( p_result );
}

/*****************************************************************************
 * MP4_BoxCount: count box given a path relative to p_box
 *****************************************************************************
 * Path Format: . .. / as usual
 *              [number] to specifie box number ex: trak[12]
 *
 * ex: /moov/trak[12]
 *     ../mdia
 *****************************************************************************/
int MP4_BoxCount( MP4_Box_t *p_box, const char *psz_fmt, ... )
{
    va_list args;
    int     i_count;
    MP4_Box_t *p_result, *p_next;

    va_start( args, psz_fmt );
    MP4_BoxGet_Internal( &p_result, p_box, psz_fmt, args );
    va_end( args );
    if( !p_result )
    {
        return( 0 );
    }

    i_count = 1;
    for( p_next = p_result->p_next; p_next != NULL; p_next = p_next->p_next)
    {
        if( p_next->i_type == p_result->i_type)
        {
            i_count++;
        }
    }
    return( i_count );
}
