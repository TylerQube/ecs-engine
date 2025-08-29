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

    std::unordered_map<std::string, std::shared_ptr<RenderMesh>> meshes;

    void init();
    void destroy();

    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);

    void uploadMesh(WorldMesh* cmesh);

    void useShader(unsigned int shaderId);

public:
    OpenGLRenderer(const char *title, unsigned int width, unsigned int height);
    GLFWwindow* get_window();

    unsigned int createShader(const char* vertexPath, const char* fragmentPath);
    void renderMesh(WorldMesh* mesh) override;

    void setViewMatrix(glm::mat4 view);
    void setProjectionMatrix(glm::mat4 projection);
    void setModelMatrix(glm::mat4 model);

    void run();
};