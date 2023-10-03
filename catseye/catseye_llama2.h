//---------------------------------------------------------
//	Cat's eye
//
//		©2016-2023 Yuichiro Nakada
//---------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <fcntl.h>

#if defined _WIN32
  #include "win.h"
#else
  #include <sys/mman.h>
  #include <unistd.h>
#endif

#if defined(__SSE__) || defined(__AVX__)
  #include <xmmintrin.h>
  #include <immintrin.h>
#endif

#ifdef DTYPE
typedef DTYPE	real; // _Float16
#else
#define real	float
#endif

#define ALIGN		256
#if defined(_MSC_VER) || defined(__MINGW32__)
#define malloc(size)	_aligned_malloc(size, ALIGN)
#define free(p)		_aligned_free(p)
#else
#define malloc(size)	({ void* _p; posix_memalign((void**) &_p, ALIGN, (size))==0 ? _p : NULL; })
#define free(p)		free(p)
#endif  /* _MSC_VER */
//#define calloc(n, size)	({ uint64_t _s = n * size; void* _p = malloc(_s); memset(_p, 0, _s)!=0 ? _p : NULL; })
#define calloc(n, size)	({ uint64_t _s = n * size; void* _p; posix_memalign((void**) &_p, ALIGN, (_s))==0 ? _p : NULL; })

#undef MIN
#undef MAX
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

//---------------------------------------------------------
// Quantize

#define QK4_0	32
#define QK8	32

// FP16 <-> FP32
#ifdef __F16C__
#ifdef _MSC_VER
#define cats_fp16_to_fp32(x) _mm_cvtss_f32(_mm_cvtph_ps(_mm_cvtsi32_si128(x)))
#define cats_compute_fp32_to_fp16(x) _mm_extract_epi16(_mm_cvtps_ph(_mm_set_ss(x), 0), 0)
#else
#define cats_fp16_to_fp32(x) _cvtsh_ss(x)
#define cats_compute_fp32_to_fp16(x) _cvtss_sh(x, 0)
#endif
#else
// ref: https://github.com/Maratyszcza/FP16
static inline float fp32_from_bits(uint32_t w)
{
	union {
		uint32_t as_bits;
		float as_value;
	} fp32;
	fp32.as_bits = w;
	return fp32.as_value;
}
static inline uint32_t fp32_to_bits(real f)
{
	union {
		float as_value;
		uint32_t as_bits;
	} fp32;
	fp32.as_value = f;
	return fp32.as_bits;
}
static inline float cats_fp16_to_fp32(uint16_t h)
{
	const uint32_t w = (uint32_t) h << 16;
	const uint32_t sign = w & UINT32_C(0x80000000);
	const uint32_t two_w = w + w;

	const uint32_t exp_offset = UINT32_C(0xE0) << 23;
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) || defined(__GNUC__) && !defined(__STRICT_ANSI__)
	const float exp_scale = 0x1.0p-112f;
#else
	const float exp_scale = fp32_from_bits(UINT32_C(0x7800000));
#endif
	const float normalized_value = fp32_from_bits((two_w >> 4) + exp_offset) * exp_scale;

	const uint32_t magic_mask = UINT32_C(126) << 23;
	const float magic_bias = 0.5f;
	const float denormalized_value = fp32_from_bits((two_w >> 17) | magic_mask) - magic_bias;

	const uint32_t denormalized_cutoff = UINT32_C(1) << 27;
	const uint32_t result = sign |
		(two_w < denormalized_cutoff ? fp32_to_bits(denormalized_value) : fp32_to_bits(normalized_value));
	return fp32_from_bits(result);
}
static inline uint16_t cats_compute_fp32_to_fp16(real f)
{
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) || defined(__GNUC__) && !defined(__STRICT_ANSI__)
	const float scale_to_inf = 0x1.0p+112f;
	const float scale_to_zero = 0x1.0p-110f;
#else
	const float scale_to_inf = fp32_from_bits(UINT32_C(0x77800000));
	const float scale_to_zero = fp32_from_bits(UINT32_C(0x08800000));
#endif
	float base = (fabsf(f) * scale_to_inf) * scale_to_zero;

	const uint32_t w = fp32_to_bits(f);
	const uint32_t shl1_w = w + w;
	const uint32_t sign = w & UINT32_C(0x80000000);
	uint32_t bias = shl1_w & UINT32_C(0xFF000000);
	if (bias < UINT32_C(0x71000000)) {
		bias = UINT32_C(0x71000000);
	}

	base = fp32_from_bits((bias >> 1) + UINT32_C(0x07800000)) + base;
	const uint32_t bits = fp32_to_bits(base);
	const uint32_t exp_bits = (bits >> 13) & UINT32_C(0x00007C00);
	const uint32_t mantissa_bits = bits & UINT32_C(0x00000FFF);
	const uint32_t nonsign = exp_bits + mantissa_bits;
	return (sign >> 16) | (shl1_w > UINT32_C(0xFF000000) ? UINT16_C(0x7E00) : nonsign);
}
#endif

#ifdef __AVX__
static void quantize_q8(int8_t* q, real* x, int n)
{
	int nb = n / QK8;
	for (int i=0; i<nb; i++) {
		// Load elements into 4 AVX vectors
		__m256 v0 = _mm256_load_ps(x);
		__m256 v1 = _mm256_load_ps(x+8);
		__m256 v2 = _mm256_load_ps(x+16);
		__m256 v3 = _mm256_load_ps(x+24);
		x += 32;

		// Compute max(abs(e)) for the block
		const __m256 signBit = _mm256_set1_ps(-0.0);
		__m256 maxAbs = _mm256_andnot_ps(signBit, v0);
		maxAbs = _mm256_max_ps(maxAbs, _mm256_andnot_ps(signBit, v1));
		maxAbs = _mm256_max_ps(maxAbs, _mm256_andnot_ps(signBit, v2));
		maxAbs = _mm256_max_ps(maxAbs, _mm256_andnot_ps(signBit, v3));

		__m128 max4 = _mm_max_ps(_mm256_extractf128_ps(maxAbs, 1), _mm256_castps256_ps128(maxAbs));
		max4 = _mm_max_ps(max4, _mm_movehl_ps(max4, max4));
		max4 = _mm_max_ss(max4, _mm_movehdup_ps(max4));
		const float maxScalar = _mm_cvtss_f32(max4);

		// Quantize these floats
		const float d = maxScalar / 127.;
//		y[i].d = GGML_FP32_TO_FP16(d);
		const float id = ( maxScalar != 0.0 ) ? 127. / maxScalar : 0.0;
		const __m256 mul = _mm256_set1_ps(id);

		// Apply the multiplier
		v0 = _mm256_mul_ps(v0, mul);
		v1 = _mm256_mul_ps(v1, mul);
		v2 = _mm256_mul_ps(v2, mul);
		v3 = _mm256_mul_ps(v3, mul);

		// Round to nearest integer
		v0 = _mm256_round_ps(v0, _MM_ROUND_NEAREST);
		v1 = _mm256_round_ps(v1, _MM_ROUND_NEAREST);
		v2 = _mm256_round_ps(v2, _MM_ROUND_NEAREST);
		v3 = _mm256_round_ps(v3, _MM_ROUND_NEAREST);

		// Convert floats to integers
		__m256i i0 = _mm256_cvtps_epi32(v0);
		__m256i i1 = _mm256_cvtps_epi32(v1);
		__m256i i2 = _mm256_cvtps_epi32(v2);
		__m256i i3 = _mm256_cvtps_epi32(v3);

		__m128i ni0 = _mm256_castsi256_si128(i0);
		__m128i ni1 = _mm256_extractf128_si256(i0, 1);
		__m128i ni2 = _mm256_castsi256_si128(i1);
		__m128i ni3 = _mm256_extractf128_si256(i1, 1);
		__m128i ni4 = _mm256_castsi256_si128(i2);
		__m128i ni5 = _mm256_extractf128_si256(i2, 1);
		__m128i ni6 = _mm256_castsi256_si128(i3);
		__m128i ni7 = _mm256_extractf128_si256(i3, 1);

		// Convert int32 to int16
		ni0 = _mm_packs_epi32(ni0, ni1);
		ni2 = _mm_packs_epi32(ni2, ni3);
		ni4 = _mm_packs_epi32(ni4, ni5);
		ni6 = _mm_packs_epi32(ni6, ni7);
		// Convert int16 to int8
		ni0 = _mm_packs_epi16(ni0, ni2);
		ni4 = _mm_packs_epi16(ni4, ni6);

		int8_t *p = q + i*(32+sizeof(float));
		_mm_storeu_si128((__m128i *)(p +  0), ni0);
		_mm_storeu_si128((__m128i *)(p + 16), ni4);
		*(float*)(p + 32) = d;
	}
}
#else
static void quantize_q8(int8_t* q, real* x, int n)
{
	int num_groups = n / QK8;

	for (int group=0; group<num_groups; group++) {
		// find the max absolute value in the current group
		real amax = 0.0;
		for (int i=0; i<QK8; i++) {
			real v = fabsf(x[group * QK8 + i]);
			amax = MAX(amax, v);
		}

		// calculate and write the scaling factor
		int8_t* p = q + group * (32+sizeof(float));
		const float d = amax / 127.0;
		const float id = d ? 1.0/d : 0.0;

		// calculate and write the quantized values
		for (int i=0; i<QK8; i++) {
			float quant_value = x[group * QK8 + i] * id;
			*p++ = roundf(quant_value); // round and clamp
		}
		*((float*)p) = d;
		p += sizeof(float);
	}
}
#endif
static void no_quantize(int8_t* q, real* x, int n) {}

static inline int nearest_int(float fval)
{
//	assert(fval <= 4194303.f);
	float val = fval + 12582912.f;
	int i; memcpy(&i, &val, sizeof(int));
	return (i & 0x007fffff) - 0x00400000;
}
static inline void quantize_q8_K(int8_t* y, real* x, int k)
{
	const int nb = k / 256;
	for (int i=0; i<nb; i++) {
		float max = 0;
		float amax = 0;
		for (int j=0; j<256; ++j) {
			float ax = fabsf(x[j]);
			if (ax > amax) {
				amax = ax; max = x[j];
			}
		}
		if (!amax) {
			memset(y, 0, 256+4);
			x += 256;
			y += 256+4;
			continue;
		}
		const float iscale = -128./max;
		for (int j=0; j<256; ++j) {
			int v = nearest_int(iscale*x[j]);
			*y++ = MIN(127, v);
		}
		/*for (int j=0; j<256/16; ++j) {
			int sum = 0;
			for (int ii = 0; ii < 16; ++ii) {
				sum += y[i].qs[j*16 + ii];
			}
			y[i].bsums[j] = sum;
		}*/
		*(float*)y = 1/iscale;
		y += sizeof(float);
		x += 256;
	}
}
static inline void* dequantize_q3_K(void * restrict _y, const void * restrict x, size_t k)
{
	const int nb = k / 256;
	float * restrict y = (float*)_y;
	const uint32_t kmask1 = 0x03030303;
	const uint32_t kmask2 = 0x0f0f0f0f;

	uint32_t aux[4];
	const int8_t * scales = (const int8_t*)aux;
	for (int i=0; i<nb; i++) {
		uint8_t *p = (uint8_t*)(x + i*(256/8 +256/4 +12 +2));
		const float d_all = cats_fp16_to_fp32(*((uint16_t*)(p +256/8 +256/4 +12)));

		const uint8_t * restrict q = p +256/8;
		const uint8_t * restrict hm = p;
		uint8_t m = 1;

		const int8_t  * restrict sc = p +256/8 +256/4;
		memcpy(aux, sc, 12);
		uint32_t tmp = aux[2];
		aux[2] = ((aux[0] >> 4) & kmask2) | (((tmp >> 4) & kmask1) << 4);
		aux[3] = ((aux[1] >> 4) & kmask2) | (((tmp >> 6) & kmask1) << 4);
		aux[0] = (aux[0] & kmask2) | (((tmp >> 0) & kmask1) << 4);
		aux[1] = (aux[1] & kmask2) | (((tmp >> 2) & kmask1) << 4);

		int is = 0;
		float dl;
		for (int n=0; n<256; n += 128) {
			int shift = 0;
			for (int j=0; j<4; ++j) {
				dl = d_all * (scales[is++] - 32);
				for (int l=0; l<16; ++l) {
					*y++ = dl * ((int8_t)((q[l+ 0] >> shift) & 3) - ((hm[l+ 0] & m) ? 0 : 4));
				}
				dl = d_all * (scales[is++] - 32);
				for (int l=0; l<16; ++l) {
					*y++ = dl * ((int8_t)((q[l+16] >> shift) & 3) - ((hm[l+16] & m) ? 0 : 4));
				}
				shift += 2;
				m <<= 1;
			}
			q += 32;
		}
	}
}
static inline void* dequantize_q6_K(void * restrict _y, const void * restrict x, size_t k)
{
	const int nb = k / 256;
	float * restrict y = (float*)_y;
	for (int i=0; i<nb; i++) {
		uint8_t *p = (uint8_t*)(x + i*(256/2 +256/4 +256/16 +2));
		const float d = cats_fp16_to_fp32(*((uint16_t*)(p +256/2 +256/4 +256/16)));

		const uint8_t * restrict ql = p;
		const uint8_t * restrict qh = p +256/2;
		const int8_t  * restrict sc = p +256/2 +256/4;

		for (int n=0; n<2; n++) {
			for (int l=0; l<32; ++l) {
				int is = l/16;
				const int8_t q1 = (int8_t)((ql[l +  0] & 0xF) | (((qh[l] >> 0) & 3) << 4)) - 32;
				const int8_t q2 = (int8_t)((ql[l + 32] & 0xF) | (((qh[l] >> 2) & 3) << 4)) - 32;
				const int8_t q3 = (int8_t)((ql[l +  0]  >> 4) | (((qh[l] >> 4) & 3) << 4)) - 32;
				const int8_t q4 = (int8_t)((ql[l + 32]  >> 4) | (((qh[l] >> 6) & 3) << 4)) - 32;
				y[l +  0] = d * sc[is + 0] * q1;
				y[l + 32] = d * sc[is + 2] * q2;
				y[l + 64] = d * sc[is + 4] * q3;
				y[l + 96] = d * sc[is + 6] * q4;
			}
			y  += 128;
			ql += 64;
			qh += 32;
			sc += 8;
		}
	}
	return 0;
}
static inline void* dequantize_q4_0(void * restrict y, const void * restrict x, size_t k)
{
	const int nb = k / 32;
	for (int i=0; i<nb; i++) {
		uint8_t *p = (uint8_t*)(x + i*18);
		const float delta = cats_fp16_to_fp32(*((uint16_t*)p));
		p += 2;

		float *y0 = (float*)(y + i*32*sizeof(float));
		float *y1 = y0 + 16;
		*y0++ = ((*p   & 0x0F) - 8)*delta;
		*y1++ = ((*p++ >>   4) - 8)*delta;
		*y0++ = ((*p   & 0x0F) - 8)*delta;
		*y1++ = ((*p++ >>   4) - 8)*delta;
		*y0++ = ((*p   & 0x0F) - 8)*delta;
		*y1++ = ((*p++ >>   4) - 8)*delta;
		*y0++ = ((*p   & 0x0F) - 8)*delta;
		*y1++ = ((*p++ >>   4) - 8)*delta;

		*y0++ = ((*p   & 0x0F) - 8)*delta;
		*y1++ = ((*p++ >>   4) - 8)*delta;
		*y0++ = ((*p   & 0x0F) - 8)*delta;
		*y1++ = ((*p++ >>   4) - 8)*delta;
		*y0++ = ((*p   & 0x0F) - 8)*delta;
		*y1++ = ((*p++ >>   4) - 8)*delta;
		*y0++ = ((*p   & 0x0F) - 8)*delta;
		*y1++ = ((*p++ >>   4) - 8)*delta;

		*y0++ = ((*p   & 0x0F) - 8)*delta;
		*y1++ = ((*p++ >>   4) - 8)*delta;
		*y0++ = ((*p   & 0x0F) - 8)*delta;
		*y1++ = ((*p++ >>   4) - 8)*delta;
		*y0++ = ((*p   & 0x0F) - 8)*delta;
		*y1++ = ((*p++ >>   4) - 8)*delta;
		*y0++ = ((*p   & 0x0F) - 8)*delta;
		*y1++ = ((*p++ >>   4) - 8)*delta;

		*y0++ = ((*p   & 0x0F) - 8)*delta;
		*y1++ = ((*p++ >>   4) - 8)*delta;
		*y0++ = ((*p   & 0x0F) - 8)*delta;
		*y1++ = ((*p++ >>   4) - 8)*delta;
		*y0++ = ((*p   & 0x0F) - 8)*delta;
		*y1++ = ((*p++ >>   4) - 8)*delta;
		*y0++ = ((*p   & 0x0F) - 8)*delta;
		*y1++ = ((*p++ >>   4) - 8)*delta;
	}
	return 0;
}
static void* _dequantize_q3_K(void * restrict a, const void * restrict b, int n, int k)
{
	dequantize_q3_K(a, b +n*k/256*110, k);
}
static void* _dequantize_q6_K(void * restrict a, const void * restrict b, int n, int k)
{
	dequantize_q6_K(a, b +n*k/256*(256/2 +256/4 +256/16 +2), k);
}
static void* _dequantize_q4_0(void * restrict a, const void * restrict b, int n, int k)
{
	dequantize_q4_0(a, b +n*(k/32*18), k);
}
static void* _dequantize_memcpy(void * restrict a, const void * restrict b, int n, int k)
{
	memcpy(a, b +n*k*sizeof(real), k*sizeof(real));
}

