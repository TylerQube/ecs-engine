#pragma once

#include <map>
#include <vector>
#include <typeinfo>
#include <cassert>

#include "Engine/System/System.h"
#include "Types.hpp"

class SystemManager
{
public:
    template <typename T>
    std::shared_ptr<T> registerSystem()
    {
        const char *typeName = typeid(T).name();

        auto system = std::make_shared<T>();
        systems.insert({typeName, system});

        return system;
    }

    template <typename T>
    void setSignature(std::bitset<MAX_COMPONENTS> signature)
    {
        const char *typeName = typeid(T).name();

        assert(systems.find(typeName) != systems.end() && "Register system before using");

        signatures.insert({typeName, signature});
    }

    void destroyEntity(Entity entity)
    {
        for (auto const &pair : systems)
        {
            auto const &system = pair.second;

            system->entities.erase(entity);
        }
    }

    void entitySignatureChanged(Entity entity, Signature newSignature)
    {
        for (auto const &pair : systems)
        {
            auto const &sysType = pair.first;
            auto const &system = pair.second;
            auto const &signature = signatures[sysType];

            if ((newSignature & signature) == signature)
            {
                system->entities.insert(entity);
            }
            else
            {
                system->entities.erase(entity);
            }
        }
    }

private:
    std::map<const char *, std::shared_ptr<System>> systems;
    std::map<const char *, Signature> signatures;
};