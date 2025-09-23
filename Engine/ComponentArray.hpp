#include "EntityManager.hpp"
#include <cassert>
#include <map>

class IComponentArray
{
};

template <typename T>
class ComponentArray : public IComponentArray
{
public:
    void addComponent(Entity entity, T component)
    {
        assert(entityToIndexMap.find(entity) == entityToIndexMap.end() && "Entity may only hold one of each component type");

        components[size] = component;
        entityToIndexMap.insert({entity, size});
        size++;
    }

    void removeComponent(Entity entity)
    {
        assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Cannot remove nonexistent component from entity");

        auto index = entityToIndexMap[entity];

        // move last component to removed index
        components[index] = components[size - 1];
        // update index map
        entityToIndexMap[components[index]] = index;
        entityToIndexMap.erase(entity);

        size--;
    }

    T &getComponent(Entity entity)
    {
        assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Component not found");

        return components[entityToIndexMap[entity]];
    }

    bool hasComponent(Entity entity)
    {
        return entityToIndexMap.find(entity) != entityToIndexMap.end();
    }

    std::vector<Entity> getEntities() {
        std::cout << "entityToIndexMap size: " << entityToIndexMap.size() << std::endl;
        std::vector<Entity> ents;
        for (auto const& pair : entityToIndexMap) {
            ents.push_back(pair.first);
        }
        return ents;
    }

private:
    std::array<T, MAX_ENTITIES> components;
    std::map<Entity, unsigned int> entityToIndexMap;
    unsigned int size = 0;
};