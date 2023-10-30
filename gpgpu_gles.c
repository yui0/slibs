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
precision mediump float;
in vec2 uv; // DATA_XSIZE, DATA_YSIZE
uniform sampler2D X;
out vec4 result;
void main() {
  result = texelFetch(X, ivec2(int(uv.x*DATA_XSIZE.), int(uv.y*DATA_YSIZE.)), 0);
}
);

// get the data from X
char shader3_source[] = "#version 300 es\n" STRINGIFY(
precision mediump float;
in vec2 uv; // DATA_XSIZE, DATA_YSIZE
uniform sampler2D X;
out vec4 result;
void main() {
  ivec2 X_size = textureSize(X, 0);
  vec2 texcoord = gl_FragCoord.xy / vec2(float(X_size[0]), float(X_size[1]));
  result = texture(X, texcoord);
}
);

// get the data from X
char shader4_source[] = "#version 300 es\n" STRINGIFY(
precision mediump float;
in vec2 uv; // DATA_XSIZE, DATA_YSIZE
uniform sampler2D X;
out vec4 result;
// arg = vec2(1. / size.x, 1. / size.x / size.y);
vec4 fetchElement(sampler2D tex, float index, vec2 arg)
{
  return texture(tex, arg * (index + 0.5));
}
/*uint convertCoordToIndex(uvec2 coord, uint u_size) {
  return coord.x + coord.y * u_size;
}*/
void main() {
  ivec2 X_size = textureSize(X, 0);
  vec2 arg = vec2(1. / float(X_size.x), 1. / float(X_size.x) / float(X_size.y));
  vec2 elem2d = floor(gl_FragCoord.xy);
  float elem1d = elem2d.x + elem2d.y * float(X_size.x);
  result = fetchElement(X, elem1d, arg);

  //uint index = convertCoordToIndex(uvec2(gl_FragCoord.xy), uint(X_size.x));
  //result = fetchElement(X, float(index), arg);

  //float elem1d = gl_FragCoord.x + gl_FragCoord.y * float(X_size.x);
  //result = fetchElement(X, elem1d, arg);
}
);

#if 0
char matmul_source[] = "#version 300 es\n" STRINGIFY(
precision mediump float;
in vec2 uv; // DATA_XSIZE, DATA_YSIZE
uniform float K;
uniform sampler2D A;
uniform sampler2D B_t;
out vec4 result;
void main() {
  //int i = int(uv.y*DATA_YSIZE.*DATA_XSIZE. + uv.x*DATA_XSIZE.);
  //vec4 x = texelFetch(X, ivec2(int(uv.x*DATA_XSIZE.), int(uv.y*DATA_YSIZE.)), 0);
  float delta = 1.0/K *4.0/*4dot*/;
  result = 0.0;
  for (int j=0; j<K/4; j++) {
    vec4 a_ik = texture(  A, vec2(z, uv.y));
    vec4 b_kj = texture(B_t, vec2(z, uv.x));
    result += dot(a_ik, b_kj);
    z += delta;
  }
}
);
#endif
char matmul_source[] = "#version 300 es\n" STRINGIFY(
precision highp float;
uniform highp sampler2D A;
uniform highp sampler2D B;
out vec4 result;
void main() {
  ivec2 A_size = textureSize(A, 0);
  int commonDim = A_size[0] / 4;

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
    /*sum0 += dot(a, b0);
    sum1 += dot(a, b1);
    sum2 += dot(a, b2);
    sum3 += dot(a, b3);*/
  }
  result = vec4(sum0.x+sum0.y+sum0.z+sum0.w, sum1.x+sum1.y+sum1.z+sum1.w, sum2.x+sum2.y+sum2.z+sum2.w, sum3.x+sum3.y+sum3.z+sum3.w);
}
);
char __matmul_source[] = "#version 300 es\n" STRINGIFY(
precision highp float;
uniform highp sampler2D A;
uniform highp sampler2D B;
out float result;
void main() {
  ivec2 A_size = textureSize(A, 0);
  int commonDim = A_size[0];

  int w = int(gl_FragCoord.x);
  result = 0.0;
  for (int i=0; i<commonDim; ++i) {
    float a = texelFetch(A, ivec2(i, 0), 0).r;
    float b = texelFetch(B, ivec2(i, w), 0).r;
    result += a * b;
  }
}
);
/*char matmul_source[] = "#version 300 es\n" STRINGIFY(
precision highp float;

in vec2 uv;
uniform sampler2D A;
uniform sampler2D B;
uniform sampler2D C;
uniform bool addC;
out vec4 outColor;

void main() {
  ivec2 A_size = textureSize(A, 0);
  ivec2 B_size = textureSize(B, 0);
  int out_x = int(float(B_size[0]) * uv.x);
  int out_y = int(float(A_size[1]) * uv.y);
  int commonDim = A_size[0];

  float sum = 0.;
  for (int i = 0; i < commonDim; ++i) {
    float a = texelFetch(A, ivec2(i, out_y), 0).r;
    float b = texelFetch(B, ivec2(out_x, i), 0).r;
    sum += a * b;
  }

  if (addC) {
    sum += texelFetch(C, ivec2(out_x, 0), 0).r;
  }

  outColor = vec4(sum);
}
);*/

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

	prog = coCreateProgram(shader3_source);
	coBindVertices(prog);
//	GLuint texture0 = coCreateDataTexture(DATA_XSIZE, DATA_YSIZE, 0, GLES_FLOAT, GPGPU_TEX_PADDING);
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

	prog = coCreateProgram(shader4_source);
	coBindVertices(prog);
//	GLuint texture0 = coCreateDataTexture(DATA_XSIZE, DATA_YSIZE, 0, GLES_FLOAT, GPGPU_TEX_PADDING);
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

	{
	// A (1 x 4)
	float a[1][4*4] = {{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0}};
	// B (4 x 4)
	float b[4][4*4] =
	{{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0},
         {2.0, 3.0, 4.0, 5.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0},
         {3.0, 4.0, 5.0, 6.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0},
         {4.0, 5.0, 6.0, 7.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0}};
	int m = 1;
	int n = 4;
	int k = 4;

	prog = coCreateProgram(matmul_source);
	coBindVertices(prog);
	GLuint texture0 = coCreateDataTexture(n, m, 0, GLES_FLOAT, GPGPU_TEX_PADDING);
	coTransferData(texture0, 0, 0, n, m, GLES_FLOAT, a);
	GLuint texture1 = coCreateDataTexture(k, n, 0, GLES_FLOAT, GPGPU_TEX_PADDING);
	coTransferData(texture1, 0, 0, k, n, GLES_FLOAT, b);
	GLuint texture2 = coCreateDataTexture(n, m, 0, GLES_FLOAT, 0);
	coBindInputTexture(prog, texture0, GL_TEXTURE0, "A");
	coBindInputTexture(prog, texture1, GL_TEXTURE1, "B");
	coBindOutputTexture(n, m, texture2);
	coCompute();
	d = coReadDataf(n, m, 0);
	for (int i=0; i<m; i++) {
		for (int j=0; j<n*4; j++) printf("%2.2f ", d[i*n*4+j]);
		printf("\n");
	}
	printf("\n");
	}

	coTerm();
	return 0;
}
