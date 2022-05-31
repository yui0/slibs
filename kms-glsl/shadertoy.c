#include <stdio.h>
#include <GLFW/glfw3.h>
#include <GLES3/gl3.h>
#include <GL/glext.h>

#define USE_GLFW
#include "shadertoy.h"

#define PARG_IMPLEMENTATION
#include "parg.h"

const int WIDTH  = 320;
const int HEIGHT = 240;

static void error_callback(int e, const char *d)
{
	printf("Error %d: %s\n", e, d);
}

int main(int argc, char *argv[])
{
	unsigned char data[256*3] = {0};
	const char* texture_files[4] = {0};

	struct parg_state ps;
	int c;

	parg_init(&ps);
	while ((c = parg_getopt(&ps, argc, argv, "h0:1:2:3:k:d:")) != -1) {
		switch (c) {
		case 'h':
			info(
			"\nUsage: shadertoy [OPTIONS]\n"
			"Example: shadertoy filename.glsl\n\n"
			"Options:\n"
			" -h, --help \t\tshows this help.\n"
			" -0, --texture0 <path> \tload texture for iChannel0\n"
			" -1, --texture1 <path> \tload texture for iChannel1\n"
			" -2, --texture2 <path> \tload texture for iChannel2\n"
			" -3, --texture3 <path> \tload texture for iChannel3\n"
			" -k, --keyboard <ch> \tbind keyboard to iChannel<ch>\n"
			);
			return 0;
		case '0':
			texture_files[0] = ps.optarg;
			break;
		case '1':
			texture_files[1] = ps.optarg;
			break;
		case '2':
			texture_files[2] = ps.optarg;
			break;
		case '3':
			texture_files[3] = ps.optarg;
			break;
		case 'k':
//			bindKeyboard = atoi(ps.optarg);
			break;
		case 'd':
			info("Bind texture for '%s'\n", ps.optarg);
			memcpy(data, ps.optarg, strlen(ps.optarg));
//			for (int i=0; i<256; i++) data[i]=i;
			break;
		}
	}

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		fprintf(stdout, "[GFLW] failed to init!\n");
		return 1;
	}

/*	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);*/

	window = glfwCreateWindow(WIDTH, HEIGHT, "Shadertoy", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
//	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	st_init(WIDTH, HEIGHT, argv[1]);
	st_populate_key_lookup();
	st_reset_keystate();
	texture_bind(data, 256, 3);
#ifdef STBI_INCLUDE_STB_IMAGE_H
	st_load_images_and_bind(texture_files);
#endif

	info("Press [ESC] or [q] to exit.\n");
	info("Run with --help flag for more information.\n\n");

	uint64_t frame = 0;
	glfwSetTime(0.0);
	while (!glfwWindowShouldClose(window)) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, 1);
		}

		glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

//		glUseProgram(shader_program);
		st_update_keystate();
		st_render(glfwGetTime(), frame++);
		st_partially_reset_keystate();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}
