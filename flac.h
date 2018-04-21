/* public domain Simple, Minimalistic, FLAC encoder based on Flake
 *	©2018 Yuichiro Nakada
 *
 * Basic usage:
 *	flac_encode("music.flac", pcm, len, 44100, 16, 2, 9);
 *
 * */

/**
 * Flake: FLAC audio encoder
 * Copyright (c) 2006 Justin Ruggles
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// flake.h
#ifndef FLAKE_H
#define FLAKE_H

#define FLAKE_STRINGIFY(s)      FLAKE_TOSTRING(s)
#define FLAKE_TOSTRING(s) #s

#define FLAKE_VERSION		0.11+
#define FLAKE_IDENT		"Flake" FLAKE_STRINGIFY(FLAKE_VERSION)

#define FLAKE_ORDER_METHOD_MAX    0
#define FLAKE_ORDER_METHOD_EST    1
#define FLAKE_ORDER_METHOD_2LEVEL 2
#define FLAKE_ORDER_METHOD_4LEVEL 3
#define FLAKE_ORDER_METHOD_8LEVEL 4
#define FLAKE_ORDER_METHOD_SEARCH 5
#define FLAKE_ORDER_METHOD_LOG    6

#define FLAKE_STEREO_METHOD_INDEPENDENT  0
#define FLAKE_STEREO_METHOD_ESTIMATE     1

#define FLAKE_PREDICTION_NONE      0
#define FLAKE_PREDICTION_FIXED     1
#define FLAKE_PREDICTION_LEVINSON  2

typedef struct FlakeEncodeParams {

	// compression quality
	// set by user prior to calling flake_encode_init
	// standard values are 0 to 8
	// 0 is lower compression, faster encoding
	// 8 is higher compression, slower encoding
	// extended values 9 to 12 are slower and/or use
	// higher prediction orders
	int compression;

	// prediction order selection method
	// set by user prior to calling flake_encode_init
	// if set to less than 0, it is chosen based on compression.
	// valid values are 0 to 5
	// 0 = use maximum order only
	// 1 = use estimation
	// 2 = 2-level
	// 3 = 4-level
	// 4 = 8-level
	// 5 = full search
	// 6 = log search
	int order_method;

	// stereo decorrelation method
	// set by user prior to calling flake_encode_init
	// if set to less than 0, it is chosen based on compression.
	// valid values are 0 to 2
	// 0 = independent L+R channels
	// 1 = mid-side encoding
	int stereo_method;

	// block size in samples
	// set by the user prior to calling flake_encode_init
	// if set to 0, a block size is chosen based on block_time_ms
	// can also be changed by user before encoding a frame
	int block_size;

	// block time in milliseconds
	// set by the user prior to calling flake_encode_init
	// used to calculate block_size based on sample rate
	// can also be changed by user before encoding a frame
	int block_time_ms;

	// padding size in bytes
	// set by the user prior to calling flake_encode_init
	// if set to less than 0, defaults to 4096
	int padding_size;

	// maximum encoded frame size
	// this is set by flake_encode_init based on input audio format
	// it can be used by the user to allocate an output buffer
	int max_frame_size;

	// minimum prediction order
	// set by user prior to calling flake_encode_init
	// if set to less than 0, it is chosen based on compression.
	// valid values are 0 to 4 for fixed prediction and 1 to 32 for non-fixed
	int min_prediction_order;

	// maximum prediction order
	// set by user prior to calling flake_encode_init
	// if set to less than 0, it is chosen based on compression.
	// valid values are 0 to 4 for fixed prediction and 1 to 32 for non-fixed
	int max_prediction_order;

	// type of linear prediction
	// set by user prior to calling flake_encode_init
	// if set to less than 0, it is chosen based on compression.
	// 0 = fixed prediction
	// 1 = Levinson-Durbin recursion
	int prediction_type;

	// minimum partition order
	// set by user prior to calling flake_encode_init
	// if set to less than 0, it is chosen based on compression.
	// valid values are 0 to 8
	int min_partition_order;

	// maximum partition order
	// set by user prior to calling flake_encode_init
	// if set to less than 0, it is chosen based on compression.
	// valid values are 0 to 8
	int max_partition_order;

	// whether to use variable block sizes
	// set by user prior to calling flake_encode_init
	// 0 = fixed block size
	// 1 = variable block size
	int variable_block_size;

} FlakeEncodeParams;

typedef struct FlakeContext {

	// number of audio channels
	// set by user prior to calling flake_encode_init
	// valid values are 1 to 8
	int channels;

	// audio sample rate in Hz
	// set by user prior to calling flake_encode_init
	int sample_rate;

	// sample size in bits
	// set by user prior to calling flake_encode_init
	// only 16-bit is currently supported
	int bits_per_sample;

	// total stream samples
	// set by user prior to calling flake_encode_init
	// if 0, stream length is unknown
	unsigned int samples;

	FlakeEncodeParams params;

	// maximum frame size in bytes
	// set by flake_encode_init
	// this can be used to allocate memory for output
	int max_frame_size;

	// MD5 digest
	// set by flake_encode_close;
	unsigned char md5digest[16];

	// header bytes
	// allocated by flake_encode_init and freed by flake_encode_close
	unsigned char *header;

	// encoding context, which is hidden from the user
	// allocated by flake_encode_init and freed by flake_encode_close
	void *private_ctx;

} FlakeContext;

/**
 * Sets encoding defaults based on compression level
 * params->compression must be set prior to calling
 */
extern int flake_set_defaults(FlakeEncodeParams *params);

/**
 * Validates encoding parameters
 * @return -1 if error. 0 if ok. 1 if ok but non-Subset.
 */
extern int flake_validate_params(FlakeContext *s);

extern int flake_encode_init(FlakeContext *s);

extern int flake_encode_frame(FlakeContext *s, unsigned char *frame_buffer,
                              short *samples);

extern void flake_encode_close(FlakeContext *s);

#endif /* FLAKE_H */



// common.h
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <math.h>
#ifndef M_PI
#define M_PI  3.14159265358979323846
#endif
#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif

#ifndef EMULATE_INTTYPES
#include <inttypes.h>
#else
#if defined(_WIN32) && defined(_MSC_VER)
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif
#endif /* EMULATE_INTTYPES */

#define ABS(a) ((a) >= 0 ? (a) : (-(a)))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) > (b) ? (b) : (a))
#define CLIP(x,min,max) MAX(MIN((x), (max)), (min))

static inline int
log2i(uint32_t v)
{
	int i;
	int n = 0;
	if (v & 0xffff0000) {
		v >>= 16;
		n += 16;
	}
	if (v & 0xff00) {
		v >>= 8;
		n += 8;
	}
	for (i=2; i<256; i<<=1) {
		if (v >= i) {
			n++;
		} else {
			break;
		}
	}
	return n;
}

#include <string.h>

// strnlen is a GNU extention. providing implementation if needed.
#ifndef HAVE_STRNLEN
/*static*/ inline size_t
strnlen(const char *s, size_t maxlen)
{
	size_t i = 0;
	while ((s[i] != '\0') && (i < maxlen)) {
		i++;
	}
	return i;
}
#elif !defined(__USE_GNU)
extern size_t strnlen(const char *s, size_t maxlen);
#endif



// bswap.h
static inline uint16_t bswap_16(uint16_t x)
{
	return (x>>8) | (x<<8);
}

static inline uint32_t bswap_32(uint32_t x)
{
	x= ((x<<8)&0xFF00FF00) | ((x>>8)&0x00FF00FF);
	return (x>>16) | (x<<16);
}

static inline uint64_t bswap_64(uint64_t x)
{
	union {
		uint64_t ll;
		uint32_t l[2];
	} w, r;
	w.ll = x;
	r.l[0] = bswap_32(w.l[1]);
	r.l[1] = bswap_32(w.l[0]);
	return r.ll;
}

// be2me ... BigEndian to MachineEndian
// le2me ... LittleEndian to MachineEndian

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



// bitio.h
typedef struct BitWriter {
	uint32_t bit_buf;
	int bit_left;
	uint8_t *buffer, *buf_ptr, *buf_end;
	int eof;
} BitWriter;

static inline void
bitwriter_init(BitWriter *bw, void *buf, int len)
{
	if (len < 0) {
		len = 0;
		buf = NULL;
	}
	bw->buffer = buf;
	bw->buf_end = bw->buffer + len;
	bw->buf_ptr = bw->buffer;
	bw->bit_left = 32;
	bw->bit_buf = 0;
	bw->eof = 0;
}

static inline uint32_t
bitwriter_count(BitWriter *bw)
{
	// TODO: simplify
	return ((((bw->buf_ptr - bw->buffer) << 3) + 32 - bw->bit_left) + 7) >> 3;
}

static inline void
bitwriter_flush(BitWriter *bw)
{
	bw->bit_buf <<= bw->bit_left;
	while (bw->bit_left < 32 && !bw->eof) {
		if (bw->buf_ptr >= bw->buf_end) {
			bw->eof = 1;
			break;
		}
		if (bw->buffer != NULL) {
			*bw->buf_ptr = bw->bit_buf >> 24;
		}
		bw->buf_ptr++;
		bw->bit_buf <<= 8;
		bw->bit_left += 8;
	}
	bw->bit_left = 32;
	bw->bit_buf = 0;
}

static inline void
bitwriter_writebits(BitWriter *bw, int bits, uint32_t val)
{
	uint32_t bb=0;
	assert(bits == 32 || val < (1U << bits));

	if (bits == 0 || bw->eof) {
		return;
	}
	if ((bw->buf_ptr+3) >= bw->buf_end) {
		bw->eof = 1;
		return;
	}
	if (bits < bw->bit_left) {
		bw->bit_buf = (bw->bit_buf << bits) | val;
		bw->bit_left -= bits;
	} else {
		if (bw->bit_left == 32) {
			assert(bits == 32);
			bb = val;
		} else {
			bb = (bw->bit_buf << bw->bit_left) | (val >> (bits - bw->bit_left));
			bw->bit_left += (32 - bits);
		}
		if (bw->buffer != NULL) {
			*(uint32_t *)bw->buf_ptr = be2me_32(bb);
		}
		bw->buf_ptr += 4;
		bw->bit_buf = val;
	}
}

static inline void
bitwriter_writebits_signed(BitWriter *bw, int bits, int32_t val)
{
	assert(bits >= 0 && bits <= 31);
	bitwriter_writebits(bw, bits, val & ((1ULL<<bits)-1));
}

