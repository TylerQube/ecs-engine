#pragma once

#include "Renderer/Renderer.h"
#include "Engine/Component/Transform.h"
#include "Engine/Component/Camera.h"

#include "Engine/System/RenderSystem.hpp"
#include "Engine/System/CameraSystem.hpp"
#include "Engine/System/TransformSystem.hpp"
#include "Engine/System/ColliderSystem.hpp"

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
        engine->registerComponent<Collider>();
        engine->registerComponent<AABB>();

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

        auto movementSystem = engine->registerSystem<TransformSystem>();
        signature.reset();
        signature.set(engine->getComponentId<Transform>());
        engine->setSignature<TransformSystem>(signature);
        movementSystem->init(*engine);

        auto colliderSystem = engine->registerSystem<ColliderSystem>();
        signature.reset();
        signature.set(engine->getComponentId<Collider>());
        signature.set(engine->getComponentId<Transform>());
        engine->setSignature<ColliderSystem>(signature);
        colliderSystem->init(*engine);


        Entity player = engine->createEntity("player");
        auto playerTransform = Transform{glm::vec3(0.0f, 3.0f, 0.0f),
                                         glm::vec3(0.0f),
                                         glm::vec3(0.0f),
                                         {0.0f, 0.0f, 0.0f},
                                         glm::vec3(1.0f)};
        engine->addComponent(player, playerTransform);
        auto playerCamera = Camera{glm::vec3(0.0f, 0.0f, 1.0f),
                                   glm::vec3(0.0f, 1.0f, 0.0f),
                                   0.1f,
                                   45.0f};
        engine->addComponent(player, playerCamera);

        auto playerCollider = Collider{};
        auto playerAABB = AABB{glm::vec3(-0.2f, -0.7f, -0.2f), glm::vec3(0.2f, 0.2f, 0.2f)};
        engine->addComponent(player, playerCollider);
        engine->addComponent(player, playerAABB);

        unsigned int stoneTexId = engine->loadTextureFromFile("./textures/stone_tile.jpg");
        auto stoneTexture = Texture{
            stoneTexId,
            "texture_diffuse",
            "./textures/stone_tile.jpg"};

        Entity wall = engine->createEntity("wall");
        auto wallTransform = Transform{glm::vec3(0.0f, 1.0f, 0.0f),
                                       glm::vec3(0.0f),
                                       glm::vec3(0.0f),
                                       {0.0f, -90.0f, 0.0f},
                                       glm::vec3(1.0f)};
        engine->addComponent(wall, wallTransform);
        Renderable wallRenderable;
        WorldMesh mesh;
        mesh.shaderId = engine->loadShader("shaders/cont_vertex.glsl", "shaders/cont_fragment.glsl");
        mesh.vertices = {
            {{-1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {-1.0f, -1.0f}},
            {{-1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {-1.0f, 1.0f}},
            {{1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, -1.0f}},
            {{1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        };
        mesh.indices = {0, 1, 2, 1, 3, 2};
        mesh.name = "wallMesh";
        mesh.textures.push_back(stoneTexture);
        wallRenderable.meshes.push_back(mesh);
        engine->addComponent(wall, wallRenderable);

        auto wallCollider = Collider{};
        auto wallAABB = AABB({glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(1.0f, 0.0f, 1.0f)});
        engine->addComponent(wall, wallCollider);
        engine->addComponent(wall, wallAABB);

        while (true)
        {
            float currentFrame = engine->getTime();
            float deltaTime = currentFrame - lastFrame;
            if (engine->startFrame() == -1)
                break;

            cameraSystem->update(deltaTime);
            colliderSystem->update(deltaTime);
            movementSystem->update(deltaTime);
            renderSystem->update(deltaTime);

            engine->endFrame();

            lastFrame = currentFrame;
        }
    }
};