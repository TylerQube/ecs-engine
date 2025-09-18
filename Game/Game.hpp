#pragma once

#include "Renderer/Renderer.h"
#include "Engine/Component/Transform.h"
#include "Engine/Component/Camera.h"

#include "Engine/System/RenderSystem.hpp"
#include "Engine/System/CameraSystem.hpp"

#include "Engine/Engine.hpp"
#include "Engine/Types.hpp"

class Game
{
private:
    float lastFrame = 0;
    bool running = true;
    bool paused = false;
    std::shared_ptr<Engine> engine;

public:
    void init()
    {
        engine = std::make_unique<Engine>();
        engine->init();
    }

    void run()
    {
        engine->registerComponent<Transform>();
        engine->registerComponent<Renderable>();
        engine->registerComponent<Camera>();

        auto renderSystem = engine->registerSystem<RenderSystem>();
        Signature signature;
        signature.set(engine->getComponentId<Transform>());
        signature.set(engine->getComponentId<Renderable>());
        engine->setSignature<RenderSystem>(signature);

        renderSystem->init(*engine);

        auto cameraSystem = engine->registerSystem<CameraSystem>();
        signature.reset();
        signature.set(engine->getComponentId<Transform>());
        signature.set(engine->getComponentId<Camera>());
        engine->setSignature<CameraSystem>(signature);

        engine->subscribeSystemToInput(cameraSystem);

        cameraSystem->init(*engine);

        Entity player = engine->createEntity("player");
        auto playerTransform = Transform{glm::vec3(-5.0f, 0.5f, 0.0f),
                                         glm::vec3(0.0f),
                                         { 0.0f, 0.0f, 0.0f },
                                         glm::vec3(1.0f)};
        engine->addComponent(player, playerTransform);
        auto playerCamera = Camera{glm::vec3(0.0f, 0.0f, 1.0f),
                                   glm::vec3(0.0f, 1.0f, 0.0f),
                                   0.1f,
                                   45.0f};
        engine->addComponent(player, playerCamera);

        Entity wall = engine->createEntity("wall");
        auto wallTransform = Transform{glm::vec3(0.0f, 0.0f, 0.0f),
                                       glm::vec3(0.0f),
                                       { 0.0f, -90.0f, 0.0f },
                                       glm::vec3(1.0f)};
        engine->addComponent(wall, wallTransform);
        Renderable wallRenderable;
        WorldMesh mesh;
        mesh.shaderId = engine->loadShader("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
        mesh.vertices = {
            {{-1.0f, 0.0f, -1.0f}},
            {{-1.0f, 0.0f,  1.0f}},
            {{ 1.0f, 0.0f, -1.0f}},
            {{ 1.0f, 0.0f,  1.0f}},
        };
        mesh.indices = {0, 1, 2, 1, 3, 2};
        mesh.name = "wallMesh";
        wallRenderable.meshes.push_back(mesh);

        engine->addComponent(wall, wallRenderable);

        while (true)
        {
            float currentFrame = engine->getTime();
            float deltaTime = currentFrame - lastFrame;
            if(engine->startFrame() == -1) break;

            cameraSystem->update(deltaTime);
            renderSystem->update(deltaTime);

            engine->endFrame();

            lastFrame = currentFrame;
        }
    }
};