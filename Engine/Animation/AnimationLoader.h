#pragma once

#include <Engine/Animation/AnimationClip.h>
#include <Engine/Component/Model.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <iostream>
#include <string>

class AnimationLoader {
  public:
    static AnimationMap loadClips(const std::string &animationPath, Skeleton &skeleton) {
        Assimp::Importer importer;
        AnimationMap clips;

        const aiScene *scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
        if (!scene || !scene->mRootNode) {
            std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            assert(scene && scene->mRootNode);
        }

        if (scene->mNumAnimations == 0) {
            std::cerr << "WARNING::ASSIMP model has no animation data" << std::endl;
            return {};
        }

        for (int i = 0; i < scene->mNumAnimations; i++) {
            auto clip = AnimationClip{
                .name = scene->mAnimations[i]->mName.C_Str(),
                .rootNode = AssimpNodeData{},
                .duration = static_cast<float>(scene->mAnimations[i]->mDuration),
                .ticksPerSecond = static_cast<float>(scene->mAnimations[i]->mTicksPerSecond),
            };
            ReadHierarchyData(clip.rootNode, *scene->mRootNode);
            ReadMissingBones(*scene->mAnimations[i], clip, skeleton);
            clips[clip.name] = std::make_shared<AnimationClip>(std::move(clip));
        }

        return clips;
    }

  private:
    static void ReadMissingBones(const aiAnimation &animation, AnimationClip &clip, Skeleton &skeleton) {
        int size = animation.mNumChannels;

        auto &boneInfoMap = skeleton.bones; // getting m_BoneInfoMap from Model class
        int &boneCount = skeleton.boneCount; // getting the m_BoneCounter from Model class

        // reading channels(bones engaged in an animation and their keyframes)
        for (int i = 0; i < size; i++) {
            const aiNodeAnim *channel = animation.mChannels[i];
            std::string boneName = channel->mNodeName.data;

            if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
                boneInfoMap[boneName].id = boneCount;
                boneCount++;
            }

            clip.bones.emplace_back(boneName, boneInfoMap[boneName].id, channel);
        }
    }

    static void ReadHierarchyData(AssimpNodeData &dst, const aiNode &src) {
        assert(src.mName.length > 0);

        dst.name = src.mName.data;
        dst.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src.mTransformation);
        dst.childrenCount = src.mNumChildren;

        for (int i = 0; i < src.mNumChildren; i++) {
            AssimpNodeData newData;
            ReadHierarchyData(newData, *src.mChildren[i]);
            dst.children.push_back(newData);
        }
    }
};