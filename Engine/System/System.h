#pragma once
#include <set>
#include "Engine/Types.hpp"

class System
{
public:
    void update(float dt);
    std::set<Entity> entities;

    virtual void onKeyEvent(KeyCode key, KeyAction action) = 0;
};