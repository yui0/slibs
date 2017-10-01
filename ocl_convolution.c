// clang -Os convolution.c -o convolution `pkg-config --libs --cflags OpenCL`
// clang -Os convolution.c -o convolution -framework opencl
// https://codereview.stackexchange.com/questions/106258/two-step-opencl-convolution-for-series-of-matrices
// https://github.com/victusfate/opencl-book-examples/blob/master/src/Chapter_3/OpenCLConvolution/Convolution.cpp
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "ocl.h"

char kernel_code[] =
#include "ocl_convolution.cl"

/*cl_mem filter_buffer;
cl_mem output_buffer;
cl_mem width_buffer;
args_t noise_args[] = {
	{ CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(filter), &filter_buffer, 0, OCL_READ },
	{ CL_MEM_WRITE_ONLY, IMAGE_SIZE, &output_buffer, 0, OCL_WRITE },
	{ CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(width), &width_buffer, 0, OCL_READ },
	{ 0, 0, 0, 0, 0, 0 },
};
args_t sobel_args[] = {
	{ CL_MEM_WRITE_ONLY, IMAGE_SIZE, &output_buffer, 0, OCL_WRITE },
	{ CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(width), &width_buffer, 0, OCL_READ },
	{ 0, 0, 0, 0, 0, 0 },
};
ocl_t kernel[] = {
	{ "convolute_unrolled",	0, {256,0,0,},{256,0,0,}, noise_args },
	{ "sobel",		0, {1024,0,0,},{256,0,0,}, sobel_args },
};*/

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
	{ CL_MEM_READ_ONLY /*| CL_MEM_COPY_HOST_PTR*/, sizeof(unsigned int) * inputSignalHeight * inputSignalWidth, &inputSignalBuffer, inputSignal, OCL_WRITE },
	{ CL_MEM_READ_ONLY /*| CL_MEM_COPY_HOST_PTR*/, sizeof(unsigned int) * maskHeight * maskWidth, &maskBuffer, mask, OCL_WRITE },
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

	/*cl_mem image_buffer;
	cl_mem brightness_buffer;

	struct timeval t0, t1;
	clock_t startClock, endClock;
	gettimeofday(&t0, NULL);
	startClock = clock();
	for (int i=0; i<line_count; ++i) {
		// remove single pixel noise
		image_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, IMAGE_SIZE, input_data[i], &err);
		err = clSetKernelArg(noise_kernel, 0, sizeof(cl_mem), &image_buffer);

		queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
		size_t work_items[2] = {DIM_Y - 2, DIM_X - 2};

		err = clEnqueueNDRangeKernel(queue, noise_kernel, 2, NULL, &work_items, NULL, 0, NULL, NULL);

		err = clEnqueueReadBuffer(queue, output_buffer, CL_TRUE, 0, IMAGE_SIZE, input_data[i], 0, NULL, NULL);

		// apply sobel operator
		err = clSetKernelArg(sobel_kernel, 0, sizeof(cl_mem), &image_buffer);
		err = clEnqueueNDRangeKernel(queue, sobel_kernel, 2, NULL, &work_items, NULL, 0, NULL, NULL);

		err = clEnqueueReadBuffer(queue, output_buffer, CL_TRUE, 0, IMAGE_SIZE, input_data[i], 0, NULL, NULL);
	}
	clFinish(queue);

	gettimeofday(&t1, NULL);
	endClock = clock();
	time_t diffsec = difftime(t1.tv_sec, t0.tv_sec);
	suseconds_t diffsub = t1.tv_usec - t0.tv_usec;
	double realsec = diffsec+diffsub*1e-6;
	double cpusec = (endClock - startClock)/(double)CLOCKS_PER_SEC;
	double percent = 100.0*cpusec/realsec;
	printf("Time spent on GPU: %f\n", realsec);
	printf("CPU utilization: %f\n", cpusec);*/
}
