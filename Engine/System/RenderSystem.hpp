#include "Engine/Engine.hpp"
#include "Engine/Coordinator.hpp"
#include "System.h"

class RenderSystem : public System
{
public:
    Coordinator *coordinator;
    void init(Coordinator &c)
    {
        this->coordinator = &c;
    }

    void update(float dt)
    {
        for (Entity entity : entities)
        {
            auto renderable = coordinator->getComponent<Renderable>(entity);
        }

        for (Entity entity : entities)
        {
            auto renderable = coordinator->getComponent<Renderable>(entity);
            auto transform = coordinator->getComponent<Transform>(entity);
            for (auto &mesh : renderable.meshes)
            {
                auto model = glm::mat4(1.0f);
                model = glm::translate(model, transform.position);
                model = glm::rotate(model, glm::radians(transform.rotation.yaw), glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, glm::radians(transform.rotation.pitch), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, glm::radians(transform.rotation.roll), glm::vec3(0.0f, 0.0f, 1.0f));
                coordinator->setModelMatrix(model);

                coordinator->uploadMesh(&mesh);
                coordinator->renderMesh(&mesh);
            }
        }
    }

    void onKeyEvent(KeyCode key, KeyAction action) {}
    void onMouseEvent(double xpos, double ypos) {}
};