#pragma once

#include <Engine/Types.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Component/Transform.h>
#include <Engine/Component/Collider.h>
#include <Engine/Animation/ModelLoader.h>

const int HEALTH = 10;
const int SPEED = 10;
const std::string tag = "enemy";


const float WIDTH = 0.5;
const float HEIGHT = 0.615;
const float aabbPadding = 0.1;

const std::string modelPath = "resources/models/dancing_vampire.dae";
shared_ptr<Model> enemyModel;
shared_ptr<Animation> enemyAnim; 

struct Enemy
{
    static Entity create(std::shared_ptr<Engine> engine, glm::vec3 location)
    {
        if(enemyModel == nullptr) enemyModel = std::make_shared<Model>(ModelLoader::loadModel(modelPath, 0));
        if(enemyAnim == nullptr) enemyAnim = std::make_shared<Animation>(modelPath, enemyModel.get());

        Entity enemy = engine->createEntity(tag);

        auto tf = Transform{
            location,
            .velocity = glm::vec3(0.0f),
            .acceleration = glm::vec3(0.0f),
            .rotation = {0.0f, 0.0f, 0.0f},
            .scale = glm::vec3(0.6f)};

        engine->addComponent(enemy, tf);

        unsigned int shaderId = engine->loadShader("shaders/model_loading.vs", "shaders/model_loading.fs");
        enemyModel->shaderId = shaderId;
        // WorldMesh mesh;
        // mesh.vertices = {
        //     {{-WIDTH / 2.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        //     {{WIDTH / 2.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        //     {{-WIDTH / 2.0f, HEIGHT, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        //     {{WIDTH / 2.0f, HEIGHT, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        // };
        // mesh.indices = {0, 1, 2, 1, 3, 2};
        // mesh.name = "enemy" + std::to_string(enemy);
        // mesh.textures.push_back(enemyTex);
        // renderable.meshes.push_back(mesh);
        engine->addComponent(enemy, *enemyModel);
        AnimationComponent animationComponent{
            .currentAnimation = enemyAnim,
            .finalBoneMatrices = std::vector<glm::mat4>(enemyModel->boneCount, glm::mat4(1.0f)),
            .play = true,
            .currentTime = 0.0f,
        };
        engine->addComponent(enemy, animationComponent);

        auto collider = Collider{};
        auto aabb = AABB({glm::vec3(-WIDTH / 2.0f, 0.0f, -aabbPadding), glm::vec3(WIDTH / 2.0f, HEIGHT, aabbPadding)});
        engine->addComponent(enemy, collider);
        engine->addComponent(enemy, aabb);

        return enemy;
    }
};