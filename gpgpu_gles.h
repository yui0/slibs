//---------------------------------------------------------
//	Cat's eye
//
//		©2023 Yuichiro Nakada
//---------------------------------------------------------

#include <stdio.h>
#include <assert.h>

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

//#define DEBUG
#ifdef DEBUG
#define GPU_CHECK() gpu_checkError(__LINE__);
#define debug(fmt, ... ) \
	fprintf(stderr, \
		"[%s] %s:%u # " fmt "\n", \
		__DATE__, __FILE__, \
		__LINE__, ##__VA_ARGS__ \
	)
#else
#define GPU_CHECK()
#define debug( fmt, ... ) ((void)0)
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

GLuint gpu_load_shader(GLenum shaderType, const char* pSource)
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
	GLuint fragmentShader = gpu_load_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
	if (!fragmentShader) return 0;
	GLuint vertexShader = gpu_load_shader(GL_VERTEX_SHADER, src);
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

#ifndef GPGPU_USE_GLES
void gpu_init()
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
/*	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	GLFWwindow *window = glfwCreateWindow(320, 240, "Catgl", 0, 0);
	if (!window) {
		glfwTerminate();
		assert(!"glfwCreateWindow error!");
	}
	glfwMakeContextCurrent(window);
	glDisable(GL_DEPTH_TEST);

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
void gpu_term()
{
	glfwTerminate();
}

#else

int32_t gpu_fd;
struct gbm_device *gpu_gbm;
EGLDisplay gpu_egl_dpy;
EGLContext gpu_core_ctx;

void gpu_init()
{
	/*bool*/int res;
	int32_t gpu_fd = open("/dev/dri/renderD128", O_RDWR);
	assert(gpu_fd > 0);

	gpu_gbm = gbm_create_device(gpu_fd);
	assert(gpu_gbm != NULL);

	/* setup EGL from the GBM device */
	gpu_egl_dpy = eglGetPlatformDisplay(EGL_PLATFORM_GBM_MESA, gpu_gbm, NULL);
	assert(gpu_egl_dpy != NULL);

	res = eglInitialize(gpu_egl_dpy, NULL, NULL);
	assert(res);

	const char *egl_extension_st = eglQueryString(gpu_egl_dpy, EGL_EXTENSIONS);
	assert(strstr(egl_extension_st, "EGL_KHR_create_context") != NULL);
	assert(strstr(egl_extension_st, "EGL_KHR_surfaceless_context") != NULL);

	static const EGLint config_attribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
		EGL_NONE
	};
	EGLConfig cfg;
	EGLint count;

	res = eglChooseConfig(gpu_egl_dpy, config_attribs, &cfg, 1, &count);
	assert(res);

	res = eglBindAPI(EGL_OPENGL_ES_API);
	assert(res);

	static const EGLint attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 3,
		EGL_NONE
	};
	gpu_core_ctx = eglCreateContext(gpu_egl_dpy, cfg, EGL_NO_CONTEXT, attribs);
	assert(gpu_core_ctx != EGL_NO_CONTEXT);

	res = eglMakeCurrent(gpu_egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, gpu_core_ctx);
	assert(res);
}
void gpu_term()
{
	eglDestroyContext(gpu_egl_dpy, gpu_core_ctx);
	eglTerminate(gpu_egl_dpy);
	gbm_device_destroy(gpu_gbm);
	close(gpu_fd);
}
#endif
