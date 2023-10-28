// gcc -Os gpgpu_gles.c -o gpgpu_gles -lglfw -lGL
// clang -Os gpgpu_gles.c -o gpgpu_gles `pkg-config --libs --cflags glesv2 egl gbm` -lglfw
#include <stdlib.h>
#define DEBUG
//#define GPGPU_USE_GLES
#include "gpgpu_gles.h"

#define DATA_XSIZE	4
#define DATA_YSIZE	4
#define KERNEL_ARRAY	9

// what is uv?
char shader1_source[] = "#version 300 es\n" STRINGIFY(
precision highp float;
in vec2 uv; // DATA_XSIZE, DATA_YSIZE
out vec4 result;
void main() {
  result = vec4(1.0, 2.0, uv.x, uv.y);
}
);

// get the data from X
char shader2_source[] = "#version 300 es\n" STRINGIFY(
precision highp float;
in vec2 uv; // DATA_XSIZE, DATA_YSIZE
uniform sampler2D X;
out vec4 result;
void main() {
  result = texelFetch(X, ivec2(int(uv.x*DATA_XSIZE.), int(uv.y*DATA_YSIZE.)), 0);
}
);

int main(int argc, char* argv[])
{
	int M = DATA_XSIZE;
	int N = DATA_YSIZE;
	coInit();

	GLuint prog = coCreateProgram(shader1_source);
	coBindVertices(prog);
//	GLuint texture0 = coCreateDataTexture(DATA_XSIZE, DATA_YSIZE, 0, GLES_FLOAT, GPGPU_TEX_PADDING);
//	coTransferData(texture0, 0, 0, DATA_XSIZE, DATA_YSIZE, GLES_FLOAT, mat);
//	GLuint texture1 = coCreateDataTexture(1, KERNEL_ARRAY, w, GLES_FLOAT, 0);
	GLuint texture3 = coCreateDataTexture(DATA_XSIZE, DATA_YSIZE, 0, GLES_FLOAT, 0);
//	coBindInputTexture(prog, texture0, GL_TEXTURE0, "X");
//	coBindInputTexture(prog, texture1, GL_TEXTURE1, "W");
	coBindOutputTexture(N, M, texture3);
	coCompute();
	float *d = coReadDataf(N, M, 0);
	for (int i=0; i<M; i++) {
		for (int j=0; j<N*4; j++) printf("%2.2f ", d[i*N*4+j]);
		printf("\n");
	}
	printf("\n");

	float mat[M*N*4];
	for (int i=0; i<M; i++) {
		for (int j=0; j<N*4; j++) mat[i*N*4+j] = i*N*4+j;
	}
	prog = coCreateProgram(shader2_source);
	coBindVertices(prog);
	GLuint texture0 = coCreateDataTexture(DATA_XSIZE, DATA_YSIZE, 0, GLES_FLOAT, GPGPU_TEX_PADDING);
	coTransferData(texture0, 0, 0, DATA_XSIZE, DATA_YSIZE, GLES_FLOAT, mat);
//	GLuint texture3 = coCreateDataTexture(DATA_XSIZE, DATA_YSIZE, 0, GLES_FLOAT, 0);
	coBindInputTexture(prog, texture0, GL_TEXTURE0, "X");
	coBindOutputTexture(N, M, texture3);
	coCompute();
	d = coReadDataf(N, M, 0);
	for (int i=0; i<M; i++) {
		for (int j=0; j<N*4; j++) printf("%2.2f ", d[i*N*4+j]);
		printf("\n");
	}
	printf("\n");

	coTerm();
	return 0;
}
