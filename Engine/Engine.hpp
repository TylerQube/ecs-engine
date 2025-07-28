#include "Renderer/Renderer.h"
#include "Entity/EntityManager.h"

class Engine
{
    std::shared_ptr<Renderer> renderer;
    // input manager, etc.

    std::shared_ptr<EntityManager> entityManager;
    std::shared_ptr<ComponentManager> componentManager;
    std::shared_ptr<SystemManager> systemManager;

    int currentFrame = 0;
    bool running = true;
    bool paused = false;

    void init();
};