#include <glm/glm.hpp>

struct Rotation {
    float yaw;
    float pitch;
    float roll;
};

struct Transform {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    Rotation rotation;
    glm::vec3 scale;
};