#pragma once

#include <Engine/Animation/Animation.h>

struct AnimationComponent {
    std::shared_ptr<Animation> currentAnimation;
    std::vector<glm::mat4> finalBoneMatrices;
    bool play = false;
    float currentTime = 0;
};