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

    std::unordered_map<unsigned int, std::shared_ptr<RenderMesh>> meshes;

    void init();
    void destroy();

    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);

    void uploadMesh(WorldMesh* cmesh);

public:
    OpenGLRenderer(const char *title, unsigned int width, unsigned int height);
    GLFWwindow* get_window();

    void renderMesh(WorldMesh* mesh) override;

    void run();
};