#include "../IPlatform.h"
#include "glad/glad.h"
#include "glfw/glfw3.h"
#include <cassert>

class OpenGLPlatform : public IPlatform
{
private:
    GLFWwindow *window;
    static bool initialized;
    void init();

    void destroy();

    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
public:
    OpenGLPlatform(char *title, unsigned int width, unsigned int height);
    GLFWwindow* get_window();

    void run();
};