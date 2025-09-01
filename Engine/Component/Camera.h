#pragma once

#include <glm/vec3.hpp>

struct Camera {
    glm::vec3 front;
    glm::vec3 up;
    float sensitivity;
    float zoom;
};