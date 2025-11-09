#pragma once
#include <Engine/Engine.hpp>
#include <iostream>
#include <Engine/Component/Enemy.h>

class EnemySystem : public System
{
public:
    Engine *engine;
    Entity *player;
    void init(Engine &c, Entity &player)
    {
        this->engine = &c;
        this->player = &player;
    }

    void update(float dt)
    {
        if (this->engine == nullptr)
        {
            std::cerr << "EnemySystem called before initialized" << std::endl;
            return;
        }
        for (Entity entity : entities)
        {
            auto &stats = engine->getComponent<Enemy>(entity);
            auto &transform = engine->getComponent<Transform>(entity);
            auto rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(transform.rotation.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
            auto lookDir = glm::vec3(rotationMatrix * glm::vec4(stats.eye, 1.0f));

            auto &pTransform = engine->getComponent<Transform>(*player);
            glm::vec3 toPlayer = glm::normalize(pTransform.position - transform.position);
            toPlayer.y = 0;

            glm::vec3 move = toPlayer * stats.speed * dt;

            float angleDiff = atan2(pTransform.position.x, pTransform.position.z) - atan2(transform.position.x, transform.position.z);

            float rotSpeed = 100.0f;
            float turnSign = angleDiff > 0 ? -1.0f : 1.0f;
            std::cout << turnSign << std::endl;

            float deltaYaw = std::min(rotSpeed * dt, angleDiff);
            std::cout << deltaYaw * turnSign << std::endl;
            transform.rotation.yaw += deltaYaw * turnSign;

            move.y = 0.0f;
            transform.position += move;
        }
    }
};