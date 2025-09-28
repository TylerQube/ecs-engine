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

    float epsilon = 0.01f;
    void update(float dt)
    {
        if (this->engine == nullptr)
        {
            std::cerr << "ColliderSystem called before initialized" << std::endl;
            return;
        }
        for (Entity entity : entities)
        {
            auto &transform = engine->getComponent<Transform>(entity);
            if (glm::length(transform.velocity) < 0.0001f)
            {
                continue;
            }

            bool hasAABB = engine->hasComponent<AABB>(entity);
            if (!hasAABB)
                continue;

            for (Entity other : entities)
            {
                if (entity == other)
                    continue;
                bool canCollide = engine->hasComponent<AABB>(entity) && engine->hasComponent<AABB>(other);
                if (!hasAABB)
                    continue;

                auto collision = aabbCollisionTime(entity, other, dt);
                if (collision.time < 1.0f)
                {
                    auto moveToImpact = transform.velocity * collision.time;

                    auto remaining = transform.velocity - moveToImpact;
                    float remainingTime = 1.0f - collision.time;
                    auto remainingMove = remaining * remainingTime;

                    float dot = glm::dot(remainingMove, collision.normal);
                    auto slideMove = remainingMove - collision.normal * dot;

                    transform.velocity = moveToImpact + slideMove;
                    transform.velocity += collision.normal * epsilon;
                }
            }
        }
    }

private:
    struct Collision
    {
        float time;
        glm::vec3 normal;
    };

    Collision aabbCollisionTime(Entity &a, Entity &b, float dt)
    {
        auto &transformA = engine->getComponent<Transform>(a);
        auto &transformB = engine->getComponent<Transform>(b);
        auto va = transformA.velocity * dt;
        auto vb = transformB.velocity * dt;

        auto &aabbA = engine->getComponent<AABB>(a);
        auto &aabbB = engine->getComponent<AABB>(b);

        auto a0 = transformA.position + aabbA.min;
        auto a1 = transformA.position + aabbA.max;
        auto b0 = transformB.position + aabbB.min;
        auto b1 = transformB.position + aabbB.max;
        auto aToB = b0 - a0;

        float dxEntry, dxExit;
        float dyEntry, dyExit;
        float dzEntry, dzExit;

        if (va.x > 0.0f)
        {
            dxEntry = b0.x - a1.x;
            dxExit = b1.x - a0.x;
        }
        else
        {
            dxEntry = b1.x - a0.x;
            dxExit = b0.x - a1.x;
        }

        if (va.y > 0.0f)
        {
            dyEntry = b0.y - a1.y;
            dyExit = b1.y - a0.y;
        }
        else
        {
            dyEntry = b1.y - a0.y;
            dyExit = b0.y - a1.y;
        }

        if (va.z > 0.0f)
        {
            dzEntry = b0.z - a1.z;
            dzExit = b1.z - a0.z;
        }
        else
        {
            dzEntry = b1.z - a0.z;
            dzExit = b0.z - a1.z;
        }

        float txEntry, txExit;
        float tyEntry, tyExit;
        float tzEntry, tzExit;

        if (va.x == 0.0f)
        {
            txEntry = -std::numeric_limits<float>::infinity();
            txExit = std::numeric_limits<float>::infinity();
        }
        else
        {
            txEntry = dxEntry / va.x;
            txExit = dxExit / va.x;
        }

        if (va.y == 0.0f)
        {
            tyEntry = -std::numeric_limits<float>::infinity();
            tyExit = std::numeric_limits<float>::infinity();
        }
        else
        {
            tyEntry = dyEntry / va.y;
            tyExit = dyExit / va.y;
        }

        if (va.z == 0.0f)
        {
            tzEntry = -std::numeric_limits<float>::infinity();
            tzExit = std::numeric_limits<float>::infinity();
        }
        else
        {
            tzEntry = dzEntry / va.z;
            tzExit = dzExit / va.z;
        }

        float entryTime = std::max({txEntry, tyEntry, tzEntry});
        float exitTime = std::min({txExit, tyExit, tzExit});

        if (entryTime > exitTime || (txEntry < 0.0f && tyEntry < 0.0f && tzEntry < 0.0f) || txEntry > 1.0f || tyEntry > 1.0f || tzEntry > 1.0f)
            return Collision{
                1.0f,
                glm::vec3(0.0f)};

        auto normal = glm::vec3(0.0f);
        if (entryTime == txEntry)
        {
            normal.x = (va.x > 0.0f) ? -1.0f : 1.0f;
        }
        else if (entryTime == tyEntry)
        {
            normal.y = (va.y > 0.0f) ? -1.0f : 1.0f;
        }
        else if (entryTime == tzEntry)
        {
            normal.z = (va.z > 0.0f) ? -1.0f : 1.0f;
        }

        return Collision{
            entryTime,
            normal};
    }
};