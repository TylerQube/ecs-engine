#pragma once
#include "Engine/Engine.hpp"
#include "Engine/System/System.h"
#include "Engine/Types.hpp"
#include "Engine/Util/Collision.hpp"
#include <cmath>
#include <set>

#include "Game/Component/EnemyAI.h"

class EnemyAISystem : public System {
  public:
    Engine *engine;
    Entity player;
    void init(Engine &c, Entity p) {
        this->engine = &c;
        this->player = p;
    }

    void update(float dt) {

        for (Entity entity : entities) {
            updateEnemy(entity, dt);
        }
    }

  private:
    // Wraps a degree-space angle into (-180, 180].
    static float wrapAngle(float a) {
        while (a <= -180.0f)
            a += 360.0f;
        while (a > 180.0f)
            a -= 360.0f;
        return a;
    }

    void updateEnemy(Entity enemy, float dt) {
        auto &aiInfo = engine->getComponent<EnemyAI>(enemy);
        std::cout << "eState: " << EnemyStateToString(aiInfo.state) << std::endl;
        std::cout << "time: " << aiInfo.timeSinceSeenPlayer << std::endl;
        auto &eTransform = engine->getComponent<Transform>(enemy);
        bool hasLoS;
        switch (aiInfo.state) {
        case EnemyState::Passive: {
            // raycast for player
            hasLoS = seesPlayer(enemy);
            if (hasLoS) {
                aiInfo.state = EnemyState::Following;
                break;
            }
            eTransform.velocity = glm::vec3(0.0f);
            // rotationSpeed is degrees/sec, yaw is degrees -> units already match.
            eTransform.rotation.yaw += aiInfo.rotationSpeed * dt;
            break;
        }
        case EnemyState::Following: {
            // raycast to maintain player vision
            // transition to passive if player LoS lost
            // transition to attacking if within range of player
            hasLoS = seesPlayer(enemy);
            if (!hasLoS) {
                aiInfo.timeSinceSeenPlayer += dt;
                bool forgotPlayer = aiInfo.timeSinceSeenPlayer > aiInfo.lostPlayerCooldown;
                std::cout << "forgot: " << forgotPlayer << std::endl;
                if (forgotPlayer) {
                    aiInfo.state = EnemyState::Passive;
                    aiInfo.timeSinceSeenPlayer = -1;
                    break;
                }
            }

            Transform playerTransform = engine->getComponent<Transform>(player);
            glm::vec3 toPlayerDir = glm::normalize(playerTransform.position - eTransform.position);
            toPlayerDir.y = 0.0f;

            // compute target yaw (rotation around Y) from enemy to player, in degrees
            // to match eTransform.rotation.yaw's units.
            float targetYaw = glm::degrees(std::atan2(toPlayerDir.x, toPlayerDir.z));

            float currentYaw = wrapAngle(eTransform.rotation.yaw);
            float desiredYaw = wrapAngle(targetYaw);
            float diff = wrapAngle(desiredYaw - currentYaw);

            const float epsilon = 1.0f;                // stop within 1 degree
            float maxStep = aiInfo.rotationSpeed * dt; // degrees/sec * sec = degrees
            eTransform.velocity = toPlayerDir * aiInfo.moveSpeed;
            if (std::fabs(diff) <= epsilon) {
                eTransform.rotation.yaw = desiredYaw;
            } else {
                float step = (std::fabs(diff) < maxStep) ? diff : (diff > 0.0f ? maxStep : -maxStep);
                eTransform.rotation.yaw = wrapAngle(currentYaw + step);
            }
            break;
        }
        }
    }

    const float numRays = 100;
    bool seesPlayer(Entity enemy) {
        Transform playerTransform = engine->getComponent<Transform>(player);
        Transform eTransform = engine->getComponent<Transform>(enemy);

        AABB pAABBLocal = engine->getComponent<AABB>(player);
        AABB eAABBLocal = engine->getComponent<AABB>(enemy);

        // AABBs are stored relative to their transform -> convert to world space.
        AABB pAABB{pAABBLocal.min + playerTransform.position, pAABBLocal.max + playerTransform.position};
        AABB eAABB{eAABBLocal.min + eTransform.position, eAABBLocal.max + eTransform.position};

        glm::vec3 rayOrigin = eTransform.position;
        rayOrigin.y = eAABB.min.y + (eAABB.max.y - eAABB.min.y) * 0.5f;

        glm::vec3 targetPoint = playerTransform.position;
        targetPoint.y = pAABB.min.y + (pAABB.max.y - pAABB.min.y) * 0.5f;

        glm::vec3 toPlayerDir = glm::normalize(targetPoint - rayOrigin);
        toPlayerDir.y = 0.0f;
        auto &aiInfo = engine->getComponent<EnemyAI>(enemy);
        if (glm::length2(targetPoint - rayOrigin) > pow(aiInfo.viewDistance, 2)) {
            return false;
        }

        const float halfFov = glm::radians(aiInfo.fovAngle * 0.5f);
        for (int i = 0; i < numRays; i++) {
            const float t = (numRays <= 1) ? 0.5f : static_cast<float>(i) / static_cast<float>(numRays - 1);
            const float angle = -halfFov + (2.0f * halfFov * t);
            const float c = cos(angle);
            const float s = sin(angle);
            glm::vec3 rayDir(toPlayerDir.x * c - toPlayerDir.z * s, 0.0f, toPlayerDir.x * s + toPlayerDir.z * c);
            rayDir = glm::normalize(rayDir);
            rayDir.y = 1e-6f; // avoid exact-zero direction on the Y axis (NaN in the slab test)

            Ray ray{rayOrigin, rayDir, 1.0f / rayDir};

            if (rayIntersectAABB(ray, pAABB)) // now testing against the WORLD-SPACE box
                return true;
        }
        return false;
    }
};