//---------------------------------------------------------
// utilities

static long time_in_ms()
{
	// return time in milliseconds, for benchmarking the model speed
	struct timespec time;
	clock_gettime(CLOCK_REALTIME, &time);
	return time.tv_sec * 1000 + time.tv_nsec / 1000000;
}

uint64_t rng_seed;
static uint32_t random_u32()
{
	// xorshift rng: https://en.wikipedia.org/wiki/Xorshift#xorshift.2A
	rng_seed ^= rng_seed >> 12;
	rng_seed ^= rng_seed << 25;
	rng_seed ^= rng_seed >> 27;
	return (rng_seed * 0x2545F4914F6CDD1Dull) >> 32;
}
static float random_f32()
{
	// random float32 in [0,1)
	return (random_u32() >> 8) / 16777216.0;
}

//---------------------------------------------------------
// define struct

static void* (*dequantize)(void * restrict, const void * restrict, int, int) = _dequantize_memcpy;
static void (*quantize)(int8_t*, real*, int) = no_quantize;
static void (*matmul)(real* restrict, void* restrict, void* restrict, int, int);

#define CATS_NAME_MAX		48
#define CATS_MAX_DIM		4
typedef struct {
	char name[CATS_NAME_MAX];
	int type;
	uint32_t n_dim;
	uint64_t dim[CATS_MAX_DIM];
	void *data;

	void (*quantize)(int8_t*, real*, int);
	void (*matmul)(real* restrict, void* restrict, void* restrict, int, int);

	uint64_t size;		// size of the file
	size_t offset;		// offset of the file
} cats_tensor;

#define CATS_LLAMA_LAYER	256 // 32
typedef struct {
	// GGML header
	uint32_t n_vocab;	// vocab size: 32000
	uint32_t n_embd;	// dimention: 4096
	uint32_t n_mult;	// 256
	uint32_t n_head;	// 32
	uint32_t n_layer;	// 32
	uint32_t n_rot;		// 128
	uint32_t ftype;
	// optional data
	uint32_t n_hidden;	// hidden dimention
	uint32_t n_kv_head;
	uint32_t seq_len;	// max sequence length

	// vocab
	char **vocab;
	real *score;
	uint8_t byte_pieces[512]; // stores all single-byte strings
	uint32_t max_token_length;

	// weights
	cats_tensor tensor[3+9*CATS_LLAMA_LAYER];

	cats_tensor *token_embedding_table;		// (vocab_size, dim)
	cats_tensor *rms_att_weight[CATS_LLAMA_LAYER];	// (layer, dim) rmsnorm weights
	cats_tensor *rms_ffn_weight[CATS_LLAMA_LAYER];	// (layer, dim)
	cats_tensor *wq[CATS_LLAMA_LAYER];		// (layer, dim, n_heads * head_size)
	cats_tensor *wk[CATS_LLAMA_LAYER];		// (layer, dim, n_kv_heads * head_size)
	cats_tensor *wv[CATS_LLAMA_LAYER];		// (layer, dim, n_kv_heads * head_size)
	cats_tensor *wo[CATS_LLAMA_LAYER];		// (layer, n_heads * head_size, dim)
	cats_tensor *w1[CATS_LLAMA_LAYER];		// (layer, hidden_dim, dim)
	cats_tensor *w2[CATS_LLAMA_LAYER];		// (layer, dim, hidden_dim)
	cats_tensor *w3[CATS_LLAMA_LAYER];		// (layer, hidden_dim, dim)
	cats_tensor *rms_final_weight;			// (dim,)
	cats_tensor *wcls;				// (optional) classifier weights for the logits, on the last layer

	// state
	real *x;		// activation at current time stamp (dim,)
	real *xb;		// same, but inside a residual branch (dim,)
	real *xb2;		// an additional buffer just for convenience (dim,)
	real *hb;		// buffer for hidden dimension in the ffn (hidden_dim,)
	real *hb2;		// buffer for hidden dimension in the ffn (hidden_dim,)
	real *q;		// query (dim,)
//	real *k;		// key (dim,)
//	real *v;		// value (dim,)
	real *att;		// buffer for scores/attention values (n_heads, seq_len)
	real *logits;		// output logits
	real *key_cache;	// (layer, seq_len, dim)
	real *value_cache;	// (layer, seq_len, dim)
} cats_ggml_model;

//---------------------------------------------------------

static void malloc_run_state(cats_ggml_model* m)
{
	// we calloc instead of malloc to keep valgrind happy
	int kv_dim = (m->n_embd * m->n_kv_head) / m->n_head;
	m->x = calloc(m->n_embd, sizeof(real));
	m->xb = calloc(m->n_embd, sizeof(real));
	m->xb2 = calloc(m->n_embd, sizeof(real));
	m->hb = calloc(m->n_hidden, sizeof(real));
	m->hb2 = calloc(m->n_hidden, sizeof(real));
	m->q = calloc(m->n_embd, sizeof(real));
//	m->k = calloc(kv_dim, sizeof(real));
//	m->v = calloc(kv_dim, sizeof(real));
	m->att = calloc(m->n_head * m->seq_len, sizeof(real));
	m->logits = calloc(m->n_vocab, sizeof(real));
	m->key_cache = calloc(m->n_layer * m->seq_len * kv_dim, sizeof(real));
	m->value_cache = calloc(m->n_layer * m->seq_len * kv_dim, sizeof(real));
	// ensure all mallocs went fine
	if (!m->x || !m->xb || !m->xb2 || !m->hb || !m->hb2 || !m->q
		|| /*!m->k || !m->v ||*/ !m->att || !m->logits || !m->key_cache
		|| !m->value_cache/* || !s->probindex*/) {
		fprintf(stderr, "malloc_run_state: malloc failed!\n");
		exit(1);
	}
}

static void free_run_state(cats_ggml_model* m)
{
	free(m->x);
	free(m->xb);
	free(m->xb2);
	free(m->hb);
	free(m->hb2);
	free(m->q);
//	free(m->k);
//	free(m->v);
	free(m->att);
	free(m->logits);
	free(m->key_cache);
	free(m->value_cache);
}

//---------------------------------------------------------
// neural net blocks

// Root Mean Square Normalization
static inline void rmsnorm(real* o, real* x, real* weight, int size)
{
	// calculate sum of squares
	real ss = 0.0;
	for (int j=0; j<size; j++) {
		ss += x[j] * x[j];
	}
	ss /= size;
	ss += 1e-5;
	ss = 1.0 / sqrtf(ss);
	// normalize and scale
	for (int j=0; j<size; j++) {
		o[j] = weight[j] * (ss * x[j]);
	}
}

static inline void softmax(real* x, int size)
{
	// find max value (for numerical stability)
	real max_val = x[0];
	for (int i=1; i<size; i++) {
		if (x[i] > max_val) {
			max_val = x[i];
		}
	}
	// exp and sum
	real sum = 0.0;
	for (int i=0; i<size; i++) {
		x[i] = expf(x[i] - max_val);
		sum += x[i];
	}
	// normalize
//	float inv_sum = 1.0 / sum;
	for (int i=0; i<size; i++) {
		x[i] /= sum;
//		x[i] *= inv_sum;
	}
}

#ifdef __AVX__
//#include "catseye_llama2_matmul.h"
static inline void sgemm7_(const int M, const int N, const int K, const float *A, const float *B, float *C)
{
	int j, k;
	int nn = N>>1<<1;
	int kk = K&~(8-1);
	memset(C, 0, M*N*sizeof(real));
	#pragma omp parallel for
	for (int i=0; i<M; i+=2) {
		for (j=0; j<nn; j+=2) {
			/*static*/ __attribute__((aligned(32))) real c[4] = {0};
			for (int k=0; k<K; k++) {
				float A0k = A[i*K+k];
				float A1k = A[(i+1)*K+k];
				float Bk0 = B[k*N+j];
				float Bk1 = B[k*N+j+1];

				c[0] += A0k * Bk0;
				c[1] += A0k * Bk1;
				c[2] += A1k * Bk0;
				c[3] += A1k * Bk1;
			}
			C[i    *N+j  ] += c[0];
			C[i    *N+j+1] += c[1];
			C[(i+1)*N+j  ] += c[2];
			C[(i+1)*N+j+1] += c[3];
		}
		for (; j<N; j++) {
			__m256 sum0_vec = _mm256_setzero_ps(); // for AVX2, sum of 8 reals
			__m256 sum1_vec = _mm256_setzero_ps();
			for (k=0; k<kk; k+=8) {
				__m256 a0_vec = _mm256_load_ps(&A[i*K+k]);
				__m256 a1_vec = _mm256_load_ps(&A[(i+1)*K+k]);
				__m256 b_vec = _mm256_load_ps(&B[k*N+j]);

				sum0_vec = _mm256_add_ps(sum0_vec, _mm256_mul_ps(a0_vec, b_vec));
				sum1_vec = _mm256_add_ps(sum1_vec, _mm256_mul_ps(a1_vec, b_vec));
			}
			sum0_vec = _mm256_hadd_ps(sum0_vec, sum0_vec);
			sum0_vec = _mm256_hadd_ps(sum0_vec, sum0_vec);
			sum1_vec = _mm256_hadd_ps(sum1_vec, sum1_vec);
			sum1_vec = _mm256_hadd_ps(sum1_vec, sum1_vec);
			static __attribute__((aligned(32))) real c[2];
			static __attribute__((aligned(32))) real vals[8];
			_mm256_store_ps(vals, sum0_vec);
			c[0] = vals[0] + vals[4];
			_mm256_store_ps(vals, sum1_vec);
			c[1] = vals[0] + vals[4];

//			__attribute__((aligned(32))) real c[2] = {0};
			for (; k<K; k++) {
				real A0k = A[i*K+k];
				real A1k = A[(i+1)*K+k];
				real Bk0 = B[k*N+j];

				c[0] += A0k * Bk0;
				c[1] += A1k * Bk0;
			}
			C[i    *N+j  ] += c[0];
			C[(i+1)*N+j  ] += c[1];
		}
	}
}
// gcc -I/usr/include/openblas/ -lopenblas -o llama2 -Ofast -fopenmp -march=native -mfpmath=both -mavx -ffast-math -funroll-loops llama2.c -lm
//#include <cblas.h>
/*static void matmul(real* o, const real* x, const real* w, int n, int d)
{
	sgemm7_(d, 1, n, w, x, o); // 500
//	sgemm_block_parallel(d, 1, n, w, x, o);
//	cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, d, 1, n, 1.0, w, n, x, 1, 0.0, o, 1); // slow
//	cblas_sgemv(CblasRowMajor, CblasNoTrans, d, n, 1.0, w, n, x, 1, 0.0, o, 1);
}*/

// multiply int8_t, add results pairwise twice
static inline __m128i mul_sum_i8_pairs(const __m128i x, const __m128i y)
{
	// Get absolute values of x vectors
	const __m128i ax = _mm_sign_epi8(x, x);
	// Sign the values of the y vectors
	const __m128i sy = _mm_sign_epi8(y, x);
	// Perform multiplication and create 16-bit values
	const __m128i dot = _mm_maddubs_epi16(ax, sy);
	const __m128i ones = _mm_set1_epi16(1);
	return _mm_madd_epi16(ones, dot);
}
// horizontally add 8 floats
/*static inline float hsum_float_8(const __m256 x)
{
	__m128 res = _mm256_extractf128_ps(x, 1);
	res = _mm_add_ps(res, _mm256_castps256_ps128(x)); // ( x3+x7, x2+x6, x1+x5, x0+x4 )
	res = _mm_add_ps(res, _mm_movehl_ps(res, res)); // ( -, -, x1+x3+x5+x7, x0+x2+x4+x6 )
	res = _mm_add_ss(res, _mm_movehdup_ps(res)); // ( -, -, -, x0+x1+x2+x3+x4+x5+x6+x7 )
	return _mm_cvtss_f32(res);
}*/
static inline float hsum_float_8(__m256 acc)
{
/*	acc = _mm256_hadd_ps(acc, acc);
	acc = _mm256_hadd_ps(acc, acc);
	static __attribute__((aligned(32))) float vals[8];
	_mm256_store_ps(vals, acc);
	return vals[0] + vals[4];*/
	acc = _mm256_hadd_ps(acc, acc);
	acc = _mm256_hadd_ps(acc, acc);
	return _mm_cvtss_f32(_mm_add_ps(_mm256_extractf128_ps(acc, 1), _mm256_castps256_ps128(acc)));
}
static inline __m256i __mm256_cvtepi8_epi32(__m128i v)
{
	// expand signed 16bit
	__m128i lo = _mm_srai_epi16(v, 8);

	// expand 32bit
	__m128i lo32_0 = _mm_cvtepi16_epi32(lo);
	__m128i lo32_1 = _mm_cvtepi16_epi32(_mm_shuffle_epi32(lo, 0xee));

	// convert to __m256i
	__m256i lo256 = _mm256_insertf128_si256(_mm256_castsi128_si256(lo32_0), lo32_1, 1);
	return lo256;
}
//#define MM256_SET_M128I(a, b) _mm256_insertf128_si256(_mm256_castsi128_si256(b), (a), 1)
#define MM256_SET_M128I(a, b) _mm256_set_m128i((a), (b))
static inline float dot_q4_0_q8_avx(uint8_t *a, int8_t *b, int n)
{
	const __m128i lowMask = _mm_set1_epi8(0xF);
	const __m128i off = _mm_set1_epi8(8);
	int nb = n>>5; // 32
	__m256 acc = _mm256_setzero_ps();
	for (int i=0; i<nb; ++i) {
		// Compute combined scale for the block
		uint8_t *x = a +i*18;
		float delta = cats_fp16_to_fp32(*(uint16_t*)x);
		x += 2;
//		const __m256 d = _mm256_set1_ps( GGML_FP16_TO_FP32(x[i].d) * GGML_FP16_TO_FP32(y[i].d) );

		int8_t *y = b +i*(32+4);
		const __m128i tmp = _mm_loadu_si128((const __m128i *)x); // 4bit * 16 = 32byte

		__m128i bx = _mm_and_si128(lowMask, tmp); // 4bit -> 8 bit
		__m128i by = _mm_loadu_si128((const __m128i *)y); // 8bit * 16 = 128byte
		bx = _mm_sub_epi8(bx, off);
		const __m128i i32_0 = mul_sum_i8_pairs(bx, by);

		bx = _mm_and_si128(lowMask, _mm_srli_epi64(tmp, 4));
		by = _mm_loadu_si128((const __m128i *)(y +16));
		bx = _mm_sub_epi8(bx, off);
		const __m128i i32_1 = mul_sum_i8_pairs(bx, by);

		// Convert int32_t to float
		__m256 p = _mm256_cvtepi32_ps(MM256_SET_M128I(i32_0, i32_1));

		// Apply the scale, and accumulate
		delta *= *((float*)(y +32));
		const __m256 d = _mm256_set1_ps(delta);
		acc = _mm256_add_ps(_mm256_mul_ps(d, p), acc);
	}
	return hsum_float_8(acc);
}
static inline float dot_avx_q4_0(uint8_t* __restrict a, real* __restrict b, int n)
{
	int nn = n>>5; // 32
	__m256 *b_ptr = (__m256*)b;
        const __m128i lowMask = _mm_set1_epi8(0xF);
        const __m128i off = _mm_set1_epi8(8);
	__m256 acc = _mm256_setzero_ps();

	_mm_prefetch(b_ptr, _MM_HINT_T0);
	for (int i=0; i<nn; i++) {
		uint8_t *p = a +i*18;
		_mm_prefetch(p, _MM_HINT_T0);
		const __m256 d = _mm256_set1_ps(cats_fp16_to_fp32(*((uint16_t*)p)));
		p += 2;

		const __m128i w = _mm_loadu_si128((const __m128i *)p); // 16 byte
//		_mm_prefetch(b_ptr, _MM_HINT_T0);
		__m128i iw0 = _mm_and_si128(lowMask, w); // ((*p   & 0x0F) - 8)
		iw0 = _mm_sub_epi8(iw0, off);
		__m128i iw1 = _mm_and_si128(lowMask, _mm_srli_epi64(w, 4)); // ((*p++ >>   4) - 8)
		iw1 = _mm_sub_epi8(iw1, off);

		__m256 w0 = _mm256_mul_ps(_mm256_cvtepi32_ps(__mm256_cvtepi8_epi32(_mm_unpacklo_epi8(iw0, iw0))), d);
		__m256 w1 = _mm256_mul_ps(_mm256_cvtepi32_ps(__mm256_cvtepi8_epi32(_mm_unpackhi_epi8(iw0, iw0))), d);

		__m256 w2 = _mm256_mul_ps(_mm256_cvtepi32_ps(__mm256_cvtepi8_epi32(_mm_unpacklo_epi8(iw1, iw1))), d);
		__m256 w3 = _mm256_mul_ps(_mm256_cvtepi32_ps(__mm256_cvtepi8_epi32(_mm_unpackhi_epi8(iw1, iw1))), d);

		acc = _mm256_add_ps(acc, _mm256_mul_ps(w0, *b_ptr++));
		acc = _mm256_add_ps(acc, _mm256_mul_ps(w1, *b_ptr++));
		acc = _mm256_add_ps(acc, _mm256_mul_ps(w2, *b_ptr++));
		acc = _mm256_add_ps(acc, _mm256_mul_ps(w3, *b_ptr++));
	}
	return hsum_float_8(acc);
}

