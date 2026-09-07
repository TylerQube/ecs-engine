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
    void updateEnemy(Entity enemy, float dt) {
        auto &aiInfo = engine->getComponent<EnemyAI>(enemy);
        std::cout << "eState: " << EnemyStateToString(aiInfo.state) << std::endl;
        auto &eTransform = engine->getComponent<Transform>(enemy);
        bool hasLoS;
        switch (aiInfo.state) {
        case EnemyState::Passive:
            // raycast for player
            hasLoS = seesPlayer(enemy);
            if (hasLoS)
                aiInfo.state = EnemyState::Following;
            eTransform.velocity = glm::vec3(0.0f);
            eTransform.rotation.yaw += aiInfo.rotationSpeed * dt;
            break;
        case EnemyState::Following:
            // raycast to maintain player vision
            // transition to passive if player LoS lost
            // transition to attacking if within range of player
            hasLoS = seesPlayer(enemy);
            if (!hasLoS) {
                aiInfo.state = EnemyState::Passive;
                break;
            }
            Transform playerTransform = engine->getComponent<Transform>(player);
            glm::vec3 toPlayer = glm::normalize(playerTransform.position - eTransform.position);
            toPlayer.y = 0.0f;
            // compute target yaw (rotation around Y) from enemy to player
            float targetYaw = std::atan2(toPlayer.x, toPlayer.z); // note: atan2(x,z) matches yaw convention
            // normalize angles to [-PI, PI]
            auto wrapPi = [](float a) {
                const float PI = 3.14159265358979323846f;
                while (a <= -PI)
                    a += 2.0f * PI;
                while (a > PI)
                    a -= 2.0f * PI;
                return a;
            };

            float currentYaw = wrapPi(eTransform.rotation.yaw);
            float desiredYaw = wrapPi(targetYaw);
            float diff = wrapPi(desiredYaw - currentYaw);

            const float epsilon = glm::radians(1.0f); // stop within 1 degree
            float maxStep = aiInfo.rotationSpeed * dt;
            if (std::fabs(diff) <= 10.0f)
                eTransform.velocity = toPlayer * aiInfo.moveSpeed;
            if (std::fabs(diff) <= epsilon) {
                eTransform.rotation.yaw = desiredYaw;
            } else {
                float step = (std::fabs(diff) < maxStep) ? diff : (diff > 0.0f ? maxStep : -maxStep);
                eTransform.rotation.yaw = wrapPi(currentYaw + step);
            }
            break;
        }
    }

    const float numRays = 10;
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

        glm::vec3 toPlayer = glm::normalize(targetPoint - rayOrigin);
        toPlayer.y = 0.0f;
        auto &aiInfo = engine->getComponent<EnemyAI>(enemy);
        if(glm::length2(toPlayer) > pow(aiInfo.viewDistance, 2)) {
            std::cout << "out of range" << std::endl;
            return false;
        }

        const float halfFov = glm::radians(aiInfo.fovAngle * 0.5f);
        for (int i = 0; i < numRays; i++) {
            const float t = (numRays <= 1) ? 0.5f : static_cast<float>(i) / static_cast<float>(numRays - 1);
            const float angle = -halfFov + (2.0f * halfFov * t);
            const float c = cos(angle);
            const float s = sin(angle);
            glm::vec3 rayDir(toPlayer.x * c - toPlayer.z * s, 0.0f, toPlayer.x * s + toPlayer.z * c);
            rayDir = glm::normalize(rayDir);
            rayDir.y = 1e-6f; // avoid exact-zero direction on the Y axis (NaN in the slab test)

            Ray ray{rayOrigin, rayDir, 1.0f / rayDir};

            if (rayIntersectAABB(ray, pAABB)) // now testing against the WORLD-SPACE box
                return true;
        }
        return false;
    }
};