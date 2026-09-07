#include "Engine/Engine.hpp"
#include "System.h"
#include "Engine/System/FrustumCulling.hpp"

class RenderSystem : public System {
  public:
    Engine *engine;
    void init(Engine &c) {
        this->engine = &c;
    }

    void update(float dt) {
        struct CachedPointLight {
            glm::vec3 position;
            glm::vec3 color;
            float intensity;
        };

        std::vector<CachedPointLight> pointLights;
        auto pointLightEntities = engine->queryEntitiesWith<PointLight>();
        const int MAX_LIGHTS = 32;
        int lightCount = std::min((int)pointLightEntities.size(), MAX_LIGHTS);
        pointLights.reserve(lightCount);
        for (int i = 0; i < lightCount; ++i) {
            Entity le = pointLightEntities[i];
            auto &pl = engine->getComponent<PointLight>(le);
            pointLights.push_back({pl.position, pl.color, pl.intensity});
        }

        glm::vec3 cameraPosition(0.0f);
        bool hasCamera = false;
        auto cameraEntities = engine->queryEntitiesWith<Camera, Transform>();
        if (!cameraEntities.empty()) {
            auto cameraEntity = cameraEntities[0];
            auto &camTransform = engine->getComponent<Transform>(cameraEntity);
            cameraPosition = camTransform.position;
            hasCamera = true;
        }

        for (Entity entity : entities) {
            auto &model = engine->getComponent<Model>(entity);
            auto &transform = engine->getComponent<Transform>(entity);

            if (!model.visibleInFrustum) {
                continue;
            }

            glm::mat4 modelMat = FrustumCulling::buildModelMatrix(transform);

            // Set bone matrices for animation
            if (engine->hasComponent<AnimationComponent>(entity)) {
                auto &animation = engine->getComponent<AnimationComponent>(entity);

                engine->setUniform(model.shaderId, "useSkinning", animation.useSkinning ? 1 : 0);

                if (animation.useSkinning) {
                    for (size_t i = 0; i < animation.finalBoneMatrices.size(); i++) {
                        engine->setUniform(model.shaderId, "finalBonesMatrices[" + std::to_string(i) + "]",
                                           animation.finalBoneMatrices[i]);
                    }
                }
            } else {
                engine->setUniform(model.shaderId, "useSkinning", 0);
            }

            engine->setUniform(model.shaderId, "numPointLights", lightCount);
            for (int i = 0; i < lightCount; ++i) {
                const auto &pl = pointLights[i];
                engine->setUniform(model.shaderId, "pointLights[" + std::to_string(i) + "].position", pl.position);
                engine->setUniform(model.shaderId, "pointLights[" + std::to_string(i) + "].color", pl.color);
                engine->setUniform(model.shaderId, "pointLights[" + std::to_string(i) + "].intensity", pl.intensity);
            }

            // Upload view/camera position for specular calculations
            if (hasCamera) {
                engine->setUniform(model.shaderId, "viewPos", cameraPosition);
            }

            for (auto &mesh : model.meshes) {
                engine->setUniform(model.shaderId, "material_receivesLight", mesh.material.receivesLight ? 1 : 0);

                engine->setModelMatrix(modelMat);

                engine->renderMesh(&mesh, model.shaderId);
            }
        }
    }
};