#pragma once

#include <Engine/Animation/Animation.h>

struct AnimationComponent {
    std::shared_ptr<Animation> currentAnimation;
    std::vector<glm::mat4> finalBoneMatrices;
    bool play = false;
    bool useSkinning = true;
    float currentTime = 0;
    float skinningDisableDistance = 24.0f;
};