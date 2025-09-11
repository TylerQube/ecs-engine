#include "Types.hpp"
#include "Engine/System/System.h"

class InputManager
{
private:
    std::vector<std::shared_ptr<System>> subscribers;

public:
    void keyCallback(KeyCode key, KeyAction action)
    {
        for (auto sub : subscribers)
        {
            sub->onKeyEvent(key, action);
        }
    }

    void mouseCallback(double xpos, double ypos)
    {
        for (auto sub : subscribers)
        {
            sub->onMouseEvent(xpos, ypos);
        }
    }

    void subscribe(std::shared_ptr<System> system)
    {
        subscribers.push_back(system);
    }
};