static inline real dot_q4_0_q8(uint8_t *a, int8_t *b, int n)
{
	int nn = n>>5; // 32
	real val = 0.0;
	for (int i=0; i<nn; i++) {
		uint8_t *p = a +i*18;
		real delta = cats_fp16_to_fp32(*((uint16_t*)p));
		p += sizeof(uint16_t);

		int8_t *b0 = b +i*(32+sizeof(float));
//		delta *= *((float*)b0);
//		b0 += 4;//sizeof(float);
		int8_t *b1 = b0+16;

		int32_t ival;
		ival  = ((int32_t)((*p   & 0x0F) - 8) * *b0++); // 0
		ival += ((int32_t)((*p++ >>   4) - 8) * *b1++);
		ival += ((int32_t)((*p   & 0x0F) - 8) * *b0++);
		ival += ((int32_t)((*p++ >>   4) - 8) * *b1++);
		ival += ((int32_t)((*p   & 0x0F) - 8) * *b0++);
		ival += ((int32_t)((*p++ >>   4) - 8) * *b1++);
		ival += ((int32_t)((*p   & 0x0F) - 8) * *b0++);
		ival += ((int32_t)((*p++ >>   4) - 8) * *b1++);

		ival += ((int32_t)((*p   & 0x0F) - 8) * *b0++);
		ival += ((int32_t)((*p++ >>   4) - 8) * *b1++);
		ival += ((int32_t)((*p   & 0x0F) - 8) * *b0++);
		ival += ((int32_t)((*p++ >>   4) - 8) * *b1++);
		ival += ((int32_t)((*p   & 0x0F) - 8) * *b0++);
		ival += ((int32_t)((*p++ >>   4) - 8) * *b1++);
		ival += ((int32_t)((*p   & 0x0F) - 8) * *b0++);
		ival += ((int32_t)((*p++ >>   4) - 8) * *b1++);

		ival += ((int32_t)((*p   & 0x0F) - 8) * *b0++);
		ival += ((int32_t)((*p++ >>   4) - 8) * *b1++);
		ival += ((int32_t)((*p   & 0x0F) - 8) * *b0++);
		ival += ((int32_t)((*p++ >>   4) - 8) * *b1++);
		ival += ((int32_t)((*p   & 0x0F) - 8) * *b0++);
		ival += ((int32_t)((*p++ >>   4) - 8) * *b1++);
		ival += ((int32_t)((*p   & 0x0F) - 8) * *b0++);
		ival += ((int32_t)((*p++ >>   4) - 8) * *b1++);

		ival += ((int32_t)((*p   & 0x0F) - 8) * *b0++);
		ival += ((int32_t)((*p++ >>   4) - 8) * *b1++);
		ival += ((int32_t)((*p   & 0x0F) - 8) * *b0++);
		ival += ((int32_t)((*p++ >>   4) - 8) * *b1++);
		ival += ((int32_t)((*p   & 0x0F) - 8) * *b0++);
		ival += ((int32_t)((*p++ >>   4) - 8) * *b1++);
		ival += ((int32_t)((*p   & 0x0F) - 8) * *b0++);
		ival += ((int32_t)((*p++ >>   4) - 8) * *b1++);

		delta *= *((float*)b1);
//		b1 += 4;//sizeof(float);
		val += ival * delta;
	}
	return val;
}
static inline real dot_q4_0(uint8_t *a, real *b, int n)
{
	int nn = n>>5; // 32
	static __attribute__((aligned(32))) real w[32];
	real val = 0.0;
	for (int i=0; i<nn; i++) {
		uint8_t *p = a +i*18;
		real delta = cats_fp16_to_fp32(*((uint16_t*)p));
		p += 2;
		w[ 0] = (((*p   & 0x0F) - 8)*delta);
		w[ 1] = (((*p++ >>   4) - 8)*delta);
		w[ 2] = (((*p   & 0x0F) - 8)*delta);
		w[ 3] = (((*p++ >>   4) - 8)*delta);
		w[ 4] = (((*p   & 0x0F) - 8)*delta);
		w[ 5] = (((*p++ >>   4) - 8)*delta);
		w[ 6] = (((*p   & 0x0F) - 8)*delta);
		w[ 7] = (((*p++ >>   4) - 8)*delta);

		w[ 8] = (((*p   & 0x0F) - 8)*delta);
		w[ 9] = (((*p++ >>   4) - 8)*delta);
		w[10] = (((*p   & 0x0F) - 8)*delta);
		w[11] = (((*p++ >>   4) - 8)*delta);
		w[12] = (((*p   & 0x0F) - 8)*delta);
		w[13] = (((*p++ >>   4) - 8)*delta);
		w[14] = (((*p   & 0x0F) - 8)*delta);
		w[15] = (((*p++ >>   4) - 8)*delta);

		w[16] = (((*p   & 0x0F) - 8)*delta);
		w[17] = (((*p++ >>   4) - 8)*delta);
		w[18] = (((*p   & 0x0F) - 8)*delta);
		w[19] = (((*p++ >>   4) - 8)*delta);
		w[20] = (((*p   & 0x0F) - 8)*delta);
		w[21] = (((*p++ >>   4) - 8)*delta);
		w[22] = (((*p   & 0x0F) - 8)*delta);
		w[23] = (((*p++ >>   4) - 8)*delta);

		w[24] = (((*p   & 0x0F) - 8)*delta);
		w[25] = (((*p++ >>   4) - 8)*delta);
		w[26] = (((*p   & 0x0F) - 8)*delta);
		w[27] = (((*p++ >>   4) - 8)*delta);
		w[28] = (((*p   & 0x0F) - 8)*delta);
		w[29] = (((*p++ >>   4) - 8)*delta);
		w[30] = (((*p   & 0x0F) - 8)*delta);
		w[31] = (((*p++ >>   4) - 8)*delta);

		val += w[ 0] * (*b++);
		val += w[ 2] * (*b++);
		val += w[ 4] * (*b++);
		val += w[ 6] * (*b++);
		val += w[ 8] * (*b++);
		val += w[10] * (*b++);
		val += w[12] * (*b++);
		val += w[14] * (*b++);

		val += w[16] * (*b++);
		val += w[18] * (*b++);
		val += w[20] * (*b++);
		val += w[22] * (*b++);
		val += w[24] * (*b++);
		val += w[26] * (*b++);
		val += w[28] * (*b++);
		val += w[30] * (*b++);

		val += w[ 1] * (*b++);
		val += w[ 3] * (*b++);
		val += w[ 5] * (*b++);
		val += w[ 7] * (*b++);
		val += w[ 9] * (*b++);
		val += w[11] * (*b++);
		val += w[13] * (*b++);
		val += w[15] * (*b++);

		val += w[17] * (*b++);
		val += w[19] * (*b++);
		val += w[21] * (*b++);
		val += w[23] * (*b++);
		val += w[25] * (*b++);
		val += w[27] * (*b++);
		val += w[29] * (*b++);
		val += w[31] * (*b++);
	}
	return val;
}
static inline float sdot_avx(const real *a, const real *b, int n)
{
	int i;
	int nn = n&~(16-1);
	__m256 acc = _mm256_setzero_ps();
	for (i=0; i<nn; i+=16) {
		__m256 x0 = _mm256_load_ps(a+i);
		__m256 x1 = _mm256_load_ps(a+i+8);
		__m256 y0 = _mm256_load_ps(b+i);
		__m256 y1 = _mm256_load_ps(b+i+8);
		acc = _mm256_add_ps(acc, _mm256_mul_ps(x0, y0));
		acc = _mm256_add_ps(acc, _mm256_mul_ps(x1, y1));
	}
	float val = hsum_float_8(acc);
	for (; i<n; ++i) val += a[i] * b[i];
	return val;
}
static inline real dot_q3_K_q8_K(uint8_t *x, int8_t *y, int n)
{
	const uint32_t kmask1 = 0x03030303;
	const uint32_t kmask2 = 0x0f0f0f0f;
	const int nb = n / 256;
	int8_t  aux8[256];
	int16_t aux16[8];
	float   sums [8];
	int32_t aux32[8];
	memset(sums, 0, 8*sizeof(float));

	uint32_t auxs[4];
	const int8_t * scales = (const int8_t*)auxs;

	float sumf = 0;
	for (int i=0; i<nb; ++i) {
		uint8_t *p = (uint8_t*)(x + i*(256/8 +256/4 +12 +2));
		const uint8_t * restrict q3 = p +256/8;
		const uint8_t * restrict hm = p;
		memset(aux32, 0, 8*sizeof(int32_t));
		int8_t * restrict a = aux8;
		uint8_t m = 1;
		for (int j=0; j<256; j+=128) {
			for (int l=0; l<32; ++l) a[l] = q3[l] & 3;
			for (int l=0; l<32; ++l) a[l] -= (hm[l] & m ? 0 : 4);
			a += 32; m <<= 1;
			for (int l=0; l<32; ++l) a[l] = (q3[l] >> 2) & 3;
			for (int l=0; l<32; ++l) a[l] -= (hm[l] & m ? 0 : 4);
			a += 32; m <<= 1;
			for (int l=0; l<32; ++l) a[l] = (q3[l] >> 4) & 3;
			for (int l=0; l<32; ++l) a[l] -= (hm[l] & m ? 0 : 4);
			a += 32; m <<= 1;
			for (int l=0; l<32; ++l) a[l] = (q3[l] >> 6) & 3;
			for (int l=0; l<32; ++l) a[l] -= (hm[l] & m ? 0 : 4);
			a += 32; m <<= 1;
			q3 += 32;
		}
		a = aux8;

		const int8_t * restrict sc = p +256/8 +256/4;
		memcpy(auxs, sc, 12);
		uint32_t tmp = auxs[2];
		auxs[2] = ((auxs[0] >> 4) & kmask2) | (((tmp >> 4) & kmask1) << 4);
		auxs[3] = ((auxs[1] >> 4) & kmask2) | (((tmp >> 6) & kmask1) << 4);
		auxs[0] = (auxs[0] & kmask2) | (((tmp >> 0) & kmask1) << 4);
		auxs[1] = (auxs[1] & kmask2) | (((tmp >> 2) & kmask1) << 4);
		const int8_t * restrict q8 = y +i*(4+256);
		for (int j=0; j<256/16; ++j) {
			for (int l=0; l<8; ++l) aux16[l] = q8[l] * a[l];
			for (int l=0; l<8; ++l) aux32[l] += (scales[j] - 32) * aux16[l];
			q8 += 8; a += 8;
			for (int l=0; l<8; ++l) aux16[l] = q8[l] * a[l];
			for (int l=0; l<8; ++l) aux32[l] += (scales[j] - 32) * aux16[l];
			q8 += 8; a += 8;
		}
		const float d = cats_fp16_to_fp32(*((uint16_t*)(p +256/8 +256/4 +12))) * (*(float*)q8);
		for (int l=0; l<8; ++l) sums[l] += d * aux32[l];
	}
	for (int l=0; l<8; ++l) sumf += sums[l];
	return sumf;
}
static inline real dot_q3_K_q8_K_avx(uint8_t *x, int8_t *y, int n)
{
	const uint32_t kmask1 = 0x03030303;
	const uint32_t kmask2 = 0x0f0f0f0f;
	const int nb = n / 256;

	const __m128i m3 = _mm_set1_epi8(3);
	const __m128i mone = _mm_set1_epi8(1);
	const __m128i m32 = _mm_set1_epi8(32);
	const __m128i m2 = _mm_set1_epi8(2);

	__m256 acc = _mm256_setzero_ps();

	uint32_t *aux;

	for (int i=0; i<nb; ++i) {
		uint8_t *p = (uint8_t*)(x + i*(256/8 +256/4 +12 +2));
		const uint8_t * restrict q3 = p +256/8;
		const uint8_t * restrict hm = p;
		const int8_t * restrict sc = p +256/8 +256/4;
		const int8_t * restrict q8 = y +i*(4+256);

		// Set up scales
		aux = (uint32_t *)sc;
		__m128i scales128 = _mm_set_epi32(
			((aux[1] >> 4) & kmask2) | (((aux[2] >> 6) & kmask1) << 4),
			((aux[0] >> 4) & kmask2) | (((aux[2] >> 4) & kmask1) << 4),
			(aux[1] & kmask2) | (((aux[2] >> 2) & kmask1) << 4),
			(aux[0] & kmask2) | (((aux[2] >> 0) & kmask1) << 4));
		scales128 = _mm_sub_epi8(scales128, m32);
		const __m128i scales_0 = _mm_cvtepi8_epi16(scales128);
		const __m128i scales_1 = _mm_cvtepi8_epi16(_mm_unpackhi_epi64(scales128, scales128));
		const __m128i scales[2] = { scales_0, scales_1 };

		// high bit *128*2 from block_q3_K.hmask[QK_K/8]
		const __m128i hbits_0 = _mm_loadu_si128((const __m128i*)&hm[0]);
		const __m128i hbits_1 = _mm_loadu_si128((const __m128i*)&hm[16]);

		// integer accumulator
		__m128i sumi_0 = _mm_setzero_si128();
		__m128i sumi_1 = _mm_setzero_si128();

		for (int j=0; j<256/128; ++j) {
			// load low 2 bits *64*2 from block_q3_K.qs[QK_K/4]
			const __m128i q3bits_0 = _mm_loadu_si128((const __m128i*)q3); q3 += 16;
			const __m128i q3bits_1 = _mm_loadu_si128((const __m128i*)q3); q3 += 16;

			// prepare low and high bits
			const int bit = j << 2;

			const __m128i q3l_0 = _mm_and_si128(q3bits_0, m3);
			const __m128i q3l_1 = _mm_and_si128(q3bits_1, m3);
			const __m128i q3h_0 = _mm_slli_epi16(_mm_srli_epi16(_mm_andnot_si128(hbits_0, _mm_slli_epi16(mone, bit)), bit), 2);
			const __m128i q3h_1 = _mm_slli_epi16(_mm_srli_epi16(_mm_andnot_si128(hbits_1, _mm_slli_epi16(mone, bit)), bit), 2);

			const __m128i q3l_2 = _mm_and_si128(_mm_srli_epi16(q3bits_0, 2), m3);
			const __m128i q3l_3 = _mm_and_si128(_mm_srli_epi16(q3bits_1, 2), m3);
			const __m128i q3h_2 = _mm_slli_epi16(_mm_srli_epi16(_mm_andnot_si128(hbits_0, _mm_slli_epi16(mone, bit+1)), bit+1), 2);
			const __m128i q3h_3 = _mm_slli_epi16(_mm_srli_epi16(_mm_andnot_si128(hbits_1, _mm_slli_epi16(mone, bit+1)), bit+1), 2);

			const __m128i q3l_4 = _mm_and_si128(_mm_srli_epi16(q3bits_0, 4), m3);
			const __m128i q3l_5 = _mm_and_si128(_mm_srli_epi16(q3bits_1, 4), m3);
			const __m128i q3h_4 = _mm_slli_epi16(_mm_srli_epi16(_mm_andnot_si128(hbits_0, _mm_slli_epi16(mone, bit+2)), bit+2), 2);
			const __m128i q3h_5 = _mm_slli_epi16(_mm_srli_epi16(_mm_andnot_si128(hbits_1, _mm_slli_epi16(mone, bit+2)), bit+2), 2);

			const __m128i q3l_6 = _mm_and_si128(_mm_srli_epi16(q3bits_0, 6), m3);
			const __m128i q3l_7 = _mm_and_si128(_mm_srli_epi16(q3bits_1, 6), m3);
			const __m128i q3h_6 = _mm_slli_epi16(_mm_srli_epi16(_mm_andnot_si128(hbits_0, _mm_slli_epi16(mone, bit+3)), bit+3), 2);
			const __m128i q3h_7 = _mm_slli_epi16(_mm_srli_epi16(_mm_andnot_si128(hbits_1, _mm_slli_epi16(mone, bit+3)), bit+3), 2);

			// load Q8 quants from block_q8_K.qs[QK_K]
			const __m128i q8_0 = _mm_loadu_si128((const __m128i*)q8); q8 += 16;
			const __m128i q8_1 = _mm_loadu_si128((const __m128i*)q8); q8 += 16;
			const __m128i q8_2 = _mm_loadu_si128((const __m128i*)q8); q8 += 16;
			const __m128i q8_3 = _mm_loadu_si128((const __m128i*)q8); q8 += 16;
			const __m128i q8_4 = _mm_loadu_si128((const __m128i*)q8); q8 += 16;
			const __m128i q8_5 = _mm_loadu_si128((const __m128i*)q8); q8 += 16;
			const __m128i q8_6 = _mm_loadu_si128((const __m128i*)q8); q8 += 16;
			const __m128i q8_7 = _mm_loadu_si128((const __m128i*)q8); q8 += 16;

			// Dot product: we multiply the 2 low bits and 1 high bit part separately, so we can use _mm256_maddubs_epi16,
			// and then subtract. The high bit part has the 2 already subtracted (and so, it is zero if the high bit was not set,
			// and 2 if the high bit was set)
			__m128i q8s_0 = _mm_maddubs_epi16(q3h_0, q8_0);
			__m128i q8s_1 = _mm_maddubs_epi16(q3h_1, q8_1);
			__m128i q8s_2 = _mm_maddubs_epi16(q3h_2, q8_2);
			__m128i q8s_3 = _mm_maddubs_epi16(q3h_3, q8_3);
			__m128i q8s_4 = _mm_maddubs_epi16(q3h_4, q8_4);
			__m128i q8s_5 = _mm_maddubs_epi16(q3h_5, q8_5);
			__m128i q8s_6 = _mm_maddubs_epi16(q3h_6, q8_6);
			__m128i q8s_7 = _mm_maddubs_epi16(q3h_7, q8_7);

			__m128i p16_0 = _mm_maddubs_epi16(q3l_0, q8_0);
			__m128i p16_1 = _mm_maddubs_epi16(q3l_1, q8_1);
			__m128i p16_2 = _mm_maddubs_epi16(q3l_2, q8_2);
			__m128i p16_3 = _mm_maddubs_epi16(q3l_3, q8_3);
			__m128i p16_4 = _mm_maddubs_epi16(q3l_4, q8_4);
			__m128i p16_5 = _mm_maddubs_epi16(q3l_5, q8_5);
			__m128i p16_6 = _mm_maddubs_epi16(q3l_6, q8_6);
			__m128i p16_7 = _mm_maddubs_epi16(q3l_7, q8_7);

			p16_0 = _mm_sub_epi16(p16_0, q8s_0);
			p16_1 = _mm_sub_epi16(p16_1, q8s_1);
			p16_2 = _mm_sub_epi16(p16_2, q8s_2);
			p16_3 = _mm_sub_epi16(p16_3, q8s_3);
			p16_4 = _mm_sub_epi16(p16_4, q8s_4);
			p16_5 = _mm_sub_epi16(p16_5, q8s_5);
			p16_6 = _mm_sub_epi16(p16_6, q8s_6);
			p16_7 = _mm_sub_epi16(p16_7, q8s_7);

			// multiply with scales
			__m128i shuffle = _mm_set1_epi16(0x0100);
			p16_0 = _mm_madd_epi16(_mm_shuffle_epi8(scales[j], shuffle), p16_0);
			shuffle = _mm_add_epi16(shuffle, m2);
			p16_1 = _mm_madd_epi16(_mm_shuffle_epi8(scales[j], shuffle), p16_1);
			shuffle = _mm_add_epi16(shuffle, m2);
			p16_2 = _mm_madd_epi16(_mm_shuffle_epi8(scales[j], shuffle), p16_2);
			shuffle = _mm_add_epi16(shuffle, m2);
			p16_3 = _mm_madd_epi16(_mm_shuffle_epi8(scales[j], shuffle), p16_3);
			shuffle = _mm_add_epi16(shuffle, m2);
			p16_4 = _mm_madd_epi16(_mm_shuffle_epi8(scales[j], shuffle), p16_4);
			shuffle = _mm_add_epi16(shuffle, m2);
			p16_5 = _mm_madd_epi16(_mm_shuffle_epi8(scales[j], shuffle), p16_5);
			shuffle = _mm_add_epi16(shuffle, m2);
			p16_6 = _mm_madd_epi16(_mm_shuffle_epi8(scales[j], shuffle), p16_6);
			shuffle = _mm_add_epi16(shuffle, m2);
			p16_7 = _mm_madd_epi16(_mm_shuffle_epi8(scales[j], shuffle), p16_7);

			// accumulate
			p16_0 = _mm_add_epi32(p16_0, p16_1);
			p16_2 = _mm_add_epi32(p16_2, p16_3);
			p16_4 = _mm_add_epi32(p16_4, p16_5);
			p16_6 = _mm_add_epi32(p16_6, p16_7);
			sumi_0 = _mm_add_epi32(sumi_0, _mm_add_epi32(p16_0, p16_2));
			sumi_1 = _mm_add_epi32(sumi_1, _mm_add_epi32(p16_4, p16_6));
		}

		// multiply with block scale and accumulate
		const float d = cats_fp16_to_fp32(*((uint16_t*)(p +256/8 +256/4 +12))) * (*(float*)q8);
		__m256i sumi = _mm256_set_m128i(sumi_1, sumi_0);
		acc = _mm256_add_ps(_mm256_mul_ps(_mm256_broadcast_ss(&d), _mm256_cvtepi32_ps(sumi)), acc);
	}

	return hsum_float_8(acc);
}
/*static inline real dot_q4_K_q8_K(uint8_t *x, int8_t *y, int n)
{
	const int nb = n / 256;
	static const uint32_t kmask1 = 0x3f3f3f3f;
	static const uint32_t kmask2 = 0x0f0f0f0f;
	static const uint32_t kmask3 = 0x03030303;

	uint32_t utmp[4];
	const uint8_t * scales = (const uint8_t*)&utmp[0];
	const uint8_t * mins   = (const uint8_t*)&utmp[2];

	int8_t  aux8[QK_K];
	int16_t aux16[8];
	float   sums [8];
	int32_t aux32[8];
	memset(sums, 0, 8*sizeof(float));

	float sumf = 0;
	for (int i=0; i<nb; ++i) {
		uint8_t *p = (uint8_t*)(x + i*(256/2 +256/4 +256/16 +2));
		const uint8_t * restrict q4 = p;
//		const uint8_t * restrict qh = p +256/2;
		const int8_t * restrict sc = p +256/2 +256/4;
		const int8_t * restrict q8 = y +i*(4+256);

//		const uint8_t * restrict q4 = x[i].qs;
//		const  int8_t * restrict q8 = y[i].qs;

		memset(aux32, 0, 8*sizeof(int32_t));
		int8_t * restrict a = aux8;
		for (int j=0; j<256/64; ++j) {
			for (int l=0; l<32; ++l) a[l] = (int8_t)(q4[l] & 0xF);
			a += 32;
			for (int l=0; l<32; ++l) a[l] = (int8_t)(q4[l]  >> 4);
			a += 32; q4 += 32;
		}
		memcpy(utmp, sc, 12);
		utmp[3] = ((utmp[2] >> 4) & kmask2) | (((utmp[1] >> 6) & kmask3) << 4);
		const uint32_t uaux = utmp[1] & kmask1;
		utmp[1] = (utmp[2] & kmask2) | (((utmp[0] >> 6) & kmask3) << 4);
		utmp[2] = uaux;
		utmp[0] &= kmask1;

		int sumi = 0;
		for (int j=0; j<256/16; ++j) sumi += y[i].bsums[j] * mins[j/2];
		a = aux8;
		int is = 0;
		for (int j = 0; j < QK_K/32; ++j) {
		int32_t scale = scales[is++];
		for (int l = 0; l < 8; ++l) aux16[l] = q8[l] * a[l];
		for (int l = 0; l < 8; ++l) aux32[l] += scale * aux16[l];
		q8 += 8; a += 8;
		for (int l = 0; l < 8; ++l) aux16[l] = q8[l] * a[l];
		for (int l = 0; l < 8; ++l) aux32[l] += scale * aux16[l];
		q8 += 8; a += 8;
		for (int l = 0; l < 8; ++l) aux16[l] = q8[l] * a[l];
		for (int l = 0; l < 8; ++l) aux32[l] += scale * aux16[l];
		q8 += 8; a += 8;
		for (int l = 0; l < 8; ++l) aux16[l] = q8[l] * a[l];
		for (int l = 0; l < 8; ++l) aux32[l] += scale * aux16[l];
		q8 += 8; a += 8;
		}
		const float d = ggml_fp16_to_fp32(x[i].d) * y[i].d;
		for (int l = 0; l < 8; ++l) sums[l] += d * aux32[l];
		const float dmin = ggml_fp16_to_fp32(x[i].dmin) * y[i].d;
		sumf -= dmin * sumi;
	}
	for (int l=0; l<8; ++l) sumf += sums[l];
	return sumf;
}*/
static inline real dot_q6_K_q8_K(uint8_t *x, int8_t *y, int n)
{
	const int nb = n / 256;
	int8_t  aux8[256];
	int16_t aux16[8];
	float   sums [8];
	int32_t aux32[8];
	memset(sums, 0, 8*sizeof(float));

	float sumf = 0;
	for (int i=0; i<nb; ++i) {
		uint8_t *p = (uint8_t*)(x + i*(256/2 +256/4 +256/16 +2));
		const uint8_t * restrict q4 = p;
		const uint8_t * restrict qh = p +256/2;
		memset(aux32, 0, 8*sizeof(int32_t));
		int8_t * restrict a = aux8;
		for (int j=0; j<256; j+=128) {
			for (int l=0; l<32; ++l) {
				a[l +  0] = (int8_t)((q4[l +  0] & 0xF) | (((qh[l] >> 0) & 3) << 4)) - 32;
				a[l + 32] = (int8_t)((q4[l + 32] & 0xF) | (((qh[l] >> 2) & 3) << 4)) - 32;
				a[l + 64] = (int8_t)((q4[l +  0] >>  4) | (((qh[l] >> 4) & 3) << 4)) - 32;
				a[l + 96] = (int8_t)((q4[l + 32] >>  4) | (((qh[l] >> 6) & 3) << 4)) - 32;
			}
			a  += 128;
			q4 += 64;
			qh += 32;
		}
		a = aux8;
		const int8_t * restrict sc = p +256/2 +256/4;
		const int8_t * restrict q8 = y +i*(4+256);
		for (int j=0; j<256/16; ++j) {
			for (int l=0; l<8; ++l) aux16[l] = q8[l] * a[l];
			for (int l=0; l<8; ++l) aux32[l] += (*sc) * aux16[l];
			q8 += 8; a += 8;
			for (int l=0; l<8; ++l) aux16[l] = q8[l] * a[l];
			for (int l=0; l<8; ++l) aux32[l] += (*sc) * aux16[l];
			q8 += 8; a += 8;
			sc++;
		}
		const float d = cats_fp16_to_fp32(*((uint16_t*)(p +256/2 +256/4 +256/16))) * (*(float*)q8);
		for (int l=0; l<8; ++l) sums[l] += d * aux32[l];
	}
	for (int l=0; l<8; ++l) sumf += sums[l];
	return sumf;
}
static inline real dot_q6_K_q8_K_avx(uint8_t *x, int8_t *y, int n)
{
	const int nb = n / 256;
	const __m128i m4 = _mm_set1_epi8(0xF);
	const __m128i m3 = _mm_set1_epi8(3);
	const __m128i m32s = _mm_set1_epi8(32);
	const __m128i m2 = _mm_set1_epi8(2);

	__m256 acc = _mm256_setzero_ps();
	for (int i=0; i<nb; ++i) {
		uint8_t *p = (uint8_t*)(x + i*(256/2 +256/4 +256/16 +2));
		const uint8_t * restrict q4 = p;
		const uint8_t * restrict qh = p +256/2;
		const int8_t * restrict sc = p +256/2 +256/4;
		const int8_t * restrict q8 = y +i*(4+256);
//		const float d = cats_fp16_to_fp32(*((uint16_t*)(p +256/2 +256/4 +256/16))) * (*(float*)(q8+256));

		const __m128i scales = _mm_loadu_si128((const __m128i*)sc);

		__m128i sumi_0 = _mm_setzero_si128();
		__m128i sumi_1 = _mm_setzero_si128();

		__m128i shuffle = _mm_set_epi64x(0x0101010101010101, 0x0000000000000000);
		for (int j=0; j<256/128; j++) {
			const __m128i q4bitsH_0 = _mm_loadu_si128((const __m128i*)qh); qh += 16;
			const __m128i q4bitsH_1 = _mm_loadu_si128((const __m128i*)qh); qh += 16;

			const __m128i q4h_0 = _mm_slli_epi16(_mm_and_si128(q4bitsH_0, m3), 4);
			const __m128i q4h_1 = _mm_slli_epi16(_mm_and_si128(q4bitsH_1, m3), 4);
			const __m128i q4h_2 = _mm_slli_epi16(_mm_and_si128(_mm_srli_epi16(q4bitsH_0, 2), m3), 4);
			const __m128i q4h_3 = _mm_slli_epi16(_mm_and_si128(_mm_srli_epi16(q4bitsH_1, 2), m3), 4);
			const __m128i q4h_4 = _mm_slli_epi16(_mm_and_si128(_mm_srli_epi16(q4bitsH_0, 4), m3), 4);
			const __m128i q4h_5 = _mm_slli_epi16(_mm_and_si128(_mm_srli_epi16(q4bitsH_1, 4), m3), 4);
			const __m128i q4h_6 = _mm_slli_epi16(_mm_and_si128(_mm_srli_epi16(q4bitsH_0, 6), m3), 4);
			const __m128i q4h_7 = _mm_slli_epi16(_mm_and_si128(_mm_srli_epi16(q4bitsH_1, 6), m3), 4);

			const __m128i q4bits1_0 = _mm_loadu_si128((const __m128i*)q4); q4 += 16;
			const __m128i q4bits1_1 = _mm_loadu_si128((const __m128i*)q4); q4 += 16;
			const __m128i q4bits2_0 = _mm_loadu_si128((const __m128i*)q4); q4 += 16;
			const __m128i q4bits2_1 = _mm_loadu_si128((const __m128i*)q4); q4 += 16;

			const __m128i q4_0 = _mm_or_si128(_mm_and_si128(q4bits1_0, m4), q4h_0);
			const __m128i q4_1 = _mm_or_si128(_mm_and_si128(q4bits1_1, m4), q4h_1);
			const __m128i q4_2 = _mm_or_si128(_mm_and_si128(q4bits2_0, m4), q4h_2);
			const __m128i q4_3 = _mm_or_si128(_mm_and_si128(q4bits2_1, m4), q4h_3);
			const __m128i q4_4 = _mm_or_si128(_mm_and_si128(_mm_srli_epi16(q4bits1_0, 4), m4), q4h_4);
			const __m128i q4_5 = _mm_or_si128(_mm_and_si128(_mm_srli_epi16(q4bits1_1, 4), m4), q4h_5);
			const __m128i q4_6 = _mm_or_si128(_mm_and_si128(_mm_srli_epi16(q4bits2_0, 4), m4), q4h_6);
			const __m128i q4_7 = _mm_or_si128(_mm_and_si128(_mm_srli_epi16(q4bits2_1, 4), m4), q4h_7);

			const __m128i q8_0 = _mm_loadu_si128((const __m128i*)q8); q8 += 16;
			const __m128i q8_1 = _mm_loadu_si128((const __m128i*)q8); q8 += 16;
			const __m128i q8_2 = _mm_loadu_si128((const __m128i*)q8); q8 += 16;
			const __m128i q8_3 = _mm_loadu_si128((const __m128i*)q8); q8 += 16;
			const __m128i q8_4 = _mm_loadu_si128((const __m128i*)q8); q8 += 16;
			const __m128i q8_5 = _mm_loadu_si128((const __m128i*)q8); q8 += 16;
			const __m128i q8_6 = _mm_loadu_si128((const __m128i*)q8); q8 += 16;
			const __m128i q8_7 = _mm_loadu_si128((const __m128i*)q8); q8 += 16;

			__m128i q8s_0 = _mm_maddubs_epi16(m32s, q8_0);
			__m128i q8s_1 = _mm_maddubs_epi16(m32s, q8_1);
			__m128i q8s_2 = _mm_maddubs_epi16(m32s, q8_2);
			__m128i q8s_3 = _mm_maddubs_epi16(m32s, q8_3);
			__m128i q8s_4 = _mm_maddubs_epi16(m32s, q8_4);
			__m128i q8s_5 = _mm_maddubs_epi16(m32s, q8_5);
			__m128i q8s_6 = _mm_maddubs_epi16(m32s, q8_6);
			__m128i q8s_7 = _mm_maddubs_epi16(m32s, q8_7);

			__m128i p16_0 = _mm_maddubs_epi16(q4_0, q8_0);
			__m128i p16_1 = _mm_maddubs_epi16(q4_1, q8_1);
			__m128i p16_2 = _mm_maddubs_epi16(q4_2, q8_2);
			__m128i p16_3 = _mm_maddubs_epi16(q4_3, q8_3);
			__m128i p16_4 = _mm_maddubs_epi16(q4_4, q8_4);
			__m128i p16_5 = _mm_maddubs_epi16(q4_5, q8_5);
			__m128i p16_6 = _mm_maddubs_epi16(q4_6, q8_6);
			__m128i p16_7 = _mm_maddubs_epi16(q4_7, q8_7);

			p16_0 = _mm_sub_epi16(p16_0, q8s_0);
			p16_1 = _mm_sub_epi16(p16_1, q8s_1);
			p16_2 = _mm_sub_epi16(p16_2, q8s_2);
			p16_3 = _mm_sub_epi16(p16_3, q8s_3);
			p16_4 = _mm_sub_epi16(p16_4, q8s_4);
			p16_5 = _mm_sub_epi16(p16_5, q8s_5);
			p16_6 = _mm_sub_epi16(p16_6, q8s_6);
			p16_7 = _mm_sub_epi16(p16_7, q8s_7);

			const __m128i scale_0 = _mm_shuffle_epi8(scales, shuffle);
			shuffle = _mm_add_epi8(shuffle, m2);
			const __m128i scale_1 = _mm_shuffle_epi8(scales, shuffle);
			shuffle = _mm_add_epi8(shuffle, m2);
			const __m128i scale_2 = _mm_shuffle_epi8(scales, shuffle);
			shuffle = _mm_add_epi8(shuffle, m2);
			const __m128i scale_3 = _mm_shuffle_epi8(scales, shuffle);
			shuffle = _mm_add_epi8(shuffle, m2);

			p16_0 = _mm_madd_epi16(_mm_cvtepi8_epi16(scale_0), p16_0);
			p16_1 = _mm_madd_epi16(_mm_cvtepi8_epi16(_mm_unpackhi_epi64(scale_0, scale_0)), p16_1);
			p16_2 = _mm_madd_epi16(_mm_cvtepi8_epi16(scale_1), p16_2);
			p16_3 = _mm_madd_epi16(_mm_cvtepi8_epi16(_mm_unpackhi_epi64(scale_1, scale_1)), p16_3);
			p16_4 = _mm_madd_epi16(_mm_cvtepi8_epi16(scale_2), p16_4);
			p16_5 = _mm_madd_epi16(_mm_cvtepi8_epi16(_mm_unpackhi_epi64(scale_2, scale_2)), p16_5);
			p16_6 = _mm_madd_epi16(_mm_cvtepi8_epi16(scale_3), p16_6);
			p16_7 = _mm_madd_epi16(_mm_cvtepi8_epi16(_mm_unpackhi_epi64(scale_3, scale_3)), p16_7);

			sumi_0 = _mm_add_epi32(sumi_0, _mm_add_epi32(p16_0, p16_2));
			sumi_1 = _mm_add_epi32(sumi_1, _mm_add_epi32(p16_1, p16_3));
			sumi_0 = _mm_add_epi32(sumi_0, _mm_add_epi32(p16_4, p16_6));
			sumi_1 = _mm_add_epi32(sumi_1, _mm_add_epi32(p16_5, p16_7));
		}

		const float d = cats_fp16_to_fp32(*((uint16_t*)(p +256/2 +256/4 +256/16))) * (*(float*)q8);
		__m256i sumi = MM256_SET_M128I(sumi_1, sumi_0);
		acc = _mm256_add_ps(_mm256_mul_ps(_mm256_broadcast_ss(&d), _mm256_cvtepi32_ps(sumi)), acc);
	}
	return hsum_float_8(acc);
}
#define MATMUL_FUNC(name, dotfunc, wtype, xtype, len) \
static void name(real* __restrict xout, void* __restrict x, void* __restrict w, int n, int d) \
{ \
	omp_set_num_threads(8); \
	_Pragma("omp parallel for") \
	for (int i=0; i<(d); i++) { \
		xout[i] = dotfunc(((wtype*)w)+len, (xtype*)x, (n)); \
	} \
}
MATMUL_FUNC(matmul_avx, sdot_avx, real, real, i*n); // 580
MATMUL_FUNC(matmul_q4_0_avx, dot_avx_q4_0, uint8_t, real, i*n/32*18); // 2.9
MATMUL_FUNC(matmul_q4_0_q8, dot_q4_0_q8, uint8_t, int8_t, i*n/32*18); // 1.0
MATMUL_FUNC(matmul_q4_0_q8_avx, dot_q4_0_q8_avx, uint8_t, int8_t, i*n/32*18); // 5.5
//MATMUL_FUNC(matmul_q3_K_q8_K, dot_q3_K_q8_K, uint8_t, int8_t, i*n/256*110); // 0.95
MATMUL_FUNC(matmul_q3_K_q8_K, dot_q3_K_q8_K_avx, uint8_t, int8_t, i*n/256*110); // 4.5
//MATMUL_FUNC(matmul_q6_K_q8_K, dot_q6_K_q8_K, uint8_t, int8_t, i*n/256*210); // 4.9 (case of only 1 use)
MATMUL_FUNC(matmul_q6_K_q8_K, dot_q6_K_q8_K_avx, uint8_t, int8_t, i*n/256*210); // 5.4
/*static void matmul_arm(float *xout, float *x, float *w, int n, int d)
{
	int i;
	#pragma omp parallel for private(i)
	for (i=0; i<d; i++) {
		float32x4_t val = vdupq_n_f32(0.0);
		for (int j=0; j<n; j+=4) {
			val = vaddq_f32(val, vmulq_f32(vld1q_f32(&x[j]), vld1q_f32(&w[i * n + j])));
			// The below line works slower
			// val = vmlaq_f32(val, vld1q_f32(&x[j]), vld1q_f32(&w[i * n + j]));
		}
		xout[i] = vaddvq_f32(val);
	}
}*/
#else
static void matmul_naive(real* xout, real* x, real* w, int n, int d)
{
	// W (d,n) @ x (n,) -> xout (d,)
	// by far the most amount of time is spent inside this little function
	#pragma omp parallel for
	for (int i=0; i<d; i++) { // 338
		real val = 0.0;
		const int i_n = i * n;
		for (int j=0; j<n; j+=4) {
			val += w[i_n+j  ] * x[j];
			val += w[i_n+j+1] * x[j+1];
			val += w[i_n+j+2] * x[j+2];
			val += w[i_n+j+3] * x[j+3];
		}
		xout[i] = val;
	}
}
#endif

