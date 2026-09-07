#pragma once

#include <limits>

#include "Engine/Component/Animation.h"
#include "Engine/Component/Camera.h"
#include "Engine/Component/Gravity.h"
#include "Engine/Component/Model.h"
#include "Engine/Component/Transform.h"
#include "Renderer/Renderer.h"
#include "Engine/Component/Light.h"

#include "Engine/System/AnimationSystem.hpp"
#include "Engine/System/CameraSystem.hpp"
#include "Engine/System/ColliderSystem.hpp"
#include "Engine/System/GravitySystem.hpp"
#include "Engine/System/RenderSystem.hpp"
#include "Engine/System/TransformSystem.hpp"

#include "Engine/Engine.hpp"
#include "Engine/Types.hpp"

#include "Enemy.hpp"
#include "Game/Component/EnemyAI.h";
#include "Game/System/EnemyAISystem.hpp";
#include "LevelGen.hpp"

class Game {
  private:
    float lastFrame = 0;
    bool running = true;
    bool paused = false;
    std::shared_ptr<Engine> engine;
    std::vector<LevelGenerator::PointLightSpawn> pendingPointLightSpawns;

    void spawnEnemiesInDungeon(const std::shared_ptr<Engine> engine, BSPNode &dungeon, int numEnemies) {
        std::vector<BSPLeaf *> rooms;
        LevelGenerator::collectRooms(&dungeon, rooms);

        for (int i = 0; i < numEnemies; ++i) {
            if (rooms.empty())
                break;

            BSPLeaf *room = rooms[LevelGenerator::randRange(0, (int)rooms.size() - 1)];
            float x = LevelGenerator::randRange(room->x + 1, room->x + room->width - 2);
            float z = LevelGenerator::randRange(room->y + 1, room->y + room->height - 2);
            glm::vec3 spawnLocation((float)x, 0.0f, (float)z);

            Enemy::create(engine, spawnLocation);
        }
    }

  public:
    void init() {
        engine = std::make_shared<Engine>();
        engine->init();
    }

