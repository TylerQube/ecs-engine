#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Engine/Engine.hpp"
#include "System.h"

#include "Engine/Component/Camera.h"

class CameraSystem : public System
{
private:
    float mouseSensitivity = 0.1f;

    bool firstMouse = true;
    double xpos = 0;
    double ypos = 0;

    float maxVelocity = 4.0f;
    float acceleration = 10.0f;

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

            // cap velocity
            if(glm::length(transform.velocity) > maxVelocity)
                transform.velocity = glm::normalize(transform.velocity) * maxVelocity;

            transform.position += transform.velocity * dt;

            glm::mat4 view = glm::lookAt(transform.position, transform.position + camera.front, camera.up);
            engine->setViewMatrix(view);

            glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), engine->getAspectRatio(), 0.1f, 100.0f);
            engine->setProjectionMatrix(projection);
        }
    }

    const float epsilon = 1e-5;
    void updateAcceleration(Entity& entity, float dt) {
        auto &camera = this->engine->getComponent<Camera>(entity);
        auto &transform = this->engine->getComponent<Transform>(entity);

        auto frontFlat = glm::vec3(camera.front.x, 0.0f, camera.front.z);
        auto right = glm::cross(camera.front, camera.up);
        auto rightFlat = glm::vec3(right.x, 0.0f, right.z);
        auto up = glm::vec3(0.0f, 1.0f, 0.0f);

        transform.acceleration = glm::vec3(0.0f, 0.0f, 0.0f);

        if(keysDown.size() == 0) transform.acceleration = -transform.velocity;

        if(keysDown.count(W)) transform.acceleration += frontFlat;
        if(keysDown.count(A)) transform.acceleration -= rightFlat;
        if(keysDown.count(S)) transform.acceleration -= frontFlat;
        if(keysDown.count(D)) transform.acceleration += rightFlat;
        if(keysDown.count(SPACE)) transform.acceleration -= up;
        if(keysDown.count(LEFT_SHIFT)) transform.acceleration += up;

        if(glm::length(transform.acceleration) > epsilon)
            transform.acceleration = glm::normalize(transform.acceleration) * acceleration;
        else
            transform.acceleration = glm::vec3(0.0f, 0.0f, 0.0f);

        if(keysDown.size() == 0) { 
            transform.acceleration.x /= 1.7f; 
            transform.acceleration.z /= 1.7f; 
        }
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
        glm::vec3 right = glm::normalize(glm::cross(camera.front, glm::vec3(0.0f, 1.0f, 0.0f)));
        camera.up = glm::normalize(glm::cross(camera.front, right));
    }

    void processMouse(double dx, double dy)
    {
        for (Entity entity : entities)
        {
            auto &transform = this->engine->getComponent<Transform>(entity);
            auto &camera = this->engine->getComponent<Camera>(entity);

            transform.rotation.yaw -= dx * mouseSensitivity;
            transform.rotation.pitch -= dy * mouseSensitivity;

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
                keysDown.erase(key);
            }

            if (action == PRESS) {
                keysDown.insert(key);
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