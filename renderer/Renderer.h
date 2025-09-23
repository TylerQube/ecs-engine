#pragma once
#include "Engine/Component/Renderable.h"
#include <Engine/Types.hpp>

class Renderer
{
public:
    virtual void uploadMesh(WorldMesh *mesh) = 0;
    virtual void renderMesh(WorldMesh *mesh) = 0;

    virtual void setViewMatrix(glm::mat4 view) = 0;
    virtual void setProjectionMatrix(glm::mat4 projection) = 0;
    virtual void setModelMatrix(glm::mat4 model) = 0;

    virtual unsigned int loadShader(const char *vertexPath, const char *fragmentPath) = 0;

    float getAspectRatio() { return (float)WIDTH / (float)HEIGHT; }

    virtual int beginFrame() = 0;
    virtual void endFrame() = 0;

    virtual float getTime() = 0;

    virtual ~Renderer() = default;

    virtual void registerKeyCallback(std::function<void(KeyCode, KeyAction)> callback) = 0;
    virtual void registerMouseCallback(std::function<void(double, double)> callback) = 0;

    virtual void setMouseCapture(bool capture) = 0;

    virtual unsigned int loadTextureFromFile(const char *path) = 0;

protected:
    int WIDTH;
    int HEIGHT;
};