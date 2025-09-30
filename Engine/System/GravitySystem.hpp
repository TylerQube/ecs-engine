#pragma once
#include <Engine/Engine.hpp>
#include <Engine/Component/Transform.h>
#include <Engine/Component/Gravity.h>
#include <iostream>

class GravitySystem : public System
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
            std::cerr << "GravitySystem called before initialized" << std::endl;
            return;
        }
        for (Entity entity : entities)
        {
            auto &transform = engine->getComponent<Transform>(entity);
            auto &gravity = engine->getComponent<Gravity>(entity);
            transform.acceleration += gravity.direction;
        }
    }
};