#pragma once

#include "Engine/Component/Model.h"

struct RenderMesh {
    unsigned int VBO;
    unsigned int EBO;
    unsigned int VAO;
    int index_count;
};