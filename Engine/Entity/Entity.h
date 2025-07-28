#include <string>
#include <bitset>

#define MAX_COMPONENTS 32

class Entity
{
    const size_t id = 0;
    const std::string tag = "Default";
    bool alive = true;

    std::bitset<MAX_COMPONENTS> signature;

private:
    friend class EntityManager;
    Entity(const std::string &tag, size_t id);

public:
    void destroy() { alive = false; }

    const std::string &tag() { return tag; }
};