#pragma once

#include "EntityManager.hpp"
#include "ComponentManager.hpp"
#include "SystemManager.hpp"
#include "InputManager.hpp"
#include <Renderer/Renderer.h>
#include <Renderer/OpenGLRenderer/OpenGLRenderer.h>

class Coordinator
{
public:
    void init()
    {
        entityManager = std::make_unique<EntityManager>();
        componentManager = std::make_unique<ComponentManager>();
        systemManager = std::make_unique<SystemManager>();

        inputManager = std::make_unique<InputManager>();

        renderer = std::make_unique<OpenGLRenderer>("Hello Engine!", 800, 600);
        renderer->setMouseCapture(true);

        renderer->registerKeyCallback(
            [this](KeyCode key, KeyAction action)
            {
                inputManager->keyCallback(key, action);
            });
        renderer->registerMouseCallback(
            [this](double xpos, double ypos)
            {
                inputManager->mouseCallback(xpos, ypos);
            });

    }

    template <typename T>
    std::shared_ptr<T> registerSystem()
    {
        return systemManager->registerSystem<T>();
    }

    void subscribeSystemToInput(std::shared_ptr<System> system) {
        inputManager->subscribe(system);
    }

    Entity createEntity(const std::string &tag)
    {
        return entityManager->createEntity(tag);
    }

    template <typename T>
    void registerComponent()
    {
        componentManager->registerComponent<T>();
    }

    template <typename T>
    void addComponent(Entity entity, T component)
    {
        componentManager->addComponent(entity, component);

        auto signature = entityManager->getSignature(entity);
        signature.set(componentManager->getComponentId<T>(), true);
        entityManager->setSignature(entity, signature);

        systemManager->entitySignatureChanged(entity, signature);
    }

    template <typename T>
    T &getComponent(Entity entity)
    {
        return componentManager->getComponent<T>(entity);
    }

    template <typename T>
    int getComponentId()
    {
        return componentManager->getComponentId<T>();
    }

    template <typename T>
    void setSignature(std::bitset<MAX_COMPONENTS> signature)
    {
        systemManager->setSignature<T>(signature);
    }

    unsigned int loadShader(const char *vertexPath, const char *fragmentPath)
    {
        return renderer->loadShader(vertexPath, fragmentPath);
    }

    void uploadMesh(WorldMesh *wMesh)
    {
        renderer->uploadMesh(wMesh);
    }

    void renderMesh(WorldMesh *wMesh)
    {
        renderer->renderMesh(wMesh);
    }

    void setViewMatrix(glm::mat4 view)
    {
        renderer->setViewMatrix(view);
    }
    void setProjectionMatrix(glm::mat4 projection)
    {
        renderer->setProjectionMatrix(projection);
    }
    void setModelMatrix(glm::mat4 model)
    {
        renderer->setModelMatrix(model);
    }
    float getAspectRatio()
    {
        return renderer->getAspectRatio();
    }

    int startFrame()
    {
        return renderer->beginFrame();
    }

    void endFrame()
    {
        renderer->endFrame();
    }

    float getTime()
    {
        return renderer->getTime();
    }

private:
    // input manager, etc.

    std::unique_ptr<EntityManager> entityManager;
    std::unique_ptr<ComponentManager> componentManager;
    std::unique_ptr<SystemManager> systemManager;

    std::unique_ptr<InputManager> inputManager;

    std::unique_ptr<Renderer> renderer;
};