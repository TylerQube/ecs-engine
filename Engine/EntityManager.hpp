#pragma once

#include <array>
#include <vector>
#include <queue>
#include <string>
#include "Types.hpp"
#include <assert.h>

class EntityManager
{
public:
    EntityManager() {
        for(Entity e = 0; e < MAX_ENTITIES; ++e) {
            availableEntities.push(e);
            alive[e] = false;
        }
        livingEntityCount = 0;
    }

    Entity createEntity(const std::string &tag)
    {
        assert(livingEntityCount < MAX_ENTITIES && "Maximum entity count reached");
        Entity id = availableEntities.front();
        availableEntities.pop();
        alive[id] = true;
        livingEntityCount++;
        return id;
    };

    void destroyEntity(Entity entity) {
        assert(entity < MAX_ENTITIES && "Invalid entity id");

        signatures[entity].reset();
        alive[entity] = false;
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

    std::vector<Entity> getEntitiesWithSignature(const Signature &requiredSignature) const {
        std::vector<Entity> entities;
        entities.reserve(livingEntityCount);

        for (Entity e = 0; e < MAX_ENTITIES; ++e) {
            if (!alive[e])
                continue;

            const auto &sig = signatures[e];
            if ((sig & requiredSignature) == requiredSignature) {
                entities.push_back(e);
            }
        }

        return entities;
    }


private:
    std::queue<Entity> availableEntities{};
    std::array<Signature, MAX_ENTITIES> signatures{};
    std::array<bool, MAX_ENTITIES> alive{};
    uint32_t livingEntityCount;
};