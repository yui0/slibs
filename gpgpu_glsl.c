// clang -Os gpgpu_conv2d.c -o gpgpu_conv2d `pkg-config --libs --cflags glesv2 egl gbm`
#include <stdlib.h>
#include "gpgpu_glsl.h"

char conv2d[] = STRINGIFY(

#define INPUTPLANE 1
GPGPU_GLES_HIGHP
uniform sampler2D X;
uniform vec2 pixSize;
uniform mat3 weight[128];
uniform float bias;
varying vec2 uv;

void main() {
	vec4 inputOffset[1];
	inputOffset[0] = vec4(0.0, 0.0, 0.0, 0.0);
	float sum = 0.0;
	for (int i=0; i<INPUTPLANE; i++) {
		vec2 tuv = uv + inputOffset[i].xy;

		vec4 p[9];
		p[0] = texture2D(X, tuv + vec2(-pixSize.x, -pixSize.y));
		p[1] = texture2D(X, tuv + vec2(       0.0, -pixSize.y));
		p[2] = texture2D(X, tuv + vec2( pixSize.x, -pixSize.y));
		p[3] = texture2D(X, tuv + vec2(-pixSize.x,        0.0));
		p[4] = texture2D(X, tuv + vec2(       0.0,        0.0));
		p[5] = texture2D(X, tuv + vec2( pixSize.x,        0.0));
		p[6] = texture2D(X, tuv + vec2(-pixSize.x,  pixSize.y));
		p[7] = texture2D(X, tuv + vec2(       0.0,  pixSize.y));
		p[8] = texture2D(X, tuv + vec2( pixSize.x,  pixSize.y));
		mat3 pix[4];
		pix[0] = mat3(  p[0].x, p[1].x, p[2].x,
				p[3].x, p[4].x, p[5].x,
				p[6].x, p[7].x, p[8].x);
		/*pix[1] = mat3(  p[0].y, p[1].y, p[2].y,
				p[3].y, p[4].y, p[5].y,
				p[6].y, p[7].y, p[8].y);
		pix[2] = mat3(  p[0].z, p[1].z, p[2].z,
				p[3].z, p[4].z, p[5].z,
				p[6].z, p[7].z, p[8].z);
		pix[3] = mat3(  p[0].w, p[1].w, p[2].w,
				p[3].w, p[4].w, p[5].w,
				p[6].w, p[7].w, p[8].w);*/

		sum += dot(weight[i][0].xyz, pix[0][0].xyz);
		sum += dot(weight[i][1].xyz, pix[0][1].xyz);
		sum += dot(weight[i][2].xyz, pix[0][2].xyz);
	}
//	sum += bias;
//	sum = max(sum, 0.0) + min(sum, 0.0) * 0.1;
	gl_FragColor = vec4(sum, sum, sum, 1.0);
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
		1.0,0.,0.,0., 2.0,0.,0.,0., 3.0,0.,0.,0., 0.0,0.,0.,0.,
		4.0,0.,0.,0., 5.0,0.,0.,0., 6.0,0.,0.,0., 0.0,0.,0.,0.,
		7.0,0.,0.,0., 8.0,0.,0.,0., 9.0,0.,0.,0., 0.0,0.,0.,0.,
		0.0,0.,0.,0., 0.0,0.,0.,0., 0.0,0.,0.,0., 0.0,0.,0.,0.,
	};
	float w[] = {
		 1.0,  2.0,  1.0,
		 0.0,  0.0,  0.0,
		-1.0, -2.0, -1.0,
	};

	GLuint texture0 = coCreateDataTexture(M, N, mat, GL_FLOAT);
	GLuint texture3 = coCreateDataTexture(M, N, 0, GL_FLOAT);
	coBindInputTexture(prog, texture0, GL_TEXTURE0, "X");
	coBindOutputTexture(M, N, texture3);
	coUniform2f(prog, "pixSize", 1.0/N, 1.0/M);
	coUniformMatrix3fv(prog, "weight", w);

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