static inline void
bitwriter_write_rice_signed(BitWriter *bw, int k, int32_t val)
{
	int v, q;

	if (k < 0) {
		return;
	}

	// convert signed to unsigned
	v = -2*val-1;
	v ^= (v>>31);

	// write quotient in unary
	q = (v >> k) + 1;
	while (q > 31) {
		bitwriter_writebits(bw, 31, 0);
		q -= 31;
	}
	bitwriter_writebits(bw, q, 1);

	// write write remainder in binary using 'k' bits
	bitwriter_writebits(bw, k, v&((1<<k)-1));
}



// crc.c
static void
crc_init_table(uint16_t *table, int bits, int poly)
{
	int i, j, crc;

	poly = (poly + (1<<bits));
	for (i=0; i<256; i++) {
		crc = i;
		for (j=0; j<bits; j++) {
			if (crc & (1<<(bits-1))) {
				crc = (crc << 1) ^ poly;
			} else {
				crc <<= 1;
			}
		}
		table[i] = (crc & ((1<<bits)-1));
	}
}

/* CRC key for polynomial, x^8 + x^2 + x^1 + 1 */
#define CRC8_POLY 0x07

/* CRC key for polynomial, x^16 + x^15 + x^2 + 1 */
#define CRC16_POLY 0x8005

static uint16_t crc8tab[256];
static uint16_t crc16tab[256];

void
crc_init()
{
	crc_init_table(crc8tab, 8, CRC8_POLY);
	crc_init_table(crc16tab, 16, CRC16_POLY);
}

static uint16_t
calc_crc(const uint16_t *table, int bits, const uint8_t *data, uint32_t len)
{
	uint16_t crc, v1, v2;

	crc = 0;
	while (len--) {
		v1 = (crc << 8) & ((1 << bits) - 1);
		v2 = (crc >> (bits - 8)) ^ *data++;
		assert(v2 < 256);
		crc = v1 ^ table[v2];
	}
	return crc;
}

uint8_t
calc_crc8(const uint8_t *data, uint32_t len)
{
	uint8_t crc;

	if (data == NULL) {
		return 0;
	}

	crc = calc_crc(crc8tab, 8, data, len);
	return crc;
}

uint16_t
calc_crc16(const uint8_t *data, uint32_t len)
{
	uint16_t crc;

	if (data == NULL) {
		return 0;
	}

	crc = calc_crc(crc16tab, 16, data, len);
	return crc;
}



// lpc.c
#define MAX_LPC_ORDER 32

/**
 * Apply Welch window function to audio block
 */
static inline void
apply_welch_window(const int32_t *data, int len, double *w_data)
{
	int i;
	double c;

	c = (2.0 / (len - 1.0)) - 1.0;
	for (i=0; i<(len >> 1); i++) {
		double w = 1.0 - ((c-i) * (c-i));
		w_data[i] = data[i] * w;
		w_data[len-1-i] = data[len-1-i] * w;
	}
}

/**
 * Calculates autocorrelation data from audio samples
 * A Welch window function is applied before calculation.
 */
static void
compute_autocorr(const int32_t *data, int len, int lag, double *autoc)
{
	int i, j;
	double *data1;
	double temp, temp2;

	data1 = malloc((len+16) * sizeof(double));
	apply_welch_window(data, len, data1);
	data1[len] = 0;

	for (i=0; i<=lag; ++i) {
		temp = 1.0;
		temp2 = 1.0;
		for (j=0; j<=lag-i; ++j) {
			temp += data1[j+i] * data1[j];
		}

		for (j=lag+1; j<=len-1; j+=2) {
			temp += data1[j] * data1[j-i];
			temp2 += data1[j+1] * data1[j+1-i];
		}
		autoc[i] = temp + temp2;
	}

	free(data1);
}

/**
 * Levinson-Durbin recursion.
 * Produces LPC coefficients from autocorrelation data.
 */
static void
compute_lpc_coefs(const double *autoc, int max_order, double *ref,
                  double lpc[][MAX_LPC_ORDER])
{
	int i, j, i2;
	double r, err, tmp;
	double lpc_tmp[MAX_LPC_ORDER];

	for (i=0; i<max_order; i++) {
		lpc_tmp[i] = 0;
	}
	err = 1.0;
	if (autoc) {
		err = autoc[0];
	}

	for (i=0; i<max_order; i++) {
		if (ref) {
			r = ref[i];
		} else {
			r = -autoc[i+1];
			for (j=0; j<i; j++) {
				r -= lpc_tmp[j] * autoc[i-j];
			}
			r /= err;
			err *= 1.0 - (r * r);
		}

		i2 = (i >> 1);
		lpc_tmp[i] = r;
		for (j=0; j<i2; j++) {
			tmp = lpc_tmp[j];
			lpc_tmp[j] += r * lpc_tmp[i-1-j];
			lpc_tmp[i-1-j] += r * tmp;
		}
		if (i & 1) {
			lpc_tmp[j] += lpc_tmp[j] * r;
		}

		for (j=0; j<=i; j++) {
			lpc[i][j] = -lpc_tmp[j];
		}
	}
}

/**
 * Compute LPC coefs for FLAKE_ORDER_METHOD_EST
 * Faster LPC coeff computation by first calculating the reflection coefficients
 * using Schur recursion. That allows for estimating the optimal order before
 * running Levinson recursion.
 */
static int
compute_lpc_coefs_est(const double *autoc, int max_order,
                      double lpc[][MAX_LPC_ORDER])
{
	int i, j;
	double error;
	double gen[2][MAX_LPC_ORDER];
	double ref[MAX_LPC_ORDER];
	int order_est;

	// Schur recursion
	for (i=0; i<max_order; i++) {
		gen[0][i] = gen[1][i] = autoc[i+1];
	}
	error = autoc[0];
	ref[0] = -gen[1][0] / error;
	error += gen[1][0] * ref[0];
	for (i=1; i<max_order; i++) {
		for (j=0; j<max_order-i; j++) {
			gen[1][j] = gen[1][j+1] + ref[i-1] * gen[0][j];
			gen[0][j] = gen[1][j+1] * ref[i-1] + gen[0][j];
		}
		ref[i] = -gen[1][0] / error;
		error += gen[1][0] * ref[i];
	}

	// Estimate optimal order using reflection coefficients
	order_est = 1;
	for (i=max_order-1; i>=0; i--) {
		if (fabs(ref[i]) > 0.10) {
			order_est = i+1;
			break;
		}
	}

	// Levinson recursion
	compute_lpc_coefs(NULL, order_est, ref, lpc);

	return order_est;
}

/**
 * Quantize LPC coefficients
 */
static void
quantize_lpc_coefs(double *lpc_in, int order, int precision, int32_t *lpc_out,
                   int *shift)
{
	int i;
	double d, cmax, error;
	int32_t qmax;
	int sh, q;

	// define maximum levels
	qmax = (1 << (precision - 1)) - 1;

	// find maximum coefficient value
	cmax = 0.0;
	for (i=0; i<order; i++) {
		d = fabs(lpc_in[i]);
		if (d > cmax) {
			cmax = d;
		}
	}
	// if maximum value quantizes to zero, return all zeros
	if (cmax * (1 << 15) < 1.0) {
		*shift = 0;
		memset(lpc_out, 0, sizeof(int32_t) * order);
		return;
	}

	// calculate level shift which scales max coeff to available bits
	sh = 15;
	while ((cmax * (1 << sh) > qmax) && (sh > 0)) {
		sh--;
	}

	// since negative shift values are unsupported in decoder, scale down
	// coefficients instead
	if (sh == 0 && cmax > qmax) {
		double scale = ((double)qmax) / cmax;
		for (i=0; i<order; i++) {
			lpc_in[i] *= scale;
		}
	}

	// output quantized coefficients and level shift
	error=0;
	for (i=0; i<order; i++) {
		error += lpc_in[i] * (1 << sh);
		q = error + 0.5;
		if (q <= -qmax) {
			q = -qmax+1;
		}
		if (q > qmax) {
			q = qmax;
		}
		error -= q;
		lpc_out[i] = q;
	}
	*shift = sh;
}

/**
 * Calculate LPC coefficients for multiple orders
 */
int
lpc_calc_coefs(const int32_t *samples, int blocksize, int max_order,
               int precision, int omethod, int32_t coefs[][MAX_LPC_ORDER],
               int *shift)
{
	double autoc[MAX_LPC_ORDER+1];
	double lpc[MAX_LPC_ORDER][MAX_LPC_ORDER];
	int i;
	int opt_order;

	compute_autocorr(samples, blocksize, max_order+1, autoc);

	opt_order = max_order;
	if (omethod == FLAKE_ORDER_METHOD_EST) {
		opt_order = compute_lpc_coefs_est(autoc, max_order, lpc);
	} else {
		compute_lpc_coefs(autoc, max_order, NULL, lpc);
	}

	switch (omethod) {
	case FLAKE_ORDER_METHOD_MAX:
	case FLAKE_ORDER_METHOD_EST:
		i = opt_order-1;
		quantize_lpc_coefs(lpc[i], i+1, precision, coefs[i], &shift[i]);
		break;
	default:
		for (i=0; i<max_order; i++) {
			quantize_lpc_coefs(lpc[i], i+1, precision, coefs[i], &shift[i]);
		}
		break;
	}

	return opt_order;
}



// md5.c
typedef struct {
	uint32_t lo, hi;
	uint32_t a, b, c, d;
	uint8_t buffer[64];
	uint32_t block[16];
} MD5Context;

/*
 * The basic MD5 functions.
 *
 * F is optimized compared to its RFC 1321 definition just like in Colin
 * Plumb's implementation.
 */
#define F(x, y, z)  ((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z)  ((y) ^ ((z) & ((x) ^ (y))))
#define H(x, y, z)  ((x) ^ (y) ^ (z))
#define I(x, y, z)  ((y) ^ ((x) | ~(z)))

/*
 * The MD5 transformation for all four rounds.
 */
#define STEP(f, a, b, c, d, x, t, s) \
    (a) += f((b), (c), (d)) + (x) + (t); \
    (a) = (((a) << (s)) | (((a) & 0xFFFFFFFF) >> (32 - (s)))); \
    (a) += (b);

/*
 * SET reads 4 input bytes in little-endian byte order and stores them
 * in a properly aligned word in host byte order.
 */
#define SET(n) \
    (ctx->block[(n)] = \
    (uint32_t)ptr[(n) * 4] | \
    ((uint32_t)ptr[(n) * 4 + 1] << 8) | \
    ((uint32_t)ptr[(n) * 4 + 2] << 16) | \
    ((uint32_t)ptr[(n) * 4 + 3] << 24))

#define GET(n) \
    (ctx->block[(n)])

/*
 * This processes one or more 64-byte data blocks, but does NOT update
 * the bit counters.  There are no alignment requirements.
 */