    void run() {
        engine->registerComponent<Transform>();
        engine->registerComponent<Gravity>();
        engine->registerComponent<Model>();
        engine->registerComponent<AnimationComponent>();
        engine->registerComponent<Camera>();
        engine->registerComponent<PointLight>();
        engine->registerComponent<Collider>();
        engine->registerComponent<AABB>();

        // custom components + systems
        engine->registerComponent<EnemyAI>();

        auto enemyAISystem = engine->registerSystem<EnemyAISystem>();
        Signature signature;
        signature.set(engine->getComponentId<EnemyAI>());
        signature.set(engine->getComponentId<Transform>());
        engine->setSignature<EnemyAISystem>(signature);

        auto renderSystem = engine->registerSystem<RenderSystem>();
        signature.reset();
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

        auto globalGravity = Gravity{glm::vec3(0.0f, -9.81f, 0.0f)};

        auto enemy = Enemy::create(engine, glm::vec3(3.0f, 0.0f, 0.0f));

        unsigned int stoneTexId = engine->loadTextureFromFile("./textures/stone_tile.jpg");
        auto stoneTexture = Texture{stoneTexId, "texture_diffuse", "./textures/stone_tile.jpg"};

        Entity wall = engine->createEntity("wall");
        auto wallTransform = Transform{
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f), {0.0f, 0.0f, 0.0f}, glm::vec3(1.0f)};
        engine->addComponent(wall, wallTransform);
        Model wallModel;
        WorldMesh mesh;
        unsigned int shader = engine->loadShader("shaders/gouraud_vertex.glsl", "shaders/gouraud_fragment.glsl");
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
        engine->addComponent(
            room,
            Transform{
                glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f), {0.0f, 0.0f, -90.0f}, glm::vec3(1.0f)});
        engine->addComponent(room, roomModel);

        auto dungeon = engine->createEntity("dungeon");
        auto dungeonBSP = LevelGenerator::generateDungeon(0, 0, 50, 50, 20);

        spawnEnemiesInDungeon(engine, dungeonBSP, 1);

        std::vector<BSPLeaf *> rooms;
        LevelGenerator::collectRooms(&dungeonBSP, rooms);
        BSPLeaf *startRoom = rooms.empty() ? nullptr : rooms[LevelGenerator::randRange(0, (int)rooms.size() - 1)];
        glm::vec3 playerSpawn = startRoom ? glm::vec3((float)(startRoom->x + startRoom->width / 2), 1.0f,
                                                      (float)(startRoom->y + startRoom->height / 2))
                                          : glm::vec3(0.0f, 3.0f, 0.0f);

        Entity player = engine->createEntity("player");
        auto playerTransform =
            Transform{playerSpawn, glm::vec3(0.0f), glm::vec3(0.0f), {0.0f, 0.0f, 0.0f}, glm::vec3(1.0f)};
        engine->addComponent(player, playerTransform);
        auto playerCamera = Camera{glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.1f, 45.0f};
        engine->addComponent(player, playerCamera);
        engine->addComponent(player, globalGravity);

        auto playerCollider = Collider{};
        auto playerAABB = AABB{glm::vec3(-0.1f, -0.5f, -0.1f), glm::vec3(0.1f, 0.2f, 0.1f)};
        engine->addComponent(player, playerCollider);
        engine->addComponent(player, playerAABB);

        enemyAISystem->init(*engine, player);

        unsigned int dungeonFloorTexId = engine->loadTextureFromFile("./textures/stone_tile.jpg");
        auto dungeonFloorTexture = Texture{dungeonFloorTexId, "texture_diffuse", "./textures/stone_tile.jpg"};

        unsigned int dungeonWallTexId = engine->loadTextureFromFile("./textures/cobblestone.png");
        auto dungeonWallTexture = Texture{dungeonWallTexId, "texture_diffuse", "./textures/cobblestone.png"};

        unsigned int dungeonCeilingTexId = engine->loadTextureFromFile("./textures/stone_tile.jpg");
        auto dungeonCeilingTexture = Texture{dungeonCeilingTexId, "texture_diffuse", "./textures/stone_tile.jpg"};

        auto dungeonModel = LevelGenerator::generateModelFromDungeon(&dungeonBSP, shader, &dungeonFloorTexture,
                                                                     &dungeonWallTexture, &dungeonCeilingTexture);
        pendingPointLightSpawns = LevelGenerator::generatePointLightSpawns(dungeonModel);
        std::cout << "Prepared " << pendingPointLightSpawns.size()
                  << " point-light spawn templates from dungeon geometry" << std::endl;

        for (const auto &pointLight : pendingPointLightSpawns) {
            auto pointLightEntity = engine->createEntity("dungeon_point_light");
            engine->addComponent(pointLightEntity,
                                 PointLight{pointLight.position, pointLight.color, pointLight.intensity});
        }
        engine->addComponent(
            dungeon, Transform{glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), {0.0f, 0.0f, 0.0f}, glm::vec3(1.0f)});
        engine->addComponent(dungeon, dungeonModel);

        // Register collision for generated dungeon meshes (walls, floors, ceilings) using static AABBs.
        for (const auto &generatedMesh : dungeonModel.meshes) {
            bool isWall = generatedMesh.name.rfind("wall_", 0) == 0;
            bool isFloor = generatedMesh.name.rfind("room", 0) == 0 ||
                           generatedMesh.name.rfind("corridor_h_", 0) == 0 ||
                           generatedMesh.name.rfind("corridor_v_", 0) == 0;
            bool isCeiling = generatedMesh.name.rfind("ceiling_", 0) == 0;
            if (!isWall && !isFloor && !isCeiling)
                continue;
            if (generatedMesh.vertices.empty())
                continue;

            glm::vec3 minPos(std::numeric_limits<float>::max());
            glm::vec3 maxPos(std::numeric_limits<float>::lowest());
            for (const auto &vertex : generatedMesh.vertices) {
                minPos.x = std::min(minPos.x, vertex.Position.x);
                minPos.y = std::min(minPos.y, vertex.Position.y);
                minPos.z = std::min(minPos.z, vertex.Position.z);

                maxPos.x = std::max(maxPos.x, vertex.Position.x);
                maxPos.y = std::max(maxPos.y, vertex.Position.y);
                maxPos.z = std::max(maxPos.z, vertex.Position.z);
            }

            // Add minimal thickness for planar meshes so CCD collision stays robust.
            constexpr float COLLIDER_THICKNESS_EPSILON = 0.05f;
            if (maxPos.x - minPos.x < COLLIDER_THICKNESS_EPSILON) {
                minPos.x -= COLLIDER_THICKNESS_EPSILON * 0.5f;
                maxPos.x += COLLIDER_THICKNESS_EPSILON * 0.5f;
            }
            if (maxPos.y - minPos.y < COLLIDER_THICKNESS_EPSILON) {
                minPos.y -= COLLIDER_THICKNESS_EPSILON * 0.5f;
                maxPos.y += COLLIDER_THICKNESS_EPSILON * 0.5f;
            }
            if (maxPos.z - minPos.z < COLLIDER_THICKNESS_EPSILON) {
                minPos.z -= COLLIDER_THICKNESS_EPSILON * 0.5f;
                maxPos.z += COLLIDER_THICKNESS_EPSILON * 0.5f;
            }

            // Floors/ceilings get seam overlap and extra thickness to prevent edge gaps and step-like lips.
            constexpr float FLOOR_SEAM_OVERLAP = 0.1f;
            constexpr float FLOOR_THICKNESS = 0.2f;
            constexpr float CEILING_THICKNESS = 0.2f;
            if (isFloor) {
                minPos.x -= FLOOR_SEAM_OVERLAP;
                maxPos.x += FLOOR_SEAM_OVERLAP;
                minPos.z -= FLOOR_SEAM_OVERLAP;
                maxPos.z += FLOOR_SEAM_OVERLAP;
                minPos.y -= FLOOR_THICKNESS;
                maxPos.y += FLOOR_THICKNESS;
            } else if (isCeiling) {
                minPos.x -= FLOOR_SEAM_OVERLAP;
                maxPos.x += FLOOR_SEAM_OVERLAP;
                minPos.z -= FLOOR_SEAM_OVERLAP;
                maxPos.z += FLOOR_SEAM_OVERLAP;
                minPos.y -= CEILING_THICKNESS;
                maxPos.y += CEILING_THICKNESS;
            }

            auto wallColliderEntity = engine->createEntity("dungeon_static_collider");
            engine->addComponent(
                wallColliderEntity,
                Transform{glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), {0.0f, 0.0f, 0.0f}, glm::vec3(1.0f)});
            engine->addComponent(wallColliderEntity, Collider{});
            engine->addComponent(wallColliderEntity, AABB{minPos, maxPos});
        }

        while (true) {
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
            enemyAISystem->update(deltaTime);

            engine->endFrame();

            lastFrame = currentFrame;
        }
    }
};