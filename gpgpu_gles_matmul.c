// OMP_NUM_THREADS=8 gcc -o gpgpu_gles_matmul gpgpu_gles_matmul.c -fopenmp -Ofast -march=native -mavx -funroll-loops -lglfw -lGL
// emcc -o gpgpu_gles_matmul.html gpgpu_gles_matmul.c -Oz -sALLOW_MEMORY_GROWTH=1 -sALLOW_TABLE_GROWTH=1 -s USE_GLFW=3 -s USE_WEBGL2=1 -sMAX_WEBGL_VERSION=2 -sWEBGL2_BACKWARDS_COMPATIBILITY_EMULATION -sASSERTIONS

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

// from cblas.h
typedef enum CBLAS_ORDER     {CblasRowMajor=101, CblasColMajor=102} CBLAS_ORDER;
typedef enum CBLAS_TRANSPOSE {CblasNoTrans=111, CblasTrans=112, CblasConjTrans=113, CblasConjNoTrans=114} CBLAS_TRANSPOSE;
// end from cblas.h

static inline double now_ms()
{
	struct timespec res;
	clock_gettime(CLOCK_REALTIME, &res);
	return 1000.0 * res.tv_sec + (double)res.tv_nsec / 1e6;
}

#include <stdlib.h>
#define DEBUG
//#define GPGPU_USE_GLES
#include "gpgpu_gles.h"

char matmul_source[] = "#version 300 es\n" STRINGIFY(
precision highp float;
uniform highp sampler2D A;
uniform highp sampler2D B;
out vec4 result;
void main() {
  ivec2 A_size = textureSize(A, 0);
  int commonDim = A_size[0];
  int w = int(gl_FragCoord.x) * 4;
  vec4 sum0 = vec4(0.0);
  vec4 sum1 = vec4(0.0);
  vec4 sum2 = vec4(0.0);
  vec4 sum3 = vec4(0.0);
  for (int i=0; i<commonDim; ++i) {
    vec4 a = texelFetch(A, ivec2(i, 0), 0);
    vec4 b0 = texelFetch(B, ivec2(i, w), 0);
    vec4 b1 = texelFetch(B, ivec2(i, w+1), 0);
    vec4 b2 = texelFetch(B, ivec2(i, w+2), 0);
    vec4 b3 = texelFetch(B, ivec2(i, w+3), 0);
    sum0 += a * b0;
    sum1 += a * b1;
    sum2 += a * b2;
    sum3 += a * b3;
  }
  result = vec4(sum0.x+sum0.y+sum0.z+sum0.w, sum1.x+sum1.y+sum1.z+sum1.w, sum2.x+sum2.y+sum2.z+sum2.w, sum3.x+sum3.y+sum3.z+sum3.w);
}
);

GLuint gpu_prog_matmul;
static void matmul_gpu(float* __restrict xout, void* __restrict _x, void* __restrict _w, int n/*x4*/, int d/*x4*/)
{
	// W (d,n) @ x (n,) -> xout (d,)
	float* __restrict x = _x;
	float* __restrict w = _w;
	int nn = n/4;
	int dd = d/4;
	coBindVertices(gpu_prog_matmul);
	GLuint texture0 = coCreateDataTexture(nn, 1, x, GLES_FLOAT, GPGPU_TEX_PADDING);
	GLuint texture1 = coCreateDataTexture(nn, d, w, GLES_FLOAT, GPGPU_TEX_PADDING);
	GLuint texture2 = coCreateDataTexture(dd, 1, 0, GLES_FLOAT, 0);
	coBindInputTexture(gpu_prog_matmul, texture0, GL_TEXTURE0, "A");
	coBindInputTexture(gpu_prog_matmul, texture1, GL_TEXTURE1, "B");
	coBindOutputTexture(dd, 1, texture2);
#ifdef DEBUG
	double t0 = now_ms();
#endif
	coCompute();
#ifdef DEBUG
	double t1 = now_ms();
	double mtime = t1-t0;
	printf("  matmul_gpu elapsed time: %f ms, %f GFlops\n", mtime, (2*n*d)/(mtime*1e6));
#endif
	coReadDataf(dd, 1, xout);
}
static void matmul_gpu_init()
{
	coInit();
	gpu_prog_matmul = coCreateProgram(matmul_source);
}
static void matmul_gpu_term()
{
	coTerm();
}

