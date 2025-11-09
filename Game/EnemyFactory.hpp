#pragma once

#include <Engine/Types.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Component/Transform.h>
#include <Engine/Component/Collider.h>
#include <Engine/Component/Enemy.h>

const int HEALTH = 10;
const float SPEED = 1.0f;
const std::string tag = "enemy";

const float WIDTH = 0.5;
const float HEIGHT = 0.615;
const float aabbPadding = 0.1;

struct EnemyFactory
{
    static Entity create(std::shared_ptr<Engine> engine, glm::vec3 location)
    {
        Entity enemy = engine->createEntity(tag);

        Enemy ec = {
            .speed=SPEED,
            .health=HEALTH,
            .eye=glm::vec3(0.0f, 0.0f, 1.0f)
        };

        engine->addComponent(enemy, ec);

        auto tf = Transform{
            location,
            .velocity = glm::vec3(0.0f),
            .acceleration = glm::vec3(0.0f),
            .rotation = {0.0f, 0.0f, 0.0f},
            .scale = glm::vec3(1.0f)};

        engine->addComponent(enemy, tf);

        unsigned int enemySprite = engine->loadTextureFromFile("./textures/enemy.png");
        auto enemyTex = Texture{
            enemySprite,
            "texture_diffuse",
            "./textures/stone_tile.jpg"};

        Renderable wallRenderable;
        WorldMesh mesh;
        auto mat = new Material("shaders/cont_vertex.glsl", "shaders/cont_fragment.glsl");
        mesh.material = mat;
        mesh.vertices = {
            {{-WIDTH / 2.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{WIDTH / 2.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-WIDTH / 2.0f, HEIGHT, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{WIDTH / 2.0f, HEIGHT, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        };
        mesh.indices = {0, 1, 2, 1, 3, 2};
        mesh.name = "enemy" + std::to_string(enemy);
        mesh.textures.push_back(enemyTex);
        wallRenderable.meshes.push_back(mesh);
        engine->addComponent(enemy, wallRenderable);

        auto collider = Collider{};
        auto aabb = AABB({glm::vec3(-WIDTH / 2.0f, 0.0f, -WIDTH / 2.0f), glm::vec3(WIDTH / 2.0f, HEIGHT, WIDTH / 2.0f)});
        engine->addComponent(enemy, collider);
        engine->addComponent(enemy, aabb);

        return enemy;
    }
};