static void rope_falcon(real *q, real *k, int dim, int kv_dim, int head_size, int pos)
{
	int i;
	#pragma omp parallel for private(i)
	for (i=0; i<dim; i++) {
		int hdim = i % (head_size/2);
		int n = i + hdim;
		int m = i + hdim + head_size/2;
		float freq = 1.0 / powf(10000.0, 2.0 * hdim / (float)head_size);
		float val = pos * freq;
		float fcr = cosf(val);
		float fci = sinf(val);
		float q0 = q[n];
		float q1 = q[m];
		q[n] = q0 * fcr - q1 * fci;
		q[m] = q0 * fci + q1 * fcr;
		if (i < kv_dim) {
			float k0 = k[n];
			float k1 = k[m];
			k[n] = k0 * fcr - k1 * fci;
			k[m] = k0 * fci + k1 * fcr;
		}
	}
}
static void rope_llama(real *q, real *k, int dim, int kv_dim, int head_size, int pos)
{
	int i;
	#pragma omp parallel for private(i)
	for (i=0; i<dim; i+=2) {
		int head_dim = i % head_size;
		float freq = 1.0 / powf(10000.0, head_dim / (float)head_size);
		float val = pos * freq;
		float fcr = cosf(val);
		float fci = sinf(val);
		real q0 = q[i];
		real q1 = q[i+1];
		q[i]   = q0 * fcr - q1 * fci;
		q[i+1] = q0 * fci + q1 * fcr;
		// rotate k
		if (i < kv_dim) {
			real k0 = k[i];
			real k1 = k[i+1];
			k[i  ] = k0 * fcr - k1 * fci;
			k[i+1] = k0 * fci + k1 * fcr;
		}
	}
}

