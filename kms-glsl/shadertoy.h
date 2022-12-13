// berry-dm
// Copyright © 2022 Yuichiro Nakada

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/mman.h>
#include <sys/stat.h>
//#include <GLES3/gl3.h>

// OpenGL ES 2.0 / GLSL 100

static const char common_shader_header_gles2[] =
    "#version 100\n"
    "precision highp float;";

static const char vertex_shader_body_gles2[] =
    "attribute vec4 iPosition;"
    "void main(){gl_Position=iPosition;}";

static const char fragment_shader_header_gles2[] =
    "uniform vec3 iResolution;"
    "uniform float iGlobalTime;" // legacy
    "uniform float iTime;"
    "uniform float iTimeDelta;"
    "uniform int   iFrame;"
    "uniform float iFrameRate;"
    "uniform float iChannelTime[4];"
    "uniform vec4 iMouse;"
    "uniform vec4 iDate;"
    "uniform float iSampleRate;"
    "uniform vec3 iChannelResolution[4];"
    "uniform sampler2D iChannel0;"
    "uniform sampler2D iChannel1;"
    "uniform sampler2D iChannel2;"
    "uniform sampler2D iChannel3;\n";

static const char fragment_shader_footer_gles2[] =
    "\nvoid main(){mainImage(gl_FragColor,gl_FragCoord.xy);}";

// OpenGL ES 3.0 / GLSL 300 es

static const char common_shader_header_gles3[] =
    "#version 300 es\n"
    "precision highp float;\n";

static const char vertex_shader_body_gles3[] =
    "layout(location = 0) in vec4 iPosition;"
    "void main() {"
    "  gl_Position=iPosition;"
    "}\n";

static const char fragment_shader_header_gles3[] =
    "uniform vec3 iResolution;"
    "uniform float iGlobalTime;" // legacy
    "uniform float iTime;"
    "uniform float iTimeDelta;"
    "uniform int   iFrame;"
    "uniform float iFrameRate;"
    "uniform float iChannelTime[4];"
    "uniform vec4 iMouse;"
    "uniform vec4 iDate;"
    "uniform float iSampleRate;"
    "uniform vec3 iChannelResolution[4];"
    "uniform sampler2D iChannel0;"
    "uniform sampler2D iChannel1;"
    "uniform sampler2D iChannel2;"
    "uniform sampler2D iChannel3;"
    "out vec4 fragColor;\n";

static const char fragment_shader_footer_gles3[] =
    "\nvoid main(){mainImage(fragColor, gl_FragCoord.xy);}";

// Standard ShaderToy Shader
static char *default_fragment_shader =
    "void mainImage( out vec4 fragColor, in vec2 fragCoord )"
    "{"
    "    vec2 uv = fragCoord/iResolution.xy;"
    "    vec3 col = 0.5 + 0.5*cos(iTime+uv.xyx+vec3(0,2,4));"
    "    fragColor = vec4(col,1.0);"
    "}";

static const char* common_shader_header = common_shader_header_gles3;
static const char* vertex_shader_body = vertex_shader_body_gles3;
static const char* fragment_shader_header = fragment_shader_header_gles3;
static const char* fragment_shader_footer = fragment_shader_footer_gles3;
static int gles_major = 3;
static int gles_minor = 0;

#ifdef USE_GLFW
static GLFWwindow *window;
#endif
static GLuint shader_program;
static GLint attrib_position;
static GLint sampler_channel[4];
static GLuint sampler_channel_ID[4];
static GLint uniform_cres;
static GLint uniform_ctime;
static GLint uniform_date;
static GLint uniform_gtime;
static GLint uniform_time;
static GLint iFrame;
static GLint uniform_mouse;
static GLint uniform_scroll;
static GLint uniform_res;
static GLint uniform_srate;

