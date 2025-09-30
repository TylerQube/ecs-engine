#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Engine/Engine.hpp"
#include "System.h"

#include "Engine/Component/Camera.h"
#include "Engine/Component/Collider.h"

class CameraSystem : public System
{
private:
    float mouseSensitivity = 0.1f;

    bool firstMouse = true;
    double xpos = 0;
    double ypos = 0;

    float movementAcceleration = 40.0f;
    float frictionAcc = movementAcceleration * 0.3f;

    std::set<KeyCode> keysHeld = {};
    std::set<KeyCode> keysDown = {};

public:
    Engine *engine;
    void init(Engine &c)
    {
        this->engine = &c;
    }

    glm::mat4 getViewMatrix(glm::vec3 position, glm::vec3 front, glm::vec3 up)
    {
        return glm::lookAt(position, position + front, up);
    }

    void update(float dt)
    {
        for (Entity entity : entities)
        {
            auto &camera = this->engine->getComponent<Camera>(entity);
            auto &transform = this->engine->getComponent<Transform>(entity);
            updateAcceleration(entity, dt);


            transform.velocity += transform.acceleration * dt;

            glm::mat4 view = glm::lookAt(transform.position, transform.position + camera.front, camera.up);
            engine->setViewMatrix(view);

            glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), engine->getAspectRatio(), 0.1f, 100.0f);
            engine->setProjectionMatrix(projection);
        }
    }

    const float epsilon = 1e-5;
    const float maxAcceleration = 20.0f;
    bool holdingSpace = false;
    void updateAcceleration(Entity& entity, float dt) {
        auto &camera = this->engine->getComponent<Camera>(entity);
        auto &transform = this->engine->getComponent<Transform>(entity);

        auto frontFlat = glm::vec3(camera.front.x, 0.0f, camera.front.z);
        auto right = glm::cross(camera.front, camera.up);
        auto rightFlat = glm::vec3(right.x, 0.0f, right.z);
        auto up = glm::vec3(0.0f, 1.0f, 0.0f);

        auto moveAcc = glm::vec3(0.0f);

        auto moveDir = glm::vec3(0.0f);

        if(keysHeld.count(W) || keysDown.count(W)) moveDir += frontFlat;
        if(keysHeld.count(A) || keysDown.count(A)) moveDir -= rightFlat;
        if(keysHeld.count(S) || keysDown.count(S)) moveDir -= frontFlat;
        if(keysHeld.count(D) || keysDown.count(D)) moveDir += rightFlat;
        if(keysHeld.count(LEFT_SHIFT) || keysDown.count(LEFT_SHIFT)) moveDir -= up;
        moveDir = glm::length(moveDir) > 0.0f ? glm::normalize(moveDir) : moveDir;
        moveDir *= movementAcceleration;

        moveAcc += moveDir;

        bool hasCollider = engine->hasComponent<Collider>(entity);
        if(hasCollider) {
            auto aabb = engine->getComponent<AABB>(entity);
            bool isTouching = aabb.isTouching;

            if(keysDown.count(SPACE) && isTouching) {
                std::cout << "jumping " << isTouching << std::endl;
                moveAcc += up * 500.0f;
            } 

            if(isTouching) {
                auto friction = -transform.velocity * frictionAcc;
                friction.y = 0.0f;
                moveAcc += friction;
            } else {
                auto airFriction = -transform.velocity * frictionAcc / 1.5f;
                airFriction.y = 0.0f;
                moveAcc += airFriction;
            }

        }



        transform.acceleration += moveAcc;
    }

    void updateCamVectors(Camera &camera, Transform &transform)
    {
        // calculate the new Front vector
        auto rot = transform.rotation;
        glm::vec3 dir;
        dir.x = cos(glm::radians(rot.yaw)) * cos(glm::radians(rot.pitch));
        dir.y = sin(glm::radians(rot.pitch));
        dir.z = sin(glm::radians(rot.yaw)) * cos(glm::radians(rot.pitch));

        camera.front = glm::normalize(dir);
        glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), camera.front));
        camera.up = glm::normalize(glm::cross(camera.front, right));
    }

    void processMouse(double dx, double dy)
    {
        for (Entity entity : entities)
        {
            auto &transform = this->engine->getComponent<Transform>(entity);
            auto &camera = this->engine->getComponent<Camera>(entity);

            transform.rotation.yaw += dx * mouseSensitivity;
            transform.rotation.pitch += dy * mouseSensitivity;

            if (transform.rotation.pitch > 89.0f)
                transform.rotation.pitch = 89.0f;
            if (transform.rotation.pitch < -89.0f)
                transform.rotation.pitch = -89.0f;

            updateCamVectors(camera, transform);
        }
    }

    void onKeyEvent(KeyCode key, KeyAction action)
    {
        for (Entity entity : entities)
        {
            auto &camera = this->engine->getComponent<Camera>(entity);
            auto &transform = this->engine->getComponent<Transform>(entity);

            auto front = glm::vec3(camera.front.x, 0.0f, camera.front.z);
            auto right = glm::cross(camera.front, camera.up);

            if (action == RELEASE)
            {
                std::cout << "release: " << key << std::endl;
                keysDown.erase(key);
                keysHeld.erase(key);
            }

            if (action == PRESS) {
                std::cout << "press: " << key << std::endl;
                keysDown.insert(key);
            }

            if (action == REPEAT) {
                std::cout << "repeat: " << key << std::endl;
                keysDown.erase(key);
                keysHeld.insert(key);
            }

        }
    }

    void onMouseEvent(double xpos, double ypos)
    {
        if (firstMouse)
        {
            this->xpos = xpos;
            this->ypos = ypos;
            firstMouse = false;
            return;
        }

        double dx = xpos - this->xpos;
        double dy = this->ypos - ypos;

        processMouse(dx, dy);
        this->xpos = xpos;
        this->ypos = ypos;
    }
};