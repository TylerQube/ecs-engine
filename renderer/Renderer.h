#pragma once
#include "Engine/Component/Renderable.h"

class Renderer
{
public:
    virtual void renderMesh(WorldMesh* mesh) = 0;
};