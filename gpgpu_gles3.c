// clang -Os gpgpu_gles3.c -o gpgpu_gles3 -lglfw -lGL
#include <stdio.h>
#include <stdlib.h>
#define DEBUG
#include "gpgpu_gles3.h"

const char *shader_out[] = { "res" };
const char* shader_source = "#version 320 es\n" STRINGIFY(
out vec4 res;
uniform highp samplerBuffer X;
void main() {
  res = vec4(gl_VertexID);
  //res = texelFetch(X, gl_VertexID);
}
);

void compute()
{
	uint32_t num = 100;
	float data[num];
	for (int i=0; i<num; i++) {
		data[i] = i;
	}

	GLuint program = gpu_compile(shader_source, shader_out);

	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo[2];
	glGenBuffers(2, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), /*&data[0]*/0, GL_STREAM_DRAW);
	GPU_CHECK();

	// create texture buffer object
	glBindBuffer(GL_TEXTURE_BUFFER, vbo[0]);
	glBufferData(GL_TEXTURE_BUFFER, num*sizeof(float), data, GL_STATIC_DRAW);
	glBindTexture(GL_TEXTURE_BUFFER, vbo[0]);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, vbo[0]);
	GPU_CHECK();

	// create a transform feedback object
	GLuint fb = 0;
	glGenTransformFeedbacks(1, &fb);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, fb);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo[1]);
//	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	GPU_CHECK();

/*	printf("X:%d\n", glGetUniformLocation(program, "X"));
//	glUseProgram(program);
	glUniform1ui(glGetUniformLocation(program, "X"), vbo[0]);
	GPU_CHECK();*/

	glEnable(GL_RASTERIZER_DISCARD);
//	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, fb);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, num);
	glEndTransformFeedback();
//	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	glDisable(GL_RASTERIZER_DISCARD);
//	glUseProgram(0);
	GPU_CHECK();

	// swap
/*	glBindTexture(GL_TEXTURE_BUFFER, 0);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo[0]);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	GPU_CHECK();
	glUniform1ui(glGetUniformLocation(program, "X"), vbo[1]);
	glEnable(GL_RASTERIZER_DISCARD);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, fb);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, num);
	glEndTransformFeedback();
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	glDisable(GL_RASTERIZER_DISCARD);*/

	GLfloat *feedback;
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	feedback = glMapBufferRange(GL_ARRAY_BUFFER, 0, num*sizeof(float), GL_MAP_READ_BIT);
	printf("glMapBufferRange: ");
	for (int i=0; i<num; i++) {
		printf("%f ", feedback[i]);
	}
	printf("\n");
	glUnmapBuffer(GL_ARRAY_BUFFER);
	GPU_CHECK();

	printf("glGetBufferSubData: ");
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, num*sizeof(float), data);
	GPU_CHECK();
	for (int i=0; i<num; i++) {
		printf("%f ", data[i]);
	}
	printf("\n");

	glDeleteTransformFeedbacks(1, &fb);
	glDeleteBuffers(2, vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(program);
}

int main(int argc, char* argv[])
{
	gpu_init();
	compute();
	gpu_term();
}
