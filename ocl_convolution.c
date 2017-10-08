// ©2017 Yuichiro Nakada
// clang -Os convolution.c -o convolution `pkg-config --libs --cflags OpenCL`
// clang -Os convolution.c -o convolution -framework opencl

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "ocl.h"

char kernel_code[] =
#include "ocl_convolution.cl"

const unsigned int inputSignalWidth  = 8;
const unsigned int inputSignalHeight = 8;
unsigned int inputSignal[inputSignalWidth][inputSignalHeight] =
{
	{3, 1, 1, 4, 8, 2, 1, 3},
	{4, 2, 1, 1, 2, 1, 2, 3},
	{4, 4, 4, 4, 3, 2, 2, 2},
	{9, 8, 3, 8, 9, 0, 0, 0},
	{9, 3, 3, 9, 0, 0, 0, 0},
	{0, 9, 0, 8, 0, 0, 0, 0},
	{3, 0, 8, 8, 9, 4, 4, 4},
	{5, 9, 8, 1, 8, 1, 1, 1}
};

const unsigned int outputSignalWidth  = 6;
const unsigned int outputSignalHeight = 6;
unsigned int outputSignal[outputSignalWidth][outputSignalHeight];

const unsigned int maskWidth  = 3;
const unsigned int maskHeight = 3;
unsigned int mask[maskWidth][maskHeight] =
{
	{1, 1, 1}, {1, 0, 1}, {1, 1, 1},
};

cl_mem inputSignalBuffer;
cl_mem maskBuffer;
cl_mem outputSignalBuffer;
args_t args[] = {
	{ CL_MEM_READ_ONLY, sizeof(unsigned int) * inputSignalHeight * inputSignalWidth, &inputSignalBuffer, inputSignal, OCL_WRITE },
	{ CL_MEM_READ_ONLY, sizeof(unsigned int) * maskHeight * maskWidth, &maskBuffer, mask, OCL_WRITE },
	{ CL_MEM_WRITE_ONLY, sizeof(unsigned int) * outputSignalHeight * outputSignalWidth, &outputSignalBuffer, outputSignal, OCL_READ },
	{ 0, sizeof(unsigned int), &inputSignalWidth, 0, 0 },
	{ 0, sizeof(unsigned int), &maskWidth, 0, 0 },
	{ 0, 0, 0, 0, 0 },
};
ocl_t kernel[] = {
	{ "convolve", 0, {outputSignalWidth * outputSignalHeight,0,0,},{1,0,0,}, args },
};
int ksz = sizeof(kernel)/sizeof(kernel[0]);

int main()
{
	oclSetup(0, 0);
	oclKernel(kernel, ksz, "-cl-denorms-are-zero -cl-finite-math-only -cl-fast-relaxed-math -Werror", kernel_code);
	oclKernelArgs(kernel, ksz);

	oclKernelArgsWrite(args);
	oclRun(&kernel[0]);
	oclKernelArgsRead(args);

	oclReleaseKernel(kernel, ksz);
	oclFinish();

	for (int y = 0; y < outputSignalHeight; y++) {
		for (int x = 0; x < outputSignalWidth; x++) {
			printf("%d ", outputSignal[x][y]);
		}
		printf("\n");
	}
}