static const void *
body(MD5Context *ctx, const void *data, uint32_t size)
{
	const uint8_t *ptr;
	uint32_t a, b, c, d;
	uint32_t saved_a, saved_b, saved_c, saved_d;

	ptr = data;

	a = ctx->a;
	b = ctx->b;
	c = ctx->c;
	d = ctx->d;

	do {
		saved_a = a;
		saved_b = b;
		saved_c = c;
		saved_d = d;

		// Round 1
		STEP(F, a, b, c, d, SET( 0), 0xD76AA478,  7)
		STEP(F, d, a, b, c, SET( 1), 0xE8C7B756, 12)
		STEP(F, c, d, a, b, SET( 2), 0x242070DB, 17)
		STEP(F, b, c, d, a, SET( 3), 0xC1BDCEEE, 22)
		STEP(F, a, b, c, d, SET( 4), 0xF57C0FAF,  7)
		STEP(F, d, a, b, c, SET( 5), 0x4787C62A, 12)
		STEP(F, c, d, a, b, SET( 6), 0xA8304613, 17)
		STEP(F, b, c, d, a, SET( 7), 0xFD469501, 22)
		STEP(F, a, b, c, d, SET( 8), 0x698098D8,  7)
		STEP(F, d, a, b, c, SET( 9), 0x8B44F7AF, 12)
		STEP(F, c, d, a, b, SET(10), 0xFFFF5BB1, 17)
		STEP(F, b, c, d, a, SET(11), 0x895CD7BE, 22)
		STEP(F, a, b, c, d, SET(12), 0x6B901122,  7)
		STEP(F, d, a, b, c, SET(13), 0xFD987193, 12)
		STEP(F, c, d, a, b, SET(14), 0xA679438E, 17)
		STEP(F, b, c, d, a, SET(15), 0x49B40821, 22)

		// Round 2
		STEP(G, a, b, c, d, GET( 1), 0xF61E2562,  5)
		STEP(G, d, a, b, c, GET( 6), 0xC040B340,  9)
		STEP(G, c, d, a, b, GET(11), 0x265E5A51, 14)
		STEP(G, b, c, d, a, GET( 0), 0xE9B6C7AA, 20)
		STEP(G, a, b, c, d, GET( 5), 0xD62F105D,  5)
		STEP(G, d, a, b, c, GET(10), 0x02441453,  9)
		STEP(G, c, d, a, b, GET(15), 0xD8A1E681, 14)
		STEP(G, b, c, d, a, GET( 4), 0xE7D3FBC8, 20)
		STEP(G, a, b, c, d, GET( 9), 0x21E1CDE6,  5)
		STEP(G, d, a, b, c, GET(14), 0xC33707D6,  9)
		STEP(G, c, d, a, b, GET( 3), 0xF4D50D87, 14)
		STEP(G, b, c, d, a, GET( 8), 0x455A14ED, 20)
		STEP(G, a, b, c, d, GET(13), 0xA9E3E905,  5)
		STEP(G, d, a, b, c, GET( 2), 0xFCEFA3F8,  9)
		STEP(G, c, d, a, b, GET( 7), 0x676F02D9, 14)
		STEP(G, b, c, d, a, GET(12), 0x8D2A4C8A, 20)

		// Round 3
		STEP(H, a, b, c, d, GET( 5), 0xFFFA3942,  4)
		STEP(H, d, a, b, c, GET( 8), 0x8771F681, 11)
		STEP(H, c, d, a, b, GET(11), 0x6D9D6122, 16)
		STEP(H, b, c, d, a, GET(14), 0xFDE5380C, 23)
		STEP(H, a, b, c, d, GET( 1), 0xA4BEEA44,  4)
		STEP(H, d, a, b, c, GET( 4), 0x4BDECFA9, 11)
		STEP(H, c, d, a, b, GET( 7), 0xF6BB4B60, 16)
		STEP(H, b, c, d, a, GET(10), 0xBEBFBC70, 23)
		STEP(H, a, b, c, d, GET(13), 0x289B7EC6,  4)
		STEP(H, d, a, b, c, GET( 0), 0xEAA127FA, 11)
		STEP(H, c, d, a, b, GET( 3), 0xD4EF3085, 16)
		STEP(H, b, c, d, a, GET( 6), 0x04881D05, 23)
		STEP(H, a, b, c, d, GET( 9), 0xD9D4D039,  4)
		STEP(H, d, a, b, c, GET(12), 0xE6DB99E5, 11)
		STEP(H, c, d, a, b, GET(15), 0x1FA27CF8, 16)
		STEP(H, b, c, d, a, GET( 2), 0xC4AC5665, 23)

		// Round 4
		STEP(I, a, b, c, d, GET( 0), 0xF4292244,  6)
		STEP(I, d, a, b, c, GET( 7), 0x432AFF97, 10)
		STEP(I, c, d, a, b, GET(14), 0xAB9423A7, 15)
		STEP(I, b, c, d, a, GET( 5), 0xFC93A039, 21)
		STEP(I, a, b, c, d, GET(12), 0x655B59C3,  6)
		STEP(I, d, a, b, c, GET( 3), 0x8F0CCC92, 10)
		STEP(I, c, d, a, b, GET(10), 0xFFEFF47D, 15)
		STEP(I, b, c, d, a, GET( 1), 0x85845DD1, 21)
		STEP(I, a, b, c, d, GET( 8), 0x6FA87E4F,  6)
		STEP(I, d, a, b, c, GET(15), 0xFE2CE6E0, 10)
		STEP(I, c, d, a, b, GET( 6), 0xA3014314, 15)
		STEP(I, b, c, d, a, GET(13), 0x4E0811A1, 21)
		STEP(I, a, b, c, d, GET( 4), 0xF7537E82,  6)
		STEP(I, d, a, b, c, GET(11), 0xBD3AF235, 10)
		STEP(I, c, d, a, b, GET( 2), 0x2AD7D2BB, 15)
		STEP(I, b, c, d, a, GET( 9), 0xEB86D391, 21)

		a += saved_a;
		b += saved_b;
		c += saved_c;
		d += saved_d;

		ptr += 64;
	} while (size -= 64);

	ctx->a = a;
	ctx->b = b;
	ctx->c = c;
	ctx->d = d;

	return ptr;
}

void
md5_init(MD5Context *ctx)
{
	ctx->a = 0x67452301;
	ctx->b = 0xefcdab89;
	ctx->c = 0x98badcfe;
	ctx->d = 0x10325476;

	ctx->lo = 0;
	ctx->hi = 0;
}

void
md5_update(MD5Context *ctx, const void *data, uint32_t size)
{
	uint32_t saved_lo;
	uint32_t used, free;

	saved_lo = ctx->lo;
	if ((ctx->lo = (saved_lo + size) & 0x1FFFFFFF) < saved_lo) {
		ctx->hi++;
	}
	ctx->hi += size >> 29;

	used = saved_lo & 0x3f;

	if (used) {
		free = 64 - used;

		if (size < free) {
			memcpy(&ctx->buffer[used], data, size);
			return;
		}

		memcpy(&ctx->buffer[used], data, free);
		data = (uint8_t *)data + free;
		size -= free;
		body(ctx, ctx->buffer, 64);
	}

	if (size >= 64) {
		data = body(ctx, data, size & ~(uint32_t)0x3f);
		size &= 0x3f;
	}

	memcpy(ctx->buffer, data, size);
}

void
md5_final(uint8_t *result, MD5Context *ctx)
{
	uint32_t used, free;

	used = ctx->lo & 0x3f;

	ctx->buffer[used++] = 0x80;

	free = 64 - used;

	if (free < 8) {
		memset(&ctx->buffer[used], 0, free);
		body(ctx, ctx->buffer, 64);
		used = 0;
		free = 64;
	}

	memset(&ctx->buffer[used], 0, free - 8);

	ctx->lo <<= 3;
	ctx->buffer[56] = ctx->lo;
	ctx->buffer[57] = ctx->lo >> 8;
	ctx->buffer[58] = ctx->lo >> 16;
	ctx->buffer[59] = ctx->lo >> 24;
	ctx->buffer[60] = ctx->hi;
	ctx->buffer[61] = ctx->hi >> 8;
	ctx->buffer[62] = ctx->hi >> 16;
	ctx->buffer[63] = ctx->hi >> 24;

	body(ctx, ctx->buffer, 64);

	result[0] = ctx->a;
	result[1] = ctx->a >> 8;
	result[2] = ctx->a >> 16;
	result[3] = ctx->a >> 24;
	result[4] = ctx->b;
	result[5] = ctx->b >> 8;
	result[6] = ctx->b >> 16;
	result[7] = ctx->b >> 24;
	result[8] = ctx->c;
	result[9] = ctx->c >> 8;
	result[10] = ctx->c >> 16;
	result[11] = ctx->c >> 24;
	result[12] = ctx->d;
	result[13] = ctx->d >> 8;
	result[14] = ctx->d >> 16;
	result[15] = ctx->d >> 24;

	memset(ctx, 0, sizeof(*ctx));
}

/**
 * Run md5_update on the audio signal byte stream
 */
void
md5_accumulate(MD5Context *ctx, const void *signal, int ch, int nsamples)
{
	int data_bytes = ch * nsamples * 2;

#ifdef WORDS_BIGENDIAN
	int i;
	uint16_t *sig16 = malloc(data_bytes);
	memcpy(sig16, signal, data_bytes);
	for (i=0; i<nsamples*ch; i++) {
		sig16[i] = bswap_16(sig16[i]);
	}
	md5_update(ctx, sig16, data_bytes);
	free(sig16);
#else
	md5_update(ctx, signal, data_bytes);
#endif
}

void
md5_print(uint8_t digest[16])
{
	int i;

	for (i=0; i<16; i++) {
		fprintf(stderr, "%02x", digest[i]);
	}
}



// rice.c
#define MAX_RICE_PARAM          14
#define MAX_PARTITION_ORDER     8
#define MAX_PARTITIONS          (1 << MAX_PARTITION_ORDER)

typedef struct RiceContext {
	int porder;                     /* partition order */
	int params[MAX_PARTITIONS];     /* Rice parameters */
	int esc_bps[MAX_PARTITIONS];    /* bps if using escape code */
} RiceContext;

#define rice_encode_count(sum, n, k) (((n)*((k)+1))+(((sum)-(n>>1))>>(k)))

int
find_optimal_rice_param(uint32_t sum, int n)
{
	int k, k_opt;
	uint32_t nbits[MAX_RICE_PARAM+1];

	k_opt = 0;
	nbits[0] = UINT32_MAX;
	for (k=0; k<=MAX_RICE_PARAM; k++) {
		nbits[k] = rice_encode_count(sum, n, k);
		if (nbits[k] < nbits[k_opt]) {
			k_opt = k;
		}
	}
	return k_opt;
}

