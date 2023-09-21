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

//---------------------------------------------------------
// Quantize

#define QK4_0 32

// FP16 <-> FP32
#ifdef __F16C__
#ifdef _MSC_VER
#define cats_compute_fp16_to_fp32(x) _mm_cvtss_f32(_mm_cvtph_ps(_mm_cvtsi32_si128(x)))
#define cats_compute_fp32_to_fp16(x) _mm_extract_epi16(_mm_cvtps_ph(_mm_set_ss(x), 0), 0)
#else
#define cats_compute_fp16_to_fp32(x) _cvtsh_ss(x)
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
static inline float cats_compute_fp16_to_fp32(uint16_t h)
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

#define CATS_NAME_MAX		48
typedef struct {
	char name[CATS_NAME_MAX];
	int type;
	int dim[2];
	void *data;

	void (*matmul)(void*, void*, void*, int, int);

	uint64_t size;		// size of the file
	size_t offset;		// offset of the file
} cats_tensor;

#define CATS_LLAMA_LAYER	32
typedef struct {
	// header: FIX!!
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

	cats_tensor *token_embedding_table;
	cats_tensor *rms_att_weight[CATS_LLAMA_LAYER];
	cats_tensor *rms_ffn_weight[CATS_LLAMA_LAYER];
	cats_tensor *wq[CATS_LLAMA_LAYER];
	cats_tensor *wk[CATS_LLAMA_LAYER];
	cats_tensor *wv[CATS_LLAMA_LAYER];
	cats_tensor *wo[CATS_LLAMA_LAYER];
	cats_tensor *w1[CATS_LLAMA_LAYER];
	cats_tensor *w2[CATS_LLAMA_LAYER];
	cats_tensor *w3[CATS_LLAMA_LAYER];
	cats_tensor *rms_final_weight;
	cats_tensor *wcls;

	// state
	real *x;		// activation at current time stamp (dim,)
	real *xb;		// same, but inside a residual branch (dim,)
	real *xb2;		// an additional buffer just for convenience (dim,)
	real *hb;		// buffer for hidden dimension in the ffn (hidden_dim,)
	real *hb2;		// buffer for hidden dimension in the ffn (hidden_dim,)
	real *q;		// query (dim,)
	real *k;		// key (dim,)
	real *v;		// value (dim,)
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
	m->k = calloc(kv_dim, sizeof(real));
	m->v = calloc(kv_dim, sizeof(real));
	m->att = calloc(m->n_head * m->seq_len, sizeof(real));
	m->logits = calloc(m->n_vocab, sizeof(real));
	m->key_cache = calloc(m->n_layer * m->seq_len * kv_dim, sizeof(real));
	m->value_cache = calloc(m->n_layer * m->seq_len * kv_dim, sizeof(real));
	// ensure all mallocs went fine
	if (!m->x || !m->xb || !m->xb2 || !m->hb || !m->hb2 || !m->q
		|| !m->k || !m->v || !m->att || !m->logits || !m->key_cache
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
	free(m->k);
	free(m->v);
	free(m->att);
	free(m->logits);
	free(m->key_cache);
	free(m->value_cache);
}

//---------------------------------------------------------
// neural net blocks

// Root Mean Square Normalization
static void rmsnorm(real* o, real* x, real* weight, int size)
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

static void softmax(real* x, int size)
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
#if 1
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
/*static inline void matmul(real* o, const real* x, const real* w, int n, int d)
{
	sgemm7_(d, 1, n, w, x, o); // 500
//	sgemm_block_parallel(d, 1, n, w, x, o);
//	cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, d, 1, n, 1.0, w, n, x, 1, 0.0, o, 1); // slow
//	cblas_sgemv(CblasRowMajor, CblasNoTrans, d, n, 1.0, w, n, x, 1, 0.0, o, 1);
}*/
static inline real dot_q4_0(uint8_t *a, real *b, int n)
{
	int nn = n>>5;
	static __attribute__((aligned(32))) real w[32];
	real val = 0.0;
	for (int i=0; i<nn; i++) {
		uint8_t *p = a +i*18;
		real delta = cats_compute_fp16_to_fp32(*((uint16_t*)p));
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
// horizontally add 8 floats
static inline float hsum_float_8(const __m256 x)
{
	__m128 res = _mm256_extractf128_ps(x, 1);
	res = _mm_add_ps(res, _mm256_castps256_ps128(x)); // ( x3+x7, x2+x6, x1+x5, x0+x4 )
	res = _mm_add_ps(res, _mm_movehl_ps(res, res)); // ( -, -, x1+x3+x5+x7, x0+x2+x4+x6 )
	res = _mm_add_ss(res, _mm_movehdup_ps(res)); // ( -, -, -, x0+x1+x2+x3+x4+x5+x6+x7 )
	return _mm_cvtss_f32(res);
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
		const __m256 d = _mm256_set1_ps(cats_compute_fp16_to_fp32(*((uint16_t*)p)));
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
//	return hsum_float_8(acc);

	acc = _mm256_hadd_ps(acc, acc);
	acc = _mm256_hadd_ps(acc, acc);
	static __attribute__((aligned(32))) float vals[8];
	_mm256_store_ps(vals, acc);
	return vals[0] + vals[4];
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
/*	acc = _mm256_hadd_ps(acc, acc);
	acc = _mm256_hadd_ps(acc, acc);
	float val = _mm_cvtss_f32(_mm_add_ps(_mm256_extractf128_ps(acc, 1), _mm256_castps256_ps128(acc)));*/
	float val = hsum_float_8(acc);
	for (; i<n; ++i) val += a[i] * b[i];
	return val;
}
static inline void matmul(real* __restrict xout, real* __restrict x, real* __restrict w, int n, int d)
{
	// W (d,n) @ x (n,) -> xout (d,)
	omp_set_num_threads(8);
	#pragma omp parallel for
	for (int i=0; i<d; i++) {
//		xout[i] = sdot_avx(w+i*n, x, n); // 580
//		xout[i] = dot_q4_0(((uint8_t*)w)+i*n/32*18, x, n);
		xout[i] = dot_avx_q4_0(((uint8_t*)w)+i*n/32*18, x, n);
	}
}
#else // 496
// matmul is a matrix multiplication function that computes the product of a matrix (w) and
// a vector (x) and stores the result in the output vector (o).
// The function takes the dimensions (n, d) of the matrix as input.
// This function is used for transforming input data in the application
static inline void matmul(real* o, const real* x, const real* w, int n, int d)
{
	// W (d,n) @ x (n,) -> o (d,)
	int nn = n&~(8-1); // ensure n is a multiple of 8
	#pragma omp parallel for
	for (int i=0; i<d; i++) {
		__m256 sum_vec = _mm256_setzero_ps(); // for AVX2, sum of 8 floats
		int i_n = i * n;
		for (int j=0; j<nn; j+=8) {
			// Load 8 values from w and x
//			printf("w:%x,%x\n",w,(uint64_t)(w)>>5<<5);
//			printf("x:%x,%x\n",x,(uint64_t)(x)>>5<<5);
			__m256 w_vec = _mm256_load_ps(&w[i_n + j]);
			__m256 x_vec = _mm256_load_ps(&x[j]);
//			__m256 w_vec = _mm256_loadu_ps(&w[i_n + j]);
//			__m256 x_vec = _mm256_loadu_ps(&x[j]);

			// Multiply and accumulate
			sum_vec = _mm256_add_ps(sum_vec, _mm256_mul_ps(w_vec, x_vec));
		}

		// Perform horizontal add
		sum_vec = _mm256_hadd_ps(sum_vec, sum_vec);
		sum_vec = _mm256_hadd_ps(sum_vec, sum_vec);
		static __attribute__((aligned(32))) real vals[8];
		_mm256_store_ps(vals, sum_vec);
//		_mm256_storeu_ps(vals, sum_vec);
		float val = vals[0] + vals[4];

		// handle remainder if n is not a multiple of 8
		for (int j=nn; j<n; j++) {
			val += w[i_n + j] * x[j];
		}
		o[i] = val;
	}
}
#endif
/*void matmul_arm(float *xout, float *x, float *w, int n, int d)
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
#elif defined(__SSE__)
static inline float sdot_sse(const float *x, const float *y, unsigned n)
{
	unsigned i, n8 = n>>3<<3;
	__m128 vs1, vs2;
	float s;
	static __attribute__((aligned(16))) float t[4];
	vs1 = _mm_setzero_ps();
	vs2 = _mm_setzero_ps();
	for (i=0; i<n8; i+=8) {
		__m128 vx1, vx2, vy1, vy2;
		vx1 = _mm_loadu_ps(&x[i]);
		vx2 = _mm_loadu_ps(&x[i+4]);
		vy1 = _mm_loadu_ps(&y[i]);
		vy2 = _mm_loadu_ps(&y[i+4]);
		vs1 = _mm_add_ps(vs1, _mm_mul_ps(vx1, vy1));
		vs2 = _mm_add_ps(vs2, _mm_mul_ps(vx2, vy2));
	}
	for (s=0.0; i<n; ++i) s += x[i] * y[i];
	_mm_storeu_ps(t, vs1);
	s += t[0] + t[1] + t[2] + t[3];
	_mm_storeu_ps(t, vs2);
	s += t[0] + t[1] + t[2] + t[3];
	return s;
}
static inline void matmul(float* xout, float* x, float* w, int n, int d)
{
	// W (d,n) @ x (n,) -> xout (d,)
	int i;
	#pragma omp parallel for private(i)
	for (i=0; i<d; i++) {
		xout[i] = sdot_sse(&w[i*n], x, n);
	}
}
#else
static inline void matmul(real* xout, real* x, real* w, int n, int d)
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
	real sqrt_head_size = 1.0 / sqrtf(head_size);

	// Convert the token to a real vector (int -> vector)
	// copy the token embedding into x
	memcpy(x, m->token_embedding_table->data +token * dim *sizeof(real), dim*sizeof(real));

	// pluck out the "pos" row of freq_cis_real and freq_cis_imag
//	real* freq_cis_real_row = w->freq_cis_real + pos * head_size / 2;
//	real* freq_cis_imag_row = w->freq_cis_imag + pos * head_size / 2;

	// forward all the layers
	for (int l=0; l<m->n_layer; l++) {
		// attention rmsnorm
		rmsnorm(m->xb, x, m->rms_att_weight[l]->data, dim); // S = T = norm x -> mul weight

		// qkv matmuls for this position
		matmul(m->q, m->xb, m->wq[l]->data, dim, dim); // Q = TWq
		matmul(m->k, m->xb, m->wk[l]->data, dim, kv_dim); // K = SWk
		matmul(m->v, m->xb, m->wv[l]->data, dim, kv_dim); // V = Swv

		// Positional Encoding: x + PE
		//   PE(pos,2i)   := sin(pos/10000^2*i/d)
		//   PE(pos,2i+1) := cos(pos/10000^2*i/d)
		// RoPE relative positional encoding: complex-valued rotate q and k by freq_cis in each head
		/*for (int i=0; i<dim; i+=2) {
			real q0 = s->q[i];
			real q1 = s->q[i+1];
			real fcr = freq_cis_real_row[(i % head_size) / 2];
			real fci = freq_cis_imag_row[(i % head_size) / 2];
			s->q[i]   = q0 * fcr - q1 * fci;
			s->q[i+1] = q0 * fci + q1 * fcr;
		}
		for (int i=0; i<kv_dim; i+=2) {
			real k0 = s->k[i];
			real k1 = s->k[i+1];
			real fcr = freq_cis_real_row[(i % head_size) / 2];
			real fci = freq_cis_imag_row[(i % head_size) / 2];
			s->k[i]   = k0 * fcr - k1 * fci;
			s->k[i+1] = k0 * fci + k1 * fcr;
		}*/
		int i;
		#pragma omp parallel for private(i)
		for (int i=0; i<dim; i+=2) {
			int head_dim = i % head_size;
			real freq = 1.0f / powf(10000.0f, head_dim / (real)head_size);
			real val = pos * freq;
			real fcr = cosf(val);
			real fci = sinf(val);
			// rotate q
			real q0  = m->q[i];
			real q1  = m->q[i+1];
			m->q[i]   = q0 * fcr - q1 * fci;
			m->q[i+1] = q0 * fci + q1 * fcr;
			// rotate k
			if (i < kv_dim) {
				real k0  = m->k[i];
				real k1  = m->k[i+1];
				m->k[i]   = k0 * fcr - k1 * fci;
				m->k[i+1] = k0 * fci + k1 * fcr;
			}
		}

		// save key,value at this time step (pos) to our kv cache
		int loff = l * m->seq_len * kv_dim; // kv cache layer offset for convenience
		real* key_cache_row = m->key_cache + loff + pos * kv_dim;
		real* value_cache_row = m->value_cache + loff + pos * kv_dim;
		memcpy(key_cache_row, m->k, kv_dim*sizeof(*key_cache_row));
		memcpy(value_cache_row, m->v, kv_dim*sizeof(*value_cache_row));

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
			for (int t=0; t<=pos; t++) { // Mask?
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
		matmul(m->xb2, m->xb, m->wo[l]->data, dim, dim);

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
		matmul(m->hb, m->xb, m->w1[l]->data, dim, hidden_dim);
		matmul(m->hb2, m->xb, m->w3[l]->data, dim, hidden_dim);
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
		matmul(m->xb, m->hb, m->w2[l]->data, hidden_dim, dim);

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
	matmul(m->logits, x, m->wcls->data, dim, m->n_vocab);
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
//	real coin = (real)rand() / (real)RAND_MAX;
//	real coin = random_f32();
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
/*	// encode the (string) prompt into tokens sequence
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
	free(probindex);*/

	// process the prompt, if any
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

	free(probindex);
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

static void dequantize(int fd, int type, uint64_t size, float *data, int f)
{
	if (f) {
		read(fd, data, size);
		return;
	}
	switch (type) {
//	case LLAMA_FTYPE_MOSTLY_F16:
	case LLAMA_FTYPE_ALL_F32:
		read(fd, data, size);
		break;
	case LLAMA_FTYPE_MOSTLY_Q4_0:
		uint16_t d;
		for (int i=0; i<size/18; i++) {
			read(fd, &d, sizeof(uint16_t));
			float delta = cats_compute_fp16_to_fp32(d);
			static uint8_t qs[QK4_0/2];
			read(fd, qs, QK4_0/2);

			float *y0 = data+i*32;
			float *y1 = data+i*32 +QK4_0/2;
			uint8_t *p = qs;
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
			*y1++ = ((*p   >>   4) - 8)*delta;
		}
		break;
	default:
		printf("Not support: %d\n", type);
	}
	return;
}

/*def precompute_freqs_cis(dim: int, end: int, theta: float = 10000.0):
    freqs = 1.0 / (theta ** (torch.arange(0, dim, 2)[: (dim // 2)].float() / dim))
    t = torch.arange(end, device=freqs.device)  # type: ignore
    freqs = torch.outer(t, freqs).float()  # type: ignore
    freqs_cos = torch.cos(freqs)  # real part
    freqs_sin = torch.sin(freqs)  # imaginary part
    return freqs_cos, freqs_sin*/
//freqs_cos, freqs_sin = precompute_freqs_cis(p['dim'] // p['n_heads'], p['max_seq_len'] * 2)
/*void precompute_freqs_cis(int dim, int end, float theta, float *freqs_cos, float *freqs_sin)
{
	// Compute the frequencies: cos(theta) + i sin(theta)
	float *c = freqs_cos;
	float *s = freqs_sin;
	for (int i=0; i<end; i++) {
		for (int j=0; j<dim/2; j++) {
			// 10000^2*(i-1)/d
			float freq = i / (pow(theta, j*2.0 / dim));
			*c++ = cos(freq);
			*s++ = sin(freq);
		}
	}
}*/

static int cats_checkpoint_load(char *checkpoint, cats_ggml_model *m)
{
/*	FILE *file = fopen(checkpoint, "rb");
	if (!file) {
		fprintf(stderr, "Couldn't open file %s\n", checkpoint);
		return 0;
	}
	// read in the config header
	if (fread(p, sizeof(cats_llama2_model), 1, file) != 1) {
		return 0;
	}
	// negative vocab size is hacky way of signaling unshared weights. bit yikes.
	int shared_weights = p->vocab_size > 0 ? 1 : 0;
	p->vocab_size = abs(p->vocab_size);
	// figure out the file size
	fseek(file, 0, SEEK_END); // move file pointer to end of file
	ssize_t file_size = ftell(file); // get the file size, in bytes
	fclose(file);*/

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

	return fd;
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
	printf("error at cats_ggml_getsize\n");
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
		return cats_checkpoint_load(name, m);
//		fprintf(stderr, "unknown (magic, version) combination: %08x\n", magic);
//		return -1;
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
//			m->token_embedding_table->data = w->token_embedding_table;
			dequantize(fd, m->tensor[i].type, m->tensor[i].size, m->tensor[i].data, 0); // q4 -> f32
//			for (i=0; i<500; i++) printf("%f ", w->token_embedding_table[i]);
		} else if (!strcmp(m->tensor[i].name, "norm.weight")) {
			m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_embd, 0));
			m->rms_final_weight = &m->tensor[i];
//			m->rms_final_weight->data = w->rms_final_weight;
			dequantize(fd, m->tensor[i].type, m->tensor[i].size, m->tensor[i].data, f);
//			for (i=0; i<m->tensor[i].size; i++) printf("%f ", w->rms_final_weight[i]);
		} else if (!strcmp(m->tensor[i].name, "output.weight")) {
			m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_vocab, m->n_embd));
			m->wcls = &m->tensor[i];
//			m->wcls->data = w->wcls;
			dequantize(fd, m->tensor[i].type, m->tensor[i].size, m->tensor[i].data, f);
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
//				m->wq[l]->data = w->wq +(l * m->n_embd * m->n_embd);
				dequantize(fd, m->tensor[i].type, m->tensor[i].size, m->tensor[i].data, f);
//				for (i=0; i<500; i++) printf("%f ", w->wq[i]);
			} else if (!strcmp(s, "attention.wk.weight")) {
				m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_embd, m->n_embd));
				m->wk[l] = &m->tensor[i];
