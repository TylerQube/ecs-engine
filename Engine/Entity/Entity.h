#include <string>

class Entity
{
    const size_t id = 0;
    const std::string tag = "Default";
    bool alive = true;

private:
    Entity(const std::string &tag, size_t id);

public:
    void destroy() { alive = false; }

    const std::string &tag() { return tag; }
};