static void gemm_gpu(const int M, const int N, const int K, const float *A, const float *B, float *C)
{
	matmul_gpu((float* __restrict)C, (void* __restrict)B, (void* __restrict)A, (int)K, (int)M);
}

static void sgemm_naive
(
	const int M,
	const int N,
	const int K,
	const float *A,                       // m x k (after transpose if TransA)
	const float *B,                       // k x n (after transpose if TransB)
	float *C                              // m x n
)
{
//	bool transpose_A = false;
//	bool transpose_B = false;
//	assert_sgemm_parameters(/*Order=*/CblasRowMajor, /*TransA=*/CblasNoTrans, /*TransB=*/CblasNoTrans, N, M, K, K, N, N, transpose_A, transpose_B);
	#pragma omp parallel for
	for (int m=0; m<M; ++m) {
		for (int n=0; n<N; ++n) {
			float sum = 0;
			for (int k=0; k<K; ++k) {
				// A is m x k
				// B is k x n
				sum += A[m*K + k] * B[n + k*N];
			}
			C[m*N + n] = sum;
		}
	}
}

// https://qiita.com/tomo0/items/a77e61c71ee92004679e
// http://laysakura.hateblo.jp/entry/20120106/1325881074
const int REPEAT = 2;
void run(int m, int n, int k, float *x, float *y, float *o, void (*gemm)(const int, const int, const int, const float*, const float*, float*), char *name)
{
	double t0 = now_ms();
	for (int i=0; i<REPEAT; i++) {
		gemm(m, n, k, x, y, o);
	}
	double t1 = now_ms();
	double mtime = t1-t0;
	printf("  %s elapsed time: %f ms, %f GFlops\n", name, mtime, ((float)REPEAT*2*m*n*k)/(mtime*1e6));
}

/*const int n = 960;
//const int n = 1920;
//const int n = 2880;
const int m = n;
const int k = n;*/

const int n = 1;
const int m = 11008;
const int k = 4096;
//const int m = 16;
//const int k = 4;
//const int m = 4;
//const int k = 16;
//const int k = 8;
//const int m = 4096;
//const int k = 11008;

int main(int argc, char** argv)
{
	float *data = malloc(sizeof(float)*(m*k +m*k/*xr*/ +k*n +m*n +m*n/*out2*/));
	float *x = data;
	float *xr = data +m*k;
	float *y = data +m*k +m*k;
	float *out1 = data +m*k +m*k +k*n;
	float *out2 = data +m*k +m*k +k*n +m*n;

	// Generate random data
	//srand((unsigned int)time(NULL));
	srand((unsigned int)0x100);
	for (int i=0; i<m; i++) {
		for(int j=0; j<k; j++) {
			x[i*k+j] = (float)(rand()%100) / 100.0;
//			x[i*k+j] = i*k+j;
//			xr[j*k+i] = x[i*k+j];
		}
	}
	for (int i=0; i<k; i++) {
		for(int j=0; j<n; j++) {
			y[i*n+j] = (float)(rand()%100) / 100.0;
//			y[i*n+j] = i*n+j;
		}
	}
	for (int i=0; i<m; i++){
		for(int j=0; j<n; j++) {
			out1[i*n+j] = 0.0;
			out2[i*n+j] = 0.0;
		}
	}
	printf("Computing %d x %d x %d...\n", m, n, k);

	matmul_gpu_init();
	run(m, n, k, x, y, out1, gemm_gpu, "gemm_gpu"); // only matmul
	matmul_gpu_term();

	run(m, n, k, x, y, out2, sgemm_naive, "Native GEMM");
//	cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, d, 1, n, 1.0, w, n, x, 1, 0.0, o, 1); // slow
//	cblas_sgemv(CblasRowMajor, CblasNoTrans, d, n, 1.0, w, n, x, 1, 0.0, o, 1);

	// Compare outputs
	printf("\nComputing diff...\n");
	float diff = 0.0;
	for (int i=0; i<m; i++) {
		for (int j=0; j<n; j++) {
			float u = (out1[i*n+j] - out2[i*n+j]);
			diff += u*u;
			if (i<10) printf(" (%.2f, %.2f)", out1[i*n+j], out2[i*n+j]);
		}
	}
	printf("  Norm Squared: %f\n", diff);
	int nZeros = 0;
	for (int i=0; i<m; i++) {
		for (int j=0; j<n; j++) {
			if (out1[i*n+j] == 0.0) { nZeros++; }
		}
	}
	printf("  Zeros: %d\n", nZeros);
}
