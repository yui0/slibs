// g++ -o compute compute.cpp -Os -lglfw -lGL
#include <iostream>
#include <vector>

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#define GLFW_INCLUDE_GLU
#include <GL/gl.h>
//#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace {

const char* compute_shader_source = R"(
#version 430

uniform uint element_size;

layout(std430, binding = 3) buffer layout_dst
{
    float dst[];
};

layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

void main() {
    uint index = gl_GlobalInvocationID.x;
    if (index >= element_size) { return; }

    dst[index] = mix(0.0, 3.141592653589, float(index) / element_size);
}
)";

void initOpenGL() {
    auto inits_glfw = glfwInit();
    if (inits_glfw != GLFW_TRUE) {
        throw std::runtime_error("error occurred: glfwInit!");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(1, 1, "invisible window", nullptr, nullptr);
    if (window == nullptr) {
        throw std::runtime_error("error occurred: glfwCreateWindow!");
    }
    glfwMakeContextCurrent(window);

    /*auto inits_glew = glewInit();
    if (inits_glew != GLEW_OK) {
        throw std::runtime_error("error occurred: glewInit!");
    }*/
}

void terminateOpenGL() {
    glfwTerminate();
}


GLuint createComputeShaderProgram(const char* shader_src) {
    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader, 1, &shader_src, nullptr);
    glCompileShader(shader);

    GLint compiles = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiles);
    if (compiles == GL_FALSE) {
        GLint log_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<GLchar> info_log(log_length);
        glGetShaderInfoLog(shader, log_length, &log_length, info_log.data());

        glDeleteShader(shader);

        std::string error_msg = "error occurred in compiling shader: ";
        throw std::runtime_error(error_msg + info_log.data());
    }

    GLuint program = glCreateProgram();

    glAttachShader(program, shader);
    glLinkProgram(program);

    GLint links = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &links);
    if (links == GL_FALSE) {
        GLint log_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<GLchar> info_log(log_length);
        glGetProgramInfoLog(program, log_length, &log_length, info_log.data());

        glDeleteProgram(program);
        glDeleteShader(shader);

        std::string error_msg = "error occurred in linking shader: ";
        throw std::runtime_error(error_msg + info_log.data());
    }

    glDetachShader(program, shader);
    glDeleteShader(shader);

    return program;
}

void deleteComputeShaderProgram(GLuint program) {
    glDeleteProgram(program);
}


void compute() {
    uint32_t num = 10000;

    GLuint shader_program = createComputeShaderProgram(compute_shader_source);

    // create buffer
    GLuint uniform_element_size = glGetUniformLocation(shader_program, "element_size");
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, num * sizeof(float), nullptr, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glUseProgram(shader_program);

    glUniform1ui(uniform_element_size, num);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);

    glDispatchCompute(num / 256 + 1, 1, 1);

    glUseProgram(0);

    std::vector<float> data(num);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, num * sizeof(float), data.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    for (auto v : data) { std::cout << v << '\n'; }

    glDeleteBuffers(1, &ssbo);

    deleteComputeShaderProgram(shader_program);
}

}

int main(int argc, char* argv[]) {
    try {
        initOpenGL();

        compute();

        terminateOpenGL();
    }
    catch (std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
}
