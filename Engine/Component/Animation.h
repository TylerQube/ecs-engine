#pragma once

#include <Engine/Animation/AnimationClip.h>

struct AnimationComponent {
    std::shared_ptr<AnimationClip> currentAnimation;
    float currentTime = 0;

    std::shared_ptr<AnimationClip> prevAnimation;
    float prevTime = 0;

    std::vector<glm::mat4> finalBoneMatrices;

    bool play = false;
    bool useSkinning = true;
    float skinningDisableDistance = 24.0f;
};