#pragma once

#include "Engine/Component/Collider.h"

struct Ray {
    glm::vec3 origin;
    glm::vec3 dir;
    glm::vec3 dir_inv;
};

bool rayIntersectAABB(Ray ray, AABB aabb) {
    // Compute intersection t values for all three slabs
    glm::vec3 t1 = (aabb.min - ray.origin) * ray.dir_inv;
    glm::vec3 t2 = (aabb.max - ray.origin) * ray.dir_inv;

    // Find the minimum and maximum parametric values for each axis
    glm::vec3 tmin_vec = glm::min(t1, t2);
    glm::vec3 tmax_vec = glm::max(t1, t2);

    // Determine the largest near intersection and smallest far intersection
    float tmin = std::max(std::max(tmin_vec.x, tmin_vec.y), tmin_vec.z);
    float tmax = std::min(std::min(tmax_vec.x, tmax_vec.y), tmax_vec.z);

    // If tmin > tmax, the ray misses the aabb; if tmax < 0, the aabb is behind the ray
    return tmax >= tmin && tmax >= 0.0f;
}