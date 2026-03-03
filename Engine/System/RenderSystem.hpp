#include "Engine/Engine.hpp"
#include "System.h"

class RenderSystem : public System {
  public:
    Engine *engine;
    void init(Engine &c) {
        this->engine = &c;
    }

    void update(float dt) {
        for (Entity entity : entities) {
            auto model = engine->getComponent<Model>(entity);
            auto transform = engine->getComponent<Transform>(entity);

            // Set bone matrices for animation
            if (engine->hasComponent<Skeleton>(entity)) {
                auto skeleton = engine->getComponent<Skeleton>(entity);
                for (int i = 0; i < skeleton.finalBoneMatrices.size(); i++) {
                    glm::mat4 mat = skeleton.finalBoneMatrices[i];
                    engine->setUniform(model.shaderId, "finalBonesMatrices[" + std::to_string(i) + "]", mat);
                }
            }

            for (auto &mesh : model.meshes) {
                auto modelMat = glm::mat4(1.0f);
                modelMat = glm::translate(modelMat, transform.position);
                modelMat = glm::rotate(modelMat, glm::radians(transform.rotation.yaw), glm::vec3(1.0f, 0.0f, 0.0f));
                modelMat = glm::rotate(modelMat, glm::radians(transform.rotation.pitch), glm::vec3(0.0f, 1.0f, 0.0f));
                modelMat = glm::rotate(modelMat, glm::radians(transform.rotation.roll), glm::vec3(0.0f, 0.0f, 1.0f));

                modelMat = glm::scale(modelMat, transform.scale);

                engine->setModelMatrix(modelMat);

                engine->renderMesh(&mesh, model.shaderId);
            }
        }
    }
};