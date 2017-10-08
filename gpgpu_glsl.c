// clang -Os gpgpu_glsl.c -o gpgpu_glsl `pkg-config --libs --cflags glesv2 egl gbm`
#include <stdlib.h>
#include "gpgpu_glsl.h"

// relu
char act_relu[] = STRINGIFY(

precision highp float;

varying vec2 outTex;
uniform sampler2D X;
//uniform int relu;

void main(void)
{
//	if (relu == 1) {
		gl_FragColor = max(texture2D(X, vec2(outTex.x, outTex.y)), 0.0);
		//gl_FragColor = vec4(2.0,2.0,2.0,2.0);
		//gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
		gl_FragColor = vec4(0.0, 0.0, 0.2, 0.1);
//	} else {
//		gl_FragColor = texture2D(X, vec2(outTex.x, outTex.y));
//	}
}

);

int32_t main(int32_t argc, char* argv[])
{
	coInit();

	GLuint act = coCreateProgram(act_relu);
	coBindVertices(act);

	int M = 4;
	int N = 4;
	float mat[] = {
		1.0, 2.0, 3.0, 4.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
	};
	GLuint texture0 = coCreateDataTexture(M, N, mat);
	//GLuint texture3 = coCreateOutputTexture(M, N);
	GLuint texture3 = coCreateDataTexture(M, N, 0);
	coBindInputTexture(act, texture0, GL_TEXTURE0, "X");
	coBindOutputTexture(M, N, texture3);

	glUseProgram(act);
	coCompute();

	float *d = coReadData(M, N, 0);
	for (int i=0; i<M*N; i++) printf("%f ", d[i]);
	unsigned char *dd = (unsigned char*)d;
	for (int i=0; i<M*N*4; i++) printf("%x ", dd[i]);
	printf("\n");
	free(d);

	coTerm();
	return 0;
}
