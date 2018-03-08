// ©2018 Yuichiro Nakada
// clang -Os ocl_conv2d.c -o ocl_conv2d `pkg-config --libs --cflags OpenCL` -lm
// clang -Os ocl_conv2d.c -o ocl_conv2d -framework opencl -lm
#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "ocl.h"

char kernel_code[] = OCLSTRINGIFY(

kernel void conv2d(global float4 *X/*256*256*/, global float4 *W/*3*3*/, global float4 *Z/*256*256*/)
{
	int gid = get_global_id(0);

	float4 p[9];
	p[0] = X[clamp(gid + -1 + -1*256, 0, 256*256)];
	p[1] = X[clamp(gid +  0 + -1*256, 0, 256*256)];
	p[2] = X[clamp(gid +  1 + -1*256, 0, 256*256)];
	p[3] = X[clamp(gid + -1 +  0*256, 0, 256*256)];
	p[4] = X[/*clamp(gid +  0 +  0*256, 0, 256*256)*/gid];
	p[5] = X[clamp(gid +  1 +  0*256, 0, 256*256)];
	p[6] = X[clamp(gid + -1 +  1*256, 0, 256*256)];
	p[7] = X[clamp(gid +  0 +  1*256, 0, 256*256)];
	p[8] = X[clamp(gid +  1 +  1*256, 0, 256*256)];

	float4 a[9];
	a[0] = W[0];
	a[1] = W[1];
	a[2] = W[2];
	a[3] = W[3];
	a[4] = W[4];
	a[5] = W[5];
	a[6] = W[6];
	a[7] = W[7];
	a[8] = W[8];

	float4 z = 0;
	z.x += dot((float3)(p[0].x, p[1].x, p[2].x), a[0].xyz);	// out 1
	z.x += dot((float3)(p[3].x, p[4].x, p[5].x), (float3)(a[0].w, a[1].x, a[1].y));
//	z.x += dot((float4)(p[3].x, p[4].x, p[5].x, 0.0f), (float4)(a[0].w, a[1].x, a[1].y, 0.0f));
//	z.x += p[4].x * a[1].x;
	z.x += dot((float3)(p[6].x, p[7].x, p[8].x), (float3)(a[1].z, a[1].w, a[2].x));

	z.y += dot((float3)(p[0].x, p[1].x, p[2].x), a[2].yzw);	// out 2
	z.y += dot((float3)(p[3].x, p[4].x, p[5].x), a[3].xyz);
	z.y += dot((float3)(p[6].x, p[7].x, p[8].x), (float3)(a[3].w, a[4].x, a[4].y));

	z.z += dot((float3)(p[0].x, p[1].x, p[2].x), (float3)(a[4].z, a[4].w, a[5].x));
	z.z += dot((float3)(p[3].x, p[4].x, p[5].x), a[5].yzw);
	z.z += dot((float3)(p[6].x, p[7].x, p[8].x), a[6].xyz);

	z.w += dot((float3)(p[0].x, p[1].x, p[2].x), (float3)(a[6].w, a[7].x, a[7].y));
	z.w += dot((float3)(p[3].x, p[4].x, p[5].x), (float3)(a[7].z, a[7].w, a[8].x));
	z.w += dot((float3)(p[6].x, p[7].x, p[8].x), a[8].yzw);

	Z[gid] = z;
//	Z[gid] = p[4].x;
//	Z[gid] = p[4];
}

);

float x[4*256*256], z[4*256*256];
float y[4*3*3] = {
/*	0, 1, 0,
	1, -4, 1,
	0, 1, 0,*/

// ok
// x, x, ok
// x, x, ok
/*	0, 0, 0,
	0, 1, 0,
	0, 0, 0,*/

	0,    0.125, 0,
	0.125, -0.5, 0.125,
	0,    0.125, 0,

/*	0, 4/1., 0,
	4/1., -1, 4/1.,
	0, 4/1., 0,

	0, 4/1., 0,
	4/1., -1, 4/1.,
	0, 4/1., 0,

	0, 4/1., 0,
	4/1., -1, 4/1.,
	0, 4/1., 0,

	0, 4/1., 0,
	4/1., -1, 4/1.,
	0, 4/1., 0,*/
};
args_t args[] = {
	{ CL_MEM_READ_WRITE, sizeof(float)*4*256*256, 0, x, OCL_READ|OCL_WRITE },
	{ CL_MEM_READ_WRITE, sizeof(float)*4*3*3, 0, y, OCL_READ|OCL_WRITE },
	{ CL_MEM_READ_WRITE, sizeof(float)*4*256*256, 0, z, OCL_READ|OCL_WRITE },
	{ 0, 0, 0, 0, 0 },
};
ocl_t kernel[] = {
	{ "conv2d", 0, {256*256,0,0,},{0,0,0,}, args },
};
int ksz = sizeof(kernel)/sizeof(kernel[0]);

int main(int argc, char* argv[])
{
	int w, h, bpp;
	uint8_t *pixels = stbi_load(argv[1], &w, &h, &bpp, 3);
	for (int i=0; i<256; i++) {
		for (int j=0; j<256; j++) {
			x[4*(i*256+j)] = pixels[3*(i*w+j)]/256.0;
			x[4*(i*256+j)+1] = pixels[3*(i*w+j)+1]/256.0;
			x[4*(i*256+j)+2] = pixels[3*(i*w+j)+2]/256.0;
		}
	}
	free(pixels);

	oclSetup(0, 0);
	oclKernel(kernel, ksz, "-cl-denorms-are-zero -cl-finite-math-only -cl-fast-relaxed-math -Werror", kernel_code);
	oclKernelArgs(kernel, ksz);

	oclKernelArgsWrite(args);
	oclRun(&kernel[0]);
	oclKernelArgsRead(args);

	oclReleaseKernel(kernel, ksz);
	oclFinish();

	pixels = calloc(3*256*256, 1);
	for (int i=0; i<256; i++) {
		for (int j=0; j<256; j++) {
			pixels[3*(i*256+j)] = z[4*(i*256+j)]*256;
//			pixels[3*(i*256+j)+1] = z[4*(i*256+j)+1]*256;
//			pixels[3*(i*256+j)+2] = z[4*(i*256+j)+2]*256;
		}
	}
//	stbi_write_png("output.png", 256, 256, 4, z, 0);
	stbi_write_png("output.png", 256, 256, 3, pixels, 0);
	free(pixels);
}
