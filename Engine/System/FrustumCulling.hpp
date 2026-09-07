#pragma once

#include <array>
#include <algorithm>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Engine/Component/Camera.h"
#include "Engine/Component/Model.h"
#include "Engine/Component/Transform.h"

namespace FrustumCulling {

static inline void normalizePlane(glm::vec4 &plane) {
    float length = glm::length(glm::vec3(plane));
    if (length > 0.0f) {
        plane /= length;
    }
}

static inline std::array<glm::vec4, 6> extractFrustumPlanes(const glm::mat4 &viewProjection) {
    glm::mat4 clip = glm::transpose(viewProjection);
    std::array<glm::vec4, 6> planes = {clip[3] + clip[0], clip[3] - clip[0], clip[3] + clip[1], clip[3] - clip[1],
                                       clip[3] + clip[2], clip[3] - clip[2]};

    for (auto &plane : planes) {
        normalizePlane(plane);
    }

    return planes;
}

static inline glm::mat4 buildModelMatrix(const Transform &transform) {
    glm::mat4 modelMatrix(1.0f);
    modelMatrix = glm::translate(modelMatrix, transform.position);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(transform.rotation.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(transform.rotation.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(transform.rotation.roll), glm::vec3(0.0f, 0.0f, 1.0f));
    modelMatrix = glm::scale(modelMatrix, transform.scale);
    return modelMatrix;
}

static inline bool computeModelBounds(Model &model) {
    if (model.hasLocalBounds) {
        return true;
    }

    bool hasVertex = false;
    glm::vec3 minBounds(0.0f);
    glm::vec3 maxBounds(0.0f);

    for (const auto &mesh : model.meshes) {
        for (const auto &vertex : mesh.vertices) {
            if (!hasVertex) {
                minBounds = vertex.Position;
                maxBounds = vertex.Position;
                hasVertex = true;
                continue;
            }

            minBounds.x = std::min(minBounds.x, vertex.Position.x);
            minBounds.y = std::min(minBounds.y, vertex.Position.y);
            minBounds.z = std::min(minBounds.z, vertex.Position.z);
            maxBounds.x = std::max(maxBounds.x, vertex.Position.x);
            maxBounds.y = std::max(maxBounds.y, vertex.Position.y);
            maxBounds.z = std::max(maxBounds.z, vertex.Position.z);
        }
    }

    if (!hasVertex) {
        return false;
    }

    model.localBoundsMin = minBounds;
    model.localBoundsMax = maxBounds;
    model.hasLocalBounds = true;
    return true;
}

static inline glm::vec3 transformedAABBExtent(const glm::mat4 &modelMatrix, const glm::vec3 &extents) {
    return glm::vec3(std::abs(modelMatrix[0][0]) * extents.x + std::abs(modelMatrix[1][0]) * extents.y +
                         std::abs(modelMatrix[2][0]) * extents.z,
                     std::abs(modelMatrix[0][1]) * extents.x + std::abs(modelMatrix[1][1]) * extents.y +
                         std::abs(modelMatrix[2][1]) * extents.z,
                     std::abs(modelMatrix[0][2]) * extents.x + std::abs(modelMatrix[1][2]) * extents.y +
                         std::abs(modelMatrix[2][2]) * extents.z);
}

static inline bool isAABBOutsideFrustum(const glm::vec3 &worldMin, const glm::vec3 &worldMax,
                                        const std::array<glm::vec4, 6> &planes) {
    for (const auto &plane : planes) {
        glm::vec3 positive = worldMin;
        if (plane.x >= 0.0f)
            positive.x = worldMax.x;
        if (plane.y >= 0.0f)
            positive.y = worldMax.y;
        if (plane.z >= 0.0f)
            positive.z = worldMax.z;

        if (glm::dot(glm::vec3(plane), positive) + plane.w < 0.0f) {
            return true;
        }
    }

    return false;
}

static inline bool isModelCulled(Model &model, const glm::mat4 &modelMatrix, const std::array<glm::vec4, 6> &planes) {
    if (!computeModelBounds(model)) {
        return false;
    }

    glm::vec3 localCenter = (model.localBoundsMin + model.localBoundsMax) * 0.5f;
    glm::vec3 localExtents = (model.localBoundsMax - model.localBoundsMin) * 0.5f;

    glm::vec3 worldCenter = glm::vec3(modelMatrix * glm::vec4(localCenter, 1.0f));
    glm::vec3 worldExtents = transformedAABBExtent(modelMatrix, localExtents);

    glm::vec3 worldMin = worldCenter - worldExtents;
    glm::vec3 worldMax = worldCenter + worldExtents;
    return isAABBOutsideFrustum(worldMin, worldMax, planes);
}

static inline std::array<glm::vec4, 6> cameraFrustum(const Camera &camera, const Transform &transform, float aspectRatio) {
    glm::mat4 view = glm::lookAt(transform.position, transform.position + camera.front, camera.up);
    glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), aspectRatio, 0.1f, 200.0f);
    return extractFrustumPlanes(projection * view);
}

} // namespace FrustumCulling