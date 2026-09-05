#pragma once

#include "RenderMesh.h"
#include "Renderer/Renderer.h"
#include <cassert>
#include <unordered_map>
#include <memory>

struct GLFWwindow;

class OpenGLRenderer : public Renderer
{
private:
    GLFWwindow *window;
    static bool initialized;
    unsigned int activeShader = 0;

    std::function<void(KeyCode, KeyAction)> engineKeyCallback = nullptr;
    std::function<void(double, double)> engineMouseCallback = nullptr;

    std::unordered_map<std::string, std::shared_ptr<RenderMesh>> meshes;

    void init();
    void destroy();

    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);

    void uploadMesh(WorldMesh *cmesh) override;
    void useShader(unsigned int shaderId);

    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void dispatchKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    void mouse_callback(GLFWwindow *window, double xpos, double ypos);
    static void dispatchMouseCallback(GLFWwindow *window, double xpos, double ypos);

    glm::mat4 view;
    glm::mat4 model;
    glm::mat4 projection;
    
    void updateShaderMatrices();

public:
    OpenGLRenderer(const char *title, unsigned int width, unsigned int height);
    GLFWwindow *get_window();

    unsigned int loadShader(const char *vertexPath, const char *fragmentPath) override;
    void renderMesh(WorldMesh *mesh, unsigned int shaderId) override;

    void setViewMatrix(glm::mat4 view) override;
    void setProjectionMatrix(glm::mat4 projection) override;
    void setModelMatrix(glm::mat4 model) override;

    void setUniform(unsigned int shaderId, const std::string& name, const glm::mat4 matrix) override;
    void setUniform(unsigned int shaderId, const std::string& name, const glm::vec3 vector) override;
    void setUniform(unsigned int shaderId, const std::string& name, const glm::vec4 vector) override;
    void setUniform(unsigned int shaderId, const std::string& name, float value) override;
    void setUniform(unsigned int shaderId, const std::string& name, int value) override;
    void setUniform(unsigned int shaderId, const std::string& name, unsigned int value) override;

    int beginFrame() override;
    void endFrame() override;

    float getTime() override;

    void registerKeyCallback(std::function<void(KeyCode, KeyAction)> callback) override;
    void registerMouseCallback(std::function<void(double, double)> callback) override;

    void setMouseCapture(bool capture) override;

    unsigned int loadTextureFromFile(const char *path) override;
};