//				m->wk[l]->data = w->wk +(l * m->n_embd * m->n_embd);
				dequantize(fd, m->tensor[i].type, m->tensor[i].size, m->tensor[i].data, f);
//				for (i=0; i<500; i++) printf("%f ", w->wk[i]);
			} else if (!strcmp(s, "attention.wv.weight")) {
				m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_embd, m->n_embd));
				m->wv[l] = &m->tensor[i];
//				m->wv[l]->data = w->wv +(l * m->n_embd * m->n_embd);
				dequantize(fd, m->tensor[i].type, m->tensor[i].size, m->tensor[i].data, f);
//				for (i=0; i<500; i++) printf("%f ", w->wv[i]);
			} else if (!strcmp(s, "attention.wo.weight")) {
				m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_embd, m->n_embd));
				m->wo[l] = &m->tensor[i];
//				m->wo[l]->data = w->wo +(l * m->n_embd * m->n_embd);
				dequantize(fd, m->tensor[i].type, m->tensor[i].size, m->tensor[i].data, f);
//				for (i=0; i<500; i++) printf("%f ", w->wo[i]);
			} else if (!strcmp(s, "attention_norm.weight")) {
				m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_embd, 0));
				m->rms_att_weight[l] = &m->tensor[i];
//				m->rms_att_weight[l]->data = w->rms_att_weight +(l * m->n_embd);
				dequantize(fd, m->tensor[i].type, m->tensor[i].size, m->tensor[i].data, f);
