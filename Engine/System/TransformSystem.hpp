#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Engine/Engine.hpp"
#include "System.h"

#include "Engine/Component/Camera.h"
#include "Engine/Component/Transform.h"

class TransformSystem : public System
{
private:
    float maxVelocity = 4.0f;
    float acceleration = 10.0f;
public:
    double speedTol = 0.05;

    Engine *engine;
    void init(Engine &c)
    {
        this->engine = &c;
    }

    void update(float dt)
    {
        for (Entity entity : entities)
        {
            auto &transform = this->engine->getComponent<Transform>(entity);

            std::cout << "Position " << transform.position.x << "," << transform.position.y << "," << transform.position.z << std::endl;

            if(glm::length(transform.velocity) < speedTol) transform.velocity = glm::vec3(0.0f);

            // cap velocity
            if(glm::length(transform.velocity) > maxVelocity)
                transform.velocity = glm::normalize(transform.velocity) * maxVelocity;

            transform.position += transform.velocity * dt;
        }
    }
};