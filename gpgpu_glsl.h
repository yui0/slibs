//---------------------------------------------------------
//	Cat's eye
//
//		©2017-2018 Yuichiro Nakada
//---------------------------------------------------------

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#define GLFW_INCLUDE_GLU

#ifdef _MSC_VER
#define strdup	_strdup
#endif
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
	#include <unistd.h>
#endif

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>


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
\n
precision highp float;\n
#endif
\n

attribute vec3 pos;
attribute vec2 tex;
varying vec2   uv;

void main(void)
{
	gl_Position = vec4(pos, 1.0);
	uv = tex;
}\n

);
GLuint __vertexShader;

void coPrintShaderInfo(GLuint shader, const char *str)
{
	int logSize, length;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
	if (logSize > 1) {
		//GLchar infoLog[logSize];
		GLchar infoLog[8192];
		glGetShaderInfoLog(shader, logSize, &length, infoLog);
		debug("Compile Error in %s\n%s\n", str, infoLog);
	}
}

GLuint coLoadShader(GLenum shaderType, const char* pSource)
{
	char *src = strdup(pSource);
	char *p = src;
	while (*p) {
		switch (*p++) {
		case '{':
		case ';':
			if (*p==0x20) *p = '\n';
//			if (*p==0x20 && *(p+1)!='i') *p = '\n';
		}
	}

	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, (const char**)&src, 0);
	glCompileShader(shader);
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) coPrintShaderInfo(shader, src);

	free(src);
	return shader;
}

GLuint coCreateProgram(const char* pFragmentSource)
{
#ifdef _WIN32
	if (!glCreateProgram) printf("glCreateProgram is NULL!! (OpenGL ver:%s)\n", glGetString(GL_VERSION));
#endif
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

#define GPGPU_TEX_PADDING	1
#define GPGPU_TEX_REPEAT	2
GLuint coCreateDataTexture(int w, int h, void *texels, GLuint type, int flag)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

#ifndef GPGPU_USE_GLES
	glTexImage2D(GL_TEXTURE_2D, 0, (type==GL_FLOAT ? GL_RGBA32F : GL_RGBA), w, h, 0, GL_RGBA, type, texels);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, type, texels);
#endif

	GLuint clamp = GL_CLAMP_TO_EDGE;
	if (flag & GPGPU_TEX_PADDING) clamp = GL_CLAMP_TO_BORDER;	// 0 padding
	if (flag & GPGPU_TEX_REPEAT) clamp = GL_REPEAT;

	// clamp to edge to support non-power of two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);
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

GLuint coBindOutputTexture(int N, int M, GLuint texture)
{
	// set canvas and viewport size
	glViewport(0, 0, N, M);

	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, /*level*/0);

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

void *coReadData(int N, int M, void *d)
{
	if (!d) d = malloc(4*M*N);
	glReadPixels(0, 0, N, M, GL_RGBA, GL_UNSIGNED_BYTE, d);
	return d; // M x N
}

float *coReadDataf(int N, int M, float *d)
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

#ifndef GPGPU_USE_GLES
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

#ifndef GPGPU_USE_GLES
void coInit()
{
	if (!glfwInit()) {
		printf("Can't initialize GLFW.\n");
	}
#ifdef _WIN32
//#include <GL/glut.h>
/*	int argc = 1;
	char argv[2][256] = {0};
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(320, 240);
	glutCreateWindow(argv[0]);*/
#endif
//	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_VISIBLE, 0);
	GLFWwindow *window = glfwCreateWindow(320, 240, "Catgl", 0, 0);
	if (!window) {
		glfwTerminate();
		assert(!"glfwCreateWindow error!");
	}
	glfwMakeContextCurrent(window);

#ifdef _WIN32
//#include <glad/glad.h>
//	if (!gladLoadGL()) { exit(-1); }
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

#else

int32_t __fd;
struct gbm_device *__gbm;
EGLDisplay __egl_dpy;
EGLContext __core_ctx;

void coInit()
{
	/*bool*/int res;
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
