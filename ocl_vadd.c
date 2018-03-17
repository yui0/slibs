// ©2018 Yuichiro Nakada
// clang -Os ocl_vadd.c -o ocl_vadd `pkg-config --libs --cflags OpenCL`
// clang -Os ocl_vadd.c -o ocl_vadd -framework opencl
// clang -Os ocl_vadd.c -o ocl_vadd -L/opt/amdgpu-pro/lib64/ -lOpenCL
// LD_LIBRARY_PATH=/opt/amdgpu-pro/lib64 ./ocl_vadd
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ocl.h"

char kernel_code[] = OCLSTRINGIFY(

kernel void vec_add(global float* x, global float* y, global float* z, int n)
{
	int gid = get_global_id(0);
	if (gid < n) z[gid] = x[gid] + y[gid];
}

);

int n = 10;
float x[10], y[10], z[10];
args_t args[] = {
	{ CL_MEM_READ_WRITE, sizeof(float)*10, 0, x, OCL_READ|OCL_WRITE },
	{ CL_MEM_READ_WRITE, sizeof(float)*10, 0, y, OCL_READ|OCL_WRITE },
	{ CL_MEM_READ_WRITE, sizeof(float)*10, 0, z, OCL_READ|OCL_WRITE },
	{ 0, sizeof(int), 0, &n, 0 },
	{ 0, 0, 0, 0, 0 },
};
ocl_t kernel[] = {
	{ "vec_add", 0, 1,{10,0,0,},{10,0,0,}, args },
};
int ksz = sizeof(kernel)/sizeof(kernel[0]);

int main()
{
	srand(time(NULL));
	for (int i=0; i<n; i++) x[i] = rand() / (float)RAND_MAX;
	for (int i=0; i<n; i++) y[i] = rand() / (float)RAND_MAX;
	memset(z, 0, sizeof(float) * n);

	oclSetup(0, 0);
	oclKernel(kernel, ksz, "-cl-denorms-are-zero -cl-finite-math-only -cl-fast-relaxed-math -Werror", kernel_code);
	oclKernelArgs(kernel, ksz);

	oclKernelArgsWrite(args);
	oclRun(&kernel[0]);
	oclKernelArgsRead(args);

	oclReleaseKernel(kernel, ksz);
	oclFinish();

	for (int i=0; i<n; i++) {
		printf("%f + %f = %f\n", x[i], y[i], z[i]);
	}
	printf("\n");
}
