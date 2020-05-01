/* public domain Simple, Minimalistic, Fast GEMM library
 *	©2019-2020 Yuichiro Nakada
 *
 * Basic usage:
 *	sgemm_ocl_init(platform, device, max_buffer_size);
 *	sgemm_ocl('N', 'N', M, N, K, A, B, C);
 *	sgemm_ocl_finish();
 * */

#include "ocl.h"

char sgemm_kcode[] = OCLSTRINGIFY(

__kernel void gemm_nn(__global float* restrict gm, const int8 _info)
{
	const int M = _info.s0;
	const int N = _info.s1;
	const int K = _info.s2;
	__global float* restrict A = (__global float* restrict)(gm + _info.s3);
	__global float* restrict B = (__global float* restrict)(gm + _info.s4);
	__global float* restrict C = (__global float* restrict)(gm + _info.s5);

	// Thread identifiers
	const int globalRow = get_global_id(0); // Row ID of C (0..M)
	const int globalCol = get_global_id(1); // Col ID of C (0..N)

	if (globalRow < M && globalCol < N) {
		// Compute a single element (loop over K)
		float acc = 0.0f;
		for (int k=0; k<K; k++) {
			acc += A[k*M + globalRow] * B[globalCol*K + k];
		}

		// Store the result
		C[globalCol*M + globalRow] = acc;
	}
}

__kernel void gemm_nt(__global float* restrict gm, const int8 _info)
{
	const int M = _info.s0;
	const int N = _info.s1;
	const int K = _info.s2;
	__global float* restrict A = (__global float* restrict)(gm + _info.s3);
	__global float* restrict B = (__global float* restrict)(gm + _info.s4);
	__global float* restrict C = (__global float* restrict)(gm + _info.s5);

	// Thread identifiers
	const int globalRow = get_global_id(0); // Row ID of C (0..M)
	const int globalCol = get_global_id(1); // Col ID of C (0..N)

	if (globalRow < M && globalCol < N) {
		// Compute a single element (loop over K)
		float acc = 0.0f;
		for (int k=0; k<K; k++) {
			acc += A[k*M + globalRow] * B[globalCol + N*k];
		}

		// Store the result
		C[globalCol*M + globalRow] = acc;
	}
}

#define TRANSPOSEX 16
#define TRANSPOSEY 16
// Simple transpose kernel for a P * Q matrix
__kernel void transpose(__global float* gm, const int8 _info)
{
	const int P = _info.s0;
	const int Q = _info.s1;
	__global float* input = (__global float*)(gm + _info.s2);
	__global float* output = (__global float*)(gm + _info.s3);

	// Thread identifiers
	const int tx = get_local_id(0);
	const int ty = get_local_id(1);
	const int ID0 = get_group_id(0)*TRANSPOSEX + tx; // 0..P
	const int ID1 = get_group_id(1)*TRANSPOSEY + ty; // 0..Q

	// Set-up the local memory for shuffling
	__local float buffer[TRANSPOSEX][TRANSPOSEY];

	// Swap the x and y coordinates to perform the rotation (coalesced)
	if (ID0 < P && ID1 < Q) {
		buffer[ty][tx] = input[ID1*P + ID0];
	}

	// Synchronise all threads
	barrier(CLK_LOCAL_MEM_FENCE);

	// We don't have to swap the x and y thread indices here,
	// because that's already done in the local memory
	const int newID0 = get_group_id(1)*TRANSPOSEY + tx;
	const int newID1 = get_group_id(0)*TRANSPOSEX + ty;

	// Store the transposed result (coalesced)
	if (newID0 < Q && newID1 < P) {
		output[newID1*Q + newID0] = buffer[tx][ty];
	}
}

);

// Threadblock sizes
#define TS 16

float *_mat;
int _info[8];
args_t _args[] = {
	{ CL_MEM_READ_WRITE,  0, 0, 0, OCL_BUFFER },
	{ 0, sizeof(int)*8, 0, _info, 0 },
	{ 0, 0, 0, 0, 0 },
};
ocl_t _kernel[] = {
	// global: m*MDIMC/MWG, n*NDIMC/NWG
//	{ "gemm_fast", 0, 2,{1,1,1},{MDIMC,NDIMC,1}, _args },
//	{ "gemm_nn", 0, 2,{/*M*/1,/*N*/1},{TS,TS}, _args },
	{ "gemm_nt", 0, 2,{/*M*/1,/*N*/1},{TS,TS}, _args },

	// global: k, n
	{ "transpose", 0, 2,{1,1,1},{TRANSPOSEX,TRANSPOSEY,1}, _args },
};
int _ksz = sizeof(_kernel)/sizeof(_kernel[0]);

void sgemm_ocl_init(int platform, int device, int size)
{
//	_args[0].s = _mat = malloc(size);
	_args[0].size = size;

	oclSetup(platform, device);
	oclKernel(_kernel, _ksz, "-cl-denorms-are-zero -cl-finite-math-only -cl-fast-relaxed-math -Werror", sgemm_kcode);
	oclKernelArgs(_kernel, _ksz);
}
static inline void sgemm_ocl(char ta, char tb, int m, int n, int k, float *a, float *b, float *c)
{
	int mk = m*k;
	int kn = k*n;
	int mn = m*n;
	int off_a = 0;
	int off_b = mk;

	oclWrite(_args[0].p, 0, sizeof(float)*mk, a);
	oclWrite(_args[0].p, sizeof(float)*mk, sizeof(float)*kn, b);

	if (ta=='T') {
		_info[0] = m;	// a
		_info[1] = k;	// ta
		_info[2] = 0;	// a
		_info[3] = off_a = mk +kn +mn;
		_kernel[1].global_size[0] = ceil_int(m, TRANSPOSEX);
		_kernel[1].global_size[1] = ceil_int(k, TRANSPOSEY);

		oclKernelArgsWrite(_args);
		oclRun(_kernel+1);
	}
	if (tb=='N') {
		_info[0] = k;	// b
		_info[1] = n;	// tb
		_info[2] = mk;	// b
		_info[3] = off_b = mk +kn +mn +mk;
		_kernel[1].global_size[0] = ceil_int(k, TRANSPOSEX);
		_kernel[1].global_size[1] = ceil_int(n, TRANSPOSEY);

		oclKernelArgsWrite(_args);
		oclRun(_kernel+1);
	}

	_info[0] = m;
	_info[1] = n;
	_info[2] = k;
	_info[3] = off_a;	// a
	_info[4] = off_b;	// b
	_info[5] = mk +kn;	// c
	_kernel[0].global_size[0] = ceil_int(m, TS);
	_kernel[0].global_size[1] = ceil_int(n, TS);

	oclKernelArgsWrite(_args);
	oclRun(_kernel);
//	oclKernelArgsRead(_args);
	oclRead(_args[0].p, sizeof(float)*(mk+kn), sizeof(float)*mn, c);
}
void sgemm_ocl_finish()
{
//	free(_mat);
	oclReleaseKernel(_kernel, _ksz);
	oclFinish();
}
