#include "Engine.hpp"
#include "System.h"

class RenderSystem : public System
{
    Coordinator* coordinator;
    void init(Coordinator& c)
    {
        this->coordinator = &c;
    }

    void update() {
        for (Entity entity : entities) {
            auto renderable = coordinator->getComponent<Renderable>(entity);
            auto transform = coordinator->getComponent<Transform>(entity);
            for(auto& mesh : renderable.meshes) {
                auto model = glm::mat4(1.0f);
                model = glm::translate(model, transform.position);
                model = glm::rotate(model, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
                coordinator->setModelMatrix(model);

                coordinator->uploadMesh(&mesh);
                coordinator->renderMesh(&mesh);
            }
        }
    }
};