static GLfloat viewportSizeX = 0.0;
static GLfloat viewportSizeY = 0.0;
static GLfloat mouseX = 0.0;
static GLfloat mouseY = 0.0;
static GLfloat mouseLPressed = 0.0;
static GLfloat mouseRPressed = 0.0;

static int mouseUpdating = 0;
static int maximized = 0;

// what iChannel to bind the virtual keyboard to (argument -k)
static int bindKeyboard = -1;
// for storing the keyboard state in a ST-style texture
static unsigned char keyStateTextureData[256 * 3];
static GLuint keyStateTextureID;
// for translating to JS key codes
static int keyLookup[GLFW_KEY_LAST + 1];

static void die(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	exit(EXIT_FAILURE);
}

static void info(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
}

static GLuint compile_shader(GLenum type, GLsizei nsources, const char **sources)
{
	GLuint shader;
	GLint success, len;
	GLsizei srclens[nsources];
	char *log;

	for (int i=0; i < nsources; ++i) {
		srclens[i] = (GLsizei)strlen(sources[i]);
	}

	shader = glCreateShader(type);
	glShaderSource(shader, nsources, sources, srclens);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		if (len > 1) {
			log = malloc(len);
			glGetShaderInfoLog(shader, len, NULL, log);
			fprintf(stderr, "%s\n\n", log);
			free(log);
		}
		die("Error compiling shader.\n");
	}

	return shader;
}

/*static void select_gles2()
{
	info("Selected OpenGL ES 2.0 / GLSL 100\n");
	common_shader_header = common_shader_header_gles2;
	vertex_shader_body = vertex_shader_body_gles2;
	fragment_shader_header = fragment_shader_header_gles2;
	fragment_shader_footer = fragment_shader_footer_gles2;
	gles_major = 2;
	gles_minor = 0;
}

static void select_gles3()
{
	info("Selected OpenGL ES 3.0 / GLSL 300 es\n");
	common_shader_header = common_shader_header_gles3;
	vertex_shader_body = vertex_shader_body_gles3;
	fragment_shader_header = fragment_shader_header_gles3;
	fragment_shader_footer = fragment_shader_footer_gles3;
	gles_major = 3;
	gles_minor = 0;
}*/

static void update_texture(int n, unsigned char* data, int w, int h)
{
	glActiveTexture(GL_TEXTURE0 + n);
	glBindTexture(GL_TEXTURE_2D, sampler_channel_ID[n]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);
	glUniform1i(sampler_channel[n], n);
}
static void texture_bind(unsigned char* data, int w, int h)
{
	for (int i=/*0*/3; i<4; ++i) {
		if (sampler_channel[i] < 0) {
			info("Skipping data for unused iChannel%d\n", i);
		} else {
			info("Binding data for iChannel%d\n", i);
			glGenTextures(1, &sampler_channel_ID[i]);
			update_texture(i, data, 256, 3);
			break;
		}
	}
	glActiveTexture(GL_TEXTURE0);
}

#ifdef STBI_INCLUDE_STB_IMAGE_H
static void load_images_and_bind(const char* filenames[])
{
	for (int i=0; i<4; ++i) {
		if (filenames[i]) {
			if (sampler_channel[i] < 0) {
				info("Skipping image file for unused iChannel%d: %s\n", i, filenames[i]);
			} else {
				info("Loading image file for iChannel%d: %s\n", i, filenames[i]);
				GLuint tex_2d;
				int w, h, bpp;
				unsigned char* pixels = stbi_load(filenames[i], &w, &h, &bpp, 0);
				glGenTextures(1, &tex_2d);
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, tex_2d);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
				stbi_image_free(pixels);

				glUniform1i(sampler_channel[i], i);
			}
		}
	}
	glActiveTexture(GL_TEXTURE0);
}
#endif

#ifdef USE_GLFW
// GLFW functions from here
static void resize_viewport(GLFWwindow* window/*for callback*/, int w, int h)
{
	glUniform3f(uniform_res, (float)w, (float)h, 0.0f);
	glViewport(0, 0, w, h);
	info("Setting window size to (%d,%d).\n", w, h);
	viewportSizeX = w;
	viewportSizeY = h;
}