static uint32_t
calc_optimal_rice_params(RiceContext *rc, int porder, uint32_t *sums,
                         int n, int pred_order)
{
	int i;
	int k, cnt, part;
	uint32_t all_bits;

	part = (1 << porder);
	all_bits = 0;

	cnt = (n >> porder) - pred_order;
	for (i=0; i<part; i++) {
		if (i == 1) {
			cnt = (n >> porder);
		}
		k = find_optimal_rice_param(sums[i], cnt);
		rc->params[i] = k;
		all_bits += rice_encode_count(sums[i], cnt, k);
	}
	all_bits += (4 * part);

	rc->porder = porder;

	return all_bits;
}

static void
calc_sums(int pmin, int pmax, uint32_t *data, int n, int pred_order,
          uint32_t sums[][MAX_PARTITIONS])
{
	int i, j;
	int parts, cnt;
	uint32_t *res;

	// sums for highest level
	parts = (1 << pmax);
	res = &data[pred_order];
	cnt = (n >> pmax) - pred_order;
	for (i=0; i<parts; i++) {
		if (i == 1) {
			cnt = (n >> pmax);
		}
		if (i > 0) {
			res = &data[i*cnt];
		}
		sums[pmax][i] = 0;
		for (j=0; j<cnt; j++) {
			sums[pmax][i] += res[j];
		}
	}
	// sums for lower levels
	for (i=pmax-1; i>=pmin; i--) {
		parts = (1 << i);
		for (j=0; j<parts; j++) {
			sums[i][j] = sums[i+1][2*j] + sums[i+1][2*j+1];
		}
	}
}

static uint32_t
calc_rice_params(RiceContext *rc, int pmin, int pmax, int32_t *data, int n,
                 int pred_order)
{
	int i;
	uint32_t bits[MAX_PARTITION_ORDER+1];
	int opt_porder;
	RiceContext tmp_rc;
	uint32_t *udata;
	uint32_t sums[MAX_PARTITION_ORDER+1][MAX_PARTITIONS];

	assert(pmin >= 0 && pmin <= MAX_PARTITION_ORDER);
	assert(pmax >= 0 && pmax <= MAX_PARTITION_ORDER);
	assert(pmin <= pmax);

	udata = malloc(n * sizeof(uint32_t));
	for (i=0; i<n; i++) {
		udata[i] = (2*data[i]) ^ (data[i]>>31);
	}

	calc_sums(pmin, pmax, udata, n, pred_order, sums);

	opt_porder = pmin;
	bits[pmin] = UINT32_MAX;
	for (i=pmin; i<=pmax; i++) {
		bits[i] = calc_optimal_rice_params(&tmp_rc, i, sums[i], n, pred_order);
		if (bits[i] <= bits[opt_porder]) {
			opt_porder = i;
			*rc = tmp_rc;
		}
	}

	free(udata);
	return bits[opt_porder];
}

static int
get_max_p_order(int max_porder, int n, int order)
{
	int porder = MIN(max_porder, log2i(n^(n-1)));
	if (order > 0) {
		porder = MIN(porder, log2i(n/order));
	}
	return porder;
}

uint32_t
calc_rice_params_fixed(RiceContext *rc, int pmin, int pmax, int32_t *data,
                       int n, int pred_order, int bps)
{
	uint32_t bits;
	pmin = get_max_p_order(pmin, n, pred_order);
	pmax = get_max_p_order(pmax, n, pred_order);
	bits = pred_order*bps + 6;
	bits += calc_rice_params(rc, pmin, pmax, data, n, pred_order);
	return bits;
}

uint32_t
calc_rice_params_lpc(RiceContext *rc, int pmin, int pmax, int32_t *data, int n,
                     int pred_order, int bps, int precision)
{
	uint32_t bits;
	pmin = get_max_p_order(pmin, n, pred_order);
	pmax = get_max_p_order(pmax, n, pred_order);
	bits = pred_order*bps + 4 + 5 + pred_order*precision + 6;
	bits += calc_rice_params(rc, pmin, pmax, data, n, pred_order);
	return bits;
}



// encode.h
#include <inttypes.h>

#define FLAC_MAX_CH  8
#define FLAC_MIN_BLOCKSIZE  16
#define FLAC_MAX_BLOCKSIZE  65535

#define FLAC_SUBFRAME_CONSTANT  0
#define FLAC_SUBFRAME_VERBATIM  1
#define FLAC_SUBFRAME_FIXED     8
#define FLAC_SUBFRAME_LPC      32

#define FLAC_CHMODE_NOT_STEREO      0
#define FLAC_CHMODE_LEFT_RIGHT      1
#define FLAC_CHMODE_LEFT_SIDE       8
#define FLAC_CHMODE_RIGHT_SIDE      9
#define FLAC_CHMODE_MID_SIDE       10

#define FLAC_STREAM_MARKER  0x664C6143

struct BitWriter;

typedef struct FlacSubframe {
	int type;
	int type_code;
	int order;
	int obits;
	int32_t coefs[MAX_LPC_ORDER];
	int shift;
	int32_t samples[FLAC_MAX_BLOCKSIZE];
	int32_t residual[FLAC_MAX_BLOCKSIZE];
	RiceContext rc;
} FlacSubframe;

typedef struct FlacFrame {
	int blocksize;
	int bs_code[2];
	int ch_mode;
	int ch_order[2];
	uint8_t crc8;
	FlacSubframe subframes[FLAC_MAX_CH];
} FlacFrame;

typedef struct FlacEncodeContext {
	int channels;
	int ch_code;
	int samplerate;
	int sr_code[2];
	int bps;
	int bps_code;
	uint32_t sample_count;
	FlakeEncodeParams params;
	int max_frame_size;
	int lpc_precision;
	uint32_t frame_count;
	FlacFrame frame;
	MD5Context md5ctx;
	struct BitWriter *bw;
} FlacEncodeContext;



// optimize.c
static void
encode_residual_verbatim(int32_t res[], int32_t smp[], int n)
{
	memcpy(res, smp, n*sizeof(int32_t));
}

static void
encode_residual_fixed(int32_t res[], int32_t smp[], int n, int order)
{
	int i;

	switch (order) {
	case 0:
		memcpy(res, smp, n*sizeof(int32_t));
		return;
	case 1:
		res[0] = smp[0];
		for (i=1; i<n; i++) {
			res[i] = smp[i] - (smp[i-1]);
		}
		return;
	case 2:
		res[0] = smp[0];
		res[1] = smp[1];
		for (i=2; i<n; i++) {
			res[i] = smp[i] - 2*smp[i-1] + smp[i-2];
		}
		return;
	case 3:
		res[0] = smp[0];
		res[1] = smp[1];
		res[2] = smp[2];
		for (i=3; i<n; i++) {
			res[i] = smp[i] - 3*smp[i-1] + 3*smp[i-2] - smp[i-3];
		}
		return;
	case 4:
		res[0] = smp[0];
		res[1] = smp[1];
		res[2] = smp[2];
		res[3] = smp[3];
		for (i=4; i<n; i++) {
			res[i] = smp[i] - 4*smp[i-1] + 6*smp[i-2] - 4*smp[i-3] + smp[i-4];
		}
		return;
	default:
		return;
	}
}

static void
encode_residual_lpc(int32_t res[], int32_t smp[], int n, int order,
                    int32_t coefs[], int shift)
{
	int i;
	int32_t pred;

	for (i=0; i<order; i++) {
		res[i] = smp[i];
	}
	for (i=order; i<n; i++) {
		pred = 0;
		// note that all cases fall through.
		// the result is in an unrolled loop for each order
		switch (order) {
		case 32:
			pred += coefs[31] * smp[i-32];
		case 31:
			pred += coefs[30] * smp[i-31];
		case 30:
			pred += coefs[29] * smp[i-30];
		case 29:
			pred += coefs[28] * smp[i-29];
		case 28:
			pred += coefs[27] * smp[i-28];
		case 27:
			pred += coefs[26] * smp[i-27];
		case 26:
			pred += coefs[25] * smp[i-26];
		case 25:
			pred += coefs[24] * smp[i-25];
		case 24:
			pred += coefs[23] * smp[i-24];
		case 23:
			pred += coefs[22] * smp[i-23];
		case 22:
			pred += coefs[21] * smp[i-22];
		case 21:
			pred += coefs[20] * smp[i-21];
		case 20:
			pred += coefs[19] * smp[i-20];
		case 19:
			pred += coefs[18] * smp[i-19];
		case 18:
			pred += coefs[17] * smp[i-18];
		case 17:
			pred += coefs[16] * smp[i-17];
		case 16:
			pred += coefs[15] * smp[i-16];
		case 15:
			pred += coefs[14] * smp[i-15];
		case 14:
			pred += coefs[13] * smp[i-14];
		case 13:
			pred += coefs[12] * smp[i-13];
		case 12:
			pred += coefs[11] * smp[i-12];
		case 11:
			pred += coefs[10] * smp[i-11];
		case 10:
			pred += coefs[ 9] * smp[i-10];
		case  9:
			pred += coefs[ 8] * smp[i- 9];
		case  8:
			pred += coefs[ 7] * smp[i- 8];
		case  7:
			pred += coefs[ 6] * smp[i- 7];
		case  6:
			pred += coefs[ 5] * smp[i- 6];
		case  5:
			pred += coefs[ 4] * smp[i- 5];
		case  4:
			pred += coefs[ 3] * smp[i- 4];
		case  3:
			pred += coefs[ 2] * smp[i- 3];
		case  2:
			pred += coefs[ 1] * smp[i- 2];
		case  1:
			pred += coefs[ 0] * smp[i- 1];
		case  0:
		default:
			break;
		}
		res[i] = smp[i] - (pred >> shift);
	}
}

