#pragma once

#include "Renderer/Renderer.h"
#include "Component/Transform.h"
#include "Component/Camera.h"

#include "System/RenderSystem.hpp"
#include "System/CameraSystem.hpp"

#include "Coordinator.hpp"

class Engine
{
private:
    int currentFrame = 0;
    bool running = true;
    bool paused = false;
    std::shared_ptr<Coordinator> coordinator;

public:
    void init()
    {
        coordinator = std::make_unique<Coordinator>();
        coordinator->init();
    }

    void run()
    {
        coordinator->registerComponent<Transform>();
        coordinator->registerComponent<Renderable>();
        coordinator->registerComponent<Camera>();

        auto renderSystem = coordinator->registerSystem<RenderSystem>();
        Signature signature;
        signature.set(coordinator->getComponentId<Transform>());
        signature.set(coordinator->getComponentId<Renderable>());
        coordinator->setSignature<RenderSystem>(signature);

        renderSystem->init(*coordinator);

        auto cameraSystem = coordinator->registerSystem<CameraSystem>();
        signature.set(coordinator->getComponentId<Transform>());
        signature.set(coordinator->getComponentId<Camera>());
        coordinator->setSignature<RenderSystem>(signature);

        cameraSystem->init(*coordinator);

        while (true)
        {
            renderSystem->update();
            cameraSystem->update();
        }
    }
};