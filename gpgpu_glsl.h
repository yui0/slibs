//---------------------------------------------------------
//	Cat's eye
//
//		©2017 Yuichiro Nakada
//---------------------------------------------------------

#define GPGPU_USE_GLFW

#ifdef GPGPU_USE_GLFW

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#else

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>
//#define GL_CLAMP_TO_BORDER	GL_CLAMP_TO_BORDER_OES
#endif

#include <assert.h>
#include <fcntl.h>
#include <gbm.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


//#ifndef _DEBUG_H_
//#define _DEBUG_H_

#ifdef NDEBUG
	#define debug( fmt, ... ) ((void)0)
#else /* !NDEBUG */
	#include <stdio.h>
	#define debug(fmt, ... ) \
		fprintf(stderr, \
			"[%s] %s:%u # " fmt "\n", \
			__DATE__, __FILE__, \
			__LINE__, ##__VA_ARGS__ \
		)
#endif /* NDEBUG */

//#endif /* _DEBUG_H_ */


#define _STRGF(x)	# x
#define STRINGIFY(x)	_STRGF(x)

// vertex
char pass_through[] = STRINGIFY(

#ifdef GL_ES
precision highp float;
#endif

attribute vec3 pos;
attribute vec2 tex;
varying vec2   uv;

void main(void)
{
	gl_Position = vec4(pos, 1.0);
	uv = tex;
}

);
GLuint __vertexShader;

void coPrintShaderInfo(GLuint shader, const char *str)
{
	int logSize, length;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
	if (logSize > 1) {
		GLchar infoLog[logSize];
		glGetShaderInfoLog(shader, logSize, &length, infoLog);
		debug("Compile Error in %s\n%s\n", str, infoLog);
	}
}

GLuint coLoadShader(GLenum shaderType, const char* pSource)
{
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &pSource, 0);
	glCompileShader(shader);
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) coPrintShaderInfo(shader, pSource);
	return shader;
}

GLuint coCreateProgram(const char* pFragmentSource)
{
	if (!__vertexShader) __vertexShader = coLoadShader(GL_VERTEX_SHADER, pass_through);
	GLuint pixelShader = coLoadShader(GL_FRAGMENT_SHADER, pFragmentSource);
	GLuint program = glCreateProgram();
	glAttachShader(program, __vertexShader);
	glAttachShader(program, pixelShader);
	glDeleteShader(pixelShader);
	glLinkProgram(program);
	return program;
}

void coBindVertices(GLuint prog)
{
	glUseProgram(prog);

	// bind vertices
	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	float vertices[] = {		// define a square that covers the screen
		-1.0, -1.0, 0.0,	// bottom left
		 1.0, -1.0, 0.0,	// bottom right
		 1.0,  1.0, 0.0,	// top right
		-1.0,  1.0, 0.0 };	// top left
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), (void*)vertices, GL_STATIC_DRAW);
	GLuint position = glGetAttribLocation(prog, "pos");
	glVertexAttribPointer(position, /*item size*/3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position);

	// bind texture cords
	GLuint texCoords;
	glGenBuffers(1, &texCoords);
	glBindBuffer(GL_ARRAY_BUFFER, texCoords);
	float textureCoords[] = {
		0.0, 0.0,
		1.0, 0.0,
		1.0, 1.0,
		0.0, 1.0 };
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), (void*)textureCoords, GL_STATIC_DRAW);
	GLuint texture = glGetAttribLocation(prog, "tex");
	glVertexAttribPointer(texture, /*item size*/2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(texture);

	// index to vertices
	GLuint indices;
	glGenBuffers(1, &indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
	// tesselate square into triangles
	// indeces into vertex array creating triangles, with counter-clockwise winding
	unsigned short vIndices[] = {
		0, 1, 2,	// bottom right triangle
		0, 2, 3 };	// top left triangle
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vIndices), (void*)vIndices, GL_STATIC_DRAW);
}

GLuint coCreateDataTexture(int h, int w, void *texels, GLuint type)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

#ifdef GPGPU_USE_GLFW
	glTexImage2D(GL_TEXTURE_2D, 0, (type==GL_FLOAT ? GL_RGBA32F : GL_RGBA), w, h, 0, GL_RGBA, type, texels);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, /*GL_UNSIGNED_BYTE*/type, texels);
#endif

	// clamp to edge to support non-power of two textures
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	// 0 padding
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	// don't interpolate when getting data from texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// we're done with setup, so unbind current texture
	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}

void coBindInputTexture(GLuint program, GLuint texture, GLuint textureUnit, char *name)
{
	glActiveTexture(textureUnit); // gl.TEXTURE0, gl.TEXTURE1, etc
	glBindTexture(GL_TEXTURE_2D, texture);

	GLuint sampler = glGetUniformLocation(program, name);
	glUniform1i(sampler, textureUnit - GL_TEXTURE0);
	assert(!glGetError());
}

GLuint coBindOutputTexture(int M, int N, GLuint texture)
{
	// set canvas and viewport size
	glViewport(0, 0, N, M);

	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, /*level*/0);

	/*GLuint renderbuffer;
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, N, M);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_RENDERBUFFER, renderbuffer);*/
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		debug("glGetError: %d", glGetError());
		debug("glCheckFramebufferStatus: %d", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		assert(!"Bound framebuffer is not complete.");
	}

	return framebuffer;
}

