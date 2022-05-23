#include <stdio.h>
#include <GLFW/glfw3.h>

#include "shadertoy.h"

//const int g_width  = 640;
//const int g_height = 480;
const int g_width  = 320;
const int g_height = 240;

int main(int argc, char *argv[])
{
	if (!glfwInit()) return -1;

/*	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);*/

	GLFWwindow* window = glfwCreateWindow(g_width, g_height, "Shadertoy", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	GLuint program = init_shadertoy(g_width, g_height, argv[1]);

	uint64_t start_time = get_time_ns();
	unsigned frame = 0;
//	float deltaTime = 0.0f; 
//	float lastFrame = 0.0f; 
	while (!glfwWindowShouldClose(window)) {
/*		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame; */

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, 1);
		}
		/*if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		}*/

		glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		draw_shadertoy(start_time, frame++);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}