//				for (i=0; i<500; i++) printf("%f ", w->rms_att_weight[i]);
			} else if (!strcmp(s, "feed_forward.w1.weight")) {
				m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_embd, m->n_hidden));
				m->w1[l] = &m->tensor[i];
//				m->w1[l]->data = w->w1 +(l * m->n_embd * m->n_hidden);
				dequantize(fd, m->tensor[i].type, m->tensor[i].size, m->tensor[i].data, f);
//				for (i=0; i<500; i++) printf("%f ", w->w1[i]);
			} else if (!strcmp(s, "feed_forward.w2.weight")) {
				m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_hidden, m->n_embd));
				m->w2[l] = &m->tensor[i];
//				m->w2[l]->data = w->w2 +(l * m->n_hidden * m->n_embd);
				dequantize(fd, m->tensor[i].type, m->tensor[i].size, m->tensor[i].data, f);
//				for (i=0; i<500; i++) printf("%f ", w->w2[i]);
			} else if (!strcmp(s, "feed_forward.w3.weight")) {
				m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_embd, m->n_hidden));
				m->w3[l] = &m->tensor[i];
//				m->w3[l]->data = w->w3 +(l * m->n_embd * m->n_hidden);
				dequantize(fd, m->tensor[i].type, m->tensor[i].size, m->tensor[i].data, f);
//				for (i=0; i<500; i++) printf("%f ", w->w3[i]);
			} else if (!strcmp(s, "ffn_norm.weight")) {
				m->tensor[i].data = malloc(cats_gsize(m->tensor[i].type, m->n_embd, 0));
				m->rms_ffn_weight[l] = &m->tensor[i];
//				m->rms_ffn_weight[l]->data = w->rms_ffn_weight +(l * m->n_embd);
				dequantize(fd, m->tensor[i].type, m->tensor[i].size, m->tensor[i].data, f);
//				for (i=0; i<500; i++) printf("%f ", w->rms_ffn_weight[i]);
			} else {
				printf("Not support %d: %s(%s)\n", l, m->tensor[i].name, s);
			}
		}
		putchar('.');
		fflush(stdout);
	}
	putchar('\n');

	close(fd);
	return 0;
}