// https://qiita.com/birdwatcher/items/b3e4428f63f708db37b7
// https://github.com/RahulSChand/llama2.c-for-dummies
static real* cats_llama2_transformer(int token, int pos, cats_ggml_model* m)
{
	// a few convenience variables
	real *x = m->x;
	int dim = m->n_embd; // transformer dimensional: n_heads * head_size
	int kv_dim = (m->n_embd * m->n_kv_head) / m->n_head;
	int kv_mul = m->n_head / m->n_kv_head; // integer multiplier of the kv sharing in multiquery
	int hidden_dim =  m->n_hidden;
	int head_size = dim / m->n_head;
//	real sqrt_head_size = 1.0 / sqrtf(head_size);

	// Convert the token to a real vector (int -> vector)
	// copy the token embedding into x
	dequantize(x, m->token_embedding_table->data, token, dim);

	// forward all the layers
	for (int l=0; l<m->n_layer; l++) {
		// attention rmsnorm
		rmsnorm(m->xb, x, m->rms_att_weight[l]->data, dim); // S = T = norm x -> mul weight

		// key and value point to the kv cache
		int loff = l * m->seq_len * kv_dim; // kv cache layer offset for convenience
		real* k = m->key_cache + loff + pos * kv_dim;
		real* v = m->value_cache + loff + pos * kv_dim;

		// qkv matmuls for this position
		m->wq[l]->quantize((int8_t*)m->xb, m->xb, dim);
		m->wq[l]->matmul(m->q, m->xb, m->wq[l]->data, dim, dim); // Q = TWq
		m->wk[l]->matmul(k, m->xb, m->wk[l]->data, dim, kv_dim); // K = SWk
		m->wv[l]->matmul(v, m->xb, m->wv[l]->data, dim, kv_dim); // V = Swv

		// Positional Encoding: x + PE
		//   PE(pos,2i)   := sin(pos/10000^2*i/d)
		//   PE(pos,2i+1) := cos(pos/10000^2*i/d)
		// RoPE relative positional encoding: complex-valued rotate q and k by freq_cis in each head
		rope_llama(m->q, k, dim, kv_dim, head_size, pos);

		// multihead attention. iterate over all heads
		int h;
		#pragma omp parallel for private(h)
		for (h=0; h<m->n_head; h++) {
			// Attention(Q,K,V) = softmax k(QK^T/sqrt(dk))V

			// Calculate the score (Self-Attention) that expresses the relevance between each word
			// Scaled Dot-Product Attention: QK^T/sqrt(dk)
			real* q = m->q + h * head_size; // get the query vector for this head
			real* att = m->att + h * m->seq_len; // attention scores for this head
			// iterate over all timesteps, including the current one
			for (int t=0; t<=pos; t++) { // Mask?
				// get the key vector for this head and at this timestep
				real* k = m->key_cache + loff + t * kv_dim + (h / kv_mul) * head_size;
				// calculate the attention score as the dot product of q and k
				real score = 0.0;
				for (int i=0; i<head_size; i++) {
					score += q[i] * k[i];
				}
				score /= sqrtf(head_size);
//				score *= sqrt_head_size;
				// save the score to the attention buffer
				att[t] = score;
			}
			// softmax the scores to get attention weights, from 0..pos inclusively
			softmax(att, pos+1);

			// Keep the value of featured words and make irrelevant words more irrelevant.
			// weighted sum of the values, store back into xb
			/*for (int i=0; i<head_size; i++) {
				real val = 0.0f;
				for (int t=0; t<=pos; t++) {
					val += att[t] * s->value_cache[loff + t * dim + h * head_size + i]; // note bad locality
				}
				s->xb[h * head_size + i] = val;
			}*/
			real* xb = m->xb + h * head_size;
			memset(xb, 0, head_size * sizeof(real));
//			int ts = pos>10 ? pos-10 : 0;
			for (int t=0/*ts*/; t<=pos; t++) { // Mask?
				// get the value vector for this head and at this timestep
//				real* v = s->value_cache + loff + t * dim + h * head_size;
				real* v = m->value_cache + loff + t * kv_dim + (h / kv_mul) * head_size;
				// get the attention weight for this timestep
				real a = att[t];
				// accumulate the weighted value into xb
				for (int i=0; i<head_size; i++) {
					xb[i] += a * v[i];
				}
			}
		}

		// final matmul to get the output of the attention
		m->wo[l]->quantize((int8_t*)m->xb, m->xb, dim);
		m->wo[l]->matmul(m->xb2, m->xb, m->wo[l]->data, dim, dim);

		// Add & Norm
		// residual connection back into x
		for (int i=0; i<dim; i++) {
			x[i] += m->xb2[i];
		}
		// ffn rmsnorm
		rmsnorm(m->xb, x, m->rms_ffn_weight[l]->data, dim);

		// Feed Forward
		// Now for FFN in PyTorch we have: self.w2(F.silu(self.w1(x)) * self.w3(x))
		// first calculate self.w1(x) and self.w3(x)
		m->w1[l]->quantize((int8_t*)m->xb, m->xb, dim);
		m->w1[l]->matmul(m->hb, m->xb, m->w1[l]->data, dim, hidden_dim);
		m->w3[l]->matmul(m->hb2, m->xb, m->w3[l]->data, dim, hidden_dim);
		// SwiGLU non-linearity
		for (int i=0; i<hidden_dim; i++) {
			real val = m->hb[i];
			// silu(x)=x*σ(x), where σ(x) is the logistic sigmoid
			val *= (1.0 / (1.0 + expf(-val)));
			// elementwise multiply with w3(x)
			val *= m->hb2[i];
			m->hb[i] = val;
		}
		// final matmul to get the output of the ffn
		m->w2[l]->quantize((int8_t*)m->hb, m->hb, hidden_dim);
		m->w2[l]->matmul(m->xb, m->hb, m->w2[l]->data, hidden_dim, dim);

		// Add & Norm
		// residual connection
		for (int i=0; i<dim; i++) {
			x[i] += m->xb[i];
		}
	}

	// final rmsnorm
	rmsnorm(x, x, m->rms_final_weight->data, dim);

	// Linear
	// classifier into logits
	m->wcls->quantize((int8_t*)x, x, dim);
	m->wcls->matmul(m->logits, x, m->wcls->data, dim, m->n_vocab);
	return m->logits;
}

//---------------------------------------------------------
// The Byte Pair Encoding (BPE) Tokenizer that translates strings <-> tokens

typedef struct {
	char *str;
	int id;
} TokenIndex;

int compare_tokens(const void *a, const void *b)
{
	return strcmp(((TokenIndex*)a)->str, ((TokenIndex*)b)->str);
}
int str_lookup(char *str, TokenIndex *sorted_vocab, int vocab_size)
{
	// efficiently find the perfect match for str in vocab, return its index or -1 if not found
	TokenIndex tok = { .str = str }; // acts as the key to search for
	TokenIndex *res = bsearch(&tok, sorted_vocab, vocab_size, sizeof(TokenIndex), compare_tokens);
	return res != NULL ? res->id : -1;
}

char* bpe_decode(cats_ggml_model* t, int prev_token, int token)
{
	char *piece = t->vocab[token];
	// following BOS (1) token, sentencepiece decoder strips any leading whitespace (see PR #89)
	if (prev_token == 1 && piece[0] == ' ') piece++;
	// careful, some tokens designate raw bytes, and look like e.g. '<0x01>'
	// parse this and convert and return the actual byte
	uint8_t byte_val;
	if (sscanf(piece, "<0x%02hhX>", &byte_val) == 1) {
		piece = (char*)t->byte_pieces + byte_val * 2;
	}
	return piece;
}

void sprint(char *piece)
{
	// piece might be a raw byte token, and we only want to print printable chars or whitespace
	// because some of the other bytes can be various control codes, backspace, etc.
	if (piece == NULL) return;
	if (piece[0] == '\0') return;
	if (piece[1] == '\0') {
		uint8_t byte_val = piece[0];
		if (!(isprint(byte_val) || isspace(byte_val))) {
			return; // bad byte, don't print it
		}
	}
	printf("%s", piece);
}

void bpe_encode(char *text, int bos, int eos, char **vocab, real *vocab_scores, int vocab_size, uint32_t max_token_length, int *tokens, int *n_tokens)
{
	// encode the string text (input) into an upper-bound preallocated tokens[] array
	// bos != 0 means prepend the BOS token (=1), eos != 0 means append the EOS token (=2)
	if (text == NULL) {
		fprintf(stderr, "Can't encode NULL text.\n");
		exit(EXIT_FAILURE);
	}

	// sort vocabulary
	TokenIndex *sorted_vocab = malloc(vocab_size * sizeof(TokenIndex));
	for (int i=0; i<vocab_size; i++) {
		sorted_vocab[i].str = vocab[i];
		sorted_vocab[i].id = i;
	}
	qsort(sorted_vocab, vocab_size, sizeof(TokenIndex), compare_tokens);

	// create a temporary buffer that will store merge candidates of always two consecutive tokens
	char* str_buffer = malloc((max_token_length*2+1+2) * sizeof(char)); // *2 for concat, +1 for null terminator +2 for UTF8 (in case max_token_lenght is 1)
	size_t str_len = 0;

	// start at 0 tokens
	*n_tokens = 0;

	// add optional BOS (=1) token, if desired
	if (bos) tokens[(*n_tokens)++] = 1;

	// add_dummy_prefix is true by default
	// so prepend a dummy prefix token to the input string, but only if text != ""
	// TODO: pretty sure this isn't correct in the general case but I don't have the
	// energy to read more of the sentencepiece code to figure out what it's doing
	if (text[0] != '\0') {
		tokens[(*n_tokens)++] = str_lookup(" ", sorted_vocab, vocab_size);
	}

	// Okay UTF-8 time. This will get messy. Here is the reference from Wikipedia:
	// Code point ↔ UTF-8 conversion
	// First code point	Last code point	Byte 1	Byte 2	Byte 3	Byte 4
	// U+0000	U+007F	    0xxxxxxx
	// U+0080	U+07FF	    110xxxxx	10xxxxxx
	// U+0800	U+FFFF	    1110xxxx	10xxxxxx	10xxxxxx
	// U+10000	U+10FFFF    11110xxx	10xxxxxx	10xxxxxx	10xxxxxx

	// process the raw (UTF-8) byte sequence of the input string
	for (char *c = text; *c != '\0'; c++) {
		// reset buffer if the current byte is ASCII or a leading byte
		// 0xC0 is 11000000, so (*c & 0xC0) keeps the first 2 bits and zeros the rest
		// 0x80 is 10000000
		// in UTF-8, all continuation bytes start with "10" in first two bits
		// so in English this is: "if this byte is not a continuation byte"
		if ((*c & 0xC0) != 0x80) {
			// this byte must be either a leading byte (11...) or an ASCII char (0x...)
			// => reset our location, as we're starting a new UTF-8 codepoint
			str_len = 0;
		}

		// append the current byte to the buffer
		str_buffer[str_len++] = *c; // ++ is post-increment, incremented after this line
		str_buffer[str_len] = '\0';

		// while the next character is a continuation byte, continue appending
		// but if there are too many of them, just stop to avoid overruning str_buffer size.
		if ((*(c+1) & 0xC0) == 0x80 && str_len < 4) continue;

		// ok c+1 is not a continuation byte, so we've read in a full codepoint
		int id = str_lookup(str_buffer, sorted_vocab, vocab_size);
		if (id != -1) {
			// we found this codepoint in vocab, add it as a token
			tokens[(*n_tokens)++] = id;
		} else {
			// byte_fallback encoding: just encode each byte as a token
			// +3 is here because the first 3 vocab elements are <unk>, <s>, </s>
			// so the individual bytes only start at index 3
			for (int i=0; i < str_len; i++) {
				tokens[(*n_tokens)++] = (uint8_t)str_buffer[i] + 3;
			}
		}
		str_len = 0; // protect against a sequence of stray UTF8 continuation bytes
	}

	// merge the best consecutive pair each iteration, according the scores in vocab_scores
	while (1) {
		real best_score = -1e10;
		int best_id = -1;
		int best_idx = -1;

		for (int i=0; i < (*n_tokens-1); i++) {
			// check if we can merge the pair (tokens[i], tokens[i+1])
			sprintf(str_buffer, "%s%s", vocab[tokens[i]], vocab[tokens[i+1]]);
			int id = str_lookup(str_buffer, sorted_vocab, vocab_size);
			if (id != -1 && vocab_scores[id] > best_score) {
				// this merge pair exists in vocab! record its score and position
				best_score = vocab_scores[id];
				best_id = id;
				best_idx = i;
			}
		}
		if (best_idx == -1) {
			break; // we couldn't find any more pairs to merge, so we're done
		}

		// merge the consecutive pair (best_idx, best_idx+1) into new token best_id
		tokens[best_idx] = best_id;
		// delete token at position best_idx+1, shift the entire sequence back 1
		for (int i = best_idx+1; i < (*n_tokens-1); i++) {
			tokens[i] = tokens[i+1];
		}
		(*n_tokens)--; // token length decreased
	}

	// add optional EOS (=2) token, if desired
	if (eos) tokens[(*n_tokens)++] = 2;

	free(str_buffer);
	free(sorted_vocab);
}

