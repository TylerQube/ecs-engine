#pragma once

#include <Engine/Animation/AnimationLoader.h>
#include <Engine/Animation/ModelLoader.h>
#include <Engine/Component/Collider.h>
#include <Engine/Component/Transform.h>
#include <Engine/Engine.hpp>
#include <Engine/Types.hpp>

const int HEALTH = 10;
const int SPEED = 10;
const std::string tag = "enemy";

const float WIDTH = 0.5;
const float HEIGHT = 0.615;
const float aabbPadding = 0.1;

const std::string modelPath = "resources/models/Skeleton/skeleton.fbx";
shared_ptr<Model> enemyModel;
AnimationMap enemyAnims;
unsigned int enemyShaderId = 0;

struct Enemy {
    static Entity create(std::shared_ptr<Engine> engine, glm::vec3 location) {
        if (enemyModel == nullptr) {
            enemyModel = std::make_shared<Model>(ModelLoader::loadModel(modelPath, 0));
            std::cout << "Enemy model meshes: " << enemyModel->meshes.size() << std::endl;
        }
        if (enemyAnims.empty())
            enemyAnims = AnimationLoader::loadClips(modelPath, enemyModel->skeleton);

        Entity enemy = engine->createEntity(tag);

        location.y += 0.1;
        auto tf = Transform{.position = location,
                            .velocity = glm::vec3(0.0f),
                            .acceleration = glm::vec3(0.0f),
                            .rotation = {0.0f, 0.0f, 0.0f},
                            .scale = glm::vec3(0.005f)};

        engine->addComponent(enemy, tf);

        if (enemyShaderId == 0)
            enemyShaderId = engine->loadShader("shaders/model_loading.vs", "shaders/model_loading.fs");
        enemyModel->shaderId = enemyShaderId;

        engine->addComponent(enemy, *enemyModel);
        AnimationComponent animationComponent{
            .currentAnimation = enemyAnims.begin()->second,
            .currentTime = 0.0f,
            .finalBoneMatrices = std::vector<glm::mat4>(enemyModel->skeleton.boneCount, glm::mat4(1.0f)),
            .play = true,
        };
        engine->addComponent(enemy, animationComponent);
        if (enemyModel->meshes.size() == 0)
            std::cerr << "WARNING mesh loaded with no meshes, will not render" << std::endl;

        auto collider = Collider{};
        auto aabb = AABB({glm::vec3(-WIDTH / 2.0f, 0.0f, -aabbPadding), glm::vec3(WIDTH / 2.0f, HEIGHT, aabbPadding)});
        engine->addComponent(enemy, collider);
        engine->addComponent(enemy, aabb);

        return enemy;
    }
};