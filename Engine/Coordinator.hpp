#pragma once

#include "EntityManager.hpp"
#include "ComponentManager.hpp"
#include "SystemManager.hpp"
#include <Renderer/Renderer.h>
#include<Renderer/OpenGLRenderer/OpenGLRenderer.h>

class Coordinator
{
public:
    void init()
    {
        entityManager = std::make_unique<EntityManager>();
        componentManager = std::make_unique<ComponentManager>();
        systemManager = std::make_unique<SystemManager>();

        renderer = std::make_unique<OpenGLRenderer>("Hello Engine!", 800, 600);
        renderer->run();
    }

    template <typename T>
    std::shared_ptr<T> registerSystem() {
        return systemManager->registerSystem<T>();
    }

    template <typename T>
    void registerComponent()
    {
        componentManager->registerComponent<T>();
    }

    template <typename T>
    void addComponent(Entity entity, T component) {
        componentManager->addComponent(entity, component);

        auto signature = entityManager->getSignature(entity);
        signature.set(componentManager->getComponentId(component), true);

        systemManager->entitySignatureChanged(entity, signature);
    }

    template <typename T>
    T& getComponent(Entity entity) {
        return componentManager->getComponent<T>(entity);
    }

    template <typename T>
    int getComponentId() {
        return componentManager->getComponentId<T>();
    }

    template <typename T>
    void setSignature(std::bitset<MAX_COMPONENTS> signature) {
        systemManager->setSignature<T>(signature);
    }

    void uploadMesh(WorldMesh *wMesh) {
        renderer->uploadMesh(wMesh);
    }

    void renderMesh(WorldMesh *wMesh) {
        renderer->renderMesh(wMesh);
    }

    void setViewMatrix(glm::mat4 view) {
        renderer->setViewMatrix(view);
    }
    void setProjectionMatrix(glm::mat4 projection) {
        renderer->setProjectionMatrix(projection);
    }
    void setModelMatrix(glm::mat4 model) {
        renderer->setModelMatrix(model);
    }
    float getAspectRatio() {
        return renderer->getAspectRatio();
    }

private:
    // input manager, etc.

    std::unique_ptr<EntityManager> entityManager;
    std::unique_ptr<ComponentManager> componentManager;
    std::unique_ptr<SystemManager> systemManager;

    std::unique_ptr<Renderer> renderer;
};