// ----------------------------------------------------------------------------
// The Sampler, which takes logits and returns a sampled token
// sampling can be done in a few ways: greedy argmax, sampling, top-p sampling

typedef struct {
	real prob;
	int index;
} ProbIndex; // struct used when sorting probabilities during top-p sampling

static int sample_argmax(real* probabilities, int n)
{
	// return the index that has the highest probability
	int max_i = 0;
	real max_p = probabilities[0];
	for (int i=1; i<n; i++) {
		if (probabilities[i] > max_p) {
			max_i = i;
			max_p = probabilities[i];
		}
	}
	return max_i;
}

static int sample_mult(real* probabilities, int n, real coin)
{
	// sample index from probabilities (they must sum to 1!)
	real cdf = 0.0;
	for (int i=0; i<n; i++) {
		cdf += probabilities[i];
		if (coin < cdf) {
			return i;
		}
	}
	return n-1; // in case of rounding errors
}

int compare(const void* a, const void* b)
{
	ProbIndex* a_ = (ProbIndex*) a;
	ProbIndex* b_ = (ProbIndex*) b;
	if (a_->prob > b_->prob) return -1;
	if (a_->prob < b_->prob) return 1;
	return 0;
}
int sample_topp(real* probabilities, int n, real topp, ProbIndex* probindex, real coin)
{
	// top-p sampling (or "nucleus sampling") samples from the smallest set of
	// tokens that exceed probability topp. This way we never sample tokens that
	// have very low probabilities and are less likely to go "off the rails".

	int n0 = 0;
	// quicksort indices in descending order of probabilities
	// values smaller than (1 - topp) / (n - 1) cannot be part of the result
	// so for efficiency we crop these out as candidates before sorting
//	const real cutoff = (1.0 - topp) / (n - 1);
	const real cutoff = 0.001; //a practical lowerbound for the probabilities that get sampled
	for (int i=0; i<n; i++) {
		if (probabilities[i] >= cutoff) {
			probindex[n0].index = i;
			probindex[n0].prob = probabilities[i];
			n0++;
		}
	}
	qsort(probindex, n0, sizeof(ProbIndex), compare);

	// truncate the list where cumulative probability exceeds topp
	real cumulative_prob = 0.0;
	int last_idx = n0 - 1; // in case of rounding errors consider all elements
	for (int i=0; i<n0; i++) {
		cumulative_prob += probindex[i].prob;
		if (cumulative_prob > topp) {
			last_idx = i;
			break; // we've exceeded topp by including last_idx
		}
	}

	// sample from the truncated list
	real r = coin * cumulative_prob;
	real cdf = 0.0;
	for (int i=0; i<=last_idx; i++) {
		cdf += probindex[i].prob;
		if (r < cdf) {
			return probindex[i].index;
		}
	}
	return probindex[last_idx].index; // in case of rounding errors
}

void generate(cats_ggml_model *m, char *prompt, int steps, real temperature, real topp)
{
	// encode the (string) prompt into tokens sequence
	if (prompt == NULL) prompt = "";
	int num_prompt_tokens = 0;
	int* prompt_tokens = (int*)malloc((strlen(prompt)+3) * sizeof(int)); // +3 for '\0', ?BOS, ?EOS
	bpe_encode(prompt, 1, 0, m->vocab, m->score, m->n_vocab, m->max_token_length, prompt_tokens, &num_prompt_tokens);
	if (num_prompt_tokens < 1) {
		fprintf(stderr, "Something is wrong, expected at least 1 prompt token.\n");
		exit(EXIT_FAILURE);
	}
	for (int i=0; i<num_prompt_tokens; i++) {
		printf("%d ", prompt_tokens[i]);
	}
	printf("\n");
	ProbIndex *probindex = calloc(m->n_vocab, sizeof(ProbIndex));

	// start the main loop
	long start = 0;	// used to time our code, only initialized after first iteration
	int next;	// will store the next token in the sequence
	int token = prompt_tokens[0]; // kick off with the first token in the prompt
	int pos = 0;	// position in the sequence
	while (pos < steps) {
		// forward the transformer to get logits for the next token
		real* logits = cats_llama2_transformer(token, pos, m);

		// advance the state machine
		if (pos < num_prompt_tokens - 1) {
			// if we are still processing the input prompt, force the next prompt token
			next = prompt_tokens[pos + 1];
		} else {
			// otherwise sample the next token from the logits
			if (temperature == 0.0) {
				// greedy argmax sampling
				next = sample_argmax(logits, m->n_vocab);
			} else {
				// apply the temperature to the logits
				for (int q=0; q<m->n_vocab; q++) {
					logits[q] /= temperature;
				}
				// apply softmax to the logits to get the probabilities for next token
				softmax(logits, m->n_vocab);
				// flip a (float) coin (this is our source of entropy for sampling)
//				float coin = random_f32(&sampler->rng_state);
				float coin = random_f32();
				// we sample from this distribution to get the next token
				if (topp <= 0 || topp >= 1) {
					// simply sample from the predicted probability distribution
					next = sample_mult(logits, m->n_vocab, coin);
				} else {
					// top-p (nucleus) sampling, clamping the least likely tokens to zero
					next = sample_topp(logits, m->n_vocab, topp, probindex, coin);
				}
			}
		}
		pos++;

		// data-dependent terminating condition: the BOS (=1) token delimits sequences
		if (next == 1) break;

		// print the token as string, decode it with the Tokenizer object
		char* piece = bpe_decode(m, token, next);
		sprint(piece); // same as printf("%s", piece), but skips "unsafe" bytes
		fflush(stdout);
		token = next;

		// init the timer here because the first iteration can be slower
		if (start == 0) start = time_in_ms();
	}
	printf("\n");

	// report achieved tok/s (pos-1 because the timer starts after first iteration)
	if (pos > 1) {
		long end = time_in_ms();
		fprintf(stderr, "achieved tok/s: %f\n", (pos-1) / (double)(end-start)*1000);
	}
	free(prompt_tokens);
	free(probindex);

/*	// process the prompt, if any
	int *prompt_tokens = NULL;
	int num_prompt_tokens = 0;
	if (prompt != NULL) {
		prompt_tokens = (int*)malloc((strlen(prompt)+1) * sizeof(int));
		bpe_encode(prompt, 1, 0, m->vocab, m->score, m->n_vocab, m->max_token_length, prompt_tokens, &num_prompt_tokens);
		for (int i=0; i<num_prompt_tokens; i++) {
			printf("%d ", prompt_tokens[i]);
		}
		printf("\n");
	}
	ProbIndex *probindex = calloc(m->n_vocab, sizeof(ProbIndex));

	// start the main loop
	long start = 0;	// used to time our code, only initialized after first iteration
	int next;	// will store the next token in the sequence
	int token = 1;	// init with token 1 (=BOS), as done in Llama-2 sentencepiece tokenizer
	int pos = 0;	// position in the sequence
	//printf("<s>\n"); // explicit print the initial BOS token (=1), stylistically symmetric
	while (pos < steps) {
		// forward the transformer to get logits for the next token
		real* logits = cats_llama2_transformer(token, pos, m);

		// advance the state state machine
		if (pos < num_prompt_tokens) {
			// if we are still processing the input prompt, force the next prompt token
			next = prompt_tokens[pos];
		} else {
			// sample the next token
			if (temperature == 0.0) {
				// greedy argmax sampling
				next = sample_argmax(logits, m->n_vocab);
			} else {
				// apply the temperature to the logits
				for (int q=0; q<m->n_vocab; q++) {
					logits[q] /= temperature;
				}
				// apply softmax to the logits to get the probabilities for next token
				softmax(logits, m->n_vocab);
				// flip a (float) coin (this is our source of entropy for sampling)
				float coin = random_f32();
				// we sample from this distribution to get the next token
				if (topp <= 0 || topp >= 1) {
					// simply sample from the predicted probability distribution
					next = sample_mult(logits, m->n_vocab, coin);
				} else {
					// top-p (nucleus) sampling, clamping the least likely tokens to zero
					next = sample_topp(logits, m->n_vocab, topp, probindex, coin);
				}
			}
		}
		pos++;

		// data-dependent terminating condition: the BOS (1) token delimits sequences
		if (next==1) break;

		// following BOS token (1), sentencepiece decoder strips any leading whitespace (see PR #89)
		char *token_str = (token == 1 && m->vocab[next][0] == ' ') ? m->vocab[next]+1 : m->vocab[next];
		// careful, some tokens designate raw bytes, and look like e.g. '<0x01>'
		uint8_t byte_val;
		if (sscanf(token_str, "<0x%02hhX>", &byte_val) == 1) {
			// ok this token is a raw byte token, carefuly to only print printable chars or whitespace
			// some of the other bytes can be various control codes, backspace, etc. => skip
			if (isprint(byte_val) || isspace(byte_val)) {
				char byte_piece[2];
				byte_piece[0] = byte_val;
				byte_piece[1] = '\0';
				printf("%s", byte_piece);
			}
		} else {
			printf("%s", token_str);
		}
		fflush(stdout);
		token = next;

		// init the timer here because the first iteration can be slower
		if (start == 0) { start = time_in_ms(); }
	}
	// report achieved tok/s
	long end = time_in_ms();
	printf("\nachieved tok/s: %f\n", (steps-1) / (double)(end-start)*1000);

	free(probindex);*/
}

static void read_stdin(const char* guide, char* buffer, size_t bufsize)
{
	// read a line from stdin, up to but not including \n
	printf("%s", guide);
	if (fgets(buffer, bufsize, stdin) != NULL) {
		size_t len = strlen(buffer);
		if (len > 0 && buffer[len - 1] == '\n') {
			buffer[len - 1] = '\0'; // strip newline
		}
	}
}

void chat(cats_ggml_model *m, char *cli_user_prompt, char *cli_system_prompt, int steps, real temperature, real topp)
{
	ProbIndex *probindex = calloc(m->n_vocab, sizeof(ProbIndex));

	// buffers for reading the system prompt and user prompt from stdin
	// you'll notice they are soomewhat haphazardly and unsafely set atm
	char system_prompt[512];
	char user_prompt[512];
	char rendered_prompt[1152];
	int num_prompt_tokens = 0;
	int* prompt_tokens = (int*)malloc(1152 * sizeof(int));
	int user_idx;

	// start the main loop
	int8_t user_turn = 1; // user starts
	int next;        // will store the next token in the sequence
	int token;       // stores the current token to feed into the transformer
	int prev_token;
	int pos = 0;     // position in the sequence
	while (pos < steps) {
		// when it is the user's turn to contribute tokens to the dialog...
		if (user_turn) {
			// get the (optional) system prompt at position 0
			if (pos == 0) {
				// at position 0, the user can also contribute a system prompt
				if (cli_system_prompt == NULL) {
					// system prompt was not passed in, attempt to get it from stdin
					read_stdin("Enter system prompt (optional): ", system_prompt, sizeof(system_prompt));
				} else {
					// system prompt was passed in, use it
					strcpy(system_prompt, cli_system_prompt);
				}
			}
			// get the user prompt
			if (pos == 0 && cli_user_prompt != NULL) {
				// user prompt for position 0 was passed in, use it
				strcpy(user_prompt, cli_user_prompt);
			} else {
				// otherwise get user prompt from stdin
				read_stdin("User: ", user_prompt, sizeof(user_prompt));
			}
			// render user/system prompts into the Llama 2 Chat schema
			if (pos == 0 && system_prompt[0] != '\0') {
				char system_template[] = "[INST] <<SYS>>\n%s\n<</SYS>>\n\n%s [/INST]";
				sprintf(rendered_prompt, system_template, system_prompt, user_prompt);
			} else {
				char user_template[] = "[INST] %s [/INST]";
				sprintf(rendered_prompt, user_template, user_prompt);
			}
			// encode the rendered prompt into tokens
			bpe_encode(rendered_prompt, 1, 0, m->vocab, m->score, m->n_vocab, m->max_token_length, prompt_tokens, &num_prompt_tokens);
			user_idx = 0; // reset the user index
			user_turn = 0;
			printf("Assistant: ");
		}

		// determine the token to pass into the transformer next
		if (user_idx < num_prompt_tokens) {
			// if we are still processing the input prompt, force the next prompt token
			token = prompt_tokens[user_idx++];
		} else {
			// otherwise use the next token sampled from previous turn
			token = next;
		}
		// EOS (=2) token ends the Assistant turn
		if (token == 2) user_turn = 1;

		// forward the transformer to get logits for the next token
		real* logits = cats_llama2_transformer(token, pos, m);
//		next = sample(sampler, logits);
		// sample the next token
		if (temperature == 0.0) {
			// greedy argmax sampling
			next = sample_argmax(logits, m->n_vocab);
		} else {
			// apply the temperature to the logits
			for (int q=0; q<m->n_vocab; q++) {
				logits[q] /= temperature;
			}
			// apply softmax to the logits to get the probabilities for next token
			softmax(logits, m->n_vocab);
			// flip a (float) coin (this is our source of entropy for sampling)
//			float coin = random_f32(&sampler->rng_state);
			float coin = random_f32();
			// we sample from this distribution to get the next token
			if (topp <= 0 || topp >= 1) {
				// simply sample from the predicted probability distribution
				next = sample_mult(logits, m->n_vocab, coin);
			} else {
				// top-p (nucleus) sampling, clamping the least likely tokens to zero
				next = sample_topp(logits, m->n_vocab, topp, probindex, coin);
			}
		}
		pos++;

		if (user_idx >= num_prompt_tokens && next != 2) {
			// the Assistant is responding, so print its output
			char* piece = bpe_decode(m, token, next);
			sprint(piece); // same as printf("%s", piece), but skips "unsafe" bytes
			fflush(stdout);
		}
		if (next == 2) printf("\n");
	}
	printf("\n");
	free(prompt_tokens);
	free(probindex);
}

//---------------------------------------------------------
// GGLM format

#define LLAMA_FILE_MAGIC_GGJT        0x67676a74u // 'ggjt'
#define LLAMA_FILE_MAGIC_GGLA        0x67676c61u // 'ggla'
#define LLAMA_FILE_MAGIC_GGMF        0x67676d66u // 'ggmf'
#define LLAMA_FILE_MAGIC_GGML        0x67676d6cu // 'ggml'
#define LLAMA_FILE_MAGIC_GGSN        0x6767736eu // 'ggsn'

enum llama_file_version {
	LLAMA_FILE_VERSION_GGML,
	LLAMA_FILE_VERSION_GGMF_V1, // added version field and scores in vocab
	LLAMA_FILE_VERSION_GGJT_V1, // added padding
	LLAMA_FILE_VERSION_GGJT_V2, // changed quantization format
	LLAMA_FILE_VERSION_GGJT_V3, // changed Q4 and Q8 quantization format
};

