/* public domain Simple, Minimalistic, Fast GEMM library
 *	©2018 Yuichiro Nakada
 *
 * Basic usage:
 *	sgemm_sse('R', 'N', 'N', m_col_a, n_row_b, k_row_a_col_b, alpha, a, col_a, b, col_b, beta, c, col_c);
 * */

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#define MC  384
#define KC  384
#define NC  4096

#define MR  8
#define NR  8

//  Local buffers for storing panels from A, B and C
static float _a[MC*KC] __attribute__ ((aligned (16)));
static float _b[KC*NC] __attribute__ ((aligned (16)));
static float _c[MR*NR] __attribute__ ((aligned (16)));

//  Packing complete panels from A (i.e. without padding)
static inline void _pack_MRxk(int k, const float *A, int incRowA, int incColA, float *buffer)
{
	int i, j;

	for (j=0; j<k; ++j) {
		for (i=0; i<MR; ++i) {
			buffer[i] = A[i*incRowA];
		}
		buffer += MR;
		A      += incColA;
	}
}

//  Packing panels from A with padding if required
static inline void _pack_A(int mc, int kc, const float *A, int incRowA, int incColA, float *buffer)
{
	int mp  = mc / MR;
	int _mr = mc % MR;

	int i, j;

	for (i=0; i<mp; ++i) {
		_pack_MRxk(kc, A, incRowA, incColA, buffer);
		buffer += kc*MR;
		A      += MR*incRowA;
	}
	if (_mr>0) {
		for (j=0; j<kc; ++j) {
			for (i=0; i<_mr; ++i) {
				buffer[i] = A[i*incRowA];
			}
			for (i=_mr; i<MR; ++i) {
				buffer[i] = 0.0;
			}
			buffer += MR;
			A      += incColA;
		}
	}
}

//  Packing complete panels from B (i.e. without padding)
static inline void _pack_kxNR(int k, const float *B, int incRowB, int incColB, float *buffer)
{
	int i, j;

	for (i=0; i<k; ++i) {
		for (j=0; j<NR; ++j) {
			buffer[j] = B[j*incColB];
		}
		buffer += NR;
		B      += incRowB;
	}
}

//  Packing panels from B with padding if required
static inline void _pack_B(int kc, int nc, const float *B, int incRowB, int incColB, float *buffer)
{
	int np  = nc / NR;
	int _nr = nc % NR;

	int i, j;

	for (j=0; j<np; ++j) {
		_pack_kxNR(kc, B, incRowB, incColB, buffer);
		buffer += kc*NR;
		B      += NR*incColB;
	}
	if (_nr>0) {
		for (i=0; i<kc; ++i) {
			for (j=0; j<_nr; ++j) {
				buffer[j] = B[j*incColB];
			}
			for (j=_nr; j<NR; ++j) {
				buffer[j] = 0.0;
			}
			buffer += NR;
			B      += incRowB;
		}
	}
}