static void st_populate_key_lookup()
{
	// Convert GLFW key codes to Shadertoy / JS
	for (int i=0; i <= GLFW_KEY_LAST; ++i) {
		keyLookup[i] = -1;
	}
	for (int i = GLFW_KEY_0; i <= GLFW_KEY_9; ++i) {
		keyLookup[i] = i;
	}
	for (int i = GLFW_KEY_A; i <= GLFW_KEY_Z; ++i) {
		keyLookup[i] = i;
	}
	for (int i = GLFW_KEY_F1; i <= GLFW_KEY_F12; ++i) {
		keyLookup[i] = 112 + i - GLFW_KEY_F1;
	}
	for (int i = GLFW_KEY_KP_0; i <= GLFW_KEY_KP_9; ++i) {
		keyLookup[i] = 96 + i - GLFW_KEY_KP_0;
	}
	keyLookup[GLFW_KEY_SPACE]         = 32;
	keyLookup[GLFW_KEY_APOSTROPHE]    = 222;
	keyLookup[GLFW_KEY_COMMA]         = 188;
	keyLookup[GLFW_KEY_MINUS]         = 173;
	keyLookup[GLFW_KEY_PERIOD]        = 190;
	keyLookup[GLFW_KEY_SLASH]         = 191;
	keyLookup[GLFW_KEY_SEMICOLON]     = 59;
	keyLookup[GLFW_KEY_EQUAL]         = 61;
	keyLookup[GLFW_KEY_LEFT_BRACKET]  = 219;
	keyLookup[GLFW_KEY_BACKSLASH]     = 220;
	keyLookup[GLFW_KEY_RIGHT_BRACKET] = 221;
	keyLookup[GLFW_KEY_GRAVE_ACCENT]  = 192;
	keyLookup[GLFW_KEY_ESCAPE]        = 27;
	keyLookup[GLFW_KEY_ENTER]         = 13;
	keyLookup[GLFW_KEY_TAB]           = 9;
	keyLookup[GLFW_KEY_BACKSPACE]     = 8;
	keyLookup[GLFW_KEY_INSERT]        = 45;
	keyLookup[GLFW_KEY_DELETE]        = 46;
	keyLookup[GLFW_KEY_RIGHT]         = 39;
	keyLookup[GLFW_KEY_LEFT]          = 37;
	keyLookup[GLFW_KEY_DOWN]          = 40;
	keyLookup[GLFW_KEY_UP]            = 38;
	keyLookup[GLFW_KEY_PAGE_UP]       = 33;
	keyLookup[GLFW_KEY_PAGE_DOWN]     = 34;
	keyLookup[GLFW_KEY_HOME]          = 36;
	keyLookup[GLFW_KEY_END]           = 35;
	keyLookup[GLFW_KEY_CAPS_LOCK]     = 20;
	keyLookup[GLFW_KEY_SCROLL_LOCK]   = 145;
	keyLookup[GLFW_KEY_NUM_LOCK]      = 144;
	keyLookup[GLFW_KEY_PRINT_SCREEN]  = 42;
	keyLookup[GLFW_KEY_PAUSE]         = 19;
	keyLookup[GLFW_KEY_KP_DECIMAL]    = 110;
	keyLookup[GLFW_KEY_KP_DIVIDE]     = 111;
	keyLookup[GLFW_KEY_KP_MULTIPLY]   = 106;
	keyLookup[GLFW_KEY_KP_SUBTRACT]   = 109;
	keyLookup[GLFW_KEY_KP_ADD]        = 107;
	keyLookup[GLFW_KEY_KP_ENTER]      = 13;
	keyLookup[GLFW_KEY_LEFT_SHIFT]    = 16;
	keyLookup[GLFW_KEY_LEFT_CONTROL]  = 17;
	keyLookup[GLFW_KEY_LEFT_ALT]      = 18;
	keyLookup[GLFW_KEY_LEFT_SUPER]    = 91;
	keyLookup[GLFW_KEY_RIGHT_SHIFT]   = 16;
	keyLookup[GLFW_KEY_RIGHT_CONTROL] = 17;
	keyLookup[GLFW_KEY_RIGHT_ALT]     = 225;
	keyLookup[GLFW_KEY_RIGHT_SUPER]   = 91;
	keyLookup[GLFW_KEY_MENU]          = 93;
}

