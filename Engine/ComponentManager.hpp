#include "ComponentArray.hpp"
#include <map>
#include <vector>
#include <typeinfo>
#include <cassert>

class ComponentManager
{
public:
    template <typename T>
    void registerComponent()
    {
        const char *typeName = typeid(T).name();

        componentIds.insert({typeName, nextComponentId});
        nextComponentId++;

        componentArrays.insert({typeName, ComponentArray<T>()});
    }

    template <typename T>
    void addComponent(Entity entity, T component)
    {
        assert(getComponentId(component) && "Component type not registered!");

        unsigned int id = getComponentId(component);

        std::shared_ptr<ComponentArray<T>> componentArr = getComponentArray<T>();
        componentArr->addComponent(entity, component);
    }

    template <typename T>
    void removeComponent(std::shared_ptr<Entity> entity, T component)
    {
        assert(getComponentId(component) && "Component type not registered!");

        unsigned int id = getComponentId(component);

        std::shared_ptr<ComponentArray<T>> componentArr = getComponentArray<T>();
        componentArr->removeComponent(entity);
    }

    template <typename T>
    T& getComponent(std::shared_ptr<Entity> entity) {
        getComponentArray<T>()->getComponent(entity);
    }

    template <typename T>
    int getComponentId(T component)
    {
        const char *typeName = typeid(component).name();
        if (componentIds.find(typeName) == componentIds.end())
            return -1;

        return componentTypes[typeName];
    }

private:
    unsigned int nextComponentId = 0;
    std::map<const char *, unsigned int> componentIds;
    std::map<const char *, std::shared_ptr<IComponentArray>> componentArrays;

    template <typename T>
    std::shared_ptr<ComponentArray<T>> getComponentArray()
    {
        assert(getComponentId(component) && "Component type not registered!");

        const char *name = typeid(T).name();

        unsigned int id = getComponentId(component);

        return std::static_pointer_cast<ComponentArray<T>>(componentArrays[name]);
    }
};