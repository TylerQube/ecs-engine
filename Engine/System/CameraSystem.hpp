#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Engine/Engine.hpp"
#include "System.h"

#include "Engine/Component/Camera.h"

class CameraSystem : public System
{
private:
    float moveSpeed = 5.0f;
    float mouseSensitivity = 0.1f;

    bool firstMouse = true;
    double xpos = 0;
    double ypos = 0;

public:
    Coordinator *coordinator;
    void init(Coordinator &c)
    {
        this->coordinator = &c;
    }

    glm::mat4 getViewMatrix(glm::vec3 position, glm::vec3 front, glm::vec3 up)
    {
        return glm::lookAt(position, position + front, up);
    }

    void update(float dt)
    {
        for (Entity entity : entities)
        {
            auto &camera = this->coordinator->getComponent<Camera>(entity);
            auto &transform = this->coordinator->getComponent<Transform>(entity);

            transform.position += transform.velocity * dt;

            glm::mat4 view = glm::lookAt(transform.position, transform.position + camera.front, camera.up);
            coordinator->setViewMatrix(view);

            glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), coordinator->getAspectRatio(), 0.1f, 100.0f);
            coordinator->setProjectionMatrix(projection);
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
            auto &transform = this->coordinator->getComponent<Transform>(entity);
            auto &camera = this->coordinator->getComponent<Camera>(entity);

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
            auto &camera = this->coordinator->getComponent<Camera>(entity);
            auto &transform = this->coordinator->getComponent<Transform>(entity);

            auto front = glm::vec3(camera.front.x, 0.0f, camera.front.z);
            auto right = glm::cross(camera.front, camera.up);

            if (action == RELEASE)
            {
                transform.velocity = glm::vec3(0.0f);
                continue;
            }

            switch (key)
            {
            case W:
            {
                transform.velocity += front;
                break;
            }
            case A:
            {
                transform.velocity -= right;
                break;
            }
            case S:
            {
                transform.velocity -= front;
                break;
            }
            case D:
            {
                transform.velocity += right;
                break;
            }
            case SPACE:
            {
                transform.velocity -= glm::vec3(0.0f, 1.0f, 0.0f);
                break;
            }
            case LEFT_SHIFT:
            {
                transform.velocity += glm::vec3(0.0f, 1.0f, 0.0f);
                break;
            }
            }

            transform.velocity = glm::normalize(transform.velocity) * moveSpeed;
            std::cout << "Velocity: " << transform.velocity.x << ", " << transform.velocity.y << ", " << transform.velocity.z << std::endl;
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