int
encode_residual(FlacEncodeContext *ctx, int ch)
{
	int i;
	FlacFrame *frame;
	FlacSubframe *sub;
	int32_t coefs[MAX_LPC_ORDER][MAX_LPC_ORDER];
	int shift[MAX_LPC_ORDER];
	int n, max_order, opt_order, min_porder, max_porder;
	int min_order;
	int32_t *res, *smp;
	int est_order, omethod;

	frame = &ctx->frame;
	sub = &frame->subframes[ch];
	res = sub->residual;
	smp = sub->samples;
	n = frame->blocksize;

	// CONSTANT
	for (i=1; i<n; i++) {
		if (smp[i] != smp[0]) {
			break;
		}
	}
	if (i == n) {
		sub->type = sub->type_code = FLAC_SUBFRAME_CONSTANT;
		res[0] = smp[0];
		return sub->obits;
	}

	// VERBATIM
	if (n < 5 || ctx->params.prediction_type == FLAKE_PREDICTION_NONE) {
		sub->type = sub->type_code = FLAC_SUBFRAME_VERBATIM;
		encode_residual_verbatim(res, smp, n);
		return sub->obits * n;
	}

	omethod = ctx->params.order_method;
	min_order = ctx->params.min_prediction_order;
	max_order = ctx->params.max_prediction_order;
	opt_order = max_order;
	min_porder = ctx->params.min_partition_order;
	max_porder = ctx->params.max_partition_order;

	// FIXED
	if (ctx->params.prediction_type == FLAKE_PREDICTION_FIXED || n <= max_order) {
		uint32_t bits[5];
		if (max_order > 4) {
			max_order = 4;
		}
		opt_order = min_order;
		bits[opt_order] = UINT32_MAX;
		for (i=min_order; i<=max_order; i++) {
			encode_residual_fixed(res, smp, n, i);
			bits[i] = calc_rice_params_fixed(&sub->rc, min_porder, max_porder, res,
			                                 n, i, sub->obits);
			if (bits[i] < bits[opt_order]) {
				opt_order = i;
			}
		}
		sub->order = opt_order;
		sub->type = FLAC_SUBFRAME_FIXED;
		sub->type_code = sub->type | sub->order;
		if (sub->order != max_order) {
			encode_residual_fixed(res, smp, n, sub->order);
			return calc_rice_params_fixed(&sub->rc, min_porder, max_porder, res, n,
			                              sub->order, sub->obits);
		}
		return bits[sub->order];
	}

	// LPC
	est_order = lpc_calc_coefs(smp, n, max_order, ctx->lpc_precision,
	                           omethod, coefs, shift);

	if (omethod == FLAKE_ORDER_METHOD_MAX) {
		// always use maximum order
		opt_order = max_order;
	} else if (omethod == FLAKE_ORDER_METHOD_EST) {
		// estimated order
		opt_order = est_order;
	} else if (omethod == FLAKE_ORDER_METHOD_2LEVEL ||
	                omethod == FLAKE_ORDER_METHOD_4LEVEL ||
	                omethod == FLAKE_ORDER_METHOD_8LEVEL) {
		int levels = 1 << (omethod-1);
		uint32_t bits[8];
		int order;
		int opt_index = levels-1;
		opt_order = max_order-1;
		bits[opt_index] = UINT32_MAX;
		for (i=opt_index; i>=0; i--) {
			order = min_order + (((max_order-min_order+1) * (i+1)) / levels)-1;
			if (order < 0) {
				order = 0;
			}
			encode_residual_lpc(res, smp, n, order+1, coefs[order], shift[order]);
			bits[i] = calc_rice_params_lpc(&sub->rc, min_porder, max_porder,
			                               res, n, order+1, sub->obits,
			                               ctx->lpc_precision);
			if (bits[i] < bits[opt_index]) {
				opt_index = i;
				opt_order = order;
			}
		}
		opt_order++;
	} else if (omethod == FLAKE_ORDER_METHOD_SEARCH) {
		// brute-force optimal order search
		uint32_t bits[MAX_LPC_ORDER];
		opt_order = 0;
		bits[0] = UINT32_MAX;
		for (i=0; i<max_order; i++) {
			encode_residual_lpc(res, smp, n, i+1, coefs[i], shift[i]);
			bits[i] = calc_rice_params_lpc(&sub->rc, min_porder, max_porder,
			                               res, n, i+1, sub->obits,
			                               ctx->lpc_precision);
			if (bits[i] < bits[opt_order]) {
				opt_order = i;
			}
		}
		opt_order++;
	} else if (omethod == FLAKE_ORDER_METHOD_LOG) {
		// log search (written by Michael Niedermayer for FFmpeg)
		uint32_t bits[MAX_LPC_ORDER];
		int step;

		opt_order = min_order - 1 + (max_order-min_order)/3;
		memset(bits, -1, sizeof(bits));

		for (step=16; step>0; step>>=1) {
			int last = opt_order;
			for (i=last-step; i<=last+step; i+= step) {
				if (i<min_order-1 || i>=max_order || bits[i] < UINT32_MAX) {
					continue;
				}
				encode_residual_lpc(res, smp, n, i+1, coefs[i], shift[i]);
				bits[i] = calc_rice_params_lpc(&sub->rc, min_porder, max_porder,
				                               res, n, i+1, sub->obits,
				                               ctx->lpc_precision);
				if (bits[i] < bits[opt_order]) {
					opt_order = i;
				}
			}
		}
		opt_order++;
	} else {
		return -1;
	}

	sub->order = opt_order;
	sub->type = FLAC_SUBFRAME_LPC;
	sub->type_code = sub->type | (sub->order-1);
	sub->shift = shift[sub->order-1];
	for (i=0; i<sub->order; i++) {
		sub->coefs[i] = coefs[sub->order-1][i];
	}
	encode_residual_lpc(res, smp, n, sub->order, sub->coefs, sub->shift);
	return calc_rice_params_lpc(&sub->rc, min_porder, max_porder, res, n,
	                            sub->order, sub->obits, ctx->lpc_precision);
}

void
reencode_residual_verbatim(FlacEncodeContext *ctx, int ch)
{
	FlacFrame *frame;
	FlacSubframe *sub;

	frame = &ctx->frame;
	sub = &frame->subframes[ch];

	sub->type = sub->type_code = FLAC_SUBFRAME_VERBATIM;
	encode_residual_verbatim(sub->residual, sub->samples, frame->blocksize);
}



// vbs.c
#define SPLIT_THRESHOLD 100

/**
 * Split single frame into smaller frames using predictability comparison.
 * This algorithm computes predictablity estimates for sections of the frame
 * by first summing the absolute value of fixed 2nd order residual, averaged
 * across all channels.  The predictability is compared between adjacent
 * sections to determine if they should be merged based on a fixed comparison
 * threshold.
 */
static void
split_frame_v1(int16_t *samples, int channels, int block_size,
               int *frames, int sizes[8])
{
	int i, ch, j;
	int n = block_size >> 3;
	int64_t res[8];
	int layout[8];
	int16_t *sptr, *sptr0, *sptr1, *sptr2;

	// calculate absolute sum of 2nd order residual
	for (i=0; i<8; i++) {
		sptr = &samples[i*n*channels];
		res[i] = 0;
		for (ch=0; ch<channels; ch++) {
			sptr0 = sptr + (2*channels+ch);
			sptr1 = sptr0 - channels;
			sptr2 = sptr1 - channels;
			for (j=2; j<n; j++) {
				res[i] += abs((*sptr0) - 2*(*sptr1) + (*sptr2));
				sptr0 += channels;
				sptr1 += channels;
				sptr2 += channels;
			}
		}
		res[i] /= channels;
		res[i]++;
	}

	// determine frame layout
	memset(layout, 0, 8 * sizeof(int));
	layout[0] = 1;
	for (i=0; i<7; i++) {
		if (labs(res[i]-res[i+1])*200 / res[i] > SPLIT_THRESHOLD) {
			layout[i+1] = 1;
		}
	}

	// generate frame count and frame sizes from layout
	frames[0] = 0;
	memset(sizes, 0, 8 * sizeof(int));
	for (i=0; i<8; i++) {
		if (layout[i]) {
			frames[0]++;
		}
		sizes[frames[0]-1] += n;
	}
}

static void
split_frame_v2(FlakeContext *s, int16_t *samples, int *frames, int sizes[8])
{
	int fsizes[4][8];
	int layout[8];
	int i, j, n, ch;
	FlacEncodeContext *ctx = (FlacEncodeContext *) s->private_ctx;
	ch = ctx->channels;

	// encode for each level to get sizes
	for (i=0; i<4; i++) {
		int levels, bs;
		levels = (1<<i);
		s->params.block_size /= levels;
		bs = s->params.block_size;
		for (j=0; j<levels; j++) {
			fsizes[i][j] = encode_frame(s, NULL, &samples[bs*j*ch]);
		}
		s->params.block_size *= levels;
	}

	// initialize layout
	for (i=0; i<8; i++) {
		layout[i] = 1;
	}
	// level 3 merge
	if (fsizes[2][0] < (fsizes[3][0]+fsizes[3][1])) {
		layout[1] = 0;
	}
	if (fsizes[2][1] < (fsizes[3][2]+fsizes[3][3])) {
		layout[3] = 0;
	}
	if (fsizes[2][2] < (fsizes[3][4]+fsizes[3][5])) {
		layout[5] = 0;
	}
	if (fsizes[2][3] < (fsizes[3][6]+fsizes[3][7])) {
		layout[7] = 0;
	}
	// level 2 merge
	if (layout[1] == 0 && layout[3] == 0) {
		if (fsizes[1][0] < (fsizes[2][0]+fsizes[2][1])) {
			layout[2] = 0;
		}
	}
	if (layout[5] == 0 && layout[7] == 0) {
		if (fsizes[1][1] < (fsizes[2][2]+fsizes[2][3])) {
			layout[6] = 0;
		}
	}
	// level 1 merge
	if (layout[2] == 0 && layout[6] == 0) {
		if (fsizes[0][0] < (fsizes[1][0]+fsizes[1][1])) {
			layout[4] = 0;
		}
	}

	// generate frame count and frame sizes from layout
	n = s->params.block_size >> 3;
	frames[0] = 0;
	memset(sizes, 0, 8 * sizeof(int));
	for (i=0; i<8; i++) {
		if (layout[i]) {
			frames[0]++;
		}
		sizes[frames[0]-1] += n;
	}
}

int
encode_frame_vbs(FlakeContext *s, uint8_t *frame_buffer, int16_t *samples)
{
	int fs;
	int frames;
	int sizes[8];
	FlacEncodeContext *ctx;

	ctx = (FlacEncodeContext *) s->private_ctx;

	switch (ctx->params.variable_block_size) {
	case 1:
		split_frame_v1(samples, s->channels, s->params.block_size, &frames, sizes);
		break;
	case 2:
		split_frame_v2(s, samples, &frames, sizes);
		break;
	default:
		frames = 1;
		break;
	}
	if (frames > 1) {
		int i, fpos, spos, bs;
		fpos = 0;
		spos = 0;
		bs = s->params.block_size;
		for (i=0; i<frames; i++) {
			s->params.block_size = sizes[i];
			fs = encode_frame(s, &frame_buffer[fpos], &samples[spos*ctx->channels]);
			if (fs < 0) {
				return -1;
			}
			fpos += fs;
			spos += sizes[i];
		}
		s->params.block_size = bs;
		assert(spos == bs);
		return fpos;
	}
	fs = encode_frame(s, frame_buffer, samples);
	return fs;
}