// model file types
enum llama_ftype {
	LLAMA_FTYPE_ALL_F32              = 0,
	LLAMA_FTYPE_MOSTLY_F16           = 1, // except 1d tensors
	LLAMA_FTYPE_MOSTLY_Q4_0          = 2, // except 1d tensors
	LLAMA_FTYPE_MOSTLY_Q4_1          = 3, // except 1d tensors
	LLAMA_FTYPE_MOSTLY_Q4_1_SOME_F16 = 4, // tok_embeddings.weight and output.weight are F16
	// LLAMA_FTYPE_MOSTLY_Q4_2       = 5, // support has been removed
	// LLAMA_FTYPE_MOSTLY_Q4_3       = 6, // support has been removed
	LLAMA_FTYPE_MOSTLY_Q8_0          = 7, // except 1d tensors
	LLAMA_FTYPE_MOSTLY_Q5_0          = 8, // except 1d tensors
	LLAMA_FTYPE_MOSTLY_Q5_1          = 9, // except 1d tensors
	LLAMA_FTYPE_MOSTLY_Q2_K          = 10,// except 1d tensors
	LLAMA_FTYPE_MOSTLY_Q3_K_S        = 11,// except 1d tensors
	LLAMA_FTYPE_MOSTLY_Q3_K_M        = 12,// except 1d tensors
	LLAMA_FTYPE_MOSTLY_Q3_K_L        = 13,// except 1d tensors
	LLAMA_FTYPE_MOSTLY_Q4_K_S        = 14,// except 1d tensors
	LLAMA_FTYPE_MOSTLY_Q4_K_M        = 15,// except 1d tensors
	LLAMA_FTYPE_MOSTLY_Q5_K_S        = 16,// except 1d tensors
	LLAMA_FTYPE_MOSTLY_Q5_K_M        = 17,// except 1d tensors
	LLAMA_FTYPE_MOSTLY_Q6_K          = 18,// except 1d tensors
};

static void cats_read_vocab(int fd, int size, char **word, float *score, int type)
{
	for (uint32_t i=0; i<size; i++) {
		if (type==1) read(fd, &score[i], sizeof(float));
		uint32_t len;
		read(fd, &len, sizeof(uint32_t));
		word[i] = (char*)malloc(len+1);
		read(fd, word[i], len);
		word[i][len] = 0;
		if (type==0) read(fd, &score[i], sizeof(float));
	}
	/*for (uint32_t i=100; i<200; i++) {
		printf("%06d [%s] %f\n", i, word[i], score[i]);
	}*/
}

static void dequantize_file(int fd, cats_tensor *t, int f)
{
	if (f) {
		t->quantize = quantize_q8;
		t->matmul = matmul_q4_0_q8_avx;
		read(fd, t->data, t->size);
		return;
	}

	// unzip to f32
	t->matmul = matmul_avx;
	switch (t->type) {
//	case LLAMA_FTYPE_MOSTLY_F16:
	case LLAMA_FTYPE_ALL_F32:
		read(fd, t->data, t->size);
		break;
	case LLAMA_FTYPE_MOSTLY_Q4_0:
		uint8_t *d = malloc(t->size);
		read(fd, d, t->size);
		dequantize_q4_0(t->data, d, (t->size/18)*32);
		free(d);
		break;
	default:
		printf("Not support: %d\n", t->type);
	}
	return;
}

static inline uint64_t cats_gsize(int type, uint32_t shape0, uint32_t shape1)
{
//	uint64_t size = shape1 ? shape0*shape1 : shape0;
	switch (type) {
	case LLAMA_FTYPE_ALL_F32:
		return shape0 * sizeof(float);
	case LLAMA_FTYPE_MOSTLY_Q4_0:
		return shape0*shape1 / 32 * 18;
	case LLAMA_FTYPE_MOSTLY_Q4_1:
		return shape0*shape1 / 32 * 24;
	}
	printf("error at cats_gsize\n");
	return 0;
}

static int cats_ggml_load(char *name, cats_ggml_model *m)
{
	FILE *fp = fopen(name, "rb");
	if (!fp) {
		fprintf(stderr, "Couldn't open file %s\n", name);
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	size_t fsize = ftell(fp);
	fclose(fp);

	int fd = open(name, O_RDONLY);
	if (fd==-1) {
		fprintf(stderr, "Couldn't open file %s\n", name);
		return -1;
	}

	// magic
	uint32_t magic, version;
	read(fd, &magic, sizeof(uint32_t));
	switch (magic) {
	case LLAMA_FILE_MAGIC_GGML:
		printf("magic: ggml\n");
		break;
	case LLAMA_FILE_MAGIC_GGMF:
		read(fd, &version, sizeof(uint32_t));
		printf("magic: ggmf ver %d\n", version);
		break;
	case LLAMA_FILE_MAGIC_GGJT:
		read(fd, &version, sizeof(uint32_t));
		printf("magic: ggjt ver %d\n", version);
		break;
	default:
//		return cats_checkpoint_load(name, m);
		fprintf(stderr, "unknown (magic, version) combination: %08x\n", magic);
		return -1;
	}

	// hparams
	read(fd, m, sizeof(uint32_t)*7);
	printf("n_vocab: %d\n", m->n_vocab);	// vocab size: 32000
	printf("n_embd: %d\n", m->n_embd);	// dimention: 4096
	printf("n_mult: %d\n", m->n_mult);	// 256
	printf("n_head: %d\n", m->n_head);	// 32
	printf("n_layer: %d\n", m->n_layer);	// 32
	printf("n_rot: %d\n", m->n_rot);	// 128
	printf("ftype: %d\n\n", m->ftype);

	// vocab
	m->vocab = (char**)malloc(m->n_vocab * sizeof(char*));
	m->score = (float*)malloc(m->n_vocab * sizeof(float));
	cats_read_vocab(fd, m->n_vocab, m->vocab, m->score, 0);

	// metadata
	int n = 0;
	while (lseek(fd, 0, SEEK_CUR) < fsize) {
		uint32_t shape_len, len, type;
		read(fd, &shape_len, sizeof(uint32_t));
		read(fd, &len, sizeof(uint32_t));
		read(fd, &type, sizeof(uint32_t));

		uint32_t shape[shape_len];
		read(fd, shape, sizeof(uint32_t)*shape_len);
//		for (int i=0; i<shape_len; i++) printf("shape[%d]: %d\n", i, shape[i]);

		char name[256];
		read(fd, name, len);
		name[len] = 0;

		// skip to the next multiple of 32 bytes
		if (magic==LLAMA_FILE_MAGIC_GGJT && version >= LLAMA_FILE_VERSION_GGJT_V1) {
			lseek(fd, (lseek(fd, 0, SEEK_CUR)+31)&-32, SEEK_SET);
		}

		int size = cats_gsize(type, shape[0], shape[1]);
/*		int size = 0;
		switch (type) {
		case LLAMA_FTYPE_ALL_F32:
			size = shape[0]*sizeof(float);
			break;
		case LLAMA_FTYPE_MOSTLY_Q4_0:
			// See GPTQForLLaMaQuantizedTensor.ggml_ndarray()
//			size = 20 * (shape[1] / 32) * shape[0];
//			size = shape[1] * shape[0] * 0.5625;
			size = 18 * (shape[1] / 32) * shape[0];
//			if (shape_len==1) size = shape[0] * 0.5625;
			break;
		case LLAMA_FTYPE_MOSTLY_Q4_1:
			size = 24 * (shape[1] / 32) * shape[0];
			break;
		default:
		}*/

		// set weights informaton
		strncpy(m->tensor[n].name, name, CATS_NAME_MAX);
		m->tensor[n].type = type;
		m->tensor[n].dim[1] = 0;
		for (int i=0; i<shape_len; i++) m->tensor[n].dim[i] = shape[i];
		m->tensor[n].size = size;
		m->tensor[n].offset = lseek(fd, 0, SEEK_CUR);
		printf("%s(%d): %d (%d,%d)\n", m->tensor[n].name, m->tensor[n].type, m->tensor[n].size, m->tensor[n].dim[0], m->tensor[n].dim[1]);
//		if (!strcmp("layers.0.feed_forward.w1.weight", name)) hidden_dim = m->tensor[n].dim[1];
		n++;

		lseek(fd, size, SEEK_CUR);
		// grep "norm*" llama-2-7b-chat.ggmlv3.q4_0.bin -o -a -b
	}

	m->n_hidden = ((2*(4*m->n_embd)/3 + m->n_mult - 1)/m->n_mult)*m->n_mult; // (2*4*4096/3+256-1)/256*256
	printf("hidden dim: %d\n", m->n_hidden);
	m->n_kv_head/*FIXME*/ = m->n_head;
	m->seq_len = 2048;

	// clac needed memory size
	int head_size = m->n_embd / m->n_head;
	uint64_t total_size = (m->n_vocab * m->n_embd);		// token_embedding_table
	total_size += (m->n_layer * m->n_embd);	 		// rms_att_weight: attention_norm.weight
	total_size += (m->n_layer * m->n_embd * m->n_embd); 	// wq
	total_size += (m->n_layer * m->n_embd * m->n_embd); 	// wk
	total_size += (m->n_layer * m->n_embd * m->n_embd); 	// wv
	total_size += (m->n_layer * m->n_embd * m->n_embd); 	// wo
	total_size += (m->n_layer * m->n_embd); 		// rms_ffn_weight: ffn_norm.weight
	total_size += (m->n_layer * m->n_embd * m->n_hidden); 	// w1
	total_size += (m->n_layer * m->n_hidden * m->n_embd); 	// w2
	total_size += (m->n_layer * m->n_embd * m->n_hidden); 	// w3
	total_size += (m->n_embd); 				// rms_final_weight: norm.weight
	total_size += (m->seq_len * head_size / 2);		// freq_cis_real
	total_size += (m->seq_len * head_size / 2);		// freq_cis_imag
	total_size += (m->n_vocab * m->n_embd);			// wcls: shared_weights: output.weight
	printf("Needed memory size: %u (%uMB)\n", total_size, total_size/1024/1024*sizeof(float));

//	precompute_freqs_cis(head_size, model->seq_len, 10000.0, w->freq_cis_real, w->freq_cis_imag);

	int f = 1; // no expand
	for (int i=0; i<n; i++) {
		lseek(fd, m->tensor[i].offset, SEEK_SET);
		if (!strcmp(m->tensor[i].name, "tok_embeddings.weight")) {
			m->tensor[i].data = malloc(cats_gsize(LLAMA_FTYPE_ALL_F32, m->n_vocab * m->n_embd, 0));
			m->token_embedding_table = &m->tensor[i];
//			dequantize_file(fd, m->tensor[i].type, m->tensor[i].size, m->tensor[i].data, 0); // q4 -> f32
			dequantize_file(fd, &m->tensor[i], f);
//			for (i=0; i<500; i++) printf("%f ", w->token_embedding_table[i]);
		} else if (!strcmp(m->tensor[i].name, "norm.weight")) {
			m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_embd, 0));
			m->rms_final_weight = &m->tensor[i];
			dequantize_file(fd, &m->tensor[i], f);
//			for (i=0; i<m->tensor[i].size; i++) printf("%f ", w->rms_final_weight[i]);
		} else if (!strcmp(m->tensor[i].name, "output.weight")) {
			m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_vocab, m->n_embd));
			m->wcls = &m->tensor[i];
			dequantize_file(fd, &m->tensor[i], f);
//			for (i=0; i<500; i++) printf("%f ", w->wcls[i]);
		} else {
			char *s = m->tensor[i].name +7;
			int l = atoi(s);
//			printf("layer %d: %s\n", l, s);

			s += 2;
			if (l>9) s++;

			if (!strcmp(s, "attention.wq.weight")) {
				m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_embd, m->n_embd));
				m->wq[l] = &m->tensor[i];
				dequantize_file(fd, &m->tensor[i], f);
//				for (i=0; i<500; i++) printf("%f ", w->wq[i]);
			} else if (!strcmp(s, "attention.wk.weight")) {
				m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_embd, m->n_embd));
				m->wk[l] = &m->tensor[i];
				dequantize_file(fd, &m->tensor[i], f);
//				for (i=0; i<500; i++) printf("%f ", w->wk[i]);
			} else if (!strcmp(s, "attention.wv.weight")) {
				m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_embd, m->n_embd));
				m->wv[l] = &m->tensor[i];
				dequantize_file(fd, &m->tensor[i], f);
//				for (i=0; i<500; i++) printf("%f ", w->wv[i]);
			} else if (!strcmp(s, "attention.wo.weight")) {
				m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_embd, m->n_embd));
				m->wo[l] = &m->tensor[i];
				dequantize_file(fd, &m->tensor[i], f);
//				for (i=0; i<500; i++) printf("%f ", w->wo[i]);
			} else if (!strcmp(s, "attention_norm.weight")) {
				m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_embd, 0));
				m->rms_att_weight[l] = &m->tensor[i];
				dequantize_file(fd, &m->tensor[i], f);
//				for (i=0; i<500; i++) printf("%f ", w->rms_att_weight[i]);
			} else if (!strcmp(s, "feed_forward.w1.weight")) {
				m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_embd, m->n_hidden));
				m->w1[l] = &m->tensor[i];
				dequantize_file(fd, &m->tensor[i], f);
//				for (i=0; i<500; i++) printf("%f ", w->w1[i]);
			} else if (!strcmp(s, "feed_forward.w2.weight")) {
				m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_hidden, m->n_embd));
				m->w2[l] = &m->tensor[i];
				dequantize_file(fd, &m->tensor[i], f);
//				for (i=0; i<500; i++) printf("%f ", w->w2[i]);
			} else if (!strcmp(s, "feed_forward.w3.weight")) {
				m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_embd, m->n_hidden));
				m->w3[l] = &m->tensor[i];
				dequantize_file(fd, &m->tensor[i], f);
//				for (i=0; i<500; i++) printf("%f ", w->w3[i]);
			} else if (!strcmp(s, "ffn_norm.weight")) {
				m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_embd, 0));
				m->rms_ffn_weight[l] = &m->tensor[i];
				dequantize_file(fd, &m->tensor[i], f);
//				for (i=0; i<500; i++) printf("%f ", w->rms_ffn_weight[i]);
			} else {
				printf("Not support %d: %s(%s)\n", l, m->tensor[i].name, s);
			}
		}
		putchar('.');
		fflush(stdout);
	}
	putchar('\n');

	// q4_0 * float: 2.8 tokens/sec
//	quantize = no_quantize;
//	matmul = matmul_q4_0_avx;
	// q4_0 * q8: 5.4 tokens/sec
	dequantize = _dequantize_q4_0;
	quantize = quantize_q8;
	matmul = matmul_q4_0_q8_avx;

	close(fd);
	return 0;
}

//---------------------------------------------------------
// GGUF format
#define GGML_MAX_DIMS		4
#define GGUF_MAX_STR		64

enum ggml_type {
	GGML_TYPE_F32  = 0,
	GGML_TYPE_F16  = 1,
	GGML_TYPE_Q4_0 = 2,
	GGML_TYPE_Q4_1 = 3,
	// GGML_TYPE_Q4_2 = 4, support has been removed
	// GGML_TYPE_Q4_3 (5) support has been removed
	GGML_TYPE_Q5_0 = 6,
	GGML_TYPE_Q5_1 = 7,
	GGML_TYPE_Q8_0 = 8,
	GGML_TYPE_Q8_1 = 9,
	// k-quantizations
	GGML_TYPE_Q2_K = 10,
	GGML_TYPE_Q3_K = 11,
	GGML_TYPE_Q4_K = 12,
	GGML_TYPE_Q5_K = 13,
	GGML_TYPE_Q6_K = 14,
	GGML_TYPE_Q8_K = 15,
	GGML_TYPE_I8,
	GGML_TYPE_I16,
	GGML_TYPE_I32,
	GGML_TYPE_COUNT,
};
void matmul_null(real* restrict, void* restrict, void* restrict, int, int)
{
}
void (*gguf_matmul[])(real* restrict, void* restrict, void* restrict, int, int) = {
	[GGML_TYPE_F32]  = matmul_avx,
	[GGML_TYPE_Q4_0] = matmul_q4_0_q8_avx,
	[GGML_TYPE_Q3_K] = matmul_q3_K_q8_K,
	[GGML_TYPE_Q6_K] = matmul_q6_K_q8_K,
};
void quantize_null(int8_t*, real*, int)
{
}
void (*gguf_quantize[])(int8_t*, real*, int) = {
	[GGML_TYPE_F32]  = quantize_null,
	[GGML_TYPE_Q4_0] = quantize_q8,
	[GGML_TYPE_Q3_K] = quantize_q8_K,
	[GGML_TYPE_Q6_K] = quantize_q8_K,
};
void* (*gguf_dequantize[])(void * restrict, const void * restrict, int, int) = {
	[GGML_TYPE_F32]  = _dequantize_memcpy,
	[GGML_TYPE_Q4_0] = _dequantize_q4_0,
	[GGML_TYPE_Q3_K] = _dequantize_q3_K,
	[GGML_TYPE_Q6_K] = _dequantize_q6_K,
};

