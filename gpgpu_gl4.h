//---------------------------------------------------------
//	Cat's eye
//
//		©2020 Yuichiro Nakada
//---------------------------------------------------------

// clang -Os gpgpu_gl4.c -o gpgpu_gl4 `pkg-config --libs --cflags gl egl gbm` -lglfw
// dnf install mesa-libgbm-devel libdrm-devel mesa-libGL-devel mesa-libGLU-devel mesa-libEGL-devel mesa-libGLES-devel glfw-

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
//#define GLFW_INCLUDE_GLU

#ifdef _WIN32
	#include <windows.h>
	#include <GL/glew.h>
	#include <GLFW/glfw3.h>
#elif __APPLE__
	#include <OpenGL/gl3.h>
	#include <GLFW/glfw3.h>
#elif __linux
	#ifdef GPGPU_USE_GLES
		#include <EGL/egl.h>
		#include <EGL/eglext.h>
		#include <GLES3/gl32.h>
		#include <GLES3/gl3ext.h>
		//#define GL_CLAMP_TO_BORDER	GL_CLAMP_TO_BORDER_OES
	#else
		#include <GL/gl.h>
		#include <GLFW/glfw3.h>
	#endif
	//#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#define _STRGF(x)	# x
#define STRINGIFY(x)	_STRGF(x)

GLuint coCreateShaderProgram(const char* shader_src)
{
	GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(shader, 1, &shader_src, 0);
	glCompileShader(shader);

	GLint compiles = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiles);
	if (compiles == GL_FALSE) {
		GLint log_length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
		char *info_log = calloc(log_length, sizeof(char));
		glGetShaderInfoLog(shader, log_length, &log_length, info_log);
		glDeleteShader(shader);

		printf("%s\n", info_log);
		printf("\n%s\n", shader_src);
		free(info_log);
	}

	GLuint program = glCreateProgram();

	glAttachShader(program, shader);
	glLinkProgram(program);

	GLint links = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &links);
	if (links == GL_FALSE) {
		GLint log_length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
		char *info_log = calloc(log_length, sizeof(char));
		glGetProgramInfoLog(program, log_length, &log_length, info_log);

		glDeleteProgram(program);
		glDeleteShader(shader);

		printf("%s\n", info_log);
		free(info_log);
	}

	glDetachShader(program, shader);
	glDeleteShader(shader);

	return program;
}
void coDeleteProgram(GLuint program)
{
	glDeleteProgram(program);
}

GLuint uniform_params;
GLuint ssbo[3];
void coCreateBuffer(GLuint program, int *size, int num)
{
	uniform_params = glGetUniformLocation(program, "param");
	glGenBuffers(3, ssbo);
	for (int i=0; i<num; i++) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[i]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size[i], 0, GL_DYNAMIC_COPY);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
}
void coDeleteBuffer()
{
	glDeleteBuffers(3, ssbo);
}
void coRun(GLuint program, int x, int y, int z, int *param)
{
	glUseProgram(program);

	glUniform1iv(uniform_params, 16, param);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo[0]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo[1]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo[2]);

	glDispatchCompute(x, y, z);

	glUseProgram(0);
}

void coRead(int n, int size, void *data)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[n]);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
//	GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
//	memcpy(p, &shader_data, sizeof(shader_data));
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
void coWrite(int n, int size, void *data)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[n]);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void coInit()
{
	if (!glfwInit()) {
		printf("Can't initialize GLFW.\n");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	GLFWwindow *window = glfwCreateWindow(1, 1, "invisible", 0, 0);
	if (!window) {
		glfwTerminate();
		//assert(!"glfwCreateWindow error!");
	}
	glfwMakeContextCurrent(window);

#ifdef _WIN32
#pragma comment(lib, "glew32.lib")
	int r = glewInit();
	if (r != GLEW_OK) {
		printf("error at glewInit!! (%s)\n", glewGetErrorString(r));
	}
#endif
	printf("%s: OpenGL %s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));
}

void coTerm()
{
	glfwTerminate();
}

