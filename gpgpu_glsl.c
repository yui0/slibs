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


#if 0
#define COMPONENTS_PER_TEXEL	4
int getPad(int n)
{
	int rem = (n % COMPONENTS_PER_TEXEL);
	return rem == 0 ? 0 : COMPONENTS_PER_TEXEL - rem;
}
/*GLuint coCreateDataTexture(int h, int w, float *texels)
{
	int rem = (w % COMPONENTS_PER_TEXEL);
	int pad = rem == 0 ? 0 : COMPONENTS_PER_TEXEL - rem;

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	if (pad == 0 || !texels) {
		// no padding required, write directly from input array
//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w / COMPONENTS_PER_TEXEL, h, 0, GL_RGBA, GL_FLOAT, (void*)texels);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)texels);
	} else {
		// must pad each row
		// create empty texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (w + pad) / COMPONENTS_PER_TEXEL, h, 0, GL_RGBA, GL_FLOAT, 0);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (w + rem) / COMPONENTS_PER_TEXEL, h, GL_RGBA, GL_FLOAT, texels);
	}

	// clamp to edge to support non-power of two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// don't interpolate when getting data from texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// we're done with setup, so unbind current texture
	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}*/
GLuint coCreateOutputTexture(int h, int w)
{
	// create and bind texture to render to
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D,/*level*/0, GL_RGBA, w + getPad(w), h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	// clamp to edge to support non-power of two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// don't interpolate when getting data from texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// we're done with setup, so unbind current texture
	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}
#endif


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
