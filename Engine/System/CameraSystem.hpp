#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Engine/Engine.hpp"
#include "System.h"

#include "Component/Camera.h"

class CameraSystem : public System
{
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

    void update()
    {
        for (Entity entity : entities)
        {
            auto &camera = this->coordinator->getComponent<Camera>(entity);
            auto &transform = this->coordinator->getComponent<Transform>(entity);

            glm::mat4 view = getViewMatrix(transform.position, camera.front, camera.up);
            coordinator->setViewMatrix(view);

            glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), coordinator->getAspectRatio(), 0.1f, 100.0f);
            coordinator->setProjectionMatrix(projection);
        }
    }
};