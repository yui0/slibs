// clang -Os gpgpu_glsl.c -o gpgpu_glsl `pkg-config --libs --cflags glesv2 egl gbm` -lglfw
#include <stdlib.h>
#include "gpgpu_glsl.h"

#define DATA_XSIZE	4096
#define DATA_YSIZE	2048
#define KERNEL_ARRAY	9

char conv2d[] = STRINGIFY(

#ifdef GL_ES
precision highp float;
#endif

#define xSize		1./DATA_XSIZE.
#define ySize		1./DATA_YSIZE.
#define INPUTPLANE	1

uniform sampler2D X;
uniform sampler2D W;
uniform float bias;
varying vec2 uv;

// https://qiita.com/YVT/items/c695ab4b3cf7faa93885
// x:GL_WRAP, y:GL_NEAREST
// arg = vec2(1./size.x, 1./size.x/size.y);
/*vec4 fetchElement(sampler2D tex, float index, vec2 arg)
{
	return texture2D(tex, arg * (index+0.5));
}*/

void main()
{
	vec2 inputOffset[1];
	inputOffset[0] = vec2(0.0, 0.0);
	vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);
	for (int i=0; i<INPUTPLANE; i++) {
		vec2 tuv = uv*vec2(4./DATA_XSIZE., 4./DATA_YSIZE.) + inputOffset[i];

		vec4 p[9];
		p[0] = texture2D(X, tuv + vec2(-xSize, -ySize));
		p[1] = texture2D(X, tuv + vec2(   0.0, -ySize));
		p[2] = texture2D(X, tuv + vec2( xSize, -ySize));
		p[3] = texture2D(X, tuv + vec2(-xSize,    0.0));
		p[4] = texture2D(X, tuv + vec2(   0.0,    0.0));
		p[5] = texture2D(X, tuv + vec2( xSize,    0.0));
		p[6] = texture2D(X, tuv + vec2(-xSize,  ySize));
		p[7] = texture2D(X, tuv + vec2(   0.0,  ySize));
		p[8] = texture2D(X, tuv + vec2( xSize,  ySize));

		const vec2 arg = vec2(1./KERNEL_ARRAY., 1./KERNEL_ARRAY./1.);// arg = vec2(1./size.x, 1./size.x/size.y);
		vec2 pos = arg * (0.0+0.5);		// arg * (index+0.5)
		vec4 a[9];
		a[0] = texture2D(W, pos); pos += arg;	// 1-4
		a[1] = texture2D(W, pos); pos += arg;	// 5-8
		a[2] = texture2D(W, pos); pos += arg;	// 9-12
		a[3] = texture2D(W, pos); pos += arg;	// 13-16
		a[4] = texture2D(W, pos); pos += arg;	// 17-20
		a[5] = texture2D(W, pos); pos += arg;	// 21-24
		a[6] = texture2D(W, pos); pos += arg;	// 25-28
		a[7] = texture2D(W, pos); pos += arg;	// 29-32
		a[8] = texture2D(W, pos);		// 33-36

		sum.x += dot(vec3(p[0].x, p[1].x, p[2].x), a[0].xyz);
		sum.x += dot(vec3(p[3].x, p[4].x, p[5].x), vec3(a[0].w, a[1].x, a[1].y));
		sum.x += dot(vec3(p[6].x, p[7].x, p[8].x), vec3(a[1].z, a[1].w, a[2].x));

		sum.y += dot(vec3(p[0].y, p[1].y, p[2].y), a[2].yzw);
		sum.y += dot(vec3(p[3].y, p[4].y, p[5].y), a[3].xyz);
		sum.y += dot(vec3(p[6].y, p[7].y, p[8].y), vec3(a[3].w, a[4].x, a[4].y));

		sum.z += dot(vec3(p[0].z, p[1].z, p[2].z), vec3(a[4].z, a[4].w, a[5].x));
		sum.z += dot(vec3(p[3].z, p[4].z, p[5].z), a[5].yzw);
		sum.z += dot(vec3(p[6].z, p[7].z, p[8].z), a[6].xyz);

		sum.w += dot(vec3(p[0].w, p[1].w, p[2].w), vec3(a[6].w, a[7].x, a[7].y));
		sum.w += dot(vec3(p[3].w, p[4].w, p[5].w), vec3(a[7].z, a[7].w, a[8].x));
		sum.w += dot(vec3(p[6].w, p[7].w, p[8].w), a[8].yzw);
	}
//	sum += bias;
//	sum = max(sum, 0.0) + min(sum, 0.0) * 0.1;
	gl_FragColor = sum;
}

);

int32_t main(int32_t argc, char* argv[])
{
	coInit();

	GLuint prog = coCreateProgram(conv2d);
	coBindVertices(prog);

	int M = 4;
	int N = 4;
	float mat[] = {	// http://www.songho.ca/dsp/convolution/convolution2d_example.html
		1.,1.,1.,1., 2.,2.,2.,2., 3.,3.,3.,3., 0.,0.,0.,0.,
		4.,4.,4.,4., 5.,5.,5.,5., 6.,6.,6.,6., 0.,0.,0.,0.,
		7.,7.,7.,7., 8.,8.,8.,8., 9.,9.,9.,9., 0.,0.,0.,0.,
		0.,0.,0.,0., 0.,0.,0.,0., 0.,0.,0.,0., 0.,0.,0.,0.,
	};
	float w[] = {
		 1.0,  2.0,  1.0,
		 0.0,  0.0,  0.0,
		-1.0, -2.0, -1.0,

		 1.0,  2.0,  1.0,
		 0.0,  0.0,  0.0,
		-1.0, -2.0, -1.0,

		 1.0,  2.0,  1.0,
		 0.0,  0.0,  0.0,
		-1.0, -2.0, -1.0,

		 1.0,  2.0,  1.0,
		 0.0,  0.0,  0.0,
		-1.0, -2.0, -1.0,
	};

	GLuint texture0 = coCreateDataTexture(DATA_YSIZE, DATA_XSIZE, 0, GL_FLOAT);
	coTransferData(texture0, 0, 0, 4, 4, GL_FLOAT, mat);
	GLuint texture1 = coCreateDataTexture(1, KERNEL_ARRAY, w, GL_FLOAT);
	GLuint texture3 = coCreateDataTexture(DATA_YSIZE, DATA_XSIZE, 0, GL_FLOAT);
	coBindInputTexture(prog, texture0, GL_TEXTURE0, "X");
	coBindInputTexture(prog, texture1, GL_TEXTURE1, "W");
	coBindOutputTexture(M, N, texture3);

	coCompute();

	float *d = coReadDataf(M, N, 0);
	for (int i=0; i<M; i++) {
		for (int j=0; j<N*4; j++) printf("%2.2f ", d[i*N*4+j]);
		printf("\n");
	}
	printf("\n");

	unsigned char *dd = (unsigned char*)d;
	for (int i=0; i<M*N*4; i++) printf("%x ", dd[i]);
	printf("\n");
	free(d);

	coTerm();
	return 0;
}
