#include <glm/vec3.hpp>
#include <vector>

struct Collider {};

struct SphereCollider {
    float radius;
};

struct Triangle {
    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;
};

struct MeshCollider {
    std::vector<Triangle> triangles;
};