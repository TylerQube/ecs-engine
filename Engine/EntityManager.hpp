#pragma once

#include <vector>
#include <queue>
#include "Types.hpp"
#include <assert.h>

class EntityManager
{
public:
    EntityManager() {
        for(Entity e = 0; e < MAX_ENTITIES; ++e) {
            availableEntities.push(e);
        }
        livingEntityCount = 0;
    }

    Entity createEntity(const std::string &tag)
    {
        assert(livingEntityCount < MAX_ENTITIES && "Maximum entity count reached");
        Entity id = availableEntities.front();
        availableEntities.pop();
        livingEntityCount++;
        return id;
    };

    void destroyEntity(Entity entity) {
        assert(entity < MAX_ENTITIES && "Invalid entity id");

        signatures[entity].reset();
        availableEntities.push(entity);
        livingEntityCount--;
    }

    void setSignature(Entity entity, Signature signature) {
        assert(entity < MAX_ENTITIES && "Invalid entity id");
        signatures[entity] = signature;
    }

    Signature getSignature(Entity entity) {
        assert(entity < MAX_ENTITIES && "Invalid entity id");
        return signatures[entity];
    }


private:
    std::queue<Entity> availableEntities{};
    std::array<Signature, MAX_ENTITIES> signatures{};
    uint32_t livingEntityCount;
};