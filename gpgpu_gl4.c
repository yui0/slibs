//---------------------------------------------------------
//	Cat's eye
//
//		©2020 Yuichiro Nakada
//---------------------------------------------------------

// clang -Os gpgpu_gl4.c -o gpgpu_gl4 `pkg-config --libs --cflags gl egl gbm` -lglfw
// clang -Os gpgpu_gl4.c -o gpgpu_gl4 `pkg-config --libs --cflags glesv2 egl gbm` -lglfw
// dnf install mesa-libgbm-devel libdrm-devel mesa-libGL-devel mesa-libGLU-devel mesa-libEGL-devel mesa-libGLES-devel glfw-
#include "gpgpu_gl4.h"

const char compute_shader_source[] = STRINGIFY(

\n#version 430\n

uniform int param[16];

layout(std430, binding = 0) buffer layout_dst
{
    float dst[];
};

layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

void main() {
    uint index = gl_GlobalInvocationID.x;
    //dst[index] = index;
    if (index >= param[0]) return;

    dst[index] = mix(0.0, 3.141592653589, float(index) / param[0]);
}

);

void compute()
{
	uint32_t num = 100;

	GLuint shader_program = coCreateShaderProgram(compute_shader_source);

	// create buffer
	int size[] = {num * sizeof(float)};
	coCreateBuffer(shader_program, size, 1);

	int param[16];
	param[0] = num;
	coRun(shader_program, num / 256 + 1, 1, 1, param);

	float data[num];
	coRead(0, num * sizeof(float), data);
	for (int i=0; i<num; i++) printf("%f ", data[i]);

	coDeleteBuffer();
	coDeleteProgram(shader_program);
}

int main(int argc, char* argv[])
{
	coInit();

	compute();

	coTerm();
}
