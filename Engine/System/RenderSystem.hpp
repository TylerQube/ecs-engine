#include "Engine/Engine.hpp"
#include "Engine/Engine.hpp"
#include "System.h"

class RenderSystem : public System
{
public:
    Engine *engine;
    void init(Engine &c)
    {
        this->engine = &c;
    }

    void update(float dt)
    {
        for (Entity entity : entities)
        {
            auto renderable = engine->getComponent<Renderable>(entity);
            auto transform = engine->getComponent<Transform>(entity);
            for (auto &mesh : renderable.meshes)
            {
                auto model = glm::mat4(1.0f);
                model = glm::translate(model, transform.position);
                model = glm::rotate(model, glm::radians(transform.rotation.yaw), glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, glm::radians(transform.rotation.pitch), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, glm::radians(transform.rotation.roll), glm::vec3(0.0f, 0.0f, 1.0f));
                engine->setModelMatrix(model);

                engine->uploadMesh(&mesh);
                engine->renderMesh(&mesh);
            }
        }
    }
};