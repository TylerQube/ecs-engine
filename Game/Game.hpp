#pragma once

#include "Renderer/Renderer.h"
#include "Engine/Component/Transform.h"
#include "Engine/Component/Model.h"
#include "Engine/Component/Animation.h"
#include "Engine/Component/Camera.h"
#include "Engine/Component/Gravity.h"

#include "Engine/System/RenderSystem.hpp"
#include "Engine/System/AnimationSystem.hpp"
#include "Engine/System/CameraSystem.hpp"
#include "Engine/System/TransformSystem.hpp"
#include "Engine/System/GravitySystem.hpp"
#include "Engine/System/ColliderSystem.hpp"

#include "Engine/Engine.hpp"
#include "Engine/Types.hpp"

#include "Enemy.hpp"
#include "LevelGen.hpp"

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
        engine->registerComponent<Gravity>();
        engine->registerComponent<Model>();
        engine->registerComponent<AnimationComponent>();
        engine->registerComponent<Camera>();
        engine->registerComponent<Collider>();
        engine->registerComponent<AABB>();

        auto renderSystem = engine->registerSystem<RenderSystem>();
        Signature signature;
        signature.set(engine->getComponentId<Transform>());
        signature.set(engine->getComponentId<Model>());
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

        auto gravitySystem = engine->registerSystem<GravitySystem>();
        signature.reset();
        signature.set(engine->getComponentId<Transform>());
        signature.set(engine->getComponentId<Gravity>());
        engine->setSignature<GravitySystem>(signature);
        gravitySystem->init(*engine);

        auto animSystem = engine->registerSystem<AnimationSystem>();
        signature.reset();
        signature.set(engine->getComponentId<Model>());
        signature.set(engine->getComponentId<AnimationComponent>());
        engine->setSignature<AnimationSystem>(signature);
        animSystem->init(*engine);
        
        auto globalGravity = Gravity { glm::vec3(0.0f, -9.81f, 0.0f) };

        auto enemy = Enemy::create(engine, glm::vec3(3.0f, 0.0f, 0.0f));

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
        // engine->addComponent(player, globalGravity);

        auto playerCollider = Collider{};
        auto playerAABB = AABB{glm::vec3(-0.1f, -0.5f, -0.1f), glm::vec3(0.1f, 0.2f, 0.1f)};
        engine->addComponent(player, playerCollider);
        engine->addComponent(player, playerAABB);

        unsigned int stoneTexId = engine->loadTextureFromFile("./textures/stone_tile.jpg");
        auto stoneTexture = Texture{
            stoneTexId,
            "texture_diffuse",
            "./textures/stone_tile.jpg"};

        Entity wall = engine->createEntity("wall");
        auto wallTransform = Transform{glm::vec3(0.0f, 0.0f, 0.0f),
                                       glm::vec3(0.0f),
                                       glm::vec3(0.0f),
                                       {0.0f, 0.0f, 0.0f},
                                       glm::vec3(1.0f)};
        engine->addComponent(wall, wallTransform);
        Model wallModel;
        WorldMesh mesh;
        unsigned int shader = engine->loadShader("shaders/cont_vertex.glsl", "shaders/cont_fragment.glsl");
        wallModel.shaderId = shader;
        float wallSize = 4.0f;
        mesh.vertices = {
            {{-wallSize, 0.0f, -wallSize}, {0.0f, wallSize, 0.0f}, {-wallSize, -wallSize}},
            {{-wallSize, 0.0f, wallSize}, {0.0f, wallSize, 0.0f}, {-wallSize, wallSize}},
            {{wallSize, 0.0f, -wallSize}, {0.0f, wallSize, 0.0f}, {wallSize, -wallSize}},
            {{wallSize, 0.0f, wallSize}, {0.0f, wallSize, 0.0f}, {wallSize, wallSize}},
        };
        mesh.indices = {0, 1, 2, 1, 3, 2};
        mesh.name = "wallMesh";
        mesh.material.textures.push_back(stoneTexture);
        wallModel.meshes.push_back(mesh);
        engine->addComponent(wall, wallModel);

        auto wallCollider = Collider{};
        auto wallAABB = AABB({glm::vec3(-4.0f, 0.0f, -4.0f), glm::vec3(4.0f, 0.0f, 4.0f)});
        engine->addComponent(wall, wallCollider);
        engine->addComponent(wall, wallAABB);


        auto room = engine->createEntity("room");
        auto roomModel = ModelLoader::loadModel("resources/models/corridor1.glb", shader);
        roomModel.shaderId = shader;
        engine->addComponent(room, Transform{glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f), {0.0f, 0.0f, -90.0f}, glm::vec3(1.0f)});
        engine->addComponent(room, roomModel);

        auto dungeon = engine->createEntity("dungeon");
        auto dungeonBSP = LevelGenerator::generateDungeon(0, 0, 70, 70, 40);
        auto dungeonModel = LevelGenerator::generateModelFromDungeon(&dungeonBSP, shader);
        engine->addComponent(dungeon, Transform{glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), {0.0f, 0.0f, 0.0f}, glm::vec3(1.0f)});
        engine->addComponent(dungeon, dungeonModel);


        while (true)
        {
            float currentFrame = engine->getTime();
            float deltaTime = currentFrame - lastFrame;
            if (engine->startFrame() == -1)
                break;

            gravitySystem->update(deltaTime);
            animSystem->update(deltaTime);
            cameraSystem->update(deltaTime);
            colliderSystem->update(deltaTime);
            movementSystem->update(deltaTime);
            renderSystem->update(deltaTime);

            engine->endFrame();

            lastFrame = currentFrame;
        }
    }
};