// encode.c
static const int flac_samplerates[16] = {
	0, 0, 0, 0,
	8000, 16000, 22050, 24000, 32000, 44100, 48000, 96000,
	0, 0, 0, 0
};

static const int flac_bitdepths[8] = {
	0, 8, 12, 0, 16, 20, 24, 0
};

static const int flac_blocksizes[15] = {
	0,
	192,
	576, 1152, 2304, 4608,
	0, 0,
	256, 512, 1024, 2048, 4096, 8192, 16384
};


/**
 * Write streaminfo metadata block to byte array
 */
static void
write_streaminfo(FlacEncodeContext *ctx, uint8_t *streaminfo, int last)
{
	memset(streaminfo, 0, 38);
	bitwriter_init(ctx->bw, streaminfo, 38);

	// metadata header
	bitwriter_writebits(ctx->bw, 1, last);
	bitwriter_writebits(ctx->bw, 7, 0);
	bitwriter_writebits(ctx->bw, 24, 34);

	if (ctx->params.variable_block_size) {
		bitwriter_writebits(ctx->bw, 16, 0);
	} else {
		bitwriter_writebits(ctx->bw, 16, ctx->params.block_size);
	}
	bitwriter_writebits(ctx->bw, 16, ctx->params.block_size);
	bitwriter_writebits(ctx->bw, 24, 0);
	bitwriter_writebits(ctx->bw, 24, ctx->max_frame_size);
	bitwriter_writebits(ctx->bw, 20, ctx->samplerate);
	bitwriter_writebits(ctx->bw, 3, ctx->channels-1);
	bitwriter_writebits(ctx->bw, 5, ctx->bps-1);

	// total samples
	if (ctx->sample_count > 0) {
		bitwriter_writebits(ctx->bw, 4, 0);
		bitwriter_writebits(ctx->bw, 32, ctx->sample_count);
	} else {
		bitwriter_writebits(ctx->bw, 4, 0);
		bitwriter_writebits(ctx->bw, 32, 0);
	}
}

/**
 * Write padding metadata block to byte array.
 */
static int
write_padding(FlacEncodeContext *ctx, uint8_t *padding, int last, int padlen)
{
	bitwriter_init(ctx->bw, padding, 4);

	// metadata header
	bitwriter_writebits(ctx->bw, 1, last);
	bitwriter_writebits(ctx->bw, 7, 1);
	bitwriter_writebits(ctx->bw, 24, padlen);

	return padlen + 4;
}

static const char *vendor_string = FLAKE_IDENT;

/**
 * Write vorbis comment metadata block to byte array.
 * Just writes the vendor string for now.
 */
static int
write_vorbis_comment(FlacEncodeContext *ctx, uint8_t *comment, int last)
{
	int vendor_len;
	uint8_t vlen_le[4];

	vendor_len = strlen(vendor_string);
	bitwriter_init(ctx->bw, comment, 4);

	// metadata header
	bitwriter_writebits(ctx->bw, 1, last);
	bitwriter_writebits(ctx->bw, 7, 4);
	bitwriter_writebits(ctx->bw, 24, vendor_len+8);

	// vendor string length
	// note: use me2le_32()
	vlen_le[0] =  vendor_len        & 0xFF;
	vlen_le[1] = (vendor_len >>  8) & 0xFF;
	vlen_le[2] = (vendor_len >> 16) & 0xFF;
	vlen_le[3] = (vendor_len >> 24) & 0xFF;
	memcpy(&comment[4], vlen_le, 4);

	memcpy(&comment[8], vendor_string, vendor_len);

	memset(&comment[vendor_len+8], 0, 4);

	return vendor_len + 12;
}

/**
 * Write fLaC stream marker & metadata headers
 */
static int
write_headers(FlacEncodeContext *ctx, uint8_t *header)
{
	int header_size, last;

	header_size = 0;
	last = 0;

	// stream marker
	header[0] = 0x66;
	header[1] = 0x4C;
	header[2] = 0x61;
	header[3] = 0x43;
	header_size += 4;

	// streaminfo
	write_streaminfo(ctx, &header[header_size], last);
	header_size += 38;

	// vorbis comment
	if (ctx->params.padding_size == 0) {
		last = 1;
	}
	header_size += write_vorbis_comment(ctx, &header[header_size], last);

	// padding
	if (ctx->params.padding_size > 0) {
		last = 1;
		header_size += write_padding(ctx, &header[header_size], last,
		                             ctx->params.padding_size);
	}

	return header_size;
}

/**
 * Set blocksize based on samplerate
 * Chooses the closest predefined blocksize >= time_ms milliseconds
 */
static int
select_blocksize(int samplerate, int time_ms)
{
	int i, target, blocksize;

	blocksize = flac_blocksizes[1];
	target = (samplerate * time_ms) / 1000;
	for (i=0; i<16; i++) {
		if (target >= flac_blocksizes[i] && flac_blocksizes[i] > blocksize) {
			blocksize = flac_blocksizes[i];
		}
	}
	return blocksize;
}

int
flake_set_defaults(FlakeEncodeParams *params)
{
	int lvl;

	if (!params) {
		return -1;
	}
	lvl = params->compression;
	if ((lvl < 0 || lvl > 12) && (lvl != 99)) {
		return -1;
	}

	// default to level 5 params
	params->order_method = FLAKE_ORDER_METHOD_EST;
	params->stereo_method = FLAKE_STEREO_METHOD_ESTIMATE;
	params->block_size = 0;
	params->block_time_ms = 105;
	params->prediction_type = FLAKE_PREDICTION_LEVINSON;
	params->min_prediction_order = 1;
	params->max_prediction_order = 8;
	params->min_partition_order = 0;
	params->max_partition_order = 6;
	params->padding_size = 4096;
	params->variable_block_size = 0;

	// differences from level 5
	switch (lvl) {
	case 0:
		params->stereo_method = FLAKE_STEREO_METHOD_INDEPENDENT;
		params->block_time_ms = 27;
		params->prediction_type = FLAKE_PREDICTION_FIXED;
		params->min_prediction_order = 2;
		params->max_prediction_order = 2;
		params->min_partition_order = 4;
		params->max_partition_order = 4;
		break;
	case 1:
		params->block_time_ms = 27;
		params->prediction_type = FLAKE_PREDICTION_FIXED;
		params->min_prediction_order = 2;
		params->max_prediction_order = 3;
		params->min_partition_order = 2;
		params->max_partition_order = 2;
		break;
	case 2:
		params->block_time_ms = 27;
		params->prediction_type = FLAKE_PREDICTION_FIXED;
		params->min_prediction_order = 2;
		params->max_prediction_order = 4;
		params->min_partition_order = 0;
		params->max_partition_order = 3;
		break;
	case 3:
		params->max_prediction_order = 6;
		params->max_partition_order = 3;
		break;
	case 4:
		params->max_partition_order = 3;
		break;
	case 5:
		break;
	case 6:
		params->order_method = FLAKE_ORDER_METHOD_2LEVEL;
		params->max_partition_order = 8;
		break;
	case 7:
		params->order_method = FLAKE_ORDER_METHOD_4LEVEL;
		params->max_partition_order = 8;
		break;
	case 8:
		params->order_method = FLAKE_ORDER_METHOD_4LEVEL;
		params->max_prediction_order = 12;
		params->max_partition_order = 8;
		break;
	case 9:
		params->order_method = FLAKE_ORDER_METHOD_LOG;
		params->max_prediction_order = 12;
		params->max_partition_order = 8;
		break;
	case 10:
		params->order_method = FLAKE_ORDER_METHOD_SEARCH;
		params->max_prediction_order = 12;
		params->max_partition_order = 8;
		break;
	case 11:
		params->order_method = FLAKE_ORDER_METHOD_LOG;
		params->max_prediction_order = 32;
		params->max_partition_order = 8;
		break;
	case 12:
		params->order_method = FLAKE_ORDER_METHOD_SEARCH;
		params->max_prediction_order = 32;
		params->max_partition_order = 8;
		break;
	case 99:
		params->order_method = FLAKE_ORDER_METHOD_SEARCH;
		params->block_time_ms = 186;
		params->max_prediction_order = 32;
		params->max_partition_order = 8;
		params->variable_block_size = 2;
		break;
	}

	return 0;
}

int
flake_validate_params(FlakeContext *s)
{
	int i;
	int subset = 0;
	int bs;
	FlakeEncodeParams *params;

	if (s == NULL) {
		return -1;
	}
	params = &s->params;

	if (s->channels < 1 || s->channels > FLAC_MAX_CH) {
		return -1;
	}

	if (s->sample_rate < 1 || s->sample_rate > 655350) {
		return -1;
	}
	for (i=4; i<12; i++) {
		if (s->sample_rate == flac_samplerates[i]) {
			break;
		}
	}
	if (i == 12) {
		subset = 1;
	}

	if (s->bits_per_sample < 4 || s->bits_per_sample > 32) {
		return -1;
	}
	for (i=1; i<8; i++) {
		if (s->bits_per_sample == flac_bitdepths[i]) {
			break;
		}
	}
	if (i == 8) {
		subset = 1;
	}

	if ((params->compression < 0 || params->compression > 12) &&
	                (params->compression != 99)) {
		return -1;
	}

	if (params->order_method < 0 || params->order_method > 6) {
		return -1;
	}

	if (params->stereo_method < 0 || params->stereo_method > 1) {
		return -1;
	}

	if (params->block_time_ms < 0) {
		return -1;
	}

	bs = params->block_size;
	if (bs == 0) {
		bs = select_blocksize(s->sample_rate, params->block_time_ms);
	}
	if (bs < FLAC_MIN_BLOCKSIZE || bs > FLAC_MAX_BLOCKSIZE) {
		return -1;
	}
	for (i=0; i<15; i++) {
		if (bs == flac_blocksizes[i]) {
			break;
		}
	}
	if (i == 15 || (s->sample_rate <= 48000 && bs > 4608)) {
		subset = 1;
	}

	if (params->prediction_type < 0 || params->prediction_type > 2) {
		return -1;
	}

	if (params->min_prediction_order > params->max_prediction_order) {
		return -1;
	}
	if (params->prediction_type == FLAKE_PREDICTION_FIXED) {
		if (params->min_prediction_order < 0 ||
		                params->min_prediction_order > 4) {
			return -1;
		}
		if (params->max_prediction_order < 0 ||
		                params->max_prediction_order > 4) {
			return -1;
		}
	} else {
		if (params->min_prediction_order < 1 ||
		                params->min_prediction_order > 32) {
			return -1;
		}
		if (params->max_prediction_order < 1 ||
		                params->max_prediction_order > 32) {
			return -1;
		}
		if (s->sample_rate <= 48000 && params->max_prediction_order > 12) {
			subset = 1;
		}
	}

	if (params->min_partition_order > params->max_partition_order) {
		return -1;
	}
	if (params->min_partition_order < 0 || params->min_partition_order > 8) {
		return -1;
	}
	if (params->max_partition_order < 0 || params->max_partition_order > 8) {
		return -1;
	}

	if (params->padding_size < 0 || params->padding_size >= (1<<24)) {
		return -1;
	}

	if (params->variable_block_size < 0 || params->variable_block_size > 2) {
		return -1;
	}
	if (params->variable_block_size > 0) {
		subset = 1;
	}

	return subset;
}

