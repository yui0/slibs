#include <err.h>
#include <time.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <GLES3/gl3.h>

#define NSEC_PER_SEC (INT64_C(1000) * USEC_PER_SEC)
#define USEC_PER_SEC (INT64_C(1000) * MSEC_PER_SEC)
#define MSEC_PER_SEC INT64_C(1000)

int create_program(const char *vs_src, const char *fs_src)
{
	GLuint vertex_shader, fragment_shader, program;
	GLint ret;

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertex_shader, 1, &vs_src, NULL);
	glCompileShader(vertex_shader);

	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &ret);
	if (!ret) {
		char *log;

		printf("vertex shader compilation failed!:\n");
		glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &ret);
		if (ret > 1) {
			log = malloc(ret);
			glGetShaderInfoLog(vertex_shader, ret, NULL, log);
			printf("%s", log);
			free(log);
		}

		return -1;
	}

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(fragment_shader, 1, &fs_src, NULL);
	glCompileShader(fragment_shader);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &ret);
	if (!ret) {
		char *log;

		printf("%s\n", fs_src);
		printf("fragment shader compilation failed!:\n");
		glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &ret);

		if (ret > 1) {
			log = malloc(ret);
			glGetShaderInfoLog(fragment_shader, ret, NULL, log);
			printf("%s", log);
			free(log);
		}

		return -1;
	}

	program = glCreateProgram();

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	return program;
}

int link_program(unsigned program)
{
	GLint ret;

	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &ret);
	if (!ret) {
		char *log;

		printf("program linking failed!:\n");
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &ret);

		if (ret > 1) {
			log = malloc(ret);
			glGetProgramInfoLog(program, ret, NULL, log);
			printf("%s", log);
			free(log);
		}

		return -1;
	}

	return 0;
}

uint64_t get_time_ns()
{
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	return tv.tv_nsec + tv.tv_sec * NSEC_PER_SEC;
}


GLint iTime, iFrame;

static const char *shadertoy_vs =
	"attribute vec3 position;                \n"
	"void main()                             \n"
	"{                                       \n"
	"    gl_Position = vec4(position, 1.0);  \n"
	"}                                       \n";

static const char *shadertoy_fs_tmpl =
	"#version 130                                                                         \n"
	"precision mediump float;                                                             \n"
	"uniform vec2      iResolution;           // viewport resolution (in pixels)          \n"
	"uniform float     iTime;                 // shader playback time (in seconds)        \n"
	"uniform int       iFrame;                // current frame number                     \n"
	"uniform vec4      iMouse;                // mouse pixel coords                       \n"
	"uniform vec4      iDate;                 // (year, month, day, time in seconds)      \n"
	"uniform float     iSampleRate;           // sound sample rate (i.e., 44100)          \n"
	"uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)           \n"
	"uniform float     iChannelTime[4];       // channel playback time (in sec)           \n"
	"                                                                                     \n"
	"%s                                                                                   \n"
	"                                                                                     \n"
	"void main()                                                                          \n"
	"{                                                                                    \n"
	"    mainImage(gl_FragColor, gl_FragCoord.xy);                                        \n"
	"}                                                                                    \n";

static const GLfloat vertices[] = {
	// First triangle:
	1.0f, 1.0f,
	-1.0f, 1.0f,
	-1.0f, -1.0f,
	// Second triangle:
	-1.0f, -1.0f,
	1.0f, -1.0f,
	1.0f, 1.0f,
};

static char *load_shader(const char *file)
{
	struct stat statbuf;
	char *frag;
	int fd, ret;

	fd = open(file, 0);
	if (fd < 0) {
		err(fd, "could not open '%s'", file);
	}

	ret = fstat(fd, &statbuf);
	if (ret < 0) {
		err(ret, "could not stat '%s'", file);
	}

	const char *text = mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	asprintf(&frag, shadertoy_fs_tmpl, text);

	return frag;
}

static void draw_shadertoy(uint64_t start_time, unsigned frame)
{
	glUniform1f(iTime, (get_time_ns() - start_time) / (double) NSEC_PER_SEC);
	// Replace the above to input ellapsed time relative to 60 FPS
	// glUniform1f(iTime, (float) frame / 60.0f);
	glUniform1ui(iFrame, frame);

//	start_perfcntrs();

	glDrawArrays(GL_TRIANGLES, 0, 6);

//	end_perfcntrs();
}

GLuint init_shadertoy(int width, int height, const char *file)
{
	int ret;
	GLuint program, vbo;
	GLint iResolution;

	char *shadertoy_fs = load_shader(file);
	ret = create_program(shadertoy_vs, shadertoy_fs);
	free(shadertoy_fs);
	if (ret < 0) {
		printf("failed to create program\n");
		return 0;
	}
	program = ret;

	ret = link_program(program);
	if (ret) {
		printf("failed to link program\n");
		return 0;
	}

	glViewport(0, 0, width, height);
	glUseProgram(program);
	iTime = glGetUniformLocation(program, "iTime");
	iFrame = glGetUniformLocation(program, "iFrame");
	iResolution = glGetUniformLocation(program, "iResolution");
	glUniform2f(iResolution, width, height);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *) (intptr_t) 0);
	glEnableVertexAttribArray(0);

	return program;
}
