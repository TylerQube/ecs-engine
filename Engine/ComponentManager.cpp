#include "Entity/Entity.h"
#include "ComponentArray.hpp"
#include <map>
#include <vector>
#include <typeinfo>
#include <cassert>

class ComponentManager
{
    template <typename T>
    void registerComponent(T newComponent)
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
    }

    template <typename T>
    void removeComponent(Entity entity, T component)
    {
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
    std::map<const char *, IComponentArray> componentArrays;
};