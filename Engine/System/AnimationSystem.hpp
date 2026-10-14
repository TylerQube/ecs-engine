#pragma once

#include "Engine/Animation/Bone.h"
#include "Engine/Component/Animation.h"
#include "Engine/Component/Camera.h"
#include "Engine/Component/Transform.h"
#include "Engine/Engine.hpp"
#include "System.h"
#include <glm/glm.hpp>
#include <iostream>
#include <string>

class AnimationSystem : public System {
  public:
    Engine *engine;
    void init(Engine &c) {
        this->engine = &c;
    }

    void update(float dt) {
        glm::vec3 cameraPosition(0.0f);
        bool hasCamera = false;
        auto cameraEntities = engine->queryEntitiesWith<Camera, Transform>();
        if (!cameraEntities.empty()) {
            auto &cameraTransform = engine->getComponent<Transform>(cameraEntities[0]);
            cameraPosition = cameraTransform.position;
            hasCamera = true;
        }

        for (Entity entity : entities) {
            auto &anim = engine->getComponent<AnimationComponent>(entity);
            auto &transform = engine->getComponent<Transform>(entity);
            auto &model = engine->getComponent<Model>(entity);

            anim.useSkinning = true;
            if (hasCamera) {
                float distanceToCamera = glm::length(transform.position - cameraPosition);
                if (distanceToCamera > anim.skinningDisableDistance) {
                    anim.useSkinning = false;
                    continue;
                }
            }

            if (!anim.currentAnimation || !anim.play)
                continue;

            anim.currentTime += anim.currentAnimation->ticksPerSecond * dt;
            anim.currentTime = fmod(anim.currentTime, anim.currentAnimation->duration);
            calculateBoneTransform(model.skeleton, anim, &anim.currentAnimation->rootNode, glm::mat4(1.0f));
        }
    }

  private:
    void calculateBoneTransform(const Skeleton &skeleton, AnimationComponent &anim, const AssimpNodeData *node,
                                glm::mat4 parentTransform) {
        std::string nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        auto bone = anim.currentAnimation->FindBone(nodeName);

        if (bone) {
            bone->Update(anim.currentTime);
            nodeTransform = bone->GetLocalTransform();
        }

        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        auto boneInfoMap = skeleton.bones;
        if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
            int index = boneInfoMap[nodeName].id;
            glm::mat4 offset = boneInfoMap[nodeName].offset;
            anim.finalBoneMatrices[index] = globalTransformation * offset;
        }

        for (int i = 0; i < node->childrenCount; i++)
            calculateBoneTransform(skeleton, anim, &node->children[i], globalTransformation);
    }
};