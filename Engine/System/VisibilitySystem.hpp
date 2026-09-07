#pragma once

#include "Engine/Engine.hpp"
#include "System.h"
#include "Engine/System/FrustumCulling.hpp"

class VisibilitySystem : public System {
  public:
    Engine *engine;

    void init(Engine &c) {
        this->engine = &c;
    }

    void update(float dt) {
        (void)dt;

        auto cameraEntities = engine->queryEntitiesWith<Camera, Transform>();
        if (cameraEntities.empty()) {
            for (Entity entity : entities) {
                auto &model = engine->getComponent<Model>(entity);
                model.visibleInFrustum = true;
            }
            return;
        }

        auto cameraEntity = cameraEntities[0];
        auto &camera = engine->getComponent<Camera>(cameraEntity);
        auto &cameraTransform = engine->getComponent<Transform>(cameraEntity);
        auto planes = FrustumCulling::cameraFrustum(camera, cameraTransform, engine->getAspectRatio());

        for (Entity entity : entities) {
            auto &model = engine->getComponent<Model>(entity);
            auto &transform = engine->getComponent<Transform>(entity);
            auto modelMatrix = FrustumCulling::buildModelMatrix(transform);
            model.visibleInFrustum = !FrustumCulling::isModelCulled(model, modelMatrix, planes);
        }
    }
};