static inline void dot8x8_avx(const float *a, const float *b, float *c, const int kc)
{
	/*register*/ __m256 a0, b0, b1, b2, b3, b4, b5, b6, b7;
	register __m256 c0, c1, c2, c3, c4, c5, c6, c7;

	__asm__ volatile( "prefetcht2 0(%0)          \n\t" : :"r"(b) );
	__asm__ volatile( "prefetcht0 0(%0)          \n\t" : :"r"(a) );
	c0 = _mm256_setzero_ps();
	c1 = _mm256_setzero_ps();
	c2 = _mm256_setzero_ps();
	c3 = _mm256_setzero_ps();
	c4 = _mm256_setzero_ps();
	c5 = _mm256_setzero_ps();
	c6 = _mm256_setzero_ps();
	c7 = _mm256_setzero_ps();

//	#pragma unroll(KC)
	for (int i=0; i<kc; i++) {
		b0 = _mm256_broadcast_ss(b);
		b1 = _mm256_broadcast_ss(b+1);
		b2 = _mm256_broadcast_ss(b+2);
		b3 = _mm256_broadcast_ss(b+3);
		b4 = _mm256_broadcast_ss(b+4);
		b5 = _mm256_broadcast_ss(b+5);
		b6 = _mm256_broadcast_ss(b+6);
		b7 = _mm256_broadcast_ss(b+7);
//#ifdef CATSEYE_ALIGNED
		a0 = _mm256_load_ps(a);
//#else
//		a0 = _mm256_loadu_ps(a);
//#endif

		a += 8;
		b += 8;
		__asm__ volatile( "prefetcht2 0(%0)          \n\t" : :"r"(b) );
		__asm__ volatile( "prefetcht0 0(%0)          \n\t" : :"r"(a) );

		c0 = _mm256_add_ps(c0, _mm256_mul_ps(a0, b0));
		c1 = _mm256_add_ps(c1, _mm256_mul_ps(a0, b1));
		c2 = _mm256_add_ps(c2, _mm256_mul_ps(a0, b2));
		c3 = _mm256_add_ps(c3, _mm256_mul_ps(a0, b3));
		c4 = _mm256_add_ps(c4, _mm256_mul_ps(a0, b4));
		c5 = _mm256_add_ps(c5, _mm256_mul_ps(a0, b5));
		c6 = _mm256_add_ps(c6, _mm256_mul_ps(a0, b6));
		c7 = _mm256_add_ps(c7, _mm256_mul_ps(a0, b7));
	}

//#ifdef CATSEYE_ALIGNED
	_mm256_store_ps(c, c0);
	_mm256_store_ps(c+8, c1);
	_mm256_store_ps(c+16, c2);
	_mm256_store_ps(c+24, c3);
	_mm256_store_ps(c+32, c4);
	_mm256_store_ps(c+40, c5);
	_mm256_store_ps(c+48, c6);
	_mm256_store_ps(c+56, c7);
/*#else
	_mm256_storeu_ps(c, c0);
	_mm256_storeu_ps(c+8, c1);
	_mm256_storeu_ps(c+16, c2);
	_mm256_storeu_ps(c+24, c3);
	_mm256_storeu_ps(c+32, c4);
	_mm256_storeu_ps(c+40, c5);
	_mm256_storeu_ps(c+48, c6);
	_mm256_storeu_ps(c+56, c7);
#endif*/
}
//  Micro kernel for multiplying panels from A and B.
static inline void _sgemm_micro_kernel(
	long kc,
	float alpha, const float *A, const float *B,
	float beta,
	float *C, long incRowC, long incColC)
{
	static float AB[MR*NR] __attribute__ ((aligned (16)));
	int i, j;

	//  Compute AB = A*B
	dot8x8_avx(A, B, AB, kc);
#if 0
	memset(AB, 0, MR*NR*sizeof(float));
	for (int l=0; l<kc; ++l) {
		dot8x8_avx(A, B, AB);
		/*for (j=0; j<NR; ++j) {
			for (i=0; i<MR; ++i) {
				AB[i+j*MR] += A[i]*B[j];
			}
		}*/
		A += MR;
		B += NR;
	}
#endif

	//  Update C <- beta*C
	if (beta==0.0) {
		for (j=0; j<NR; ++j) {
			for (i=0; i<MR; ++i) {
				C[i*incRowC+j*incColC] = 0.0;
			}
		}
	} else if (beta!=1.0) {
		for (j=0; j<NR; ++j) {
			for (i=0; i<MR; ++i) {
				C[i*incRowC+j*incColC] *= beta;
			}
		}
	}

	//  Update C <- C + alpha*AB (note: the case alpha==0.0 was already treated in
	//                                  the above layer sgemm_nn)
	if (alpha==1.0) {
		for (j=0; j<NR; ++j) {
			for (i=0; i<MR; ++i) {
				C[i*incRowC+j*incColC] += AB[i+j*MR];
			}
		}
	} else {
		for (j=0; j<NR; ++j) {
			for (i=0; i<MR; ++i) {
				C[i*incRowC+j*incColC] += alpha*AB[i+j*MR];
			}
		}
	}
}

//  Compute Y += alpha*X
static inline void sgeaxpy(
	int           m,
	int           n,
	float        alpha,
	const float  *X,
	int           incRowX,
	int           incColX,
	float        *Y,
	int           incRowY,
	int           incColY)
{
	int i, j;

	if (alpha!=1.0) {
		for (j=0; j<n; ++j) {
			for (i=0; i<m; ++i) {
				Y[i*incRowY+j*incColY] += alpha*X[i*incRowX+j*incColX];
			}
		}
	} else {
		for (j=0; j<n; ++j) {
			for (i=0; i<m; ++i) {
				Y[i*incRowY+j*incColY] += X[i*incRowX+j*incColX];
			}
		}
	}
}

//  Compute X *= alpha
static inline void sgescal(int m, int n, float alpha, float *X, int incRowX, int incColX)
{
	int i, j;

	if (alpha!=0.0) {
		for (j=0; j<n; ++j) {
			for (i=0; i<m; ++i) {
				X[i*incRowX+j*incColX] *= alpha;
			}
		}
	} else {
		for (j=0; j<n; ++j) {
			for (i=0; i<m; ++i) {
				X[i*incRowX+j*incColX] = 0.0;
			}
		}
	}
}

//  Macro Kernel for the multiplication of blocks of A and B.  We assume that
//  these blocks were previously packed to buffers _A and _B.
static inline void _sgemm_macro_kernel(
	int     mc,
	int     nc,
	int     kc,
	float  alpha,
	float  beta,
	float  *C,
	int     incRowC,
	int     incColC)
{
	int mp = (mc+MR-1) / MR;
	int np = (nc+NR-1) / NR;

	int _mr = mc % MR;
	int _nr = nc % NR;

	int mr, nr;
	int i, j;

	for (j=0; j<np; ++j) {
		nr    = (j!=np-1 || _nr==0) ? NR : _nr;

		for (i=0; i<mp; ++i) {
			mr    = (i!=mp-1 || _mr==0) ? MR : _mr;

			if (mr==MR && nr==NR) {
				_sgemm_micro_kernel(kc, alpha, &_a[i*kc*MR], &_b[j*kc*NR],
				                   beta, &C[i*MR*incRowC+j*NR*incColC], incRowC, incColC);
			} else {
				_sgemm_micro_kernel(kc, alpha, &_a[i*kc*MR], &_b[j*kc*NR], 0.0, _c, 1, MR);
				sgescal(mr, nr, beta, &C[i*MR*incRowC+j*NR*incColC], incRowC, incColC);
				sgeaxpy(mr, nr, 1.0, _c, 1, MR, &C[i*MR*incRowC+j*NR*incColC], incRowC, incColC);
			}
		}
	}
}

