#pragma once

#include <Engine/Animation/Bone.h>
#include <Engine/AssimpGLMHelpers.h>
#include <algorithm>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct AssimpNodeData {
    glm::mat4 transformation;
    std::string name;
    int childrenCount = 0;
    std::vector<AssimpNodeData> children;
};

struct AnimationClip {
    std::string name;
    AssimpNodeData rootNode;
    std::vector<Bone> bones;
    float duration = 0.0f;
    float ticksPerSecond = 0.0f;

    Bone *FindBone(const std::string &boneName) {
        auto iter =
            std::find_if(bones.begin(), bones.end(), [&](const Bone &bone) { return bone.GetBoneName() == boneName; });
        return iter == bones.end() ? nullptr : &*iter;
    }
};

using AnimationMap = std::unordered_map<std::string, std::shared_ptr<AnimationClip>>;
