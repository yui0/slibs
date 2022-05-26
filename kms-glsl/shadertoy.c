#include <stdio.h>
#include <GLFW/glfw3.h>
#include <GLES3/gl3.h>
#include <GL/glext.h>

#define USE_GLFW
#include "shadertoy.h"

const int WIDTH  = 320;
const int HEIGHT = 240;

static void error_callback(int e, const char *d)
{
	printf("Error %d: %s\n", e, d);
}

int main(int argc, char *argv[])
{
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
//	st_load_images_and_bind(texture_files);

	info("Press [ESC] or [q] to exit.\n");
	info("Run with --help flag for more information.\n\n");

	glfwSetTime(0.0);
	while (!glfwWindowShouldClose(window)) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, 1);
		}

		glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

//		glUseProgram(shader_program);
		st_update_keystate();
		st_render(glfwGetTime());
		st_partially_reset_keystate();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}