//  Compute C <- beta*C + alpha*A*B
static inline void sgemm_nn(
	int m, int n, int k, float alpha,
	const float *A, int incRowA, int incColA, const float *B, int incRowB, int incColB,
	float beta, float *C, int incRowC, int incColC)
{
	int mb = (m+MC-1) / MC;
	int nb = (n+NC-1) / NC;
	int kb = (k+KC-1) / KC;

	int _mc = m % MC;
	int _nc = n % NC;
	int _kc = k % KC;

	int mc, nc, kc;
	int i, j, l;

	float _beta;

	if (alpha==0.0 || k==0) {
		sgescal(m, n, beta, C, incRowC, incColC);
		return;
	}

	for (j=0; j<nb; ++j) {
		nc = (j!=nb-1 || _nc==0) ? NC : _nc;

		for (l=0; l<kb; ++l) {
			kc    = (l!=kb-1 || _kc==0) ? KC   : _kc;
			_beta = (l==0) ? beta : 1.0;

			_pack_B(kc, nc, &B[l*KC*incRowB+j*NC*incColB], incRowB, incColB, _b);

			for (i=0; i<mb; ++i) {
				mc = (i!=mb-1 || _mc==0) ? MC : _mc;

				_pack_A(mc, kc, &A[i*MC*incRowA+l*KC*incColA], incRowA, incColA, _a);

				_sgemm_macro_kernel(mc, nc, kc, alpha, _beta, &C[i*MC*incRowC+j*NC*incColC], incRowC, incColC);
			}
		}
	}
}

static void sgemm_sse(
	char		major,
	char		transA,
	char		transB,
	const int	m,
	const int	n,
	const int	k,
	const float	alpha,
	const float	*A,
	const int	ldA,
	const float	*B,
	const int	ldB,
	const float	beta,
	float		*C,
	const int	ldC)
{
	int i, j;

	//  Quick return if possible
	if (m==0 || n==0 || ((alpha==0.0 || k==0) && (beta==1.0))) {
		return;
	}

	//  And if alpha is exactly zero
	if (alpha==0.0) {
		if (beta==0.0) {
			for (j=0; j<n; j++) {
				for (i=0; i<m; i++) {
					C[i+j*ldC] = 0.0;
				}
			}
		} else {
			for (j=0; j<n; j++) {
				for (i=0; i<m; i++) {
					C[i+j*ldC] *= beta;
				}
			}
		}
		return;
	}

	//  Start the operations
	if (major == 'C') {
		if (transB=='N') {
			if (transA=='N') {
				// Form  C := alpha*A*B + beta*C
				sgemm_nn(m, n, k, alpha, A, 1, ldA, B, 1, ldB, beta, C, 1, ldC);
			} else {
				// Form  C := alpha*A**T*B + beta*C
				sgemm_nn(m, n, k, alpha, A, ldA, 1, B, 1, ldB, beta, C, 1, ldC);
			}
		} else {
			if (transA=='N') {
				// Form  C := alpha*A*B**T + beta*C
				sgemm_nn(m, n, k, alpha, A, 1, ldA, B, ldB, 1, beta, C, 1, ldC);
			} else {
				// Form  C := alpha*A**T*B**T + beta*C
				sgemm_nn(m, n, k, alpha, A, ldA, 1, B, ldB, 1, beta, C, 1, ldC);
			}
		}
	} else {
		if (transB=='N') {
			if (transA=='N') {
				// Form  C := alpha*A*B + beta*C
				sgemm_nn(m, n, k, alpha, A, ldA, 1, B, ldB, 1, beta, C, ldC, 1);
			} else {
				// Form  C := alpha*A**T*B + beta*C
				sgemm_nn(m, n, k, alpha, A, 1, ldA, B, ldB, 1, beta, C, ldC, 1);
			}
		} else {
			if (transA=='N') {
				// Form  C := alpha*A*B**T + beta*C
				sgemm_nn(m, n, k, alpha, A, ldA, 1, B, 1, ldB, beta, C, ldC, 1);
			} else {
				// Form  C := alpha*A**T*B**T + beta*C
				sgemm_nn(m, n, k, alpha, A, 1, ldA, B, 1, ldB, beta, C, ldC, 1);
			}
		}
	}
}

#undef MC
#undef KC
#undef NC

#undef MR
#undef NR
