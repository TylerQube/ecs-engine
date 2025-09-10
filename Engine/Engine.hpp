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
    float lastFrame = 0;
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
        signature.reset();
        signature.set(coordinator->getComponentId<Transform>());
        signature.set(coordinator->getComponentId<Camera>());
        coordinator->setSignature<CameraSystem>(signature);

        coordinator->subscribeSystemToInput(cameraSystem);

        cameraSystem->init(*coordinator);

        Entity player = coordinator->createEntity("player");
        auto playerTransform = Transform{glm::vec3(-5.0f, 0.5f, 0.0f),
                                         glm::vec3(0.0f),
                                         glm::vec3(0.0f),
                                         glm::vec3(1.0f)};
        coordinator->addComponent(player, playerTransform);
        auto playerCamera = Camera{glm::vec3(0.0f, 0.0f, 1.0f),
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
        WorldMesh mesh;
        mesh.shaderId = coordinator->loadShader("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
        mesh.vertices = {
            {{-0.5f, 0.0f, 0.5f}},
            {{0.5f, 1.0f, 0.5f}},
            {{0.0f, 0.0f, 0.0f}},
        };
        mesh.indices = {0, 1, 2};
        mesh.name = "wallMesh";
        wallRenderable.meshes.push_back(mesh);

        coordinator->addComponent(wall, wallRenderable);

        while (true)
        {

            float currentFrame = coordinator->getTime();
            float deltaTime = currentFrame - lastFrame;
            if(coordinator->startFrame() == -1) break;

            cameraSystem->update(deltaTime);
            renderSystem->update(deltaTime);

            coordinator->endFrame();


            lastFrame = currentFrame;
        }
    }
};