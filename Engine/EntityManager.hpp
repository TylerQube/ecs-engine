#pragma once

#include <vector>
#include <map>
#include <memory>
#include "Entity.h"

#define MAX_ENTITIES 10000

typedef std::vector<std::shared_ptr<Entity>> EntityVector;
typedef std::map<std::string, EntityVector> EntityMap;

class EntityManager
{
    EntityVector entities;
    EntityMap entityMap;
    size_t totalEntities = 0;

public:
    void addEntity();
    std::shared_ptr<Entity> addEntity(const std::string &tag)
    {
        auto e = std::make_shared<Entity>(tag, totalEntities++);
        entities.push_back(e);
        entityMap[tag].push_back(e);
        return e;
    };

    EntityVector &getEntities();
    EntityVector &getEntities(const std::string &tag);
};