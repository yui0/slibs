// gcc -o compute compute.c -Os -lglfw -lGL
#include <stdio.h>
#include <stdlib.h>

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#define GLFW_INCLUDE_GLU
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#define _STRGF(x)	# x
#define STRINGIFY(x)	_STRGF(x)

const char* compute_shader_source = "#version 430" STRINGIFY(
\n
uniform uint element_size;

layout(std430, binding = 3) buffer layout_dst {
	float dst[];
};

layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

void main()
{
	uint index = gl_GlobalInvocationID.x;
	if (index >= element_size) {
		return;
	}

	dst[index] = mix(0.0, 3.141592653589, float(index) / element_size);
}
);

void gpu_init()
{
	if (!glfwInit()) {
		printf("Can't initialize GLFW.\n");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(1, 1, "invisible window", 0, 0);
	if (!window) {
		printf("error occurred: glfwCreateWindow!\n");
	}
	glfwMakeContextCurrent(window);
}
void gpu_term()
{
	glfwTerminate();
}

GLuint loadShader(GLenum shaderType, const char* pSource)
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

GLuint createComputeProgram(const char* pComputeSource)
{
	GLuint computeShader = loadShader(GL_COMPUTE_SHADER, pComputeSource);
	if (!computeShader) {
		return 0;
	}

	GLuint program = glCreateProgram();
	if (program) {
		glAttachShader(program, computeShader);
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
	}
	return program;
}

void compute()
{
	uint32_t num = 10000;

	GLuint shader_program = createComputeProgram(compute_shader_source);

	// create buffer
	GLuint uniform_element_size = glGetUniformLocation(shader_program, "element_size");
	GLuint ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, num * sizeof(float), NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glUseProgram(shader_program);

	glUniform1ui(uniform_element_size, num);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);

	glDispatchCompute(num / 256 + 1, 1, 1);

	glUseProgram(0);

	float data[num];
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, num * sizeof(float), data);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	for (int i=0; i<num; i++) {
		printf("%f ", data[i]);
	}

	glDeleteBuffers(1, &ssbo);

	glDeleteProgram(shader_program);
}

int main(int argc, char* argv[])
{
	gpu_init();
	compute();
	gpu_term();
}
