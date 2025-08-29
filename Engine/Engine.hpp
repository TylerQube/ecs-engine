#include "Renderer/Renderer.h"
#include "Coordinator.hpp"
#include <Component/Transform.h>

class Engine
{
    int currentFrame = 0;
    bool running = true;
    bool paused = false;

    void init()
    {
        renderer = std::make_unique<Renderer>();

        coordinator = std::make_unique<Coordinator>();
        coordinator->init();

        setup();
    }

    void setup()
    {
        coordinator->registerComponent<Transform>();
        coordinator->registerComponent<Renderable>();
    }

    void tick()
    {
    }

private:
    std::shared_ptr<Renderer> renderer;
    std::shared_ptr<Coordinator> coordinator;
};