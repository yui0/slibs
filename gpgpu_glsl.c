// clang -Os gpgpu_conv2d.c -o gpgpu_conv2d `pkg-config --libs --cflags glesv2 egl gbm`
#include <stdlib.h>
#include "gpgpu_glsl.h"

char conv2d[] = STRINGIFY(

#define INPUTPLANE 1
GPGPU_GLES_HIGHP
uniform sampler2D X;
uniform mat3 weight[128];
//uniform vec2 pixSize;
uniform float bias;
varying vec2 uv;

//vec2 pixSize = vec2(1.0/4096, 1.0/2048);
vec2 pixSize = vec2(0.25, 0.25);

void main() {
	vec4 inputOffset[1];
	inputOffset[0] = vec4(0.0, 0.0, 0.0, 0.0);
	float sum = 0.0;
	for (int i=0; i<INPUTPLANE; i++) {
		vec2 tuv = uv + inputOffset[i].xy;
		mat3 pix = mat3(
		    vec3(texture2D(X, tuv + vec2(-pixSize.x, -pixSize.y)).x,
			 texture2D(X, tuv + vec2(       0.0, -pixSize.y)).x,
			 texture2D(X, tuv + vec2( pixSize.x, -pixSize.y)).x),
		    vec3(texture2D(X, tuv + vec2(-pixSize.x,        0.0)).x,
			 texture2D(X, tuv + vec2(       0.0,        0.0)).x,
			 texture2D(X, tuv + vec2( pixSize.x,        0.0)).x),
		    vec3(texture2D(X, tuv + vec2(-pixSize.x,  pixSize.y)).x,
			 texture2D(X, tuv + vec2(       0.0,  pixSize.y)).x,
			 texture2D(X, tuv + vec2( pixSize.x,  pixSize.y)).x)
		);
		sum += dot(weight[i][0].xyz, pix[0].xyz);
		sum += dot(weight[i][1].xyz, pix[1].xyz);
		sum += dot(weight[i][2].xyz, pix[2].xyz);
	//gl_FragColor = vec4(pix[0], 0.0);
	}
//	sum += bias;
//	sum = max(sum, 0.0) + min(sum, 0.0) * 0.1;
	gl_FragColor = vec4(sum, sum, sum, 1.0);
//	gl_FragColor = vec4(texture2D(X,uv).x, 0, 0, 0.0);
//	gl_FragColor = vec4(texture2D(X,uv+vec2(pixSize.x,0.)).x, 0., 0., 0.);
//	gl_FragColor = vec4(texture2D(X,uv+vec2(pixSize.x,pixSize.y)).x, 0, 0, 0.0);
//	gl_FragColor = vec4(texture2D(X,uv+vec2(0.06,0.)).x, 0., 0., 0.);
}

);

int32_t main(int32_t argc, char* argv[])
{
	coInit();

	GLuint prog = coCreateProgram(conv2d);
	coBindVertices(prog);

	int M = 4;
//	int N = 4*4;
	int N = 4;
	float mat[] = {
		1.0,0.,0.,0., 2.0,0.,0.,0., 3.0,0.,0.,0., 0.0,0.,0.,0.,
		4.0,0.,0.,0., 5.0,0.,0.,0., 6.0,0.,0.,0., 0.0,0.,0.,0.,
		7.0,0.,0.,0., 8.0,0.,0.,0., 9.0,0.,0.,0., 0.0,0.,0.,0.,
		0.0,0.,0.,0., 0.0,0.,0.,0., 0.0,0.,0.,0., 0.0,0.,0.,0.,
	};
	/*float w[] = {
		-1.0, -2.0, -1.0,
		 0.0,  0.0,  0.0,
		 1.0,  2.0,  1.0,
	};*/
	float w[] = {
		 1.0,  2.0,  1.0,
		 0.0,  0.0,  0.0,
		-1.0, -2.0, -1.0,
	};

	GLuint texture0 = coCreateDataTexture(M, N, mat, GL_FLOAT);
//	GLuint texture0 = coCreateDataTexture(M, N, mat, GL_UNSIGNED_BYTE);
//	GLuint texture3 = coCreateDataTexture(M, N, 0, GL_UNSIGNED_BYTE);
	GLuint texture3 = coCreateDataTexture(M, N, 0, GL_FLOAT);
	coBindInputTexture(prog, texture0, GL_TEXTURE0, "X");
	coBindOutputTexture(M, N, texture3);
	GLuint mat3 = glGetUniformLocation(prog, "weight");
	glUniformMatrix3fv(mat3, 1, GL_FALSE, w);

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
