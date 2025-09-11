#pragma once
#include <set>
#include "Engine/Types.hpp"

class System
{
public:
    void update(float dt);
    std::set<Entity> entities;

    virtual void onKeyEvent(KeyCode key, KeyAction action) = 0;
    virtual void onMouseEvent(double xpos, double ypos) = 0;
};