enum gguf_value_type {
	GGUF_TYPE_UINT8,
	GGUF_TYPE_INT8,
	GGUF_TYPE_UINT16,
	GGUF_TYPE_INT16,
	GGUF_TYPE_UINT32,
	GGUF_TYPE_INT32,
	GGUF_TYPE_FLOAT32,
	GGUF_TYPE_BOOL,
	GGUF_TYPE_STRING,
	GGUF_TYPE_ARRAY,
	GGUF_TYPE_UINT64,
	GGUF_TYPE_INT64,
	GGUF_TYPE_FLOAT64,
};
static const size_t gguf_value_size[] = {
	[GGUF_TYPE_UINT8]   = sizeof(uint8_t),
	[GGUF_TYPE_INT8]    = sizeof(int8_t),
	[GGUF_TYPE_UINT16]  = sizeof(uint16_t),
	[GGUF_TYPE_INT16]   = sizeof(int16_t),
	[GGUF_TYPE_UINT32]  = sizeof(uint32_t),
	[GGUF_TYPE_INT32]   = sizeof(int32_t),
	[GGUF_TYPE_FLOAT32] = sizeof(float),
	[GGUF_TYPE_BOOL]    = sizeof(uint8_t), //sizeof(bool),
	[GGUF_TYPE_STRING]  = 0, //sizeof(struct gguf_str),
	[GGUF_TYPE_UINT64]  = sizeof(uint64_t),
	[GGUF_TYPE_INT64]   = sizeof(int64_t),
	[GGUF_TYPE_FLOAT64] = sizeof(double),
	[GGUF_TYPE_ARRAY]   = 0, // undefined
};

union gguf_value {
	uint8_t  uint8;
	int8_t   int8;
	uint16_t uint16;
	int16_t  int16;
	uint32_t uint32;
	int32_t  int32;
	float    float32;
	uint64_t uint64;
	int64_t  int64;
	double   float64;
	uint8_t  bool_;		// bool

	char str[GGUF_MAX_STR];

	struct {
		//enum gguf_type type;
		uint32_t type;

		uint64_t n;	// GGUFv2
		void * data;
	} arr;
};

struct gguf_header {
	uint32_t magic;
	uint32_t version;
	uint64_t n_tensors; // GGUFv2
	uint64_t n_kv;      // GGUFv2
};

struct gguf_kv {
	char key[GGUF_MAX_STR];

	uint32_t type;
	union gguf_value value;
};

static int cats_gguf_load(char *name, cats_ggml_model *m)
{
	int fd = open(name, O_RDONLY);
	if (fd == -1) {
		printf("Failed to open file: %s\n", name);
		return 1;
	}

	struct gguf_header header;
	read(fd, &header, sizeof(header));
	printf("magic:%x\n", header.magic);
	printf("version:%d\n", header.version);
	printf("n_tensors:%d\n", header.n_tensors);
	printf("n_kv:%d\n", header.n_kv);

	if (header.magic != 0x46554747) {
		printf("Invalid GGUF file.\n");
		return 1;
	}
	if (header.version != 2) {
		printf("Unsupported GGUF version.\n");
		return 1;
	}

	struct gguf_kv* kv = malloc(header.n_kv * sizeof(struct gguf_kv));
	for (uint64_t i=0; i<header.n_kv; i++) {
		uint64_t len;
		read(fd, &len, sizeof(uint64_t));
		read(fd, kv[i].key, len);
		read(fd, &kv[i].type, sizeof(uint32_t));
		kv[i].key[len] = 0;
		printf("key #%d: %s (%d)\n", i, kv[i].key, kv[i].type);

		switch (kv[i].type) {
		case GGUF_TYPE_STRING:
			read(fd, &len, sizeof(uint64_t));
			read(fd, &kv[i].value, len);
			printf("string: %s(%d)\n", kv[i].value.str, len);
			break;
		case GGUF_TYPE_ARRAY:
			read(fd, &kv[i].value.arr.type, sizeof(uint32_t));
			read(fd, &kv[i].value.arr.n, sizeof(uint64_t));
			printf("type: %d (%d)\n", kv[i].value.arr.type, kv[i].value.arr.n);

			if (!strcmp(kv[i].key, "tokenizer.ggml.tokens")) {
				m->n_vocab = kv[i].value.arr.n;
				m->vocab = (char**)malloc(m->n_vocab * sizeof(char*));
				for (int n=0; n<kv[i].value.arr.n; n++) {
					read(fd, &len, sizeof(uint64_t));
					char buff[256];
					read(fd, buff, len);
					buff[len] = 0;
					uint8_t byte_val;
					if (sscanf(buff, "<0x%02hhX>", &byte_val) == 1) {
						m->vocab[n] = (char*)malloc(2);
						m->vocab[n][0] = byte_val;
						m->vocab[n][1] = 0;
					} else {
						m->vocab[n] = (char*)malloc(len+1);
						strcpy(m->vocab[n], buff);
					}
				}
				continue;
			} else if (!strcmp(kv[i].key, "tokenizer.ggml.scores")) {
				m->n_vocab = kv[i].value.arr.n;
				m->score = (float*)malloc(m->n_vocab * sizeof(float));
				read(fd, m->score, gguf_value_size[kv[i].value.arr.type]*kv[i].value.arr.n);
				continue;
			}

			if (kv[i].value.arr.type==GGUF_TYPE_STRING) {
				for (int n=0; n<kv[i].value.arr.n; n++) {
					char d[256];
					read(fd, &len, sizeof(uint64_t));
					read(fd, d, len);
					d[len] = 0;
					if (n<5 || n>kv[i].value.arr.n-5) printf("%s ", d);
				}
				printf("\n");
			} else {
				for (int n=0; n<kv[i].value.arr.n; n++) {
					union gguf_value value;
					read(fd, &value, gguf_value_size[kv[i].value.arr.type]);
					//if (n<5 || n>kv[i].value.arr.n-5) printf("%d ", value);
				}
//				read(fd, d, gguf_value_size[kv[i].value.arr.type]*kv[i].value.arr.n);
//				printf("\n");
			}
			break;
		default:
			read(fd, &kv[i].value, gguf_value_size[kv[i].type]);
			printf("value: %d\n", kv[i].value.uint32);

			if (!strcmp(kv[i].key, "llama.context_length")) m->seq_len = kv[i].value.uint32;
			else if (!strcmp(kv[i].key, "llama.embedding_length")) m->n_embd = kv[i].value.uint32;
			else if (!strcmp(kv[i].key, "llama.block_count")) m->n_layer = kv[i].value.uint32;
			else if (!strcmp(kv[i].key, "llama.feed_forward_length")) m->n_hidden = kv[i].value.uint32;
			else if (!strcmp(kv[i].key, "llama.rope.dimension_count")) m->n_rot = kv[i].value.uint32;
			else if (!strcmp(kv[i].key, "llama.attention.head_count")) m->n_head = kv[i].value.uint32;
			else if (!strcmp(kv[i].key, "llama.attention.head_count_kv")) m->n_kv_head = kv[i].value.uint32;
			else if (!strcmp(kv[i].key, "general.file_type")) m->ftype = kv[i].value.uint32;
		}
	}
	if (m->n_layer > CATS_LLAMA_LAYER) printf("error: block_count(%d) over!", m->n_layer);

	cats_tensor* tensor = m->tensor;
	for (uint64_t i=0; i<header.n_tensors; i++) {
		uint64_t len;
		read(fd, &len, sizeof(uint64_t));
		read(fd, tensor[i].name, len);
		tensor[i].name[len] = 0;

		read(fd, &tensor[i].n_dim, sizeof(uint32_t));
		read(fd, tensor[i].dim, sizeof(uint64_t)*tensor[i].n_dim);
		read(fd, &tensor[i].type, sizeof(uint32_t));
		read(fd, &tensor[i].offset, sizeof(uint64_t));

		tensor[i].size = tensor[i].dim[0];
		for (int n=1; n<tensor[i].n_dim; n++) if (tensor[i].dim[n]) tensor[i].size *= tensor[i].dim[n];
		switch (tensor[i].type) {
		case GGML_TYPE_F32:
			tensor[i].size *= sizeof(float);
			break;
		case GGML_TYPE_Q4_0: // 2
			tensor[i].size = (tensor[i].size * 18) / 32;
			break;
		case GGML_TYPE_Q4_1:
			tensor[i].size = (tensor[i].size * 24) / 32;
			break;
		case GGML_TYPE_Q3_K: // 11
			tensor[i].size = (tensor[i].size * 110) / 256;
			break;
//		case GGML_TYPE_Q4_K: // 12
		case GGML_TYPE_Q6_K: // 14
			tensor[i].size = (tensor[i].size * ((256/2)/*lo4bit*/+(256/4)/*hi2bit*/+(256/16)/*scale*/+2/*global scale*/)) / 256;
			break;
		default:
			printf("%d: Not support %d\n", i, tensor[i].type);
		}

		printf("%d: %ld %s(%d): %d [%d,%d]\n", i, tensor[i].offset, tensor[i].name, tensor[i].type, tensor[i].size, tensor[i].dim[0], tensor[i].dim[1]);
	}

//	size_t alignment, offset, size;
//	read(fd, &alignment, sizeof(size_t));
//	read(fd, &offset, sizeof(size_t));
//	read(fd, &size, sizeof(size_t));
//	printf("alignment: %d\n", alignment);
//	printf("offset: %ld\n", offset);
//	printf("size: %x\n", size);
	lseek(fd, (lseek(fd, 0, SEEK_CUR)+31)&-32, SEEK_SET); // general.alignment: 32

	size_t off = lseek(fd, 0, SEEK_CUR);
	for (int i=0; i<header.n_tensors; i++) {
		lseek(fd, off +m->tensor[i].offset, SEEK_SET);
//		printf("%d(%d)\n", i, m->tensor[i].type);

		m->tensor[i].quantize = gguf_quantize[m->tensor[i].type];
		m->tensor[i].matmul = gguf_matmul[m->tensor[i].type];
		m->tensor[i].data = malloc(m->tensor[i].size);
		if (!m->tensor[i].data) printf("malloc err\n");
		read(fd, m->tensor[i].data, m->tensor[i].size);

		if (!strcmp(m->tensor[i].name, "token_embd.weight")) {
			m->token_embedding_table = &m->tensor[i];
			dequantize = gguf_dequantize[m->tensor[i].type];
		} else if (!strcmp(m->tensor[i].name, "output_norm.weight")) {
			m->rms_final_weight = &m->tensor[i];
//			for (int n=0; n<m->tensor[i].size/sizeof(float); n++) printf("%f ", ((float*)m->tensor[i].data)[n]);
		} else if (!strcmp(m->tensor[i].name, "output.weight")) { // q6
			m->wcls = &m->tensor[i];
		} else {
			char *s = m->tensor[i].name +4; // 'blk.'
			int l = atoi(s);
//			printf("layer %d: %s\n", l, s);

			s += 2;
			if (l>9) s++;

			if (!strcmp(s, "attn_q.weight")) {
				m->wq[l] = &m->tensor[i];
			} else if (!strcmp(s, "attn_k.weight")) {
				m->wk[l] = &m->tensor[i];
			} else if (!strcmp(s, "attn_v.weight")) {
				m->wv[l] = &m->tensor[i];
			} else if (!strcmp(s, "attn_output.weight")) {
				m->wo[l] = &m->tensor[i];
			} else if (!strcmp(s, "attn_norm.weight")) {
				m->rms_att_weight[l] = &m->tensor[i];
			} else if (!strcmp(s, "ffn_gate.weight")) {
				m->w1[l] = &m->tensor[i];
			} else if (!strcmp(s, "ffn_down.weight")) {
				m->w2[l] = &m->tensor[i];
			} else if (!strcmp(s, "ffn_up.weight")) {
				m->w3[l] = &m->tensor[i];
			} else if (!strcmp(s, "ffn_norm.weight")) {
				m->rms_ffn_weight[l] = &m->tensor[i];
//				for (int n=0; n<m->tensor[i].size/sizeof(float); n++) printf("%f ", ((float*)m->tensor[i].data)[n]);
			} else {
				printf("Not support %d: %s(%s)\n", l, m->tensor[i].name, s);
			}
		}
		putchar('.');
		fflush(stdout);
	}
	putchar('\n');

//	dequantize = _dequantize_q4_0;
	quantize = quantize_q8;
	matmul = matmul_q4_0_q8_avx;

	close(fd);
}

//---------------------------------------------------------
// bin format

static int cats_checkpoint_load(char *checkpoint, cats_ggml_model *m)
{
	// memory map the Transformer weights into the data pointer
	int fd = open(checkpoint, O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "open failed!\n");
		return 0;
	}
	/*for (int i=0; i<100; i++) printf("%f,%f ", w->freq_cis_real[i], w->freq_cis_imag[i]);
	printf("\n");
	precompute_freqs_cis(p->dim / p->n_heads, p->seq_len, 10000.0, w->freq_cis_real, w->freq_cis_imag);
	for (int i=0; i<100; i++) printf("%f,%f ", w->freq_cis_real[i], w->freq_cis_imag[i]);
	printf("\n");*/

	read(fd, &m->n_embd, sizeof(m->n_embd));
	read(fd, &m->n_hidden, sizeof(m->n_hidden));
	read(fd, &m->n_layer, sizeof(m->n_layer));
	read(fd, &m->n_head, sizeof(m->n_head));
	read(fd, &m->n_kv_head, sizeof(m->n_kv_head));
	read(fd, &m->n_vocab, sizeof(m->n_vocab));
	read(fd, &m->seq_len, sizeof(m->seq_len));
	int shared_weights = m->n_vocab > 0 ? 1 : 0;
	m->n_vocab = abs(m->n_vocab);

	// for cats_ggml_model
	int head_size = m->n_embd / m->n_head;
	m->token_embedding_table = &m->tensor[0];
	m->rms_final_weight = &m->tensor[1];
	m->wcls = &m->tensor[2];
	m->token_embedding_table->data = malloc(sizeof(real) * m->n_vocab * m->n_embd);
	m->rms_final_weight->data = malloc(sizeof(real) * m->n_embd);
	m->wcls->data = shared_weights ? m->token_embedding_table->data : malloc(sizeof(real) * m->n_vocab * m->n_embd);
	for (int i=0; i<m->n_layer; i++) {
		m->rms_att_weight[i] = &m->tensor[3+i*9];
		m->rms_att_weight[i]->data = malloc(sizeof(real) * m->n_embd);
		m->wq[i] = &m->tensor[4+i*9];
		m->wq[i]->data = malloc(sizeof(real) * m->n_embd * (m->n_head * head_size));
		m->wk[i] = &m->tensor[5+i*9];
		m->wk[i]->data = malloc(sizeof(real) * m->n_embd * (m->n_kv_head * head_size));
		m->wv[i] = &m->tensor[6+i*9];
		m->wv[i]->data = malloc(sizeof(real) * m->n_embd * (m->n_kv_head * head_size));
		m->wo[i] = &m->tensor[7+i*9];
		m->wo[i]->data = malloc(sizeof(real) * (m->n_head * head_size) * m->n_embd);
		m->rms_ffn_weight[i] = &m->tensor[8+i*9];
		m->rms_ffn_weight[i]->data = malloc(sizeof(real) * m->n_embd);
		m->w1[i] = &m->tensor[9+i*9];
		m->w1[i]->data = malloc(sizeof(real) * m->n_embd * m->n_hidden);
		m->w2[i] = &m->tensor[10+i*9];
		m->w2[i]->data = malloc(sizeof(real) * m->n_hidden * m->n_embd);
		m->w3[i] = &m->tensor[11+i*9];
		m->w3[i]->data = malloc(sizeof(real) * m->n_embd * m->n_hidden);
	}

	read(fd, m->token_embedding_table->data, sizeof(real) * m->n_vocab * m->n_embd);
	for (int l=0; l<m->n_layer; l++) {
		read(fd, m->rms_att_weight[l]->data, sizeof(real) * m->n_embd);
	}
	for (int l=0; l<m->n_layer; l++) {
		read(fd, m->wq[l]->data, sizeof(real) * m->n_embd * (m->n_head * head_size));
	}
	for (int l=0; l<m->n_layer; l++) {
		read(fd, m->wk[l]->data, sizeof(real) * m->n_embd * (m->n_kv_head * head_size));
	}
	for (int l=0; l<m->n_layer; l++) {
		read(fd, m->wv[l]->data, sizeof(real) * m->n_embd * (m->n_kv_head * head_size));
	}
	for (int l=0; l<m->n_layer; l++) {
		read(fd, m->wo[l]->data, sizeof(real) * (m->n_head * head_size) * m->n_embd);
	}
	for (int l=0; l<m->n_layer; l++) {
		read(fd, m->rms_ffn_weight[l]->data, sizeof(real) * m->n_embd);
	}
	for (int l=0; l<m->n_layer; l++) {
		read(fd, m->w1[l]->data, sizeof(real) * m->n_embd * m->n_hidden);
	}
	for (int l=0; l<m->n_layer; l++) {
		read(fd, m->w2[l]->data, sizeof(real) * m->n_hidden * m->n_embd);
	}
	for (int l=0; l<m->n_layer; l++) {
		read(fd, m->w3[l]->data, sizeof(real) * m->n_embd * m->n_hidden);
	}
	read(fd, m->rms_final_weight->data, sizeof(real) * m->n_embd);
	if (!shared_weights) read(fd, m->wcls->data, sizeof(real) * m->n_vocab * m->n_embd);

	quantize = quantize_null;
	matmul = matmul_avx;
	for (int i=0; i<3+9*m->n_layer; i++) {
		m->tensor[i].quantize = quantize_null;
		m->tensor[i].matmul = matmul_avx;
	}

	return fd;
}
