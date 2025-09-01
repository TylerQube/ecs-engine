#pragma once

#include "Engine/Component/Renderable.h"

struct RenderMesh {
    unsigned int VBO;
    unsigned int EBO;
    unsigned int VAO;
};

RenderMesh* makeMesh(const WorldMesh& worldMesh);
