#pragma once
#include "Engine/Component/Renderable.h"

class Renderer
{
public:
    virtual void uploadMesh(WorldMesh* mesh) = 0;
    virtual void renderMesh(WorldMesh* mesh) = 0;

    virtual void setViewMatrix(glm::mat4 view) = 0;
    virtual void setProjectionMatrix(glm::mat4 projection) = 0;
    virtual void setModelMatrix(glm::mat4 model) = 0;

    float getAspectRatio() { return (float)WIDTH / (float)HEIGHT; }

    virtual void run() = 0;

protected:
    int WIDTH;
    int HEIGHT;
};