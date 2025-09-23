#pragma once
#include <Engine/Engine.hpp>
#include <Engine/Component/Collider.h>
#include <Engine/Component/Transform.h>

class ColliderSystem : public System
{
public:
    Engine *engine;
    void init(Engine &c)
    {
        this->engine = &c;
    }

    void update(float dt)
    {
        if(this->engine == nullptr) {
            std::cerr << "ColliderSystem called before initialized" << std::endl;
            return;
        }
        for (Entity entity : entities)
        {
            auto &transform = engine->getComponent<Transform>(entity);
            std::cout << "position: " << transform.position.x << ", " << transform.position.y << ", " << transform.position.z << std::endl;
            for (Entity other : entities)
            {
                if (entity == other)
                    continue;
                bool isSphere = engine->hasComponent<SphereCollider>(entity);
                bool otherIsSphere = engine->hasComponent<SphereCollider>(other);
                if (isSphere && otherIsSphere)
                {
                    transform.velocity = collideWithSphere(entity, other, dt);
                }
                bool isMesh = engine->hasComponent<MeshCollider>(entity);
            }
        }
    }

    glm::vec3 collideWithSphere(Entity a, Entity b, float dt)
    {
        auto &transform = engine->getComponent<Transform>(a);
        auto &otherTransform = engine->getComponent<Transform>(b);
        auto radius = engine->getComponent<SphereCollider>(a).radius;
        auto otherRadius = engine->getComponent<SphereCollider>(b).radius;

        auto allowedDist = radius + otherRadius;

        std::cout << "currentDist: " << glm::length(otherTransform.position - transform.position) << std::endl;

        std::cout << "velocity: " << transform.velocity.x << ", " << transform.velocity.y << ", " << transform.velocity.z << std::endl;
        auto delta = transform.velocity * dt;
        auto origin = transform.position;

        auto relDirToEntity = glm::normalize(otherTransform.position - origin);
        std::cout << "relDirToEntity: " << relDirToEntity.x << ", " << relDirToEntity.y << ", " << relDirToEntity.z << std::endl;
        std::cout << "delta : " << delta.x << ", " << delta.y << ", " << delta.z << std::endl;
        auto projDelta = glm::dot(delta, relDirToEntity);
        auto maxDeltaProj = glm::length(otherTransform.position - origin);

        std::cout << "projDelta: " << projDelta << ", maxDeltaProj: " << maxDeltaProj << std::endl;
        if (projDelta > maxDeltaProj)
        {
            std::cout << "Collision detected between " << a << " and " << b << std::endl;
            auto truncated = delta - relDirToEntity * (projDelta - maxDeltaProj);
            return truncated / dt;
        }
        std::cout << "No collision detected between " << a << " and " << b << std::endl;
        return transform.velocity;
    }

    glm::vec3 collideWithMesh(Entity a, Entity b, float dt)
    {
        auto transform = engine->getComponent<Transform>(a);
        auto collider = engine->getComponent<MeshCollider>(a);

        auto origin = transform.position;
        auto movement = transform.velocity * dt;
        auto dest = origin + movement;
        return transform.velocity;
    }
};