/**
 * Initialize encoder
 */
int
flake_encode_init(FlakeContext *s)
{
	FlacEncodeContext *ctx;
	int i, header_len;

	if (s == NULL) {
		return -1;
	}

	// allocate memory
	ctx = calloc(1, sizeof(FlacEncodeContext));
	s->private_ctx = ctx;

	if (flake_validate_params(s) < 0) {
		return -1;
	}

	ctx->channels = s->channels;
	ctx->ch_code = s->channels-1;

	// find samplerate in table
	for (i=4; i<12; i++) {
		if (s->sample_rate == flac_samplerates[i]) {
			ctx->samplerate = flac_samplerates[i];
			ctx->sr_code[0] = i;
			ctx->sr_code[1] = 0;
			break;
		}
	}
	// if not in table, samplerate is non-standard
	if (i == 12) {
		ctx->samplerate = s->sample_rate;
		if (ctx->samplerate % 1000 == 0 && ctx->samplerate <= 255000) {
			ctx->sr_code[0] = 12;
			ctx->sr_code[1] = ctx->samplerate / 1000;
		} else if (ctx->samplerate % 10 == 0 && ctx->samplerate <= 655350) {
			ctx->sr_code[0] = 14;
			ctx->sr_code[1] = s->sample_rate / 10;
		} else if (ctx->samplerate < 65535) {
			ctx->sr_code[0] = 13;
			ctx->sr_code[1] = ctx->samplerate;
		}
	}

	for (i=1; i<8; i++) {
		if (s->bits_per_sample == flac_bitdepths[i]) {
			ctx->bps = flac_bitdepths[i];
			ctx->bps_code = i;
			break;
		}
	}
	if (i == 8) {
		return -1;
	}
	// FIXME: For now, only 16-bit encoding is supported
	if (ctx->bps != 16) {
		return -1;
	}

	ctx->sample_count = s->samples;

	if (s->params.block_size == 0) {
		s->params.block_size = select_blocksize(ctx->samplerate, s->params.block_time_ms);
	}

	ctx->params = s->params;

	// select LPC precision based on block size
	if (     ctx->params.block_size <=   192) {
		ctx->lpc_precision =  7;
	} else if (ctx->params.block_size <=   384) {
		ctx->lpc_precision =  8;
	} else if (ctx->params.block_size <=   576) {
		ctx->lpc_precision =  9;
	} else if (ctx->params.block_size <=  1152) {
		ctx->lpc_precision = 10;
	} else if (ctx->params.block_size <=  2304) {
		ctx->lpc_precision = 11;
	} else if (ctx->params.block_size <=  4608) {
		ctx->lpc_precision = 12;
	} else if (ctx->params.block_size <=  8192) {
		ctx->lpc_precision = 13;
	} else if (ctx->params.block_size <= 16384) {
		ctx->lpc_precision = 14;
	} else {
		ctx->lpc_precision = 15;
	}

	// set maximum encoded frame size (if larger, re-encodes in verbatim mode)
	if (ctx->channels == 2) {
		ctx->max_frame_size = 16 + ((ctx->params.block_size * (ctx->bps+ctx->bps+1) + 7) >> 3);
	} else {
		ctx->max_frame_size = 16 + ((ctx->params.block_size * ctx->channels * ctx->bps + 7) >> 3);
	}
	s->max_frame_size = ctx->max_frame_size;

	// output header bytes
	ctx->bw = calloc(sizeof(BitWriter), 1);
	s->header = calloc(ctx->params.padding_size + 1024, 1);
	header_len = -1;
	if (s->header != NULL) {
		header_len = write_headers(ctx, s->header);
	}

	ctx->frame_count = 0;

	// initialize CRC & MD5
	crc_init();
	md5_init(&ctx->md5ctx);

	return header_len;
}

/**
 * Initialize the current frame before encoding
 */
static int
init_frame(FlacEncodeContext *ctx)
{
	int i, ch;
	FlacFrame *frame;

	frame = &ctx->frame;

	if (ctx->params.block_time_ms < 0) {
		return -1;
	}
	if (ctx->params.block_size == 0) {
		ctx->params.block_size = select_blocksize(ctx->samplerate, ctx->params.block_time_ms);
	}
	if (ctx->params.block_size < 1 ||
	                ctx->params.block_size > FLAC_MAX_BLOCKSIZE) {
		return -1;
	}

	// set maximum encoded frame size (if larger, re-encodes in verbatim mode)
	if (ctx->channels == 2) {
		ctx->max_frame_size = 16 + ((ctx->params.block_size * (ctx->bps+ctx->bps+1) + 7) >> 3);
	} else {
		ctx->max_frame_size = 16 + ((ctx->params.block_size * ctx->channels * ctx->bps + 7) >> 3);
	}

	// get block size codes
	i = 15;
	if (!ctx->params.variable_block_size) {
		for (i=0; i<15; i++) {
			if (ctx->params.block_size == flac_blocksizes[i]) {
				frame->blocksize = flac_blocksizes[i];
				frame->bs_code[0] = i;
				frame->bs_code[1] = -1;
				break;
			}
		}
	}
	if (i == 15) {
		frame->blocksize = ctx->params.block_size;
		if (frame->blocksize <= 256) {
			frame->bs_code[0] = 6;
			frame->bs_code[1] = frame->blocksize-1;
		} else {
			frame->bs_code[0] = 7;
			frame->bs_code[1] = frame->blocksize-1;
		}
	}

	// initialize output bps for each channel
	for (ch=0; ch<ctx->channels; ch++) {
		frame->subframes[ch].obits = ctx->bps;
	}

	return 0;
}

/**
 * Copy channel-interleaved input samples into separate subframes
 */
static void
update_md5_checksum(FlacEncodeContext *ctx, int16_t *samples)
{
	md5_accumulate(&ctx->md5ctx, samples, ctx->channels, ctx->params.block_size);
}

/**
 * Copy channel-interleaved input samples into separate subframes
 */
static void
copy_samples(FlacEncodeContext *ctx, int16_t *samples)
{
	int i, j, ch;
	FlacFrame *frame;

	frame = &ctx->frame;
	for (i=0,j=0; i<frame->blocksize; i++) {
		for (ch=0; ch<ctx->channels; ch++,j++) {
			frame->subframes[ch].samples[i] = samples[j];
		}
	}
}

/**
 * Estimate the best stereo decorrelation mode
 */
static int
calc_decorr_scores(int32_t *left_ch, int32_t *right_ch, int n)
{
	int i, best;
	int32_t lt, rt;
	uint64_t sum[4];
	uint64_t score[4];
	int k;

	// calculate sum of 2nd order residual for each channel
	sum[0] = sum[1] = sum[2] = sum[3] = 0;
	for (i=2; i<n; i++) {
		lt = left_ch[i] - 2*left_ch[i-1] + left_ch[i-2];
		rt = right_ch[i] - 2*right_ch[i-1] + right_ch[i-2];
		sum[2] += abs((lt + rt) >> 1);
		sum[3] += abs(lt - rt);
		sum[0] += abs(lt);
		sum[1] += abs(rt);
	}
	// estimate bit counts
	for (i=0; i<4; i++) {
		k = find_optimal_rice_param(2*sum[i], n);
		sum[i] = rice_encode_count(2*sum[i], n, k);
	}

	// calculate score for each mode
	score[0] = sum[0] + sum[1];
	score[1] = sum[0] + sum[3];
	score[2] = sum[1] + sum[3];
	score[3] = sum[2] + sum[3];

	// return mode with lowest score
	best = 0;
	for (i=1; i<4; i++) {
		if (score[i] < score[best]) {
			best = i;
		}
	}
	switch (best) {
	case 0:
		return FLAC_CHMODE_LEFT_RIGHT;
	case 1:
		return FLAC_CHMODE_LEFT_SIDE;
	case 2:
		return FLAC_CHMODE_RIGHT_SIDE;
	case 3:
		return FLAC_CHMODE_MID_SIDE;
	}
	return FLAC_CHMODE_LEFT_RIGHT;
}

/**
 * Perform stereo channel decorrelation
 */
static void
channel_decorrelation(FlacEncodeContext *ctx)
{
	int i;
	FlacFrame *frame;
	int32_t *left, *right;
	int32_t tmp;

	frame = &ctx->frame;
	left  = frame->subframes[0].samples;
	right = frame->subframes[1].samples;

	if (ctx->channels != 2) {
		frame->ch_mode = FLAC_CHMODE_NOT_STEREO;
		return;
	}
	if (frame->blocksize <= 32 || ctx->params.stereo_method == FLAKE_STEREO_METHOD_INDEPENDENT) {
		frame->ch_mode = FLAC_CHMODE_LEFT_RIGHT;
		return;
	}

	// estimate stereo decorrelation type
	frame->ch_mode = calc_decorr_scores(left, right, frame->blocksize);

	// perform decorrelation and adjust bits-per-sample
	if (frame->ch_mode == FLAC_CHMODE_LEFT_RIGHT) {
		return;
	}
	if (frame->ch_mode == FLAC_CHMODE_MID_SIDE) {
		for (i=0; i<frame->blocksize; i++) {
			tmp = left[i];
			left[i] = (left[i] + right[i]) >> 1;
			right[i] = tmp - right[i];
		}
		frame->subframes[1].obits++;
	} else if (frame->ch_mode == FLAC_CHMODE_LEFT_SIDE) {
		for (i=0; i<frame->blocksize; i++) {
			right[i] = left[i] - right[i];
		}
		frame->subframes[1].obits++;
	} else if (frame->ch_mode == FLAC_CHMODE_RIGHT_SIDE) {
		for (i=0; i<frame->blocksize; i++) {
			left[i] = left[i] - right[i];
		}
		frame->subframes[0].obits++;
	}
}