static void st_update_keystate()
{
	if (bindKeyboard >= 0) {
		glActiveTexture(GL_TEXTURE0 + bindKeyboard);
		glBindTexture(GL_TEXTURE_2D, keyStateTextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 256, 3, 0, GL_RED, GL_UNSIGNED_BYTE, keyStateTextureData);
		glUniform1i(sampler_channel[bindKeyboard], bindKeyboard);
	}
}

static void st_reset_keystate()
{
	memset(keyStateTextureData, 0, sizeof(keyStateTextureData));
}

static void st_partially_reset_keystate()
{
	// Reset 'just pressed' (= rising edge key event) flags
	for (int i=0; i<256; ++i) {
		keyStateTextureData[i + 256] = 0;
	}
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	printf("key %d action %d\n", key, action);
	if (action == GLFW_PRESS) {
		int js = keyLookup[key];
		if (js >= 0) {
			keyStateTextureData[js + 0] = 255;
			keyStateTextureData[js + 256] = 255;
			keyStateTextureData[js + 512] = keyStateTextureData[js + 512] ^ 0xff;
		}
		if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose(window, 1);
		} else if (key == GLFW_KEY_F) {
			if (maximized) {
				glfwRestoreWindow(window);
				maximized = 0;
			} else {
				glfwMaximizeWindow(window);
				maximized = 1;
			}
		}
	} else if (action == GLFW_RELEASE) {
		int js = keyLookup[key];
		if (js >= 0) {
			keyStateTextureData[js + 0] = 0;
			keyStateTextureData[js + 256] = 0;
		}
	}
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	mouseX = xpos;
	mouseY = viewportSizeY - ypos;
	if (mouseUpdating) {
		glUniform4f(uniform_mouse, mouseX, mouseY, mouseLPressed, mouseRPressed);
	}
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	printf("button %d action %d\n", button, action);
	if (button == 0) {
		mouseUpdating = action;
		mouseLPressed = action;
	} else if (button == 1) {
		mouseRPressed = action;
	}
	glUniform4f(uniform_mouse, mouseX, mouseY, mouseLPressed, mouseRPressed);
}

static void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	static double x = 0;
	static double y = 0;
	x += xoffset;
	y += yoffset;
	printf("scroll %f,%f\n", x, y);
	glUniform2f(uniform_scroll, x, y);
}
#endif

static char* read_file_into_str(const char *filename)
{
	long length = 0;
	char *result = NULL;
	FILE *file = fopen(filename, "r");
	if (file) {
		int status = fseek(file, 0, SEEK_END);
		if (status != 0) {
			fclose(file);
			return NULL;
		}
		length = ftell(file);
		status = fseek(file, 0, SEEK_SET);
		if (status != 0) {
			fclose(file);
			return NULL;
		}
		result = malloc((length+1) * sizeof(char));
		if (result) {
			size_t actual_length = fread(result, sizeof(char), length , file);
			result[actual_length++] = '\0';
		} 
		fclose(file);
		return result;
	}
	return NULL;
}

