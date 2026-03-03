#include <Engine/Animation/Animation.h>

struct AnimationComponent {
    std::vector<glm::mat4> finalBoneMatrices;
    std::shared_ptr<Animation> currentAnimation;
    bool play = false;
    float currentTime = 0;
};