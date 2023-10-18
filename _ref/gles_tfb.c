// gcc -o gles_tfb gles_tfb.c -Os -lglfw -lGL
#include <stdio.h>
#include <stdlib.h>

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#define GLFW_INCLUDE_GLU
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#define DEBUG
#ifdef DEBUG
#define GPU_CHECK() gpu_checkError(__LINE__);
#else
#define GPU_CHECK()
#endif
void gpu_checkError(int line)
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		printf(__FILE__ ":%d glGetError returns %d\n", line, err);
		exit(1);
	}
}

#define _STRGF(x)	# x
#define STRINGIFY(x)	_STRGF(x)

const char* fragment_shader_source = "#version 300 es\nvoid main(){}";

const char* compute_shader_source = "#version 300 es\n" STRINGIFY(
out float res1;
void main() {
  res1 = float(gl_VertexID);
}
);

void gpu_init()
{
	if (!glfwInit()) {
		printf("Can't initialize GLFW.\n");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(1, 1, "invisible window", 0, 0);
	if (!window) {
		printf("error occurred: glfwCreateWindow!\n");
	}
	glfwMakeContextCurrent(window);
	glDisable(GL_DEPTH_TEST);
}
void gpu_term()
{
	glfwTerminate();
}

GLuint gpu_loadShader(GLenum shaderType, const char* pSource)
{
	GLuint shader = glCreateShader(shaderType);
	if (shader) {
		glShaderSource(shader, 1, &pSource, NULL);
		glCompileShader(shader);
		GLint compiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled) {
			GLint infoLen = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen) {
				char* buf = (char*)malloc(infoLen);
				if (buf) {
					glGetShaderInfoLog(shader, infoLen, NULL, buf);
					printf("%s\n\nCould not compile shader %d:\n%s\n",
					       pSource, shaderType, buf);
					free(buf);
					exit(1);
				}
				glDeleteShader(shader);
				shader = 0;
			}
		}
	}
	return shader;
}

GLuint gpgpu_compile(const char* src, const char *varyings[])
{
	GLuint fragmentShader = gpu_loadShader(GL_FRAGMENT_SHADER, fragment_shader_source);
	if (!fragmentShader) return 0;
	GLuint vertexShader = gpu_loadShader(GL_VERTEX_SHADER, src);
	if (!vertexShader) return 0;

	GLuint program = glCreateProgram();
	if (!program) {
		printf("gpgpu_compile error!\n");
		return 0;
	}

	glAttachShader(program, fragmentShader);
	glAttachShader(program, vertexShader);

	// for transform feedback output
	glTransformFeedbackVaryings(program, 1, varyings, GL_INTERLEAVED_ATTRIBS);

	glLinkProgram(program);
	GLint linkStatus = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE) {
		GLint bufLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
		if (bufLength) {
			char* buf = (char*)malloc(bufLength);
			if (buf) {
				glGetProgramInfoLog(program, bufLength, NULL, buf);
				printf("Could not link program:\n%s\n", buf);
				free(buf);
				exit(1);
			}
		}
		glDeleteProgram(program);
		program = 0;
	}

	// setup variables
	glUseProgram(program);
	return program;
}

void compute()
{
	uint32_t num = 100;
	float data[num];
	const char *out[] = { "res1" };

	GLuint program = gpgpu_compile(compute_shader_source, out);

	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint BufferA, BufferB;
	// Create VBO for input on even frames and output on odd frames:
	glGenBuffers(1, &BufferA);
	glBindBuffer(GL_ARRAY_BUFFER, BufferA);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), &data[0], GL_STREAM_DRAW);
	GPU_CHECK();

	// Create VBO for output on even frames and input on odd frames:
	glGenBuffers(1, &BufferB);
	glBindBuffer(GL_ARRAY_BUFFER, BufferB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), 0, GL_STREAM_DRAW);
	GPU_CHECK();

	// Create a transform feedback object:
	GLuint Feedback = 0;
	glGenTransformFeedbacks(1, &Feedback);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, Feedback);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, BufferA);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	GPU_CHECK();

//	glUniform1ui(glGetUniformLocation(program, "element_size"), num);

	glEnable(GL_RASTERIZER_DISCARD);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, Feedback);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, num);
	glEndTransformFeedback();
//	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
//	swap(BufferA, BufferB);
	glDisable(GL_RASTERIZER_DISCARD);
//	glUseProgram(0);

	GLfloat *feedback;
	glBindBuffer(GL_ARRAY_BUFFER, BufferA);
	feedback = glMapBufferRange(GL_ARRAY_BUFFER, 0, num*sizeof(float), GL_MAP_READ_BIT);
	printf("glMapBufferRange: ");
	for (int i=0; i<num; i++) {
		printf("%f ", feedback[i]);
	}
	printf("\n");
	glUnmapBuffer(GL_ARRAY_BUFFER);

	printf("glGetBufferSubData: ");
	glBindBuffer(GL_ARRAY_BUFFER, BufferA);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, num * sizeof(float), data);
	GPU_CHECK();
	for (int i=0; i<num; i++) {
		printf("%f ", data[i]);
	}
	printf("\n");

//	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(program);
	GPU_CHECK();
}

int main(int argc, char* argv[])
{
	gpu_init();
	compute();
	gpu_term();
}
