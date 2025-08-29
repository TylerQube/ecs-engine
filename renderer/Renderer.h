#pragma once
#include "Engine/Component/Renderable.h"

class Renderer
{
public:
    virtual void uploadMesh(WorldMesh* mesh) = 0;
    virtual void renderMesh(WorldMesh* mesh) = 0;

    void setViewMatrix(glm::mat4 view);
    void setProjectionMatrix(glm::mat4 projection);
    void setModelMatrix(glm::mat4 model);

    float getAspectRatio() { return (float)WIDTH / (float)HEIGHT; }

protected:
    int WIDTH;
    int HEIGHT;
};