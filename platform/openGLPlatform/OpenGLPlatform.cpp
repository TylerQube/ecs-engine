#include "../IPlatform.h"
#include "OpenGLPlatform.h"
#include <cassert>
#include <iostream>

#include "glad/glad.h"
#include "glfw/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

void OpenGLPlatform::init()
{
    if (initialized)
        return;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    stbi_set_flip_vertically_on_load(true);

    initialized = true;
}

void OpenGLPlatform::destroy()
{
    glfwTerminate();
}

void OpenGLPlatform::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

OpenGLPlatform::OpenGLPlatform(char *title, unsigned int width, unsigned int height)
{
    if (!initialized)
        init();
    window =
        glfwCreateWindow(width, height, title, NULL, NULL);
    assert(window && "Failed to create GLFW window");

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);

    int load = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    if(!load) return;
    glEnable(GL_DEPTH_TEST);
    stbi_set_flip_vertically_on_load(true);
}

GLFWwindow* OpenGLPlatform::get_window()
{
    return window;
}

void OpenGLPlatform::run() {
    std::cout << "Running render loop" << std::endl;
    while(!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();    
    }
    destroy();
}

bool OpenGLPlatform::initialized = false;