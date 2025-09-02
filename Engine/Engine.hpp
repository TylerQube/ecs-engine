#pragma once

#include "Renderer/Renderer.h"
#include "Component/Transform.h"
#include "Component/Camera.h"

#include "System/RenderSystem.hpp"
#include "System/CameraSystem.hpp"

#include "Coordinator.hpp"
#include "Types.hpp"

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

        Entity player = coordinator->createEntity("player");
        auto playerTransform = Transform{glm::vec3(0.0f, 0.0f, 3.0f),
                                         glm::vec3(0.0f),
                                         glm::vec3(0.0f),
                                         glm::vec3(1.0f)};
        coordinator->addComponent(player, playerTransform);
        auto playerCamera = Camera{glm::vec3(0.0f, 0.0f, -1.0f),
                                   glm::vec3(0.0f, 1.0f, 0.0f),
                                   0.1f,
                                   45.0f};
        coordinator->addComponent(player, playerCamera);

        Entity wall = coordinator->createEntity("wall");
        auto wallTransform = Transform{glm::vec3(0.0f, 0.0f, 0.0f),
                                       glm::vec3(0.0f),
                                       glm::vec3(0.0f),
                                       glm::vec3(1.0f)};
        coordinator->addComponent(wall, wallTransform);
        Renderable wallRenderable;
        wallRenderable.meshes.push_back(
            {.shaderId = coordinator->loadShader("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl"),
             .vertices = {
                 // positions          // normals           // texture coords
                 {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {-1, -1, -1, -1}, {0.0f, 0.0f, 0.0f, 0.0f}},
                 {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {-1, -1, -1, -1}, {0.0f, 0.0f, 0.0f, 0.0f}},
                 {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {-1, -1, -1, -1}, {0.0f, 0.0f, 0.0f, 0.0f}},
                 {{-0.5f, 0.5f, 0.2f}, {1.2f, 2.3f, 3.4f}, {2.2f, 3.3f}, {1.2f, 2.3f, 3.4f}, {4.5f, 5.6f, 6.7f}, {-1, -1, -1, -1}, {0.0f, 0.0f, 0.0f, 0.0f}},
             },
             .indices = {0, 1, 2, 2, 3, 0},
             .textures = {},
             .name = "wallMesh"});
        coordinator->addComponent(wall, wallRenderable);

        while (true)
        {
            std::cout << "tick" << std::endl;
            renderSystem->update();
            cameraSystem->update();

            int render = coordinator->render();
            if (render == -1)
                break;
        }
    }
};