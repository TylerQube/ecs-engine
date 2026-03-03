#include "System.h"

class AnimationSystem : public System {
  public:
    Engine *engine;
    void init(Engine &c) {
        this->engine = &c;
    }

    void update(float dt) {
        for (Entity entity : entities) {
            auto anim = engine->getComponent<AnimationComponent>(entity);
            if (!anim.currentAnimation)
                continue;
            auto skeleton = engine->getComponent<Skeleton>(entity);

            anim.currentTime += anim.currentAnimation->GetTicksPerSecond() * dt;
            anim.currentTime = fmod(anim.currentTime, anim.currentAnimation->GetDuration());
            calculateBoneTransform(anim, skeleton, &anim.currentAnimation->GetRootNode(), glm::mat4(1.0f));
        }
    }

  private:
    void calculateBoneTransform(AnimationComponent &anim, Skeleton &skeleton, const AssimpNodeData *node,
                                glm::mat4 parentTransform) {
        std::string nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        Bone *Bone = anim.currentAnimation->FindBone(nodeName);

        if (Bone) {
            Bone->Update(anim.currentTime);
            nodeTransform = Bone->GetLocalTransform();
        }

        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        auto boneInfoMap = anim.currentAnimation->GetBoneIDMap();
        if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
            int index = boneInfoMap[nodeName].id;
            glm::mat4 offset = boneInfoMap[nodeName].offset;
            skeleton.finalBoneMatrices[index] = globalTransformation * offset;
        }

        for (int i = 0; i < node->childrenCount; i++)
            calculateBoneTransform(anim, skeleton, &node->children[i], globalTransformation);
    }
};