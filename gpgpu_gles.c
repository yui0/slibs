// clang -Os gpgpu_gles.c -o gpgpu_gles -lglfw -lGL
#include <stdio.h>
#include <stdlib.h>
#include "gpgpu_gles.h"

const char *shader_out[] = { "res1" };
const char* shader_source = "#version 300 es\n" STRINGIFY(
out float res1;
void main() {
  res1 = float(gl_VertexID);
}
);

void compute()
{
	uint32_t num = 100;
	float data[num];

	GLuint program = gpgpu_compile(shader_source, shader_out);

	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), &data[0], GL_STREAM_DRAW);
	GPU_CHECK();

	// Create a transform feedback object:
	GLuint fb = 0;
	glGenTransformFeedbacks(1, &fb);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, fb);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	GPU_CHECK();

//	glUniform1ui(glGetUniformLocation(program, "element_size"), num);

	glEnable(GL_RASTERIZER_DISCARD);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, fb);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, num);
	glEndTransformFeedback();
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	glDisable(GL_RASTERIZER_DISCARD);
	glUseProgram(0);

	GLfloat *feedback;
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	feedback = glMapBufferRange(GL_ARRAY_BUFFER, 0, num*sizeof(float), GL_MAP_READ_BIT);
	printf("glMapBufferRange: ");
	for (int i=0; i<num; i++) {
		printf("%f ", feedback[i]);
	}
	printf("\n");
	glUnmapBuffer(GL_ARRAY_BUFFER);

	printf("glGetBufferSubData: ");
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, num * sizeof(float), data);
	GPU_CHECK();
	for (int i=0; i<num; i++) {
		printf("%f ", data[i]);
	}
	printf("\n");

	glDeleteTransformFeedbacks(1, &fb);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(program);
}

int main(int argc, char* argv[])
{
	gpu_init();
	compute();
	gpu_term();
}
