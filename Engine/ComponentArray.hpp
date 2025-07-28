#include "EntityManager.hpp"
#include <cassert>

class IComponentArray
{
};

template <typename T>
class ComponentArray : IComponentArray
{
    void addComponent(std::shared_ptr<Entity> entity, T component) {
        assert(entityToIndexMap.find(entity) == entityToIndexMap.end() && "Entity may only hold one of each component type");

        components[size] = component;
        entityToIndexMap.insert({entity, size});
        size++;
    }

    void removeComponent(std::shared_ptr<Entity> entity) {
        assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Cannot remove nonexistent component from entity");

        auto index = entityToIndexMap[entity];

        // move last component to removed index
        components[index] = components[size-1];
        // update index map
        entityToIndexMap[components[index]] = index;
        entityToIndexMap.erase(entity);

        size--;
    }


private:
    std::array<T, MAX_ENTITIES> components;
    std::map<std::shared_ptr<Entity>, unsigned int> entityToIndexMap;
    unsigned int size = 0;
};