static void st_render(float abstime, uint64_t frame)
{
/*	static const GLfloat vertices[] = {
		-1.0f, -1.0f,
		1.0f, -1.0f,
		-1.0f, 1.0f,
		1.0f, 1.0f,
	};*/
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

	if (uniform_gtime >= 0) {
		glUniform1f(uniform_gtime, abstime);
	}
	if (uniform_time >= 0) {
		glUniform1f(uniform_time, abstime);
	}

	glUniform1ui(iFrame, frame);

//	glClearColor(0.0f, 0.0f, 0.0f, 1.0);
//	glClear(GL_COLOR_BUFFER_BIT);
	glEnableVertexAttribArray(attrib_position);
//	glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 0, vertices);
//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

GLuint st_init(int width, int height, const char *file)
{
	GLuint vtx, frag;
	const char *sources[4];
	char* log;
	GLint success, len;

	if (file) default_fragment_shader = read_file_into_str(file);

	sources[0] = common_shader_header;
	sources[1] = vertex_shader_body;
	vtx = compile_shader(GL_VERTEX_SHADER, 2, sources);

	sources[0] = common_shader_header;
	sources[1] = fragment_shader_header;
	sources[2] = default_fragment_shader;
	sources[3] = fragment_shader_footer;
	frag = compile_shader(GL_FRAGMENT_SHADER, 4, sources);

	shader_program = glCreateProgram();
	glAttachShader(shader_program, vtx);
	glAttachShader(shader_program, frag);
	glLinkProgram(shader_program);

	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &len);
		if (len > 1) {
			log = malloc(len);
			glGetProgramInfoLog(shader_program, len, &len, log);
			fprintf(stderr, "%s\n\n", log);
			free(log);
		}
		die("Error linking shader program.\n");
	}

	glDeleteShader(vtx);
	glDeleteShader(frag);
	glReleaseShaderCompiler();

	glUseProgram(shader_program);
	glValidateProgram(shader_program);

	iFrame = glGetUniformLocation(shader_program, "iFrame");
	attrib_position = glGetAttribLocation(shader_program, "iPosition");
	sampler_channel[0] = glGetUniformLocation(shader_program, "iChannel0");
	sampler_channel[1] = glGetUniformLocation(shader_program, "iChannel1");
	sampler_channel[2] = glGetUniformLocation(shader_program, "iChannel2");
	sampler_channel[3] = glGetUniformLocation(shader_program, "iChannel3");
	uniform_cres = glGetUniformLocation(shader_program, "iChannelResolution");
	uniform_ctime = glGetUniformLocation(shader_program, "iChannelTime");
	uniform_date = glGetUniformLocation(shader_program, "iDate");
	uniform_gtime = glGetUniformLocation(shader_program, "iGlobalTime");
	uniform_time = glGetUniformLocation(shader_program, "iTime");
	uniform_mouse = glGetUniformLocation(shader_program, "iMouse");
	uniform_scroll = glGetUniformLocation(shader_program, "iScroll");
	uniform_res = glGetUniformLocation(shader_program, "iResolution");
	uniform_srate = glGetUniformLocation(shader_program, "iSampleRate");

	// for iDate
	time_t timer;
	struct tm *local;
	timer = time(NULL);
	local = localtime(&timer);
	glUniform4f(uniform_date, local->tm_year+1900, local->tm_mon+1, local->tm_mday, local->tm_sec);

	resize_viewport(window, width, height);

#ifdef USE_GLFW
	glfwSetCursorPosCallback(window, &cursor_position_callback);
	glfwSetMouseButtonCallback(window, &mouse_button_callback);
	glfwSetScrollCallback(window, &mouse_scroll_callback);
	glfwSetKeyCallback(window, &key_callback);
	glfwSetFramebufferSizeCallback(window, &resize_viewport);
#endif

	bindKeyboard = 0;
	if (bindKeyboard >= 0) {
/*		if (texture_files[bindKeyboard] != NULL) {
			die("Error: cannot bind texture and keyboard to the same channel.\n");
		} else {*/
			info("Dynamic keyboard texture will be bound to iChannel%d.\n", bindKeyboard);
			glGenTextures(1, &keyStateTextureID);
//		}
	}

	return shader_program;
}
