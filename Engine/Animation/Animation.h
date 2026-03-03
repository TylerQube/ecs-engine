#include <string>
#include <map>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Engine/AssimpGLMHelpers.h>
#include <Engine/Component/Model.h>

struct AssimpNodeData
{
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

class Animation
{
public:
    Animation() = default;

    Animation(const std::string &animationPath, Model *model)
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
        assert(scene && scene->mRootNode);

        auto anim = scene->mAnimations[0];
        duration = anim->mDuration;
        ticksPerSecond = anim->mTicksPerSecond;
        ReadHierarchyData(rootNode, scene->mRootNode);
        ReadMissingBones(anim, *model);
    }

    ~Animation() {}

    Bone *FindBone(const std::string &name)
    {
        auto iter = std::find_if(bones.begin(), bones.end(),
                                 [&](const Bone &Bone)
                                 {
                                     return Bone.GetBoneName() == name;
                                 });
        if (iter == bones.end())
            return nullptr;
        else
            return &(*iter);
    }

    void ReadMissingBones(const aiAnimation* animation, Model& model)
    {
        int size = animation->mNumChannels;

        auto& modelBoneInfoMap = model.boneInfo;//getting m_BoneInfoMap from Model class
        int& boneCount = model.boneCount; //getting the m_BoneCounter from Model class

        //reading channels(bones engaged in an animation and their keyframes)
        for (int i = 0; i < size; i++)
        {
            auto channel = animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;

            if (modelBoneInfoMap.find(boneName) == modelBoneInfoMap.end())
            {
                modelBoneInfoMap[boneName].id = boneCount;
                boneCount++;
            }
            bones.push_back(Bone(channel->mNodeName.data,
                modelBoneInfoMap[channel->mNodeName.data].id, channel));
        }

        boneInfoMap = modelBoneInfoMap;
    }

    inline float GetTicksPerSecond() { return ticksPerSecond; }
    inline float GetDuration() { return duration; }
    inline const AssimpNodeData &GetRootNode() { return rootNode; }
    inline const std::map<std::string, BoneInfo> &GetBoneIDMap()
    {
        return boneInfoMap;
    }

private:
    AssimpNodeData rootNode;
    float duration;
    int ticksPerSecond;

    std::vector<Bone> bones;
    std::map<std::string, BoneInfo> boneInfoMap;

    void ReadHierarchyData(AssimpNodeData &dst, const aiNode *src)
    {
        assert(src);

        dst.name = src->mName.data;
        dst.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
        dst.childrenCount = src->mNumChildren;

        for (int i = 0; i < src->mNumChildren; i++)
        {
            AssimpNodeData newData;
            ReadHierarchyData(newData, src->mChildren[i]);
            dst.children.push_back(newData);
        }
    }
};