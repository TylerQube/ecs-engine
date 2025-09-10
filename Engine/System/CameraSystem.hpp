#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Engine/Engine.hpp"
#include "System.h"

#include "Engine/Component/Camera.h"

class CameraSystem : public System
{
private:
    bool spin = true;
    float moveSpeed = 5.0f;

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

            if (spin)
                transform.rotation.y += dt * 20.0f;

            std::cout << "Camera position: " << transform.position.x << ", " << transform.position.y << ", " << transform.position.z << std::endl;

            glm::mat4 rotMatrix = glm::mat4(1.0f);
            rotMatrix = glm::rotate(rotMatrix, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            rotMatrix = glm::rotate(rotMatrix, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            rotMatrix = glm::rotate(rotMatrix, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            glm::vec3 rotatedFront = glm::vec3(rotMatrix * glm::vec4(camera.front, 1.0f));
            glm::vec3 rotatedUp = glm::vec3(rotMatrix * glm::vec4(camera.up, 1.0f));

            camera.front = rotatedFront;
            camera.up = rotatedUp;
            transform.rotation = glm::vec3(0.0f);

            glm::mat4 view = getViewMatrix(transform.position, rotatedFront, rotatedUp);
            coordinator->setViewMatrix(view);

            glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), coordinator->getAspectRatio(), 0.1f, 100.0f);
            coordinator->setProjectionMatrix(projection);
        }
    }

    void onKeyEvent(KeyCode key, KeyAction action)
    {
        for (Entity entity : entities)
        {
            auto &camera = this->coordinator->getComponent<Camera>(entity);
            auto &transform = this->coordinator->getComponent<Transform>(entity);
            if (action == RELEASE)
            {
                transform.velocity = glm::vec3(0.0f);
                continue;
            }
            switch (key)
            {
            case W:
            {
                transform.velocity = glm::normalize(camera.front) * moveSpeed;
                break;
            }
            case A:
            {
                glm::vec3 right = glm::normalize(glm::cross(camera.front, camera.up));
                transform.velocity = -right * moveSpeed;
                break;
            }
            case S:
            {
                transform.velocity = -glm::normalize(camera.front) * moveSpeed;
                break;
            }
            case D:
            {
                glm::vec3 right = glm::normalize(glm::cross(camera.front, camera.up));
                transform.velocity = right * moveSpeed;
                break;
            }
            case SPACE:
                spin = !spin;
                break;
            }
        }
    }
};