void coUnbindInputTexture(GLuint texture)
{
	glActiveTexture(texture);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void *coReadData(int M, int N, void *d)
{
	if (!d) d = malloc(4*M*N);
	glReadPixels(0, 0, N, M, GL_RGBA, GL_UNSIGNED_BYTE, d);
	return d; // M x N
}

float *coReadDataf(int M, int N, float *d)
{
/*#ifdef GPGPU_USE_GLFW
	if (!d) d = malloc(sizeof(float)*4*M*N);
	glReadPixels(0, 0, N, M, GL_RGBA32F, GL_FLOAT, d);
#else*/
	if (!d) d = malloc(sizeof(float)*4*M*N);
	//glReadPixels(0, 0, N*4, M, GL_RGBA, GL_FLOAT, d);
	glReadPixels(0, 0, N, M, GL_RGBA, GL_FLOAT, d);
//#endif
	return d;
}

#define coCompute() {\
	glDrawElements(GL_TRIANGLES, 3*2, GL_UNSIGNED_SHORT, 0);\
}

#define coAssert() {\
	assert(!glGetError());\
}

#define coUniform1i(prog, name, v1) {\
	GLuint l = glGetUniformLocation(prog, name);\
	glUniform1i(l, v1);\
}
#define coUniform1f(prog, name, v1) {\
	GLuint l = glGetUniformLocation(prog, name);\
	glUniform1f(l, v1);\
}
#define coUniform2f(prog, name, v1, v2) {\
	GLuint l = glGetUniformLocation(prog, name);\
	glUniform2f(l, v1, v2);\
}
#define coUniform4f(prog, name, v1, v2, v3, v4) {\
	GLuint l = glGetUniformLocation(prog, name);\
	glUniform4f(l, v1, v2, v3, v4);\
}
#define coUniform1fv(prog, name, count, v1) {\
	GLuint l = glGetUniformLocation(prog, name);\
	glUniform1fv(l, count, v1);\
}
#define coUniform2fv(prog, name, count, v1) {\
	GLuint l = glGetUniformLocation(prog, name);\
	glUniform2fv(l, count, v1);\
}
#define coUniform4fv(prog, name, count, v1) {\
	GLuint l = glGetUniformLocation(prog, name);\
	glUniform4fv(l, count, v1);\
}
#define coUniformMatrix3fv(prog, name, v) {\
	GLuint l = glGetUniformLocation(prog, name);\
	glUniformMatrix3fv(l, 1, GL_FALSE, v);\
}

#ifdef GPGPU_USE_GLFW
#define coTransferData(texture, x, y, w, h, type, pix) {\
	glBindTexture(GL_TEXTURE_2D, texture);\
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, type, pix);\
	assert(!glGetError());\
	glBindTexture(GL_TEXTURE_2D, 0);\
}
#else
#define coTransferData(texture, x, y, w, h, type, pix) {\
	glBindTexture(GL_TEXTURE_2D, texture);\
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, type, pix);\
	assert(!glGetError());\
	glBindTexture(GL_TEXTURE_2D, 0);\
}
#endif

#ifdef GPGPU_USE_GLFW
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
void coInit()
{
	GLFWwindow* window;
	assert(glfwInit() != 0);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_VISIBLE, 0);
	window = glfwCreateWindow(320, 240, "Catgl", 0, 0);
	if (!window) {
		glfwTerminate();
		assert(!"glfwCreateWindow error!");
	}
	glfwMakeContextCurrent(window);
}

void coTerm()
{
}

#else

int32_t __fd;
struct gbm_device *__gbm;
EGLDisplay __egl_dpy;
EGLContext __core_ctx;

void coInit()
{
	bool res;
	int32_t __fd = open("/dev/dri/renderD128", O_RDWR);
	assert(__fd > 0);

	__gbm = gbm_create_device(__fd);
	assert(__gbm != NULL);

	/* setup EGL from the GBM device */
	__egl_dpy = eglGetPlatformDisplay(EGL_PLATFORM_GBM_MESA, __gbm, NULL);
	assert(__egl_dpy != NULL);

	res = eglInitialize(__egl_dpy, NULL, NULL);
	assert(res);

	const char *egl_extension_st = eglQueryString(__egl_dpy, EGL_EXTENSIONS);
	assert(strstr(egl_extension_st, "EGL_KHR_create_context") != NULL);
	assert(strstr(egl_extension_st, "EGL_KHR_surfaceless_context") != NULL);

	static const EGLint config_attribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
		EGL_NONE
	};
	EGLConfig cfg;
	EGLint count;

	res = eglChooseConfig(__egl_dpy, config_attribs, &cfg, 1, &count);
	assert(res);

	res = eglBindAPI(EGL_OPENGL_ES_API);
	assert(res);

	static const EGLint attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 3,
		EGL_NONE
	};
	__core_ctx = eglCreateContext(__egl_dpy, cfg, EGL_NO_CONTEXT, attribs);
	assert(__core_ctx != EGL_NO_CONTEXT);

	res = eglMakeCurrent(__egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, __core_ctx);
	assert(res);
}

void coTerm()
{
	eglDestroyContext(__egl_dpy, __core_ctx);
	eglTerminate(__egl_dpy);
	gbm_device_destroy(__gbm);
	close(__fd);
}
#endif