/**
 * Write UTF-8 encoded integer value
 * Used to encode frame number in frame header
 */
static void
write_utf8(BitWriter *bw, uint32_t val)
{
	int bytes, shift;

	if (val < 0x80) {
		bitwriter_writebits(bw, 8, val);
		return;
	}
	bytes = (log2i(val)+4) / 5;
	shift = (bytes - 1) * 6;
	bitwriter_writebits(bw, 8, (256 - (256>>bytes)) | (val >> shift));
	while (shift >= 6) {
		shift -= 6;
		bitwriter_writebits(bw, 8, 0x80 | ((val >> shift) & 0x3F));
	}
}

static void
output_frame_header(FlacEncodeContext *ctx)
{
	FlacFrame *frame;
	uint8_t crc;

	frame = &ctx->frame;

	bitwriter_writebits(ctx->bw, 16, 0xFFF8);
	bitwriter_writebits(ctx->bw, 4, frame->bs_code[0]);
	bitwriter_writebits(ctx->bw, 4, ctx->sr_code[0]);
	if (frame->ch_mode == FLAC_CHMODE_NOT_STEREO) {
		bitwriter_writebits(ctx->bw, 4, ctx->ch_code);
	} else {
		bitwriter_writebits(ctx->bw, 4, frame->ch_mode);
	}
	bitwriter_writebits(ctx->bw, 3, ctx->bps_code);
	bitwriter_writebits(ctx->bw, 1, 0);
	write_utf8(ctx->bw, ctx->frame_count);

	// custom block size
	if (frame->bs_code[1] >= 0) {
		if (frame->bs_code[1] < 256) {
			bitwriter_writebits(ctx->bw, 8, frame->bs_code[1]);
		} else {
			bitwriter_writebits(ctx->bw, 16, frame->bs_code[1]);
		}
	}

	// custom sample rate
	if (ctx->sr_code[1] > 0) {
		if (ctx->sr_code[1] < 256) {
			bitwriter_writebits(ctx->bw, 8, ctx->sr_code[1]);
		} else {
			bitwriter_writebits(ctx->bw, 16, ctx->sr_code[1]);
		}
	}

	// CRC-8 of frame header
	bitwriter_flush(ctx->bw);
	crc = calc_crc8(ctx->bw->buffer, bitwriter_count(ctx->bw));
	bitwriter_writebits(ctx->bw, 8, crc);
}

static void
output_residual(FlacEncodeContext *ctx, int ch)
{
	int i, j, p;
	int k, porder, psize, res_cnt;
	FlacFrame *frame;
	FlacSubframe *sub;

	frame = &ctx->frame;
	sub = &frame->subframes[ch];

	// rice-encoded block
	bitwriter_writebits(ctx->bw, 2, 0);

	// partition order
	porder = sub->rc.porder;
	psize = frame->blocksize >> porder;
	assert(porder >= 0);
	bitwriter_writebits(ctx->bw, 4, porder);
	res_cnt = psize - sub->order;

	// residual
	j = sub->order;
	for (p=0; p<(1 << porder); p++) {
		k = sub->rc.params[p];
		bitwriter_writebits(ctx->bw, 4, k);
		if (p == 1) {
			res_cnt = psize;
		}
		for (i=0; i<res_cnt && j<frame->blocksize; i++, j++) {
			bitwriter_write_rice_signed(ctx->bw, k, sub->residual[j]);
		}
	}
}

static void
output_subframe_constant(FlacEncodeContext *ctx, int ch)
{
	FlacSubframe *sub;

	sub = &ctx->frame.subframes[ch];
	bitwriter_writebits_signed(ctx->bw, sub->obits, sub->residual[0]);
}

static void
output_subframe_verbatim(FlacEncodeContext *ctx, int ch)
{
	int i, n;
	FlacFrame *frame;
	FlacSubframe *sub;

	frame = &ctx->frame;
	sub = &frame->subframes[ch];
	n = frame->blocksize;

	for (i=0; i<n; i++) {
		bitwriter_writebits_signed(ctx->bw, sub->obits, sub->residual[i]);
	}
}

static void
output_subframe_fixed(FlacEncodeContext *ctx, int ch)
{
	int i;
	FlacFrame *frame;
	FlacSubframe *sub;

	frame = &ctx->frame;
	sub = &frame->subframes[ch];

	// warm-up samples
	for (i=0; i<sub->order; i++) {
		bitwriter_writebits_signed(ctx->bw, sub->obits, sub->residual[i]);
	}

	// residual
	output_residual(ctx, ch);
}

static void
output_subframe_lpc(FlacEncodeContext *ctx, int ch)
{
	int i, cbits;
	FlacFrame *frame;
	FlacSubframe *sub;

	frame = &ctx->frame;
	sub = &frame->subframes[ch];

	// warm-up samples
	for (i=0; i<sub->order; i++) {
		bitwriter_writebits_signed(ctx->bw, sub->obits, sub->residual[i]);
	}

	// LPC coefficients
	cbits = ctx->lpc_precision;
	bitwriter_writebits(ctx->bw, 4, cbits-1);
	bitwriter_writebits_signed(ctx->bw, 5, sub->shift);
	for (i=0; i<sub->order; i++) {
		bitwriter_writebits_signed(ctx->bw, cbits, sub->coefs[i]);
	}

	// residual
	output_residual(ctx, ch);
}

static void
output_subframes(FlacEncodeContext *ctx)
{
	FlacFrame *frame;
	int i, ch;

	frame = &ctx->frame;

	for (i=0; i<ctx->channels; i++) {
		ch = i;

		// subframe header
		bitwriter_writebits(ctx->bw, 1, 0);
		bitwriter_writebits(ctx->bw, 6, frame->subframes[ch].type_code);
		bitwriter_writebits(ctx->bw, 1, 0);

		// subframe
		switch (frame->subframes[ch].type) {
		case FLAC_SUBFRAME_CONSTANT:
			output_subframe_constant(ctx, ch);
			break;
		case FLAC_SUBFRAME_VERBATIM:
			output_subframe_verbatim(ctx, ch);
			break;
		case FLAC_SUBFRAME_FIXED:
			output_subframe_fixed(ctx, ch);
			break;
		case FLAC_SUBFRAME_LPC:
			output_subframe_lpc(ctx, ch);
			break;
		}
	}
}

static void
output_frame_footer(FlacEncodeContext *ctx)
{
	uint16_t crc;
	bitwriter_flush(ctx->bw);
	crc = calc_crc16(ctx->bw->buffer, bitwriter_count(ctx->bw));
	bitwriter_writebits(ctx->bw, 16, crc);
	bitwriter_flush(ctx->bw);
}

int
encode_frame(FlakeContext *s, uint8_t *frame_buffer, int16_t *samples)
{
	int i, ch;
	FlacEncodeContext *ctx;

	ctx = (FlacEncodeContext *) s->private_ctx;
	if (ctx == NULL) {
		return -1;
	}

	ctx->params.block_size = s->params.block_size;
	if (init_frame(ctx)) {
		return -1;
	}
	s->params.block_size = ctx->params.block_size;

	if (frame_buffer != NULL) {
		update_md5_checksum(ctx, samples);
	}

	copy_samples(ctx, samples);

	channel_decorrelation(ctx);

	for (ch=0; ch<ctx->channels; ch++) {
		if (encode_residual(ctx, ch) < 0) {
			return -1;
		}
	}

	bitwriter_init(ctx->bw, frame_buffer, ctx->max_frame_size);
	output_frame_header(ctx);
	output_subframes(ctx);
	output_frame_footer(ctx);

	if (ctx->bw->eof) {
		// frame size too large, reencode in verbatim mode
		for (i=0; i<ctx->channels; i++) {
			ch = i;
			reencode_residual_verbatim(ctx, ch);
		}
		bitwriter_init(ctx->bw, frame_buffer, ctx->max_frame_size);
		output_frame_header(ctx);
		output_subframes(ctx);
		output_frame_footer(ctx);

		// if still too large, means my estimate is wrong.
		assert(!ctx->bw->eof);
	}
	if (frame_buffer != NULL) {
		if (ctx->params.variable_block_size) {
			ctx->frame_count += s->params.block_size;
		} else {
			ctx->frame_count++;
		}
	}
	return bitwriter_count(ctx->bw);
}

int
flake_encode_frame(FlakeContext *s, uint8_t *frame_buffer, int16_t *samples)
{
	int fs;
	FlacEncodeContext *ctx;

	ctx = (FlacEncodeContext *) s->private_ctx;
	fs = -1;
	if ((ctx->params.variable_block_size > 0) &&
	                !(s->params.block_size & 7) && s->params.block_size >= 128) {
		fs = encode_frame_vbs(s, frame_buffer, samples);
	} else {
		fs = encode_frame(s, frame_buffer, samples);
	}
	return fs;
}

void
flake_encode_close(FlakeContext *s)
{
	FlacEncodeContext *ctx;

	if (s == NULL) {
		return;
	}
	if (s->private_ctx == NULL) {
		return;
	}
	ctx = (FlacEncodeContext *) s->private_ctx;
	if (ctx) {
		md5_final(s->md5digest, &ctx->md5ctx);
		if (ctx->bw) {
			free(ctx->bw);
		}
		free(ctx);
	}
	if (s->header) {
		free(s->header);
	}
	s->private_ctx = NULL;
}

int flac_encode(char *name, void *data, int len, int sampleRate, int sampleDepth, int channels, int compr)
{
	FILE *fp = fopen(name, "wb");
	if (!fp) return 1;

	// set parameters from input audio
	FlakeContext s;
	s.channels = channels;
	s.sample_rate = sampleRate;
	s.bits_per_sample = sampleDepth;
	s.samples = len;
	s.params.compression = compr;
	if (flake_set_defaults(&s.params)) return 1;

	// initialize encoder
	int header_size = flake_encode_init(&s);
	if (header_size < 0) {
		flake_encode_close(&s);
		fprintf(stderr, "Error initializing encoder.\n");
		return 1;
	}
	fwrite(s.header, 1, header_size, fp);
	//print_params(&s);

	uint8_t frame[s.max_frame_size];
	uint8_t *pcm = data;
	for (int n=len; n>0; n-=s.params.block_size) {
		int fs = flake_encode_frame(&s, frame, (short*)pcm);
		pcm += s.params.block_size * channels * sampleDepth/8;
		if (fs < 0) {
			fprintf(stderr, "Error encoding frame\n");
		} else if (fs > 0) {
			fwrite(frame, 1, fs, fp);
		}
//		printf(".");
	}
	flake_encode_close(&s);
	fclose(fp);

	return 0;
}
